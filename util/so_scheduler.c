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

                fprintf(stderr, "SALOOT3!\n");


    pthread_join(thread->tid, NULL);



    rv = sem_destroy(&(thread->running));
    DIE(rv, "sem destroy failed!");

    free(thread);
}

static scheduler_t *scheduler;

void mark_as_ready(thread_t *thread);

static void check_ending(thread_t *thread);

static void scheduler_check();

static void check_ending(thread_t *thread)
{
    int rv;

    fprintf(stderr, "SIZE IN CHECK_ENDING: %d\n", queue_size(scheduler->ready));

    if (queue_size(scheduler->ready) == 0) {
        fprintf(stderr, "QUEUE SIZE 0 in CHECK ENDING\n");
        
        if (thread->state == TERMINATED) {
            fprintf(stderr, "UNLOCKS END\n");
    
            rv = sem_post(&(scheduler->end));
            DIE(rv, "Sem post end failed");
        }

        rv = sem_post(&(thread->running));
        DIE(rv, "Sem post thread running failed in check_ending!");
    }
}

static void scheduler_check() 
{
    int rv;
    thread_t *current, *next;

    current = scheduler->current_thread;

    if (queue_size(scheduler->ready) == 0) {
        fprintf(stderr, "QUEUE SIZE 0 in CHECK ENDING\n");
        
        if (current->state == TERMINATED) {
            fprintf(stderr, "UNLOCKS END\n");
    
            rv = sem_post(&(scheduler->end));
            DIE(rv, "Sem post end failed");
        }

        rv = sem_post(&(current->running));
        DIE(rv, "Sem post thread running failed in check_ending!");
        fprintf(stderr, "SALOOT!\n");
        return;
    }

    if (queue_size(scheduler->ready) == 0)
        fprintf(stderr, "scher_rdy size is 0\n");

     if (queue_size(scheduler->ready) == 1)
        fprintf(stderr, "scher_rdy size is 1\n");

    next = queue_pop(scheduler->ready);
    fprintf(stderr, "new_size after top %d\n", queue_size(scheduler->ready));

    

    if (!current) {
        next->state = RUNNING;
        next->time_quantum = scheduler->time_quantum;
        scheduler->current_thread = next;
        rv = sem_post(&(next->running));
        DIE(rv, "Sem post failed!");

        return;
    }

    fprintf(stderr, "BEFORE eqal!\n"); 
    next->state = RUNNING;
    fprintf(stderr, "AFTER eqal!\n"); 

    if (current->state == TERMINATED) { // IF PT IO DE ADAUGAT
       

        queue_push(scheduler->finished, current);
        fprintf(stderr, "AFTER PUSH!\n");        

        next->state = RUNNING;

        fprintf(stderr, "AFTER state!\n"); 

        next->time_quantum = scheduler->time_quantum;

        fprintf(stderr, "AFTER quant!\n"); 
        scheduler->current_thread = next;

        fprintf(stderr, "AFTER equal!\n"); 
        rv = sem_post(&(next->running));


        DIE(rv, "Sem post failed!");
        return;
    }
    
    if (current->priority < next->priority) {
        mark_as_ready(current);
        
        next->state = RUNNING;
        next->time_quantum = scheduler->time_quantum;
        scheduler->current_thread = next;
        rv = sem_post(&(next->running));
        DIE(rv, "Sem post failed!");


        return;
    }
    // POATE COMBIN ULTIMELE DOUA IF-URI
    if (current->time_quantum <= 0) {

        if (current->priority == next->priority) {
            mark_as_ready(current);
        
            next->state = RUNNING;
            next->time_quantum = scheduler->time_quantum;
            scheduler->current_thread = next;
            rv = sem_post(&(next->running));
            DIE(rv, "Sem post failed!");

            return;
        }

        current->time_quantum = scheduler->time_quantum;
    } // == 0

    
    rv = sem_post(&(current->running));
    fprintf(stderr, "HERE LAST\n");
    DIE(rv, "Sem post failed!");
}



void mark_as_ready(thread_t *thread)
{
    fprintf(stderr, "HERE! mark as ready!\n");
    thread->state = READY;
    queue_push(scheduler->ready, thread);

    fprintf(stderr, "SIZE IN MARK_AS_READY: %d\n", queue_size(scheduler->ready));
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

    rv = sem_init(&(scheduler->end), 0, 1);
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
    fprintf(stderr, "ENTER FINAL TERMINATED\n");
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

    if (scheduler->no_threads == 0)
        sem_wait(&(scheduler->end));

    thread->priority = priority;
    thread->time_quantum = scheduler->time_quantum;
    thread->handler = func;

    rv = sem_init(&(thread->running), 0, 0);
    DIE(rv, "Pthread init thread->running failed!");
    
    rv = pthread_create(&(thread->tid), NULL, start_thread, thread);
    DIE(rv, "Pthread create error!");
    
    ++(scheduler->no_threads);

    mark_as_ready(thread);

    /* If we are the first thread */
    if (scheduler->current_thread != NULL) {
        fprintf(stderr, "FIRSTTTTTTTT\n");
        so_exec();
    }
    else {
        fprintf(stderr, "SECONDDDDD\n");
        scheduler_check();
    }
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
    fprintf(stderr, "SO_END! from main thread\n");
    if (scheduler) {
        if (scheduler->no_threads) 
            sem_wait(&(scheduler->end));
        
        fprintf(stderr, "SHOULD NOT GO HERE from main thread\n");

        queue_free(scheduler->ready);
        queue_free(scheduler->finished);

        fprintf(stderr, "SALOOT2!\n");

        

        // if (scheduler->current_thread)
        //     free_func(&(scheduler->current_thread));

        fprintf(stderr, "HELLO!\n");
        
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