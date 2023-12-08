#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "common.h"

int checkIfExists(char *buf)
{
	if (access(buf, F_OK | R_OK))
		return -1;

	return 0;
}

int transferData(char *filename, int sd)
{
	int fd = open(filename, O_RDONLY);

	if (fd == -1)
	{
		perror("Opening the file");
		return -1;
	}

	char *buff = (char *)malloc(MAX_BUF_SIZE * sizeof(char));
	int wc;								   // wc = write count
	int rc = read(fd, buff, MAX_BUF_SIZE); // rc = read count
	if (rc == -1)
	{
		perror("Reading");
		close(fd);
		free(buff);
		return -1;
	}

	while (rc > 0)
	{
		wc = write(sd, buff, rc);
		printf("S-au transferat %d caractere\n", wc);
		if (wc == -1)
		{
			perror("Writing");
			close(fd);
			free(buff);
			return -1;
		}
		if (wc < rc)
		{
			printf("Unable to write all the data through the socket!\n");
			close(fd);
			free(buff);
			return 11;
		}
		rc = read(fd, buff, MAX_BUF_SIZE);
		if (rc == -1)
		{
			perror("Reading");
			close(fd);
			free(buff);
			return -1;
		}
		if (rc == 0)
		{
			break;
		}
	}

	free(buff);
	if (close(fd) == -1)
	{
		perror("Closing");
		return -1;
	}

	if (shutdown(sd, SHUT_WR) == -1)
	{
		perror("Shutdown");
		return -1;
	}

	return 0;
}

int waitForResults(int sd)
{
	int value;
	int rc;
	while (1)
	{
		rc = read(sd, &value, sizeof(int));
		if (rc == -1)
		{
			perror("Reading from socket");
			return -1;
		}
		if (rc > 0)
		{
			return value;
		}
	}

	return -1;
}

int main()
{
	int sd;
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
	// write(sd, CLIENT_GREETING, sizeof(CLIENT_GREETING));
	for (;;)
	{
		printf("ENTER THE EXECUTABLE: \n");
		scanf("%s", buf);
		if (checkIfExists(buf) == 0) // checks the existance of the file
		{
			if (transferData(buf, sd) == 0) // transfers the executable to the server
			{
				printf("SUCCESS TRANSFER OF DATA!\n");
				printf("WAITING FOR THE RESULTS...\n");
				int result = waitForResults(sd);
				if (result == -1)
				{
					printf("ERROR WHILE RECIVING THE RESULT!\n");
				}
				else
				{
					printf("The result is: %d\n", result);
				}
			}
			else
			{
				printf("ERROR!");
			}
		}
		else
		{
			printf("INVALID FILE OR CANNOT READ FROM IT!\n");
		}
	}

	close(sd);

	return 0;
}