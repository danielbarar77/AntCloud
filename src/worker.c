#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

#include "common.h"

void greet_sv(int sd){
    int buf[] = {
        MSG_TYPE_COMMAND,
        CMD_ADD,
        HOST_TYPE_WORKER
    };

    write(sd, buf, 3);
}

int main() {
    int sd, cd;
    char buf[MAX_BUF_SIZE] = "", buf1[MAX_BUF_SIZE] = "";
    struct sockaddr_in ser;

    // Create a socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
        printf("SOCKET NOT CREATED\n");

    bzero(&ser, sizeof(struct sockaddr_in));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(1101);
    inet_aton("localhost", &ser.sin_addr);

    // Connect to the server
    printf("Connecting to server...\n");
    connect(sd, (struct sockaddr *)&ser, sizeof(ser));
    printf("Connection successful!\n");

    // Greet the server
    printf("Greeting server...\n");
    greet_sv(sd);
    read(sd, buf, MAX_BUF_SIZE);
    if(strcmp(buf, CONNECTION_ACCEPTED_STR)){
        printf("ERR: Server did not accept greeting!");
        exit(-1);
    }
    printf("Server accepted greeting!\n");

    // for (;;) {

    // }

    close(sd);

    return 0;

}