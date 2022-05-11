#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include "so_scheduler.h"
#include "heap.h"

#define INITIAL_CAPACITY 10
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
    unsigned int time_quantum; /* Time left on the processor while running */
    int priority; /* Thread priority */

    /* Sync */
    pthread_mutex_t running;
    pthread_mutex_t planned;

} thread_t;

typedef struct {
    unsigned int time_quantum; /* Max allowed time quantum */
    unsigned int io; /* Max number of io devices */
    unsigned int no_threads; /* Number of threads handled by the scheduler */

    thread_t *current_thread; /* Pointer to the currently running thread */
    heap_t *ready; /* Threads which are waiting to be planned */
    heap_t *finished; /* Threads which finished their job and are waiting to be free'd */
    heap_t **waiting; /* Blocked threads by an event */
    
    /* Sync */
    pthread_mutex_t end;
} scheduler_t;


int cmp_func(const void *a, const void *b)
{
    const thread_t *t1 = (thread_t *)a;
    const thread_t *t2 = (thread_t *)b;

    return t2->priority - t1->priority;
}

void free_func(void *a)
{
    int rv;
    thread_t *thread = (thread_t *)a;

    rv = pthread_mutex_destroy(&(thread->running));
    DIE(rv, "mutex destroy failed!");

    rv = pthread_mutex_destroy(&(thread->planned));
    DIE(rv, "mutex destroy failed!");
}

static scheduler_t *scheduler;

void scheduler_check(void);

void scheduler_check(void) 
{

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

    rv = pthread_mutex_init(&(scheduler->end), NULL);
    DIE(rv, "Error pthread_mutex_init!");

    scheduler->ready = heap_create(PRIO_QUEUE, cmp_func, free_func, INITIAL_CAPACITY);
    
    scheduler->finished = heap_create(QUEUE, cmp_func, free_func, INITIAL_CAPACITY);

    scheduler->waiting = calloc(io, sizeof(heap_t *));
    DIE(!(scheduler->waiting), "Failed to calloc array of waiting queues!");

    for (int i = 0; i != (int)io; ++i)
        scheduler->waiting[i] = heap_create(QUEUE, cmp_func, free_func, INITIAL_CAPACITY);

    return 0;
}


void *start_thread(void *args)
{
    thread_t *thread = (thread_t *)args;
    int rv;

    scheduler_check();
    
    rv = pthread_mutex_unlock(&(thread->planned));
    DIE(rv, "Mutex unlock thread planned error.");

    // The thread should block here and wait until has the right to execute
    rv = pthread_mutex_lock(&(thread->running));
    DIE(rv, "Mutex lock thread running error.");



    thread->handler(thread->priority);

    // Mark the thread as done
    thread->state = TERMINATED;

    // Call the scheduler and add thread to finished queue


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

    thread = calloc(1, sizeof(thread));
    DIE(!thread, "Thread calloc failed!");

    ++(scheduler->no_threads);

    thread->priority = priority;
    thread->time_quantum = scheduler->time_quantum;
    thread->handler = func;

    rv = pthread_mutex_init(&(thread->running), NULL);
    DIE(rv, "Pthread init thread->running failed!");

    rv = pthread_mutex_init(&(thread->planned), NULL);
    DIE(rv, "Pthread init thread->planned failed!");
    
    rv = pthread_create(&(thread->tid), NULL, start_thread, thread);
    DIE(rv, "Pthread create error!");

    rv = pthread_mutex_lock(&(thread->planned));
    DIE(rv, "Pthread lock error!");

    // Might add so_exec

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
    
    if (t)  

}

void so_end(void)
{
    if (scheduler) {
        free(scheduler);
    }

    scheduler = NULL;
}