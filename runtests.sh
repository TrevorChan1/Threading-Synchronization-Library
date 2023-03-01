#!/bin/bash

# Testing shell: Run runtests.sh from the challenge_2 directory using bash
# Like bash runtests.sh, hope this helps!

rm -rf thread_test_folder
mkdir thread_test_folder
cd thread_test_folder

## Test 1: Test if pthread_create returns -1 on exceeding the MAX_THREADS count
# Can probably break this test by just having your code print "Test case passed" but... don't do that

# Create executable
gcc -Wall -Werror -std=gnu99 -O0 -g ../threads.c ../tests/full_threads.c -o testfull

# Run executable
./testfull > test1out.txt

# Check results (should pass even if you implemented error messages)
if [[ "$(cat test1out.txt)" == *"Test case passed" ]];
    then echo "Test 1 passed"
    else echo "Test 1 failed"
fi

## Test 2: Check if pthread_create is able to create more threads after reaching MAX_THREAD
##         count and exiting some threads (like creates 128 threads, frees some, then can make more)

# Create executable
gcc -Wall -Werror -std=gnu99 -O0 -g ../threads.c ../tests/makemore.c -o testmakemore

# Run executable
./testmakemore > test2out.txt

# Check results (should pass even if you implemented error messages)
if [[ "$(cat test2out.txt)" == *"Test case passed" ]];
    then echo "Test 1 passed"
    else echo "Test 1 failed"
fi
