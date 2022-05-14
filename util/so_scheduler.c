#include <semaphore.h>

#include "so_scheduler.h"
#include "prio_queue.h"

#define SO_FAIL -1

/* Enum representing the possible states a thread can be */
typedef enum {
        READY,
        RUNNING,
        WAITING,
        TERMINATED
} thread_state_t;

typedef struct {
    tid_t tid; /* Pthread id */
    so_handler *handler; /* Function handler */
    thread_state_t state; /* Current thread state */
    int time_quantum; /* Time left on the processor while running */
    int priority; /* Thread priority */

    /* Sync */
    sem_t running;
} thread_t;

typedef struct {
    int time_quantum; /* Max allowed time quantum */
    int io; /* Max number of io devices */
    int no_threads; /* Number of threads handled by the scheduler */

    thread_t *current_thread; /* Pointer to the currently running thread */
    prio_queue_t *ready; /* Threads which are waiting to be planned */
    prio_queue_t *finished; /* Threads which finished their job and are waiting to be free'd */
    prio_queue_t **waiting; /* Blocked threads by an event */
    
    /* Sync */
    sem_t end; /* Used for signaling when the scheduler should stop */
} scheduler_t;


int cmp_func(const void *a, const void *b)
{
    const thread_t *t1 = (thread_t *)a;
    const thread_t *t2 = (thread_t *)b;

    return t1->priority - t2->priority;
}

void free_func(void *a)
{
    int rv;
    thread_t *thread = (thread_t *)a;

    pthread_join(thread->tid, NULL);

    rv = sem_destroy(&(thread->running));
    DIE(rv, "sem destroy failed!");

    free(thread);
}

static scheduler_t *scheduler;

void mark_as_ready(thread_t *thread);

static void scheduler_check();

static void scheduler_check() 
{
    int rv;
    thread_t *current, *next;

    current = scheduler->current_thread;

    if (queue_size(scheduler->ready) == 0) {
        
        if (current->state == TERMINATED) {    
            rv = sem_post(&(scheduler->end));
            DIE(rv, "Sem post end failed");
        }

        rv = sem_post(&(current->running));
        DIE(rv, "Sem post thread running failed in check_ending!");
        return;
    }

    next = queue_top(scheduler->ready);

    if (!current) {
        queue_pop(scheduler->ready);
        next->state = RUNNING;
        next->time_quantum = scheduler->time_quantum;
        scheduler->current_thread = next;
        rv = sem_post(&(next->running));
        DIE(rv, "Sem post failed!");

        return;
    }

    next->state = RUNNING;

    if (current->state == WAITING) {
        queue_pop(scheduler->ready);

        next->state = RUNNING;
        next->time_quantum = scheduler->time_quantum;

        scheduler->current_thread = next;

        rv = sem_post(&(next->running));
        DIE(rv, "Sem post failed!");

       // rv = sem_wait(&(current->running));
        return;
    }

    if (current->state == TERMINATED) {
        queue_pop(scheduler->ready);


        queue_push(scheduler->finished, current);

        next->state = RUNNING;


        next->time_quantum = scheduler->time_quantum;

        scheduler->current_thread = next;

        rv = sem_post(&(next->running));
        DIE(rv, "Sem post failed!");
        return;
    }
    
    if (current->priority < next->priority) {
        queue_pop(scheduler->ready);

        mark_as_ready(current);
        
        next->state = RUNNING;
        next->time_quantum = scheduler->time_quantum;
        scheduler->current_thread = next;
        rv = sem_post(&(next->running));
        DIE(rv, "Sem post failed!");


        return;
    }
    // POATE COMBIN ULTIMELE DOUA IF-URI
    if (!current->time_quantum) {
        if (current->priority == next->priority) {
            queue_pop(scheduler->ready);
            mark_as_ready(current);
        
            next->state = RUNNING;
            next->time_quantum = scheduler->time_quantum;
            scheduler->current_thread = next;
            rv = sem_post(&(next->running));
            DIE(rv, "Sem post failed!");

            return;
        }

        current->time_quantum = scheduler->time_quantum;
    } 

    rv = sem_post(&(current->running));
    DIE(rv, "Sem post failed!");
}



