#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>

#define NUM_TASK 3
#define TIME_SLICE 2  // seconds

int current_task = 0;
pid_t tasks[NUM_TASK];

void timer_handler();
void run_task(int task_id);

int main() {
    printf("Time-Sharing System Simulation\n\n");

    // create child processes (tasks)
    for (int i = 0; i < NUM_TASK; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // child process
            run_task(i);
            exit(0);
        } else {
            tasks[i] = pid;
            // initially stop all tasks except the first one
            if (i != 0)
                kill(tasks[i], SIGSTOP);
        }
    }

    return 0;
}