#define _POSIX_C_SOURCE 200809L
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_PROCESSES 4
#define TIME_QUANTUM_MS 500

typedef enum { READY, RUNNING, TERMINATED } ProcessState;

typedef struct {
    int pid;
    char name[16];
    ProcessState state;
    int work_done;
    int work_total;
} PCB;

static PCB procs[MAX_PROCESSES];
static int current = -1;
static int num_procs = 0;
static int switches = 0;
static volatile sig_atomic_t timer_expired = 0;
static sigjmp_buf scheduler_ctx;

void timer_handler(int sig) {
    (void)sig;
    timer_expired = 1;

    if (current >= 0 && procs[current].state == RUNNING) {
        procs[current].state = READY;
        switches++;
        siglongjmp(scheduler_ctx, 1);
    }
}

void setup_timer() {
    struct sigaction sa = {0};
    sa.sa_handler = timer_handler;
    sigaction(SIGALRM, &sa, NULL);

    struct itimerval timer = {
        .it_value = {.tv_sec = 0, .tv_usec = TIME_QUANTUM_MS * 1000},
        .it_interval = {.tv_sec = 0, .tv_usec = TIME_QUANTUM_MS * 1000}
    };
    setitimer(ITIMER_REAL, &timer, NULL);
}

void init_process() {
    struct { char* name; int work; } init[] = {
        {"Calculator", 3},
        {"TextEditor", 5},
        {"Compiler", 7},
        {"Browser", 4}
    };

    for (int i = 0; i < MAX_PROCESSES; i++) {
        procs[i].pid = i;
        strcpy(procs[i].name, init[i].name);
        procs[i].state = READY;
        procs[i].work_done = 0;
        procs[i].work_total = init[i].work;
    }
    num_procs = MAX_PROCESSES;
}

void print_status() {
    printf("PID  Name        State       Progress\n");
    printf("---  ----------  ----------  --------\n");
    for (int i = 0; i < num_procs; i++) {
        const char *st = procs[i].state == READY ? "READY" : procs[i].state == RUNNING ? "RUNNING" : "DONE";
        printf("  %d    %-10s  %-10s  %d/%d\n", procs[i].pid, procs[i].name, st, procs[i].work_done, procs[i].work_total);
    }
    printf("\n");
}

void do_work(int pid) {
    volatile long c = 0;
    while (c < 50000000 && !timer_expired) c++;

    if (!timer_expired) {
        procs[pid].work_done++;

        if (procs[pid].work_done >= procs[pid].work_total) {
            procs[pid].state = TERMINATED;
        }
    }
}

int all_done() {
    for (int i = 0; i < num_procs; i++)
        if (procs[i].state != TERMINATED) return 0;
    return 1;
}

void scheduler() {
    int next = 0;

    if (sigsetjmp(scheduler_ctx, 1) != 0) {
        printf("<- timer interrupt, context switch\n\n");
    }

    while (!all_done()) {
        int found = 0;
        for (int i = 0; i < num_procs; i++) {
            int idx = (next + i) % num_procs;
            if (procs[idx].state == READY) {
                next = idx;
                found = 1;
                break;
            }
        }
        if (!found) break;

        timer_expired = 0;
        procs[next].state = RUNNING;
        current = next;

        printf("[%s] running", procs[next].name);
        fflush(stdout);

        while (!timer_expired && procs[next].state == RUNNING) {
            do_work(next);
            if (procs[next].state == RUNNING) {
                printf(".");
                fflush(stdout);
            }
        }

        if (procs[next].state == TERMINATED)
            printf(" done!\n\n");
        
        next = (next + 1) % num_procs;
    }
}

