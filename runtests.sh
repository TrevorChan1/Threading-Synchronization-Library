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

# Check results
if [[ *"Test case passed"* == "$(cat test1out.txt)" ]];
    then echo "Test 1 passed"
    else echo "Test 1 failed"
fi