void mark_as_ready(thread_t *thread)
{
    thread->state = READY;
    queue_push(scheduler->ready, thread);

}


int so_init(unsigned int time_quantum, unsigned int io)
{   
    int rv;

    if (scheduler || (io > SO_MAX_NUM_EVENTS) || !time_quantum)
        return SO_FAIL;

    scheduler = calloc(1, sizeof(scheduler_t));
    DIE(!scheduler, "Failed to initialize the scheduler!");

    scheduler->time_quantum = time_quantum;
    scheduler->io = io;
    scheduler->no_threads = 0;
    scheduler->current_thread = NULL;

    rv = sem_init(&(scheduler->end), 0, 0);
    DIE(rv, "Error pthread_mutex_init!");

    scheduler->ready = queue_init(cmp_func, free_func);
    
    scheduler->finished = queue_init(cmp_func, free_func);

    scheduler->waiting = calloc(io, sizeof(prio_queue_t *));
    DIE(!(scheduler->waiting), "Failed to calloc array of waiting queues!");

    for (int i = 0; i != (int)io; ++i)
        scheduler->waiting[i] = queue_init(cmp_func, free_func);

    return 0;
}


void *start_thread(void *args)
{

    thread_t *thread = (thread_t *)args;
    int rv;

    // The thread should block here and wait until has the right to execute
    rv = sem_wait(&(thread->running));
    DIE(rv, "Mutex lock thread running error.");

    thread->handler(thread->priority);


    // Mark the thread as done
    thread->state = TERMINATED;

    // Call the scheduler and add thread to finished queue
    scheduler_check();

    return NULL;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
    thread_t *thread;
    int rv;

    if (!func) {
        fprintf(stderr, "Handler function should not be NULL!");
        return INVALID_TID;
    }

    if (priority > SO_MAX_PRIO) {
        fprintf(stderr, "Priority exceeds the maximum allowed!");
        return INVALID_TID;
    }

    thread = calloc(1, sizeof(thread_t));
    DIE(!thread, "Thread calloc failed!");

    thread->priority = priority;
    thread->time_quantum = scheduler->time_quantum;
    thread->handler = func;

    rv = sem_init(&(thread->running), 0, 0);
    DIE(rv, "Pthread init thread->running failed!");
    
    rv = pthread_create(&(thread->tid), NULL, &start_thread, thread);
    DIE(rv, "Pthread create error!");
    
    ++(scheduler->no_threads);

    mark_as_ready(thread);

    /* If we are the first thread */
    if (scheduler->current_thread != NULL) {
        so_exec();
    }
    else {
        scheduler_check();
    }
    return thread->tid;
}


int so_wait(unsigned int io)
{
    if ((int)io >= scheduler->io)
        return SO_FAIL;

    scheduler->current_thread->state = WAITING;
    queue_push(scheduler->waiting[io], scheduler->current_thread);

    so_exec();
    return 0;

}

int so_signal(unsigned int io)
{
    int cnt = 0;
    if ((int)io >= scheduler->io)
        return SO_FAIL;

    
    thread_t *aux = queue_pop(scheduler->waiting[io]);

    while (aux) {
        aux->state = READY;
        queue_push(scheduler->ready, aux);

        ++cnt;

        aux = queue_pop(scheduler->waiting[io]);
    }

    so_exec();

    return cnt;
}

void so_exec(void)
{
    int rv;
    thread_t *current = scheduler->current_thread;
    
    if (!current)
        return;

    /* Decrease the time remaining on the processor */
    --(current->time_quantum);

    scheduler_check();

    rv = sem_wait(&(current->running));
    DIE(rv, "Sem wait failed!");
}

void so_end(void)
{
    if (scheduler) {
        if (scheduler->no_threads) 
            sem_wait(&(scheduler->end));
        
        queue_free(scheduler->ready);
        queue_free(scheduler->finished);

        if (scheduler->current_thread)
            free_func(scheduler->current_thread);
        
        for (int i = 0; i != (int)scheduler->io; ++i)
            queue_free(scheduler->waiting[i]);

        sem_destroy(&(scheduler->end));
        free(scheduler->waiting);
        free(scheduler);
    }

    scheduler = NULL;
}
