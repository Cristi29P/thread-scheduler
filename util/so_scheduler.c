#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include "so_scheduler.h"
#include "prio_queue.h"
#include <unistd.h>

#define SO_FAIL -1

/* Enum representing the possible states a thread can be */
typedef enum {
        READY,
        RUNNING,
        WAITING,
        TERMINATED
} thread_state_t;

typedef enum {
    PLANNING,
    FINISHED,
    GENERAL
} sched_modes_t;

typedef struct {
    tid_t tid; /* Pthread id */
    so_handler *handler; /* Function handler */
    thread_state_t state; /* Current thread state */
    unsigned int time_quantum; /* Time left on the processor while running */
    int priority; /* Thread priority */

    /* Sync */
    sem_t running;
    sem_t planned;

} thread_t;

typedef struct {
    unsigned int time_quantum; /* Max allowed time quantum */
    unsigned int io; /* Max number of io devices */
    unsigned int no_threads; /* Number of threads handled by the scheduler */

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

    rv = sem_destroy(&(thread->planned));
    DIE(rv, "sem destroy failed!");

    free(thread);
}

static scheduler_t *scheduler;

void plan_thread(thread_t *thread);

void terminate_thread(thread_t *thread);

void schedule_thread(thread_t *thread);

void scheduler_check(sched_modes_t sched_mode, thread_t *thread);

void scheduler_check(sched_modes_t sched_mode, thread_t *thread) 
{
    if (sched_mode == PLANNING) {
        plan_thread(thread);
    }

    if (sched_mode == FINISHED) {
        terminate_thread(thread);
    }

    if (sched_mode == GENERAL) {
        schedule_thread(thread); 
    }
}

void plan_thread(thread_t *thread)
{
    /* First thread to enter/ no other thread is running */
    if (!(scheduler->current_thread)) {

        thread->state = RUNNING;
        scheduler->current_thread = thread;
        sem_post(&(thread->running));
        return;
    }

    /* Check if your priority is higher than current running thread. If yes, preempt */
    if (thread->priority > scheduler->current_thread->priority) {
        thread_t *temp = scheduler->current_thread;
        scheduler->current_thread = thread;
        thread->state = RUNNING;
        temp->state = READY;
        queue_push(scheduler->ready, temp);
        return;
    }

    /* If current thread is not the first/has lower priority, push it to ready queue */
    thread->state = READY;
    queue_push(scheduler->ready, thread);
}

void terminate_thread(thread_t *thread)
{   
    // One more cond
    if (thread->state == TERMINATED) {
        queue_push(scheduler->finished, thread);
        scheduler->current_thread = queue_pop(scheduler->ready);

        if (scheduler->current_thread)
            sem_post(&(scheduler->current_thread->running));
        
        if (!(scheduler->current_thread) && !queue_size(scheduler->ready))
            sem_post(&(scheduler->end));
    }
}

void schedule_thread(thread_t *thread)
{
    thread_t *aux;
    if (!thread->time_quantum) {
        thread->state = READY;
        thread->time_quantum = scheduler->time_quantum;

        queue_push(scheduler->ready, thread);

        aux = queue_pop(scheduler->ready);

        // MAYBBE REMOVE OR CHANGE TO FOR
        while (aux && aux->state == TERMINATED) {
            queue_push(scheduler->finished, aux);
            aux = queue_pop(scheduler->ready);
        }

        scheduler->current_thread = aux;
        if (aux) {
            scheduler->current_thread->state = RUNNING;
            sem_post(&(scheduler->current_thread->running));
        }
    } else if (queue_size(scheduler->ready) &&
        thread->priority < ((thread_t *)queue_top(scheduler->ready))->priority) {
        thread->state = READY;
        queue_push(scheduler->ready, thread);
        scheduler->current_thread = queue_pop(scheduler->ready);
        scheduler->current_thread->state = RUNNING;
        sem_post(&(scheduler->current_thread->running));
    }

    if (thread != scheduler->current_thread)
        sem_wait(&(thread->running));

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



    scheduler_check(PLANNING, thread);


    
    /* Thread planned */
    rv = sem_post(&(thread->planned));
    DIE(rv, "Mutex unlock thread planned error.");

    // The thread should block here and wait until has the right to execute
    rv = sem_wait(&(thread->running));
    DIE(rv, "Mutex lock thread running error.");

    thread->handler(thread->priority);

    // Mark the thread as done
    thread->state = TERMINATED;

    // Call the scheduler and add thread to finished queue
    scheduler_check(FINISHED, thread);

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

    ++(scheduler->no_threads);

    thread->priority = priority;
    thread->time_quantum = scheduler->time_quantum;
    thread->handler = func;

    rv = sem_init(&(thread->running), 0, 0);
    DIE(rv, "Pthread init thread->running failed!");

    rv = sem_init(&(thread->planned), 0, 0);
    DIE(rv, "Pthread init thread->planned failed!");
    

    rv = pthread_create(&(thread->tid), NULL, start_thread, thread);
    DIE(rv, "Pthread create error!");


    /* Wait for thread to be planned */
    rv = sem_wait(&(thread->planned));
    DIE(rv, "Pthread lock error!");



    /* If we are the first thread */
    if (scheduler->current_thread != thread)
        so_exec();

    return thread->tid;
}


int so_wait(unsigned int io)
{
    return 0;
}

int so_signal(unsigned int io)
{
    return 0;
}

void so_exec(void)
{
    if (!(scheduler->current_thread))
        return;

    /* Get current thread handle */
    thread_t *current_thread = scheduler->current_thread;

    /* Decrease the time remaining on the processor */
    --(current_thread->time_quantum);

    
    scheduler_check(GENERAL, current_thread); 
}

void so_end(void)
{
    if (scheduler) {
        if (scheduler->no_threads) 
            sem_wait(&(scheduler->end));

        queue_free(scheduler->ready);
        queue_free(scheduler->finished);

        if (scheduler->current_thread)
            free_func(&(scheduler->current_thread));
        
        for (int i = 0; i != (int)scheduler->io; ++i)
            queue_free(scheduler->waiting[i]);


        sem_destroy(&(scheduler->end));
        free(scheduler->waiting);
        free(scheduler);
    }

    scheduler = NULL;
}


// TODO: JOIN THREADS WHEN FINISHED
// TODO: CLEAN UP MEMORY SO_END
// ADD DIE TO PTHREAD CALLS