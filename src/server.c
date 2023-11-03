#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

#include "common.h"

typedef int Worker;
Worker workers[10];
int workerCnt = 0;

void add_worker(int cd){
    workers[workerCnt++] = cd;
}

void remove_worker(int cd){
    // to do
}

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

int main() {



    int sd, cd;
    char buf[MAX_BUF_SIZE] = "", fname[10];
    struct sockaddr_in ser;

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

    for (;;) {
        cd = accept(sd, NULL, NULL);
        int pid = fork();

        if (pid == 0) {

            for (;;){
                listen_for_messages(cd, buf);
            }

            close(cd);

        }

    }

    close(sd);

    return 0;

}