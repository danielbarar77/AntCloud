#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>



int main() {

    int sd, cd;

    char buf[1000] = "", fname[10];

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

            printf("accept value %d\n", cd);

            read(cd, buf, 1000);

            int fd = open(buf, O_RDONLY);

            read(fd, buf, 1000);

            write(cd, buf, strlen(buf));

            printf("MESSAGE FROM CLIENT: %s\n", buf);

            close(cd);

        }

    }



    close(sd);

    return 0;

}