# SYSC4001_A3P2
This project contains the programs for Part 2A and Part 2B of assignment 3. Part 2A runs without any synchronization, so the TA processes access shared memory at the same time, causing race conditions. Part 2B adds a semaphore, which forces the TAs to take turns and eliminates data corruption.

# Compilation Instructions
To compile Part 2A:

g++ -std=c++17 -Wall -O2 A3_P2_A_101262847_101301514.cpp -pthread -o A3_P2_A_101262847_101301514


To compile Part 2B:

g++ -std=c++17 -Wall -O2 A3_P2_B_101262847_101301514.cpp -pthread -o A3_P2_B_101262847_101301514


Both programs require pthread for multithreading. No additional libraries are needed.

# Folder
The folder should contain:

A3_P2_A_101262847_101301514.cpp

A3_P2_B_101262847_101301514.cpp

rubric.txt

exams/ (a directory containing exam0001.txt to exam0020.txt)

# How to Run
To run Part 2A with 3 TAs:

./A3_P2_A_101262847_101301514 3


To run Part 2B with 3 TAs:

./A3_P2_B_101262847_101301514 3


Any number of TAs is possible (>=1). For example:

./A3_P2_B_101262847_101301514 5


# Test Cases
The test cases for this assignment are running the program with a different numbers of TA processes and observing the behavior.

## Part 2A:
1 TA:

./A3_P2_A_101262847_101301514 1

Expected outpt: No race conditions (only one TA accesses shared memory).


3 TAs:

./A3_P2_A_101262847_101301514 3

Expected output: Chaotic output, race conditions, inconsistent rubric values.


## Part 2B:
3 TAs:

./A3_P2_B_101262847_101301514 3

Expected output: Consistent behavior. Semaphore prevents races.


5 TAs:

./A3_P2_B_101262847_101301514 5

Expected output: Consistent behavior. Semaphore prevents races.
