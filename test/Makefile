LIBDIR = ../lib/lamps_scheduler/src
CC=gcc
override CFLAGS += -Wall -I$(LIBDIR)

all: test_lamps_scheduler
	@./test_lamps_scheduler

lamps_scheduler.o: $(LIBDIR)/lamps_scheduler.c $(LIBDIR)/lamps_scheduler.h 
	@$(CC) $(CFLAGS) -c $< -o $@

test_lamps_scheduler: test_lamps_scheduler.c lamps_scheduler.o
	@$(CC) $(CFLAGS) $^ -o $@

clean:
	@rm -f *.o
	@rm -f test_lamps_scheduler
