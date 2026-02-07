Roll No: MT25073
Assignment: PA02 - Analysis of Network I/O Primitives
Course: Graduate Systems (CSE638)

-------------------------------------------------------------------------
1. OVERVIEW
-------------------------------------------------------------------------
This assignment implements and benchmarks three different network data 
transmission mechanisms to analyze the cost of data movement:
1. Two-Copy (Standard I/O): Uses send() with a user-space buffer copy.
2. One-Copy (Scatter-Gather): Uses sendmsg() with struct iovec to avoid stitching.
3. Zero-Copy (Kernel Bypass): Uses sendmsg() with MSG_ZEROCOPY to avoid CPU copying.

The project includes a multithreaded server, a load-generating client, 
an automation script for profiling, and a Python script for visualization.

-------------------------------------------------------------------------
2. FILES INCLUDED
-------------------------------------------------------------------------
Source Code:
- MT25073_Part_A_Common.h      : Shared header for socket headers and constants.
- MT25073_Part_A1_Server.c     : Two-Copy Server implementation.
- MT25073_Part_A1_Client.c     : Load Generator Client.
- MT25073_Part_A2_Server.c     : One-Copy Server (Scatter-Gather).
- MT25073_Part_A2_Client.c     : Client for One-Copy.
- MT25073_Part_A3_Server.c     : Zero-Copy Server (MSG_ZEROCOPY).
- MT25073_Part_A3_Client.c     : Client for Zero-Copy.

Scripts & Data:
- MT25073_Part_C_Runner.sh     : Bash script to automate compilation and perf profiling.
- MT25073_Part_D_Plots.py      : Python script (matplotlib) to generate performance plots.
- MT25073_measurements.csv     : Raw experimental data (Throughput, Latency, Cache Misses).

Documentation:
- MT25073_Report.pdf           : Final report with analysis, plots, and screenshots.
- Makefile                     : Build script for all binaries.
- README.txt                   : This file.

-------------------------------------------------------------------------
3. HOW TO COMPILE
-------------------------------------------------------------------------
To compile all server and client binaries, run:
    $ make all

To clean up binaries:
    $ make clean

-------------------------------------------------------------------------
4. HOW TO RUN EXPERIMENTS
-------------------------------------------------------------------------
Option A: Automated (Recommended)
This runs all permutations (A1, A2, A3) across 4 message sizes and 4 thread counts.
Note: Requires 'sudo' access for 'perf' to read hardware counters.
    
    $ chmod +x MT25073_Part_C_Runner.sh
    $ sudo ./MT25073_Part_C_Runner.sh

Option B: Manual Execution
1. Start the Server (e.g., A3):
    $ ./server_a3
2. In a new terminal, run the Client:
    $ ./client_a3 <MsgSize> <Threads> <Duration>
    Example: ./client_a3 4096 4 5

-------------------------------------------------------------------------
5. HOW TO GENERATE PLOTS
-------------------------------------------------------------------------
The Python script contains hardcoded data arrays derived from the 
measurements.csv file, strictly adhering to the assignment guidelines.

To generate the plots (Throughput, Latency, Cache Misses, Efficiency):
    $ python3 MT25073_Part_D_Plots.py

This will display the plots on screen.

-------------------------------------------------------------------------
6. SYSTEM CONFIGURATION
-------------------------------------------------------------------------
- OS: Ubuntu Linux (Virtual/Native)
- Compiler: GCC with -lpthread
- Tools Used: perf (for cache/cycle analysis)

-------------------------------------------------------------------------
7. AI USAGE DECLARATION
-------------------------------------------------------------------------

I utilized Generative AI to assist with specific components of this assignment. 
My usage was strictly structured as follows:

* Code Generation: 
  - I used prompts such as "Write a C multithreaded server using the sendmsg function" 
    to generate the boilerplate for socket connections and thread management.
  - I used AI to generate the 'Makefile' and the 'MT25073_Part_D_Plots.py' script 
    structure for matplotlib subplots.

* Debugging: 
  - I used the tool to troubleshoot the 'SO_REUSEPORT' compilation error 
    (which is undefined on some Linux versions) and replaced it with 'SO_REUSEADDR'.

* Concept Explanation: 
  - I used the tool to understand the difference between User-Space Copy 
    and Kernel-Space Copy, specifically how 'struct iovec' eliminates the need 
    for a user-side buffer.

All core logic regarding the specific assignment requirements (Two-Copy vs Zero-Copy flow) 
was reviewed and implemented by me.
