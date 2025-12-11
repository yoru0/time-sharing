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

// signal handler for timer interrupt
void timer_handler() {
    printf("\n[Scheduler] Time slice expired. Switching tasks...\n");

    kill(tasks[current_task], SIGSTOP);
    current_task = (current_task + 1) % NUM_TASK;
    kill(tasks[current_task], SIGCONT);
    alarm(TIME_SLICE);
}

// task function
void run_task(int task_id) {
    int counter = 0;
    while(1) {
        printf("[Task %d] Counter: %d\n", task_id, counter++);
        sleep(1);
    }
}

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
    
    signal(SIGALRM, timer_handler);

    printf("\n[Scheduler] Starting task 0\n");
    alarm(TIME_SLICE);

    // parent process waits for all children
    for (int i = 0; i < NUM_TASK; i++) {
        waitpid(tasks[i], NULL, 0);
    }
    
    return 0;
}