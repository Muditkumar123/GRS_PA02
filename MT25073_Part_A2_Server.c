/*
 * Roll No: MT25073
 * File: MT25073_Part_A2_Server.c
 * Part: A2 (One-Copy Implementation)
 * Description: Uses sendmsg() with iovec (Scatter-Gather) to eliminate
 * the user-space copy (stitching).
 */

#include "MT25073_Part_A_Common.h"
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h> // Required for struct msghdr

volatile sig_atomic_t server_running = 1;

typedef struct {
    int client_socket;
} thread_args_t;

void *handle_client(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    int sock = args->client_socket;
    free(args);

    size_t msg_size;
    int duration;

    // 1. Handshake
    if (recv(sock, &msg_size, sizeof(msg_size), 0) <= 0) {
        close(sock);
        return NULL;
    }
    if (recv(sock, &duration, sizeof(duration), 0) <= 0) {
        close(sock);
        return NULL;
    }

    printf("[Thread %ld] A2 One-Copy: Size=%zu, Duration=%d s\n", 
           pthread_self(), msg_size, duration);

    // 2. Prepare Data
    ComplexMessage msg;
    fill_complex_message(&msg, msg_size);

    // 3. Prepare Scatter-Gather Vector (The "One-Copy" Magic)
    // instead of a malloc'd buffer, we create an array of pointers.
    struct iovec iov[8];
    struct msghdr msg_header;

    // Point the vector slots to our existing strings
    for(int i=0; i<8; i++) {
        iov[i].iov_base = msg.fields[i]; // Pointer to string
        iov[i].iov_len = msg.sizes[i];   // Length of string
    }

    // Configure the Message Header
    memset(&msg_header, 0, sizeof(msg_header));
    msg_header.msg_iov = iov;    // The list of 8 strings
    msg_header.msg_iovlen = 8;   // Count of strings

    // 4. Transfer Loop
    time_t start_time = time(NULL);
    size_t total_bytes_sent = 0;

    while ((time(NULL) - start_time) < duration) {
        // --- NO MEMCPY LOOP HERE! ---
        // We moved straight to the system call.
        
        // sendmsg reads the 8 strings directly and sends them.
        ssize_t sent = sendmsg(sock, &msg_header, 0);
        
        if (sent <= 0) break;
        total_bytes_sent += sent;
    }

    printf("[Thread %ld] Finished. Sent %zu bytes.\n", pthread_self(), total_bytes_sent);
    free_complex_message(&msg);
    close(sock);
    return NULL;
}

// --- Main Function (Identical to A1) ---
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Removed SO_REUSEPORT as discussed
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server A2 (One-Copy) listening on port %d...\n", PORT);

    while (server_running) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        thread_args_t *args = malloc(sizeof(thread_args_t));
        args->client_socket = new_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)args) != 0) {
            perror("pthread_create");
            free(args);
            close(new_socket);
        } else {
            pthread_detach(thread_id);
        }
    }
    return 0;
}