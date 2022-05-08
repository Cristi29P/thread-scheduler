#include <pthread.h>
#include <semaphore.h>

#include "so_scheduler.h"
#include "heap.h"

#define INITIAL_CAPACITY 10
#define SO_FAIL -1

/* Enum representing the possible states a thread can be */
typedef enum {
        NEW,
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
    unsigned int priority; /* Thread priority */


    /* Elemente de sincronizare */

} thread_t;


typedef struct {
    unsigned int time_quantum; /* Max allowed time quantum */
    unsigned int io; /* Max number of io devices */
    unsigned int no_threads; /* Number of threads handled by the scheduler */

    thread_t *current_thread; /* Pointer to the currently running thread */
    

} scheduler_t;


static scheduler_t *scheduler;

int so_init(unsigned int time_quantum, unsigned int io)
{
    if (scheduler || (io > SO_MAX_NUM_EVENTS) || !time_quantum)
        return SO_FAIL;

    scheduler = calloc(1, sizeof(scheduler_t));
    DIE(!scheduler, "Failed to initialize the scheduler!");

    scheduler->time_quantum = time_quantum;
    scheduler->io = io;
    scheduler->no_threads = 0;
    scheduler->current_thread = NULL;


    return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
    return INVALID_TID;
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
    return;
}

void so_end(void)
{
    if (scheduler) {
        free(scheduler);
    }
    scheduler = NULL;
}