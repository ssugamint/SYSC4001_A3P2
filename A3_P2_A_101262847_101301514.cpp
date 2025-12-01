// ta_partA_simple.cpp
// Simplified Part 2(a) version: no semaphores (intentionally has races)
// Compile: g++ -std=c++17 -Wall -O2 ta_partA_simple.cpp -o ta_partA

#define _XOPEN_SOURCE 700
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

using namespace std;

constexpr int RUBRIC_LINES = 5;
constexpr int LINE_LEN = 64;
constexpr int QUESTIONS = 5;
constexpr char EXAM_FOLDER[] = "./exams/";
constexpr char RUBRIC_FILE[] = "./rubric.txt";

struct Shared {
    char rubric[RUBRIC_LINES][LINE_LEN];
    int question_state[QUESTIONS]; // 0=unmarked,1=being,2=marked
    int exam_index;                // next exam to load (0-based)
    int exam_student_number;
    int terminate_flag;
};

Shared *shared_mem = nullptr;

static int rand_range_ms(unsigned int *seedp, int a, int b) {
    return a + (rand_r(seedp) % (b - a + 1));
}

void load_rubric() {
    ifstream f(RUBRIC_FILE);
    if (!f.is_open()) {
        for (int i = 0; i < RUBRIC_LINES; ++i)
            snprintf(shared_mem->rubric[i], LINE_LEN, "%d, %c\n", i+1, 'A'+i);
        return;
    }
    string line;
    for (int i = 0; i < RUBRIC_LINES; ++i) {
        if (getline(f, line)) {
            snprintf(shared_mem->rubric[i], LINE_LEN, "%s\n", line.c_str());
        } else {
            snprintf(shared_mem->rubric[i], LINE_LEN, "%d, %c\n", i+1, 'A'+i);
        }
    }
    f.close();
}

void save_rubric() {
    ofstream f(RUBRIC_FILE, ios::trunc);
    if (!f.is_open()) return;
    for (int i = 0; i < RUBRIC_LINES; ++i) {
        string s(shared_mem->rubric[i]);
        while (!s.empty() && (s.back()=='\n' || s.back()=='\r')) s.pop_back();
        f << s << '\n';
    }
    f.close();
}

int load_exam_by_index(int idx) {
    char path[256];
    snprintf(path, sizeof(path), "%sexam%04d.txt", EXAM_FOLDER, idx+1);
    ifstream f(path);
    if (!f.is_open()) { shared_mem->exam_student_number = 9999; return -1; }
    string line;
    if (!getline(f, line)) { shared_mem->exam_student_number = 9999; }
    else {
        istringstream iss(line);
        iss >> shared_mem->exam_student_number;
    }
    f.close();
    for (int i=0;i<QUESTIONS;++i) shared_mem->question_state[i]=0;
    return 0;
}

void ta_process(int id) {
    unsigned int seed = time(nullptr) ^ (id<<8) ^ getpid();
    while (!shared_mem->terminate_flag) {
        // read rubric (no locking)
        cout << "[TA " << id << "] Rubric:\n";
        for (int i=0;i<RUBRIC_LINES;++i) cout << "  " << shared_mem->rubric[i];
        cout.flush();

        // maybe change rubric (race-prone)
        for (int r=0;r<RUBRIC_LINES;++r) {
            usleep(rand_range_ms(&seed, 200, 500) * 1000);
            if (rand_r(&seed) % 2) {
                // unsafe modification: no lock
                char *line = shared_mem->rubric[r];
                char *c = strchr(line, ',');
                if (c) {
                    char *p = c+1;
                    while (*p==' ') ++p;
                    if (*p >= 'A' && *p < 'Z') *p = *p + 1;
                    else if (*p == 'Z') *p = 'A';
                    else if (*p == '\0') *p = 'A';
                }
                save_rubric();
                cout << "[TA " << id << "] modified rubric line " << r+1 << "\n"; cout.flush();
            }
        }

        // pick a question: UNSAFE (race)
        int picked = -1;
        for (int q=0;q<QUESTIONS;++q) {
            if (shared_mem->question_state[q] == 0) {
                // claim it (race: two TAs could do this simultaneously)
                shared_mem->question_state[q] = 1;
                picked = q;
                break;
            }
        }

        if (picked >= 0) {
            int mark_ms = rand_range_ms(&seed, 500, 1500);
            cout << "[TA " << id << "] marking student " << shared_mem->exam_student_number
                 << " q" << (picked+1) << " (" << mark_ms << "ms)\n"; cout.flush();
            usleep(mark_ms * 1000);
            shared_mem->question_state[picked] = 2;
            cout << "[TA " << id << "] finished q" << (picked+1) << "\n"; cout.flush();

            // check if all done
            bool all = true;
            for (int q=0;q<QUESTIONS;++q) if (shared_mem->question_state[q] != 2) { all=false; break; }
            if (all) {
                // UNSAFE loading: two TAs may both load the same next exam
                int idx = shared_mem->exam_index;
                cout << "[TA " << id << "] (UNPROTECTED) loading exam idx " << idx << "\n";
                if (load_exam_by_index(idx) < 0) shared_mem->terminate_flag = 1;
                shared_mem->exam_index++;
                if (shared_mem->exam_student_number == 9999) shared_mem->terminate_flag = 1;
            }
        } else {
            // nothing to do; sleep a bit
            usleep(100 * 1000);
        }
    }
    cout << "[TA " << id << "] exiting\n"; cout.flush();
    _exit(0);
}

int main(int argc, char** argv) {
    if (argc < 2) { cerr << "Usage: ./ta_partA <num_tas>\n"; return 1; }
    int n = atoi(argv[1]);
    if (n < 1) n = 1;

    shared_mem = (Shared*) mmap(NULL, sizeof(Shared), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if (shared_mem == MAP_FAILED) { perror("mmap"); return 1; }
    memset(shared_mem, 0, sizeof(Shared));

    load_rubric();
    shared_mem->exam_index = 0;
    load_exam_by_index(0);
    shared_mem->exam_index = 1;
    if (shared_mem->exam_student_number == 9999) shared_mem->terminate_flag = 1;

    pid_t *kids = (pid_t*) calloc(n, sizeof(pid_t));
    for (int i=0;i<n;++i) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return 1; }
        if (pid == 0) { ta_process(i+1); }
        kids[i] = pid;
    }
    for (int i=0;i<n;++i) waitpid(kids[i], NULL, 0);
    cout << "[Parent] done\n";
    munmap(shared_mem, sizeof(Shared));
    free(kids);
    return 0;
}