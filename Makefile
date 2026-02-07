# Roll No: MT25073
# Makefile for PA02 - Network I/O Primitives

CC = gcc
CFLAGS = -lpthread

# Default target: Compile everything
all: server_a1 client_a1 server_a2 client_a2 server_a3 client_a3

# Part A1: Two-Copy
server_a1: MT25073_Part_A1_Server.c
	$(CC) MT25073_Part_A1_Server.c -o server_a1 $(CFLAGS)

client_a1: MT25073_Part_A1_Client.c
	$(CC) MT25073_Part_A1_Client.c -o client_a1 $(CFLAGS)

# Part A2: One-Copy (Scatter-Gather)
server_a2: MT25073_Part_A2_Server.c
	$(CC) MT25073_Part_A2_Server.c -o server_a2 $(CFLAGS)

client_a2: MT25073_Part_A2_Client.c
	$(CC) MT25073_Part_A2_Client.c -o client_a2 $(CFLAGS)

# Part A3: Zero-Copy
server_a3: MT25073_Part_A3_Server.c
	$(CC) MT25073_Part_A3_Server.c -o server_a3 $(CFLAGS)

client_a3: MT25073_Part_A3_Client.c
	$(CC) MT25073_Part_A3_Client.c -o client_a3 $(CFLAGS)

# Clean up binaries
clean:
	rm -f server_a1 client_a1 server_a2 client_a2 server_a3 client_a3
