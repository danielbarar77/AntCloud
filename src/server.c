#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#include "common.h"

#define MAX_NR_THREADS 10
//#define MAX_NR_WORKERS 10

struct parameters{
    int connectionDescriptor
};

typedef struct parameters parameters_t;

// typedef int Worker;
// Worker workers[MAX_NR_WORKERS];
// int workerCnt = 0;

// void add_worker(int cd){
//     workers[workerCnt++] = cd;
// }

// void remove_worker(int cd){
//     // to do
// }

void listen_for_messages(int cd, char buf[MAX_BUF_SIZE]){
    printf("accept value %d\n", cd);
    read(cd, buf, MAX_BUF_SIZE);
    
    // define index values in common.h
    if (buf[0] == MSG_TYPE_COMMAND){
        if (buf[1] == CMD_ADD){
            if (buf[2] == HOST_TYPE_WORKER){
                add_worker(cd);
                // find a way to do this without using a string
                write(cd, CONNECTION_ACCEPTED_STR, strlen(CONNECTION_ACCEPTED_STR));
            }
        }
    }

    // int fd = open(buf, O_RDONLY);
    // read(fd, buf, MAX_BUF_SIZE);
    // write(cd, buf, strlen(buf));
    // printf("MESSAGE FROM CLIENT: %s\n", buf);
}

void* cmdListener(void* params) {
    parameters_t p = *((parameters_t*)params);
    printf("Test thread, cd: %d\n", p.connectionDescriptor);


    free(params);

    return NULL;
}


int main() {
    int sd;
    char buf[MAX_BUF_SIZE] = "", fname[10];
    struct sockaddr_in ser;

    pthread_t threads[MAX_NR_THREADS];
    int nrOfThreads = 0;

    // Create a socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
        printf("SOCKET NOT CREATED\n");

    bzero(&ser, sizeof(struct sockaddr_in));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(1101);
    inet_aton("localhost", &ser.sin_addr);

    int b = bind(sd, (struct sockaddr *)&ser, sizeof(ser));
    printf("BIND VALUE: %d\n", b);

    listen(sd, 5);

    while (1) {
        if (nrOfThreads != MAX_NR_THREADS) {
            parameters_t *params = malloc(sizeof(parameters_t));

            if (params == NULL){
                perror("Failed to allocate thread parameters!\n");
                exit(1);
            }

            memset(params, 0, sizeof(*params));

            params->connectionDescriptor = accept(sd, NULL, NULL);

            if (params->connectionDescriptor == -1) {
                perror("Could not accept connection!");
                free(params);
            } else {
                int errorCode = pthread_create(&(threads[nrOfThreads]), NULL, cmdListener, (void *) params);

                if (errorCode != 0){
                    perror("Error creating thread!\n");
                    exit(1);
                }

                nrOfThreads++;
            }
       
        }


    }

    close(sd);

    return 0;

}