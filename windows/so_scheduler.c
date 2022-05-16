#include "so_scheduler.h"
#include "prio_queue.h"

#define SO_FAIL -1

/* Enum representing the possible states a thread can find itself in */
typedef enum {
	READY,
	RUNNING,
	WAITING,
	TERMINATED
} thread_state_t;

/* Thread wrapper */
typedef struct {
	HANDLE tid; /* Thread handle */
	so_handler *handler; /* Function handler */
	thread_state_t state; /* Current thread state */
	int time_quantum; /* Time left on the processor while running */
	int priority; /* Thread priority */

	/* Synchronization elements */
	HANDLE running; /* Used for blocking a thread when it is preempted */
} thread_t;

/* Scheduler info */
typedef struct {
	int time_quantum; /* Max allowed time quantum */
	int io; /* Max number of io devices */
	int no_threads; /* Number of threads handled by the scheduler */

	thread_t *thread; /* Pointer to the currently running thread */
	prio_queue_t *ready; /* Threads which are waiting to be planned */
	prio_queue_t *finished; /* Threads which finished their tasks */
	prio_queue_t **waiting; /* Blocked threads by an event */

	/* Synchronization elements */
	HANDLE end; /* Used for signaling when the scheduler should stop */
} scheduler_t;

scheduler_t *scheduler;

void mark_as_ready(thread_t *thread);

void plan_next(void);

void scheduler_check(void);

/* Cmp func used by the prio_queue for inserting the elements */
int cmp_func(const void *t1, const void *t2)
{
	return ((thread_t *)t1)->priority - ((thread_t *)t2)->priority;
}

/* Free func used by the prio_queue for freeing up thread memory */
void free_func(void *t)
{
	int rv;
	/* Wait for the thread to finish and free the semaphore memory */
	rv = WaitForSingleObject(((thread_t *)t)->tid, INFINITE);
	DIE(rv == WAIT_FAILED, "WaitForSingleObject failed!");

	rv = CloseHandle(((thread_t *)t)->running);
	DIE(!rv, "CloseHandle failed!");

	free(t);
}

/* Gets the next ready thread from the queue and sets its state to RUNNING */
void plan_next(void)
{
	int rv;

	scheduler->thread = queue_pop(scheduler->ready);
	scheduler->thread->state = RUNNING;
	scheduler->thread->time_quantum = scheduler->time_quantum;
	/* Signal the thread it is okay to start execution */
	rv = ReleaseSemaphore(scheduler->thread->running, 1, 0);
	DIE(!rv, "ReleaseSemaphore failed!");
}

