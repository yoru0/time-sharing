/*
 * Time-Sharing System Simulation using Signals and Timers
 * Simplified Round-Robin Scheduling with Performance Metrics
 *
 * This program simulates a time-sharing system where multiple processes
 * share CPU time. It uses SIGALRM signals and interval timers to implement
 * time slicing, and calculates waiting time and turnaround time metrics.
 *
 * Simplified version with only READY and RUNNING states.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#define MAX_PROCESSES 10
#define TIME_QUANTUM 2  // Time slice in seconds

// Process states (simplified)
typedef enum {
    READY,
    RUNNING
} ProcessState;

// Process structure
typedef struct {
    int pid;
    char name[20];
    int arrival_time;      // When process arrives (all 0 in this version)
    int burst_time;        // Total CPU time needed
    int remaining_time;    // Time left to execute
    int waiting_time;      // Time spent waiting
    int turnaround_time;   // Total time from arrival to completion
    int completion_time;   // When process finished
    ProcessState state;
} Process;

// Global variables
Process processes[MAX_PROCESSES];
int num_processes = 0;
int current_process = 0;
int total_finished = 0;
int current_time = 0;

// Function prototypes
void init_processes();
void scheduler(int signum);
void display_status();
void display_final_metrics();
void setup_timer();
int find_next_process();

/*
 * Initialize processes with user input or default values
 */
void init_processes() {
    char choice;
    printf("Use default process values? (y/n): ");
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y') {
        // Default processes - all arrive at time 0
        num_processes = 4;

        processes[0].pid = 1;
        strcpy(processes[0].name, "Process-A");
        processes[0].arrival_time = 0;
        processes[0].burst_time = 8;
        processes[0].remaining_time = 8;
        processes[0].state = READY;

        processes[1].pid = 2;
        strcpy(processes[1].name, "Process-B");
        processes[1].arrival_time = 0;
        processes[1].burst_time = 6;
        processes[1].remaining_time = 6;
        processes[1].state = READY;

        processes[2].pid = 3;
        strcpy(processes[2].name, "Process-C");
        processes[2].arrival_time = 0;
        processes[2].burst_time = 4;
        processes[2].remaining_time = 4;
        processes[2].state = READY;

        processes[3].pid = 4;
        strcpy(processes[3].name, "Process-D");
        processes[3].arrival_time = 0;
        processes[3].burst_time = 10;
        processes[3].remaining_time = 10;
        processes[3].state = READY;
    } else {
        // Custom input - all processes arrive at time 0
        printf("Enter number of processes (max %d): ", MAX_PROCESSES);
        scanf("%d", &num_processes);

        if (num_processes > MAX_PROCESSES) {
            num_processes = MAX_PROCESSES;
        }

        for (int i = 0; i < num_processes; i++) {
            processes[i].pid = i + 1;
            sprintf(processes[i].name, "Process-%d", i + 1);
            processes[i].arrival_time = 0;  // All arrive at time 0

            printf("Enter burst time for Process-%d: ", i + 1);
            scanf("%d", &processes[i].burst_time);

            processes[i].remaining_time = processes[i].burst_time;
            processes[i].waiting_time = 0;
            processes[i].turnaround_time = 0;
            processes[i].completion_time = 0;
            processes[i].state = READY;
        }
    }
}

/*
 * Display current status of all processes
 */
void display_status() {
    printf("\n========================================\n");
    printf("Time: %d seconds\n", current_time);
    printf("========================================\n");
    printf("PID | Name         | State   | Burst | Remaining\n");
    printf("----+-------------+---------+-------+----------\n");

    for (int i = 0; i < num_processes; i++) {
        // Skip finished processes (remaining_time == 0)
        if (processes[i].remaining_time == 0) {
            continue;
        }

        printf("%3d | %-12s | ", processes[i].pid, processes[i].name);

        if (processes[i].state == RUNNING) {
            printf("RUNNING ");
        } else {
            printf("READY   ");
        }

        printf("| %5d | %9d\n",
               processes[i].burst_time,
               processes[i].remaining_time);
    }
    printf("========================================\n\n");
}

/*
 * Display final performance metrics
 */
