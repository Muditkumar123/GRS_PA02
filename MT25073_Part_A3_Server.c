/*
 * Roll No: MT25073
 * File: MT25073_Part_A3_Server.c
 * Part: A3 (Zero-Copy Implementation)
 * Description: Uses sendmsg() with MSG_ZEROCOPY.
 * Handles SO_ZEROCOPY and MSG_ERRQUEUE for completion notifications.
 */

#include "MT25073_Part_A_Common.h"
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <linux/errqueue.h> // Required for SO_EE_ORIGIN_ZEROCOPY

#ifndef SO_ZEROCOPY
#define SO_ZEROCOPY 60
#endif

#ifndef MSG_ZEROCOPY
#define MSG_ZEROCOPY 0x4000000
#endif

volatile sig_atomic_t server_running = 1;

typedef struct {
    int client_socket;
} thread_args_t;

// --- Helper: Read "Done" Notifications from Kernel ---
// We must read the error queue, or it will fill up and block sendmsg.
void read_zerocopy_notification(int sock) {
    struct msghdr msg = {0};
    char control[100];
    struct cmsghdr *cmsg;
    struct sock_extended_err *serr;
    
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);

    // Try to read from the Error Queue (MSG_ERRQUEUE)
    // MSG_DONTWAIT: Don't block if there is no notification yet.
    if (recvmsg(sock, &msg, MSG_ERRQUEUE | MSG_DONTWAIT) < 0) {
        return; // Nothing to read, or error
    }

    // Parse the message to confirm it is a Zero-Copy notification
    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
        if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVERR) {
            serr = (void *)CMSG_DATA(cmsg);
            if (serr->ee_errno == 0 && serr->ee_origin == SO_EE_ORIGIN_ZEROCOPY) {
                // Successfully acknowledged! 
                // In a complex app, we would decrement a reference count here.
            }
        }
    }
}

void *handle_client(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    int sock = args->client_socket;
    free(args);

    // 1. ENABLE ZERO-COPY ON SOCKET
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_ZEROCOPY, &opt, sizeof(opt))) {
        perror("Setsockopt SO_ZEROCOPY failed (Kernel might not support it)");
        // Fallback or exit? For assignment, we report error.
    }

    size_t msg_size;
    int duration;

    // Handshake
    if (recv(sock, &msg_size, sizeof(msg_size), 0) <= 0) { close(sock); return NULL; }
    if (recv(sock, &duration, sizeof(duration), 0) <= 0) { close(sock); return NULL; }

    printf("[Thread %ld] A3 Zero-Copy: Size=%zu, Duration=%d s\n", 
           pthread_self(), msg_size, duration);

    // Prepare Data
    ComplexMessage msg;
    fill_complex_message(&msg, msg_size);

    // Prepare I/O Vector (Same as A2)
    struct iovec iov[8];
    struct msghdr msg_header;

    for(int i=0; i<8; i++) {
        iov[i].iov_base = msg.fields[i];
        iov[i].iov_len = msg.sizes[i];
    }

    memset(&msg_header, 0, sizeof(msg_header));
    msg_header.msg_iov = iov;
    msg_header.msg_iovlen = 8;

    time_t start_time = time(NULL);
    size_t total_bytes_sent = 0;
    
    // Counters to balance sends and acks
    unsigned long packets_sent = 0;

    while ((time(NULL) - start_time) < duration) {
        
        // --- SEND WITH MSG_ZEROCOPY ---
        ssize_t sent = sendmsg(sock, &msg_header, MSG_ZEROCOPY);
        
        if (sent <= 0) {
            // If errno is ENOBUFS, it means we are sending too fast 
            // and the Error Queue is full. We must drain it.
            read_zerocopy_notification(sock);
            continue; 
        }
        
        total_bytes_sent += sent;
        packets_sent++;

        // Periodically check for notifications (e.g., every send)
        // to keep the queue from overflowing.
        read_zerocopy_notification(sock);
    }

    // Drain remaining notifications before closing
    // (Optional optimization, but good practice)
    read_zerocopy_notification(sock);

    printf("[Thread %ld] Finished. Sent %zu bytes.\n", pthread_self(), total_bytes_sent);
    free_complex_message(&msg);
    close(sock);
    return NULL;
}

// --- Main Function (Identical Setup) ---
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed"); exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt"); exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed"); exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    printf("Server A3 (Zero-Copy) listening on port %d...\n", PORT);

    while (server_running) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) continue;

        thread_args_t *args = malloc(sizeof(thread_args_t));
        args->client_socket = new_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)args) != 0) {
            free(args); close(new_socket);
        } else {
            pthread_detach(thread_id);
        }
    }
    return 0;
}