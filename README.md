# Round Robin Time Sharing System

A Round-Robin CPU scheduling simulator that demonstrates how operating systems manage multiple processes competing for CPU time using preemptive scheduling.

## Overview

This project simulates a time-sharing operating system using the Round-Robin (RR) scheduling algorithm. It demonstrates fundamental OS concepts including process scheduling, signal handling, and timer-based preemption.

## Features

- **Round-Robin Scheduling**: Each process receives a fixed time quantum (2 seconds) for fair CPU allocation
- **Signal-Based Context Switching**: Uses `SIGALRM` signals to trigger scheduler on time quantum expiry
- **Process State Management**: Tracks READY and RUNNING states for each process
- **Performance Metrics**: Calculates and displays waiting time and turnaround time for each process
- **Real-Time Simulation**: Uses system timers (`setitimer`) for authentic scheduling behavior

## How It Works

The simulator manages 4 processes with predefined burst times:
- Process 0: 8 seconds
- Process 1: 6 seconds
- Process 2: 4 seconds
- Process 3: 10 seconds

Each process gets a 2-second time quantum. When the quantum expires, the current process is preempted and moved to the back of the ready queue, and the next ready process gets the CPU.

## Building and Running

### Compilation

```bash
gcc time_sharing.c -o time_sharing
```

### Execution

```bash
./output/time_sharing
```

Or run the pre-compiled binary:

```bash
./output/time_sharing
```

## Output

The program displays:
- Real-time scheduling events (process starts and completions)
- Per-process metrics (burst time, waiting time, turnaround time)
- Average waiting time across all processes
- Average turnaround time across all processes

## Configuration

You can modify these constants in [time_sharing.c](time_sharing.c):
- `NUM_PROCESSES`: Number of processes to simulate (default: 4)
- `TIME_QUANTUM`: Time slice duration in seconds (default: 2)
- `burst_times[]`: Array of process burst times

## Technical Details

- **Language**: C
- **Key System Calls**: `signal()`, `setitimer()`, `pause()`
- **Timer Type**: `ITIMER_REAL` with 2-second intervals
- **Signal Handler**: Custom `SIGALRM` handler executes scheduler logic

## Educational Purpose

This project is designed for Operating Systems coursework to illustrate:
- CPU scheduling algorithms
- Process management
- Signal handling in Unix/Linux
- Context switching mechanisms
- Performance metric calculation
