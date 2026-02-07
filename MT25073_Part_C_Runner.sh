#!/bin/bash
# Roll No: MT25073
# File: MT25073_Part_C_Runner.sh
# Description: Automates compilation and profiling for PA02.

# 1. SETUP PARAMETERS
# We need 4 distinct message sizes (bytes) and 4 thread counts.
SIZES=(1024 32768 131072 1048576) # 1KB, 32KB, 128KB, 1MB
THREADS=(1 2 4 8)
DURATION=5
OUTPUT_FILE="MT25073_measurements.csv"

# 2. COMPILE EVERYTHING
echo "--- Compiling Programs ---"
gcc MT25073_Part_A1_Server.c -o server_a1 -lpthread
gcc MT25073_Part_A1_Client.c -o client_a1 -lpthread
gcc MT25073_Part_A2_Server.c -o server_a2 -lpthread
gcc MT25073_Part_A2_Client.c -o client_a2 -lpthread
gcc MT25073_Part_A3_Server.c -o server_a3 -lpthread
gcc MT25073_Part_A3_Client.c -o client_a3 -lpthread

# Initialize CSV Header
# Format: Type,MsgSize,Threads,Throughput(Gbps),Latency(us),Cycles,L1_Misses,LLC_Misses,Context_Switches
echo "Type,MsgSize,Threads,Throughput,Latency,Cycles,L1_Misses,LLC_Misses,CS" > $OUTPUT_FILE

# Function to run one experiment
run_test() {
    TYPE=$1      # A1, A2, or A3
    SERVER_BIN=$2
    CLIENT_BIN=$3
    SIZE=$4
    THREAD=$5

    echo "Running $TYPE: Size=$SIZE, Threads=$THREAD..."

    # Start Server with perf in background
    # We measure: cycles, L1-dcache-load-misses, LLC-load-misses, context-switches
    # 2>&1 redirects stderr (perf output) to a temp file
    sudo perf stat -e cycles,L1-dcache-load-misses,LLC-load-misses,cs \
        -o perf_output.txt ./$SERVER_BIN > server_log.txt 2>&1 &
    
    SERVER_PID=$!
    
    # Give server a moment to start
    sleep 1

    # Run Client and capture output
    CLIENT_OUTPUT=$(./$CLIENT_BIN $SIZE $THREAD $DURATION)

    # Extract Throughput from Client Output
    # We look for the line "Throughput: X Gbps"
    THROUGHPUT=$(echo "$CLIENT_OUTPUT" | grep "Throughput:" | awk '{print $2}')
    
    # Calculate Latency (Approximate: Time / Total Messages)
    # Total Bytes
    TOTAL_BYTES=$(echo "$CLIENT_OUTPUT" | grep "Total Bytes Received:" | awk '{print $4}')
    # Number of messages = Total Bytes / Msg Size
    if [ "$SIZE" -gt 0 ]; then
        NUM_MSGS=$(echo "$TOTAL_BYTES / $SIZE" | bc)
    else
        NUM_MSGS=1
    fi
    # Time Taken
    TIME_TAKEN=$(echo "$CLIENT_OUTPUT" | grep "Time Taken:" | awk '{print $3}')
    
    # Latency in microseconds = (Time * 1e6) / Msg Count
    # Avoid divide by zero
    if [ "$NUM_MSGS" -gt 0 ]; then
        LATENCY=$(echo "scale=2; ($TIME_TAKEN * 1000000) / $NUM_MSGS" | bc)
    else
        LATENCY="0"
    fi

    # Stop Server (SIGINT allows perf to print stats)
    sudo kill -2 $SERVER_PID
    wait $SERVER_PID 2>/dev/null

    # Extract Perf Stats from perf_output.txt
    # We use basic grep/awk to pull the numbers. 
    # Note: Perf numbers often have commas (1,234), we remove them.
    CYCLES=$(grep "cycles" perf_output.txt | awk '{print $1}' | tr -d ',')
    L1_MISS=$(grep "L1-dcache-load-misses" perf_output.txt | awk '{print $1}' | tr -d ',')
    LLC_MISS=$(grep "LLC-load-misses" perf_output.txt | awk '{print $1}' | tr -d ',')
    CS=$(grep "cs" perf_output.txt | awk '{print $1}' | tr -d ',')

    # Handle empty perf data (if perf failed)
    CYCLES=${CYCLES:-0}
    L1_MISS=${L1_MISS:-0}
    LLC_MISS=${LLC_MISS:-0}
    CS=${CS:-0}

    # Save to CSV
    echo "$TYPE,$SIZE,$THREAD,$THROUGHPUT,$LATENCY,$CYCLES,$L1_MISS,$LLC_MISS,$CS" >> $OUTPUT_FILE
    
    # Cleanup temp files
    rm -f perf_output.txt server_log.txt
}

# 3. RUN LOOPS
# Loop 1: Throughput vs Msg Size (Fixed Threads = 1)
# You can adjust this logic, but usually we iterate everything.
# To save time, let's do a full matrix as required.

# A1 Tests
for S in "${SIZES[@]}"; do
    for T in "${THREADS[@]}"; do
        run_test "TwoCopy" "server_a1" "client_a1" $S $T
    done
done

# A2 Tests
for S in "${SIZES[@]}"; do
    for T in "${THREADS[@]}"; do
        run_test "OneCopy" "server_a2" "client_a2" $S $T
    done
done

# A3 Tests
for S in "${SIZES[@]}"; do
    for T in "${THREADS[@]}"; do
        run_test "ZeroCopy" "server_a3" "client_a3" $S $T
    done
done

echo "------------------------------------------------"
echo "Experiments Complete. Results saved to $OUTPUT_FILE"
echo "------------------------------------------------"