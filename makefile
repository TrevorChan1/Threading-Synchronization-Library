override CFLAGS := -Wall -Werror -std=gnu99 -O0 -g $(CFLAGS) -I.
CC = gcc

all: check

# Build the threads.o file
threads.o: threads.c ec440threads.h

# build the busy_threads.o file
busy_threads.o: busy_threads.c ec440threads.h

# make executable
test_busy_threads: busy_threads.o threads.o

test_files=./test_busy_threads

.PHONY: clean check checkprogs all

# Build all of the test programs
checkprogs: $(test_files)

# Run the test programs
check: checkprogs
	tests/run_tests.sh $(test_files)

clean:
	rm -f *.o $(test_files) $(test_o_files)