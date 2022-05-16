* Name: Cristian-Tanase Paris 331CAb

# Homework <NR> 4 Thread Scheduler

Note: This README is common for both implementations (Linux and Windows)

General structure
-

* This homework consists of several functions whose purpose could be split up in:
creation, synchronization and signaling.
* I found this assignment particularly useful because it redefined the way I was
used to think about threads and how they interact through the synchronization
mechanisms.
* From my point of view, I think the assignment has been implemented as efficient
as possible, in a way that is easy to understand.

Implementation
-

* The entire functionality has been implemented.
* For the sake of simplicity and readability, an enum was declared to describe
the multiple states a thread can find itself.
* Each thread has a wrapper structure that contains various information used across
the functions for synchronization and scheduling.
* The scheduler structure contains all the data structures used for storing the
threads and the priority queue already implements the round robin functionality
by default.
* The data structures used are generic and I won't go into details about their
actual implementation. (see the comments in the code)
* The PQ uses two auxiliary functions, a cmp_func and a free_func used for storing
the threads according to their priority/free the memory used by them.

#### General data flow ####

1. The main process calls so_init and the scheduler is created with all the
data structures and synchronization elements needed.
2. so_fork is called and the main thread creates a child thread, which blocks
upon entering its thread routine.
3. The main thread schedules the child thread (for the first so_fork) according
to certain criteria, unblocks the child thread by incrementing the child's
semaphore and immediately goes to so_end. Child threads can as well call so_fork
or other functions according to the handler they received as parameter.
4. The main thread is going to stay blocked in so_end till all the child threads
finish their handler routines, and the last child thread increments the main
thread's semaphore (unblocks it). Then, the main thread can proceed with cleaning
up the memory.
5. When a child thread creates other threads, they are immediately pushed to a
ready queue and the thread with the highest priority is popped from the queue in
order to run. (preemption if possible)
6. A thread can be pulled out of the queue to run if the currently running thread
got preempted by a higher priority one, or its time quantum expired.
7. The so_wait function blocks a thread's semaphore and pushes it to a waiting
queue bound to the signal received.
8. The threads waiting for the respective signal can be popped from the waiting
queue if another thread calls the so_signal function with that signal. Then, the
highest priority thread is once again chosen to run and finish its work.
9. Once a thread finish its work, it is pushed to terminated queue which is
entirely freed at the end of the program to save time.
10. The so_end function frees all the DS and the synchronization objects used.

#### Difficulties ####


1. Debugging was a bit troublesome because it was hard to track which thread
would end up giving a SEGFAULT or screw up the synchronization.
2. There were some corner cases I didn't think about at first, like the fact
that I need to block the newly created thread in its thread_func so the scheduler
thread would have time to plan its execution.
3. The checkstyle script should be replaced with one that actually works.
4. Making sure the data structures used are properly implemented in a generic
fashion and with no memory leaks or random behavior.

#### Interesting facts ####


1. It was actually a bit easier to implement the homework with WIN32 functions,
because everything is a HANDLE and WaitForSingleObject works for both semaphores
and threads.
2. Just because it worked one time, it doesn't mean the second time would still
work. Debugging a multithreaded application is quite hard.

How should I compile and run this library?
-

#### Linux ####

```
make
```

This should generate the `libscheduler.so` library. Next, build the example and run the checker:

```
make -f Makefile.checker
```

In order to run a specific test, pass the test number (1 .. 20) to the run_test
executable:

```
LD_LIBRARY_PATH=. ./_test/run_test 1
```

#### Windows ####

```
nmake -f Makefile (PowerShell)
```

This should generate the `libscheduler.dll` and `libscheduler.lib` files. Next, build the example and run the checker:

```
make -f Makefile.checker (Cygwin)
```

In order to run a specific test, pass the test number (1 .. 20) to the
run_test.exe executable:

```
./_test/run_test.exe 1
```

Resources
-

* Operating Systems official GitHub: [Makefile and file skeleton](https://github.com/systems-cs-pub-ro/so/tree/master/assignments/4-scheduler)
* Lab 8: [Linux Threads](https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-08)
* Lab 9: [Windows Threads](https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-09)
* Lecture 4: [Execution planning](https://ocw.cs.pub.ro/courses/so/curs/sched)
* Lecture 9: [Threads](https://ocw.cs.pub.ro/courses/so/curs/thread)
* Lecture 10: [Synchronization](https://ocw.cs.pub.ro/courses/so/curs/sync)
* Linux Man pages: [Link](https://linux.die.net/man/)
* MSDN: [Link](https://docs.microsoft.com/en-us/windows/win32/api/)
* Operating Systems Concepts: [Book](https://cloudflare-ipfs.com/ipfs/bafykbzaceavsju4l3yz7sbukzvmdxvaxtxvtceimf5hl2oesunfqaik3tlthq?filename=Abraham%20Silberschatz%2C%20Greg%20Gagne%2C%20Peter%20B.%20Galvin%20-%20Operating%20System%20Concepts-Wiley%20%282018%29.pdf)
* Windows System Programming: [Book](https://doc.lagout.org/operating%20system%20/Windows/Windows%20System%20Programming.pdf)
* The Linux Programming Interface: [Book](https://sciencesoftcode.files.wordpress.com/2018/12/the-linux-programming-interface-michael-kerrisk-1.pdf)
* LinkedList (this is my git for homework 1): [LinkedList](https://github.com/Cristi29P/c-preprocessor.git)
* DS: [Heap concepts](https://ocw.cs.pub.ro/courses/sd-ca/laboratoare/lab-09)
  
Git
-

* Link: [thread-scheduler](https://github.com/Cristi29P/thread-scheduler.git)
repository should be public after the 21st of May. Otherwise, please contact me
on Microsoft Teams or e-mail.
