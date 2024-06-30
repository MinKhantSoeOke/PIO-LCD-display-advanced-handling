# Process Scheduling and Priorities in QNX Neutrino RTOS

**by**: Min Khant Soe Oke, Kaung Sithu

## Overview

This project explores process scheduling and priority mechanisms in the QNX Neutrino RTOS. By creating multiple child processes with varying priorities and scheduling policies, the project demonstrates how the QNX scheduler manages CPU allocation among competing processes. Understanding these concepts is essential for developing real-time applications that require precise timing and prioritization to ensure timely and efficient task execution.

## Dependencies

**To run this project, you need to have the following dependencies installed:**

* QNX Neutrino RTOS
* Basic understanding of Unix-like operating systems
* Familiarity with C programming and system calls related to process and scheduling management

## Features

**Process Priorities**
- Processes have priorities ranging from 0 to 255, with 255 being the highest.
- Higher priority processes are given CPU time before lower priority ones.

**Preemptive Scheduling**
- The scheduler always runs the highest priority ready process.
- If a higher priority process becomes ready, it preempts the currently running lower priority process.

**Scheduling Algorithms**
- **FIFO (First In, First Out)**: Processes run to completion unless preempted by a higher priority process.
- **Round Robin**: Each process runs for a time slice before the scheduler switches to the next ready process.
- **Sporadic**: Processes execute until preempted by a higher priority process or run out of their allocated time.

## How to Use

1. **Start the Main Program**:
   - Open a terminal and run `./main`.

2. **Child Process Creation**:
   - The main process creates 10 child processes with different priorities and scheduling policies.

3. **Priority and Policy Assignment**:
   - Each child process is assigned a unique priority and scheduling policy (FIFO, Round Robin, or Sporadic).

4. **Child Execution**:
   - Child processes execute a computation task and print their completion status.

5. **Parent Process Coordination**:
   - The parent process waits for all child processes to complete before finishing.

## Authors

- Min Khant Soe Oke
- Kaung Sithu
