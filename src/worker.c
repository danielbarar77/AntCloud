#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"

int readFromClient(int cd)
{
    char buff[MAX_BUF_SIZE];
    int rc;
    int val = 0;

    while ((rc = read(cd, buff, MAX_BUF_SIZE)) > 0)
    {
        printf("Read %d bytes: %s\n", rc, buff);
        for (int i = 0; i < rc; i++)
            val += buff[i] - 48;
    }

    if (rc == 0)
    {
        printf("End of data transmission from the client\n");
    }
    else
    {
        perror("Reading");
    }
    return val;
}

void sentToClient(int cd, int val)
{
    int status = write(cd, &val, sizeof(int));
    printf("status = %d\n", status);
}

int main()
{
    int sd, cd;
    char buf[MAX_BUF_SIZE] = "";
    struct sockaddr_in ser;

    // Create a socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
        printf("SOCKET NOT CREATED\n");

    bzero(&ser, sizeof(struct sockaddr_in));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(1101);
    inet_aton("localhost", &ser.sin_addr);

    //////////////////////// for testing
    int b = bind(sd, (struct sockaddr *)&ser, sizeof(ser));
    printf("BIND VALUE: %d\n", b);
    listen(sd, 5);
    printf("Connecting to client...\n");
    int vlaueToSent;
    while (1)
    {
        cd = accept(sd, NULL, NULL);
        if (cd == -1)
        {
            printf("Coudn't accept connection!\n");
            return -1;
        }
        else
        {
            printf("Connection successful!\n");
        }
        vlaueToSent = readFromClient(cd);
        sentToClient(cd, vlaueToSent);
    }
    ////////////////////////

    // Connect to the server
    // printf("Connecting to server...\n");
    // connect(sd, (struct sockaddr *)&ser, sizeof(ser));
    // printf("Connection successful!\n");

    // Greet the server
    printf("Greeting server...\n");
    write(sd, WORKER_GREETING, sizeof(WORKER_GREETING));

    close(sd);

    return 0;
}