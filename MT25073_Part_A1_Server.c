/*
 * Roll No: MT25073
 * File: MT25073_Part_A1_Server.c
 * Part: A1 (Two-Copy Implementation)
 * Description: Multithreaded server that sends data using standard send().
 * Performs explicit User-Space copy (stitching 8 strings).
 */

#include "MT25073_Part_A_Common.h"
#include <pthread.h>
#include <signal.h> // Required for signal handling (allows us to stop the loop cleanly if needed).

volatile sig_atomic_t server_running = 1;// "volatile" tells the compiler: "Don't cache this variable, check it every time."
                                         //A global flag, = 0 to close the server
                                         

                                         
typedef struct {
    int client_socket;
} thread_args_t;   // To pass socket Id  in thread, we wrap it in a struct



// The "Worker" function.
// Input: A void pointer (which we know is actually our thread_args_t struct).
// Output: NULL (we don't return anything to the main thread).
void *handle_client(void *arg) {
    
    // 1. UNPACKING ARGS
    // We cast the void pointer back to our struct type.
    thread_args_t *args = (thread_args_t *)arg;
    int sock = args->client_socket;
    free(args); // We don't need the container anymore, so free it to avoid leaks.

    // 2. PROTOCOL HANDSHAKE
    // The client needs to tell us: "How big should the message be?" and "How long should I run?"
    size_t msg_size;
    int duration;
    
    // We read exactly sizeof(size_t) bytes to get the message size.
    if (recv(sock, &msg_size, sizeof(msg_size), 0) <= 0) {
        close(sock); // If read fails (client disconnected), clean up and die.
        return NULL;
    }
    // We read exactly sizeof(int) bytes to get the duration.
    if (recv(sock, &duration, sizeof(duration), 0) <= 0) {
        close(sock);
        return NULL;
    }

    // Log what we are doing so you can see it in the terminal.
    printf("[Thread %ld] Client requested: Size=%zu, Duration=%d s\n", 
           pthread_self(), msg_size, duration);

    // 3. PREPARE THE DATA (Source of the "8 Strings")
    // We create the structure on the stack...
    ComplexMessage msg;
    // ...and fill it with 8 heap-allocated strings using our helper from Common.h.
    fill_complex_message(&msg, msg_size);

    // 4. PREPARE THE "STITCHING" BUFFER (Crucial for Two-Copy)
    // To send the 8 strings as one block using standard send(), we need a single continuous buffer.
    // We malloc this buffer *once* here.
    char *linear_buffer = (char *)malloc(msg_size);
    if (!linear_buffer) {
        perror("Buffer malloc failed");
        close(sock);
        return NULL;
    }

    // 5. THE MAIN TRANSFER LOOP
    time_t start_time = time(NULL);
    size_t total_bytes_sent = 0;

    // Run until the requested duration (e.g., 10 seconds) expires.
    while ((time(NULL) - start_time) < duration) {
        
        // --- COPY #1: USER-SPACE COPY (The "Stitching") ---
        // This is the inefficiency we are studying.
        // We have to loop through our 8 scattered strings and copy them 
        // one-by-one into the linear_buffer.
        size_t offset = 0;
        for (int i = 0; i < 8; i++) {
            // memcpy(destination, source, size)
            memcpy(linear_buffer + offset, msg.fields[i], msg.sizes[i]);
            
            // Move the offset forward so the next string is placed right after this one.
            offset += msg.sizes[i];
        }

        // --- COPY #2: KERNEL-SPACE COPY ---
        // We call send(). The OS now copies data from `linear_buffer` (User Land)
        // into the Socket Buffer (Kernel Land).
        // This is why it's called "Two-Copy": 1. memcpy above, 2. send() here.
        ssize_t sent = send(sock, linear_buffer, msg_size, 0);
        
        if (sent <= 0) break; // If send fails (network error), stop.
        total_bytes_sent += sent;
    }

    // 6. CLEANUP
    printf("[Thread %ld] Finished. Sent %zu bytes.\n", pthread_self(), total_bytes_sent);
    
    free(linear_buffer);         // Free the stitching buffer
    free_complex_message(&msg);  // Free the 8 original strings
    close(sock);                 // Hang up the phone
    return NULL;
}



int main() {
    int server_fd, new_socket;
    struct sockaddr_in address; // Struct to hold IP/Port info
    int opt = 1;
    int addrlen = sizeof(address);

    // 1. CREATE SOCKET
    // AF_INET = IPv4, SOCK_STREAM = TCP.
    // This creates the endpoint.
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE); //
    }

    // 2. SET SOCKET OPTIONS
    // SO_REUSEADDR allows us to restart the server immediately after killing it 
    // without waiting for the "TIME_WAIT" timeout. Very useful for debugging.
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt"); 
        exit(EXIT_FAILURE);
    }

    // 3. DEFINE ADDRESS
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces (localhost, wifi, etc.)
    address.sin_port = htons(PORT);       // htons converts "8080" to Network Byte Order (Big Endian)

    // 4. BIND
    // Assign the address (IP:Port) to the socket.
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 5. LISTEN
    // Tell the OS we are ready to accept connections.
    // "10" is the backlog queue size (how many callers can hold before getting busy signal).
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // 6. ACCEPT LOOP (The Infinite Loop)
    while (server_running) {
        // accept() BLOCKS. The program stops here and sleeps until a client connects.
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        // A client connected! Prepare the arguments for the worker.
        thread_args_t *args = malloc(sizeof(thread_args_t));
        args->client_socket = new_socket;

        // 7. SPAWN THREAD
        // pthread_create(id_pointer, attributes, function_to_run, arguments)
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)args) != 0) {
            perror("pthread_create");
            free(args);
            close(new_socket);
        } else {
            // pthread_detach tells the OS: "When this thread finishes, clean up its memory automatically."
            // Otherwise, we would have to call pthread_join() for every thread, which is hard in a server.
            pthread_detach(thread_id);
        }
    }

    return 0;
}