CC = gcc
CFLAGS = -Wall -Wextra

.PHONY: build
libscheduler.so: build

build: so_scheduler.o heap.o
	$(CC) -shared so_scheduler.o heap.o -o libscheduler.so

so_scheduler.o: so_scheduler.c
	$(CC) $(CFLAGS) -fPIC so_scheduler.c -c -o so_scheduler.o

heap.o: heap.c
	$(CC) $(CFLAGS) -fPIC heap.c -c -o heap.o

.PHONY: clean
clean:
	rm *.o libscheduler.so
