CC = gcc
CFLAGS = -Wall -Wextra -fPIC
LDFLAGS = -shared

.PHONY: build
libscheduler.so: build

build: so_scheduler.o heap.o
	$(CC) $(LDFLAGS) so_scheduler.o heap.o -o libscheduler.so

so_scheduler.o: so_scheduler.c
	$(CC) $(CFLAGS) so_scheduler.c -c -o so_scheduler.o

heap.o: heap.c
	$(CC) $(CFLAGS) heap.c -c -o heap.o

.PHONY: clean
clean:
	rm -f *.o libscheduler.so
