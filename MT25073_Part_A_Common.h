/*
 * Roll No: MT25073
 * File: MT25073_Part_A_Common.h
 * Description: Common definitions and structures for PA02.
 * Defines the 8-field structure required by the assignment.
 */

 #ifndef MT25073_PART_A_COMMON_H
 #define MT25073_PART_A_COMMON_H

 #include <stdio.h>
#include <stdlib.h> // Standard Library (malloc, free, exit)
#include <string.h> // String manipulation (memset, memcpy)
#include <unistd.h> // UNIX standard (read, write, close)
#include <arpa/inet.h> // Internet definitions (struct sockaddr_in, htons)
#include <time.h>
#include <sys/time.h> // System time (gettimeofday for microsecond precision)

#define PORT 8080
#define SERVER_IP "127.0.0.1"

typedef struct{
char * fields[8]; // array of pointers to  8 strings
size_t sizes[8]; // keep track of the size of each string

}ComplexMessage;


//Memory cleanup helper

void free_complex_message(ComplexMessage *msg){
    if(!msg) return; // dont free a null pointer
    for(int i = 0;i<8;i++){
        if(msg->fields[i]){
            free(msg->fields[i]); // free the actual string data
            msg->fields[i]=NULL; //dangle prevention : memory leak
        }
    }
}

void fill_complex_message(ComplexMessage *msg,size_t total_size){
    size_t chunk_size = total_size/8;
    
    for(int i =0;i<8;i++){
        if(i==7){
            chunk_size = total_size - (7*(total_size/8));
        }
        msg->sizes[i] = chunk_size;
        msg->fields[i]= (char*)malloc(chunk_size);
        memset(msg->fields[i],'A'+i,chunk_size); //Fills memory
    }

}

#endif