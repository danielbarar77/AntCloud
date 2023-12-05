#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"

int main() {
    int sd, cd;
    char buf[1000] = "";
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
    connect(sd, (struct sockaddr *)&ser, sizeof(ser));
    write(sd, CLIENT_GREETING, sizeof(CLIENT_GREETING));

    for (;;) {
        printf("ENTER THE MESSAGE: ");
        scanf("%s", buf);
        write(sd, buf, strlen(buf));
        read(sd, buf, 1000);
        printf("RECEIVED FROM SERVER: %s\n", buf);
    }

    close(sd);

    return 0;

}