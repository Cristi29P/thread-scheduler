CC = gcc
CFLAGS = -Wall -Wextra -Werror -fPIC
LDFLAGS = -shared

.PHONY: build
libscheduler.so: build

build: so_scheduler.o prio_queue.o linkedlist.o
	$(CC) $(LDFLAGS) so_scheduler.o prio_queue.o linkedlist.o -o libscheduler.so

so_scheduler.o: so_scheduler.c
	$(CC) $(CFLAGS) so_scheduler.c -c -o so_scheduler.o

prio_queue.o: prio_queue.c
	$(CC) $(CFLAGS) prio_queue.c -c -o prio_queue.o

linkedlist.o: linkedlist.c
	$(CC) $(CFLAGS) linkedlist.c -c -o linkedlist.o

.PHONY: clean
clean:
	rm -f *.o libscheduler.so
