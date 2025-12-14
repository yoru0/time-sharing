/*
 * Time-Sharing System Simulation using Signals and Timers
 * Compact Round-Robin Scheduling Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#define NUM_PROCESSES 4
#define TIME_QUANTUM 2

typedef enum { READY, RUNNING } State;

typedef struct {
    int pid;
    int burst_time;
    int remaining_time;
    int waiting_time;
    int turnaround_time;
    State state;
} Process;

Process processes[NUM_PROCESSES];
int current = 0;
int finished = 0;
int clock_time = 0;

void scheduler(int sig) {
    clock_time += TIME_QUANTUM;

    // Update current running process
    if (processes[current].state == RUNNING) {
        processes[current].remaining_time -= TIME_QUANTUM;

        if (processes[current].remaining_time <= 0) {
            processes[current].remaining_time = 0;
            processes[current].turnaround_time = clock_time;
            processes[current].waiting_time = clock_time - processes[current].burst_time;
            finished++;
            printf("Process %d completed at time %d\n", processes[current].pid, clock_time);

            if (finished >= NUM_PROCESSES) {
                printf("\n--- Final Results ---\n");
                float avg_wait = 0, avg_turn = 0;
                for (int i = 0; i < NUM_PROCESSES; i++) {
                    printf("P%d: Burst=%d, Waiting=%d, Turnaround=%d\n",
                           processes[i].pid, processes[i].burst_time,
                           processes[i].waiting_time, processes[i].turnaround_time);
                    avg_wait += processes[i].waiting_time;
                    avg_turn += processes[i].turnaround_time;
                }
                printf("Average Waiting Time: %.2f\n", avg_wait / NUM_PROCESSES);
                printf("Average Turnaround Time: %.2f\n", avg_turn / NUM_PROCESSES);
                exit(0);
            }
        }
        processes[current].state = READY;
    }

    // Find next ready process (Round-Robin)
    int start = current;
    do {
        current = (current + 1) % NUM_PROCESSES;
        if (processes[current].state == READY && processes[current].remaining_time > 0) {
            processes[current].state = RUNNING;
            printf("Time %d: Process %d running\n", clock_time, processes[current].pid);
            return;
        }
    } while (current != start);
}

int main() {
    printf("=== Time-Sharing System (Round-Robin) ===\n");
    printf("Time Quantum: %d seconds\n\n", TIME_QUANTUM);

    // Initialize processes
    int burst_times[] = {8, 6, 4, 10};
    for (int i = 0; i < NUM_PROCESSES; i++) {
        processes[i].pid = i + 1;
        processes[i].burst_time = burst_times[i];
        processes[i].remaining_time = burst_times[i];
        processes[i].state = READY;
    }

    // Start first process
    processes[0].state = RUNNING;
    printf("Time 0: Process 1 running\n");

    // Setup timer and signal
    signal(SIGALRM, scheduler);
    struct itimerval timer;
    timer.it_value.tv_sec = TIME_QUANTUM;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = TIME_QUANTUM;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);

    // Wait for signals
    while (1) pause();

    return 0;
}
