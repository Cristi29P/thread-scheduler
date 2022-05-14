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

    /* Synchronization elements */
    sem_t running; /* Used for blocking a thread when it is preempted */
} thread_t;

typedef struct {
    int time_quantum; /* Max allowed time quantum */
    int io; /* Max number of io devices */
    int no_threads; /* Number of threads handled by the scheduler */

    thread_t *current_thread; /* Pointer to the currently running thread */
    prio_queue_t *ready; /* Threads which are waiting to be planned */
    prio_queue_t *finished; /* Threads which finished their job and are waiting to be free'd */
    prio_queue_t **waiting; /* Blocked threads by an event */
    
    /* Synchronization elements */
    sem_t end; /* Used for signaling when the scheduler should stop */
} scheduler_t;

scheduler_t *scheduler;

void mark_as_ready(thread_t *thread);

void plan_next();

void scheduler_check();

int cmp_func(const void *t1, const void *t2)
{
    return ((thread_t *)t1)->priority - ((thread_t *)t2)->priority;
}

void free_func(void *t)
{
    int rv;
    thread_t *thread = (thread_t *)t;

    rv = pthread_join(thread->tid, NULL);
    DIE(rv, "pthread_join failed!");

    rv = sem_destroy(&thread->running);
    DIE(rv, "sem_destroy failed!");

    free(thread);
}

void plan_next() 
{
    thread_t *thread = queue_pop(scheduler->ready);

    thread->state = RUNNING;
    thread->time_quantum = scheduler->time_quantum;
    scheduler->current_thread = thread;

    DIE(sem_post(&thread->running), "sem_post failed!");
}

void scheduler_check() 
{
    thread_t *current = scheduler->current_thread;

    if (!queue_size(scheduler->ready)) {
        if (current->state == TERMINATED)   
            DIE(sem_post(&scheduler->end), "sem_post failed!");

        DIE(sem_post(&current->running), "sem_post failed!");
        return;
    }

    if (!current || current->state == WAITING) {
        plan_next();
        return;
    }

    if (current->state == TERMINATED) {
        queue_push(scheduler->finished, current);
        plan_next();
        return;
    }
    
    if (current->priority < ((thread_t *)queue_top(scheduler->ready))->priority) {
        mark_as_ready(current);
        plan_next();
        return;
    }
    // POATE COMBIN ULTIMELE DOUA IF-URI
    if (!current->time_quantum) {
        if (current->priority == ((thread_t *)queue_top(scheduler->ready))->priority) {
            mark_as_ready(current);
            plan_next();
            return;
        }

        current->time_quantum = scheduler->time_quantum;
    } 

    DIE(sem_post(&current->running), "sem_post failed!");
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

    rv = sem_init(&scheduler->end, 0, 0);
    DIE(rv, "sem_init failed!");

    scheduler->ready = queue_init(cmp_func, free_func);
    scheduler->finished = queue_init(cmp_func, free_func);
    scheduler->waiting = calloc(io, sizeof(prio_queue_t *));
    DIE(!scheduler->waiting, "Failed to calloc array of waiting queues!");

    for (int i = 0; i != (int)io; ++i)
        scheduler->waiting[i] = queue_init(cmp_func, free_func);

    return 0;
}


void *start_thread(void *args)
{

    thread_t *thread = (thread_t *)args;
    int rv;

    // The thread should block here and wait until has the right to execute
    rv = sem_wait(&thread->running);
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

    if (!func || priority > SO_MAX_PRIO) {
        return INVALID_TID;
    }

    thread = calloc(1, sizeof(thread_t));
    DIE(!thread, "Thread calloc failed!");

    thread->priority = priority;
    thread->time_quantum = scheduler->time_quantum;
    thread->handler = func;

    rv = sem_init(&thread->running, 0, 0);
    DIE(rv, "pthread_init failed!");
    
    rv = pthread_create(&thread->tid, NULL, start_thread, thread);
    DIE(rv, "pthread_create failed!");
    
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
    thread_t *current = scheduler->current_thread;

    /* Decrease the time remaining on the processor */
    --(current->time_quantum);

    scheduler_check();

    /* Stay here if you get preempteed */
    DIE(sem_wait(&current->running), "sem_wait failed!");
}

void so_end(void)
{
    int rv;

    if (!scheduler)
        return;

    if (scheduler->no_threads)
        DIE(sem_wait(&scheduler->end), "sem_wait failed!");

    queue_free(scheduler->ready);
    queue_free(scheduler->finished);

    if (scheduler->current_thread)
        free_func(scheduler->current_thread);
    
    for (int i = 0; i != (int)scheduler->io; ++i)
        queue_free(scheduler->waiting[i]);

    rv = sem_destroy(&scheduler->end);
    DIE(rv, "sem_destroy failed!");

    free(scheduler->waiting);
    free(scheduler);
    scheduler = NULL;
}