/* Scheduling logic function. It handles all the possible cases */
void scheduler_check(void)
{
	int rv;
	thread_t *current = scheduler->thread;

	if (!queue_size(scheduler->ready)) {
		if (current->state == TERMINATED) {
			/* Signal the scheduler to stop */
			rv = ReleaseSemaphore(scheduler->end, 1, 0);
			DIE(!rv, "ReleaseSemaphore failed!");
		}

		/* Signal the current thread it can still run */
		rv = ReleaseSemaphore(current->running, 1, 0);
		DIE(!rv, "ReleaseSemaphore failed!");
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

	if (current->priority <
		((thread_t *)queue_top(scheduler->ready))->priority) {
		mark_as_ready(current);
		plan_next();
		return;
	}

	if (!current->time_quantum) {
		if (current->priority ==
			((thread_t *)queue_top(scheduler->ready))->priority) {
			mark_as_ready(current);
			plan_next();
			return;
		}
		current->time_quantum = scheduler->time_quantum;
	}
	/* The current thread can still run */
	rv = ReleaseSemaphore(current->running, 1, 0);
	DIE(!rv, "ReleaseSemaphore failed!");
}

/* Add thread to ready queue */
void mark_as_ready(thread_t *thread)
{
	thread->state = READY;
	queue_push(scheduler->ready, thread);
}

int so_init(unsigned int time_quantum, unsigned int io)
{
	int i;

	if (scheduler || io > SO_MAX_NUM_EVENTS || !time_quantum)
		return SO_FAIL;

	DIE(!(scheduler = calloc(1, sizeof(scheduler_t))), "scheduler calloc!");

	scheduler->end = CreateSemaphore(NULL, 0, 1, NULL);
	DIE(!scheduler->end, "CreateSemaphore failed!");

	scheduler->time_quantum = time_quantum;
	scheduler->io = io;
	scheduler->ready = queue_init(cmp_func, free_func);
	scheduler->finished = queue_init(cmp_func, free_func);

	scheduler->waiting = calloc(io, sizeof(prio_queue_t *));
	DIE(!scheduler->waiting, "Failed to calloc array of waiting queues!");

	for (i = 0; i != (int)io; ++i)
		scheduler->waiting[i] = queue_init(cmp_func, free_func);

	return 0;
}

void *start_thread(void *args)
{
	int rv;
	/* The thread should block here and wait till is unlocked */
	rv = WaitForSingleObject(((thread_t *)args)->running, INFINITE);
	DIE(rv == WAIT_FAILED, "WaitForSingleObject failed!.");

	/* Thread runs its tasks via handler */
	((thread_t *)args)->handler(((thread_t *)args)->priority);

	/* Thread finished its tasks. Mark the thread as terminated */
	((thread_t *)args)->state = TERMINATED;

	/* Call the scheduler */
	scheduler_check();

	ExitThread(1);
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	thread_t *thread;

	if (!func || priority > SO_MAX_PRIO)
		return INVALID_TID;

	DIE(!(thread = calloc(1, sizeof(thread_t))), "thread calloc failed!");
	/* Init and start thread */
	thread->priority = priority;
	thread->time_quantum = scheduler->time_quantum;
	thread->handler = func;

	thread->running = CreateSemaphore(NULL, 0, 1, NULL);
	DIE(!thread->running, "CreateSemaphore failed!");

	thread->tid = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)
				start_thread, thread, 0, NULL);
	DIE(!thread->tid, "CreateThread failed!");

	++scheduler->no_threads;
	mark_as_ready(thread);

	if (scheduler->thread != NULL)
		so_exec(); /* If fork was called by another thread */
	else
		scheduler_check(); /* If we are the first thread */

	return GetThreadId(thread->tid);
}

int so_wait(unsigned int io)
{
	if ((int)io >= scheduler->io)
		return SO_FAIL;

	/* Wait for the received signal */
	scheduler->thread->state = WAITING;
	queue_push(scheduler->waiting[io], scheduler->thread);

	so_exec();
	return 0;
}

int so_signal(unsigned int io)
{
	thread_t *aux;
	int cnt;

	if ((int)io >= scheduler->io)
		return SO_FAIL;

	/* Wake-up all the threads waiting for that specific io */
	for (cnt = 0; (aux = queue_pop(scheduler->waiting[io])); ++cnt)
		mark_as_ready(aux);

	so_exec();
	return cnt;
}

void so_exec(void)
{
	int rv;
	thread_t *current = scheduler->thread;

	--current->time_quantum;

	/* Call the scheduler */
	scheduler_check();

	/* Wait here if you get preempteed */
	rv = WaitForSingleObject(current->running, INFINITE);
	DIE(rv == WAIT_FAILED, "WaitForSingleObject failed!");
}

void so_end(void)
{
	int i, rv;

	if (!scheduler)
		return;

	/* Wait for all threads to finish */
	if (scheduler->no_threads) {
		rv = WaitForSingleObject(scheduler->end, INFINITE);
		DIE(rv == WAIT_FAILED, "WaitForSingleObject failed!");
	}

	if (scheduler->thread)
		free_func(scheduler->thread);

	queue_free(scheduler->ready);
	queue_free(scheduler->finished);
	for (i = 0; i != (int)scheduler->io; ++i)
		queue_free(scheduler->waiting[i]);

	rv = CloseHandle(scheduler->end);
	DIE(!rv, "CloseHandle failed!");

	free(scheduler->waiting);
	free(scheduler);
	scheduler = NULL;
}
