* Name: Cristian-Tanase Paris 331CAb 

# Homework <NR> 4 Thread Scheduler

General structure
-

* The homework consists of 6 main parts: the handler binding function, the
SIGSEGV handler, the look-up function (whose main purpose is to retrieve
a pointer to the segment where the page fault occurred), the read function (that
transfers data between the file and the mapped memory region), a function that
converts the Linux style permission to Windows format and an init function for
the auxiliary array used for keeping track of mapped pages.
* I found this assignment particularly useful because it cleared up some things
regarding the mapping process, executable structure and page faults that I 
didn't quite understand during the lectures.
* From my point of view, I think the assignment has been implemented as efficient
as possible, in a way that is easy to understand. 

Implementation
-

* The entire functionality has been implemented.
* For the sake of simplicity and accessibility, the file handle was declared
as a static variable outside any function scope and the page size on Windows
was defined as a macro with a fixed size of 65536 bytes.

```
get_segment
```
* Receives a virtual address and returns a pointer to the segment it belongs to
or NULL otherwise.

```
my_read
```
* Receives a buffer address used for storing the read data, the amount of bytes
to read (size argument) and the file offset where to begin the reading.

```
convert_permissions
```
* Receives the Linux style permissions and converts them to the Windows
standard.

```
init_aux_data
```
* Helper function that inits a dynamically allocated vector for each segment.
It is used for keeping track of the mapped pages.

```
so_init_loader
```
* We initialize the page size and associate a new handler for SIGSEGV.

```
fault_handler
```
* We extract the virtual address that caused the fault from the info structure
and get a pointer to the segment containing the address.
* We check if the cause of the SIGSEGV is because the program tried to access
a page within an unknown segment or the segment was accessed with illegal 
permissions. In both cases, we run continue our search for a proper handler.
* If the program tried to access a page which has not yet been mapped, we 
calculate the page index within the segment, the offset inside the file where 
should we start to copy data from and the effective address where we are
supposed to map the page and then we mark the page as being mapped.
* We map the page in memory.
* We check for corner cases:
    1. Only a part of the page sits outside the file range.
	   We need to zero the rest of the page, so we calculate the remainder.
* We copy the data from the file inside the reserved memory page.
* We restore the segment permissions for that page. (there may be a case where
we try to write to copy data to a page with no write permissions, so we first
create the page with read/write and after copying the required data restore
the permissions.)

* Difficulties:
    1. Debugging was kinda hard.
    2. Resolving the corner cases and having the right conditions in the handler
    function.
    1. The Windows documentation is awful.
    2. The checkstyle script should be replaced with one that actually works.
    3. Had to switch MapViewofFileEx with VirtualAlloc because of countless errors.
    4. Some fields in the EXCEPTION RECORD are misleading. 

* Interesting facts:
    1. For the Windows implementation I had to use an auxiliary data structure
    because I couldn't find a way to check an illegal permissions access
    or if a page was already mapped using the info from the EXCEPTION RECORD
    or VirtualQuery.
    2. No need to memset an entire page if it resides outside the file size
    range. VirtualAlloc guarantees that a page will be zeroed if we use the 
    MEM_COMMIT and MEM_RESERVE flags.

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
