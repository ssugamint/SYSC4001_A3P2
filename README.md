# SYSC4001_A3P2
This project contains the programs for Part 2A and Part 2B of assignment 3. Part 2A runs without any synchronization, so the TA processes access shared memory at the same time, causing race conditions. Part 2B adds a semaphore, which forces the TAs to take turns and eliminates data corruption.

# Compilation Instructions
To compile Part 2A:

g++ -std=c++17 -Wall -O2 A3_P2_A.cpp -pthread -o A3_P2_A


To compile Part 2B:

g++ -std=c++17 -Wall -O2 A3_P2_B.cpp -pthread -o A3_P2_B


Both programs require pthread for multithreading. No additional libraries are needed.

# Folder
The folder should contain:

A3_P2_A.cpp

A3_P2_B.cpp

rubric.txt

exams/ (a directory containing exam0001.txt to exam0020.txt)

# How to Run
To run Part 2A with 3 TAs:

./A3_P2_A 3


To run Part 2B with 3 TAs:

./A3_P2_B 3


Any number of TAs is possible (>=1). For example:

./A3_P2_B 5