void display_final_metrics() {
    float total_waiting = 0, total_turnaround = 0;

    printf("\n=============================================================================\n");
    printf("                        FINAL PERFORMANCE METRICS\n");
    printf("=============================================================================\n");
    printf("PID | Name         | Arrival | Burst | Completion | Turnaround | Waiting\n");
    printf("----+-------------+---------+-------+------------+------------+---------\n");

    for (int i = 0; i < num_processes; i++) {
        printf("%3d | %-12s | %7d | %5d | %10d | %10d | %7d\n",
               processes[i].pid,
               processes[i].name,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time);

        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;
    }

    printf("=============================================================================\n");
    printf("Average Waiting Time: %.2f seconds\n", total_waiting / num_processes);
    printf("Average Turnaround Time: %.2f seconds\n", total_turnaround / num_processes);
    printf("=============================================================================\n");
}

/*
 * Find next ready process using Round-Robin
 */
int find_next_process() {
    int start = current_process;
    int next = (current_process + 1) % num_processes;

    // Search for next ready process in round-robin order
    while (next != start) {
        if (processes[next].state == READY && processes[next].remaining_time > 0) {
            return next;
        }
        next = (next + 1) % num_processes;
    }

    // Check the starting position too
    if (processes[start].state == READY && processes[start].remaining_time > 0) {
        return start;
    }

    return -1;  // No ready process found
}

/*
 * Scheduler function - called when SIGALRM signal is received
 * Implements Round-Robin scheduling algorithm
 */
void scheduler(int signum) {
    // Increment current time
    current_time += TIME_QUANTUM;

    // If there's a currently running process, update it
    if (current_process >= 0 && processes[current_process].state == RUNNING) {
        int time_used = TIME_QUANTUM;
        if (processes[current_process].remaining_time < TIME_QUANTUM) {
            time_used = processes[current_process].remaining_time;
        }

        processes[current_process].remaining_time -= time_used;

        // Check if process has finished (remaining_time == 0)
        if (processes[current_process].remaining_time == 0) {
            processes[current_process].completion_time = current_time;
            processes[current_process].turnaround_time =
                processes[current_process].completion_time - processes[current_process].arrival_time;
            processes[current_process].waiting_time =
                processes[current_process].turnaround_time - processes[current_process].burst_time;

            total_finished++;
            printf(">>> %s has COMPLETED execution at time %d!\n",
                   processes[current_process].name, current_time);

            // Check if all processes are finished
            if (total_finished >= num_processes) {
                printf("\n*** ALL PROCESSES COMPLETED ***\n");
                display_final_metrics();
                exit(0);
            }

            // Process finished, don't move to READY
            processes[current_process].state = READY;  // Doesn't matter, remaining_time == 0
        } else {
            // Process still has time remaining, move to ready state
            processes[current_process].state = READY;
        }
    }

    // Find next process to run
    int next = find_next_process();

    if (next >= 0) {
        // Found a ready process
        current_process = next;
        processes[current_process].state = RUNNING;
        printf("\n>>> CPU allocated to %s at time %d (Time Quantum: %ds)\n",
               processes[current_process].name, current_time, TIME_QUANTUM);
        display_status();
    } else {
        // All processes finished
        printf("\n*** ALL PROCESSES COMPLETED ***\n");
        display_final_metrics();
        exit(0);
    }
}

/*
 * Setup interval timer to generate SIGALRM signals
 */
void setup_timer() {
    struct itimerval timer;

    // Configure timer to fire every TIME_QUANTUM seconds
    timer.it_value.tv_sec = TIME_QUANTUM;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = TIME_QUANTUM;
    timer.it_interval.tv_usec = 0;

    // Set the timer
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("Error setting timer");
        exit(1);
    }
}

/*
 * Main function
 */
int main() {
    printf("===========================================\n");
    printf("  TIME-SHARING SYSTEM SIMULATION\n");
    printf("  Simplified Round-Robin (READY/RUNNING)\n");
    printf("===========================================\n");
    printf("Time Quantum: %d seconds\n", TIME_QUANTUM);
    printf("All processes arrive at time 0\n");
    printf("===========================================\n\n");

    // Initialize processes
    init_processes();

    // Display initial status
    printf("\nInitial Process Queue:\n");
    display_status();

    // Register signal handler for SIGALRM
    signal(SIGALRM, scheduler);

    // Start with first process
    current_process = 0;
    processes[0].state = RUNNING;
    printf(">>> Starting execution with %s at time %d\n",
           processes[0].name, current_time);

    // Setup and start the timer
    setup_timer();

    // Infinite loop - waiting for signals
    while (1) {
        pause();  // Wait for signal
    }

    return 0;
}
