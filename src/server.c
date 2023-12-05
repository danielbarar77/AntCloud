#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include "common.h"

#define MAX_NR_THREADS 10

struct parameters{
    int cd // connection descriptor
};

typedef struct parameters parameters_t;

void* cmdListener(parameters_t* params) {
    printf("Test thread, cd: %d\n", params->cd);
    char buf[MAX_BUF_SIZE];
    int rc = 0, wc = 0;

    while(1){
        rc = read(params->cd, buf, MAX_BUF_SIZE);

        if (rc == -1){
            perror("Could not read from socket!");
            exit(1);
        }

        wc = write(STDOUT_FILENO, buf, rc);

        if (wc == -1){
            perror("Could not write to socket!");
            exit(1);
        }
    }

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

            params->cd = accept(sd, NULL, NULL);

            if (params->cd == -1) {
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