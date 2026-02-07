/*
 * Roll No: MT25073
 * File: MT25073_Part_A1_Client.c
 * Part: A1 (Two-Copy Implementation)
 * Description: Multithreaded Client (Load Generator).
 * Spawns multiple threads to connect to the server and measure throughput.
 */

#include "MT25073_Part_A_Common.h"
#include <pthread.h>

// Global variable to aggregate total bytes received across all threads
// We need a mutex to protect this shared counter.
long long global_total_bytes = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

// Structure to pass arguments to each client thread
typedef struct {
    size_t msg_size;
    int duration;
    int thread_id;
} client_thread_args_t;

// --- The Worker Thread (One Simulated User) ---
void *client_thread_func(void *arg) {
    client_thread_args_t *args = (client_thread_args_t *)arg;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *buffer = (char *)malloc(args->msg_size); // Buffer to receive data
    
    // 1. Create Socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        free(buffer);
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock);
        free(buffer);
        return NULL;
    }

    // 2. Connect to Server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        free(buffer);
        return NULL;
    }

    // 3. The Handshake (Send Parameters to Server)
    // We send: [Message Size] [Duration]
    send(sock, &args->msg_size, sizeof(args->msg_size), 0);
    send(sock, &args->duration, sizeof(args->duration), 0);

    // 4. The Sink Loop (Receive Data)
    long long bytes_received = 0;
    ssize_t valread;
    
    // Keep reading until the server closes the connection (returns 0)
    while ((valread = recv(sock, buffer, args->msg_size, 0)) > 0) {
        bytes_received += valread;
    }

    // 5. Update Global Stats
    pthread_mutex_lock(&stats_mutex);
    global_total_bytes += bytes_received;
    pthread_mutex_unlock(&stats_mutex);

    close(sock);
    free(buffer);
    return NULL;
}

int main(int argc, char const *argv[]) {
    // Usage: ./client <Message Size> <Thread Count> <Duration>
    if (argc != 4) {
        printf("Usage: %s <Message Size (bytes)> <Thread Count> <Duration (s)>\n", argv[0]);
        return -1;
    }

    size_t msg_size = atoi(argv[1]);
    int thread_count = atoi(argv[2]);
    int duration = atoi(argv[3]);

    printf("Starting Client: %d Threads, %zu Bytes/Msg, %d Seconds\n", 
           thread_count, msg_size, duration);

    pthread_t threads[thread_count];
    client_thread_args_t args[thread_count];

    // Start Timer
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // 1. Spawn Threads
    for (int i = 0; i < thread_count; i++) {
        args[i].msg_size = msg_size;
        args[i].duration = duration;
        args[i].thread_id = i;
        
        if (pthread_create(&threads[i], NULL, client_thread_func, (void *)&args[i]) != 0) {
            perror("Failed to create thread");
        }
    }

    // 2. Wait for All Threads to Finish
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    // Stop Timer
    gettimeofday(&end, NULL);

    // 3. Calculate Metrics
    double time_taken = (end.tv_sec - start.tv_sec) + 
                        (end.tv_usec - start.tv_usec) / 1e6;
    
    double throughput_bps = (global_total_bytes * 8) / time_taken; // bits per second
    double throughput_gbps = throughput_bps / 1e9; // Gbps

    printf("------------------------------------------------\n");
    printf("Test Complete.\n");
    printf("Total Bytes Received: %lld bytes\n", global_total_bytes);
    printf("Time Taken:           %.4f seconds\n", time_taken);
    printf("Throughput:           %.4f Gbps\n", throughput_gbps);
    printf("------------------------------------------------\n");

    return 0;
}