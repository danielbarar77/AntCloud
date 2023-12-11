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
	// checks the existance of the file and if is readable
	if (access(buf, F_OK | R_OK))
		return -1;

	return 0;
}

int transferData(char *filename, int sd)
{
	// opens the source file that needs to be sent to the sever
	int fd = open(filename, O_RDONLY);

	if (fd == -1)
	{
		perror("Opening the file");
		return -1;
	}

	char buff[MAX_BUF_SIZE];
	int wc; // wc = write count
	int rc; // rc = read count

	while ((rc = read(fd, buff, MAX_BUF_SIZE)) > 0) // reads from the source file
	{
		if (rc == -1)
		{
			perror("Reading");
			close(fd);
			free(buff);
			return -1;
		}

		// wc = write(sd, buff, rc); // wites to the server
		wc = send(sd, buff, rc, MSG_DONTWAIT); // wites to the server

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
	}

	// close the file descriptor of the source file
	if (close(fd) == -1)
	{
		perror("Closing");
		return -1;
	}

	// writes a message the signals the end of the transmission
	if (write(sd, "END_TRANSMISSION", strlen("END_TRANSMISSION")) == -1)
	{
		perror("Shutdown");
	}

	return 0;
}

void reciveData(int sd)
{
	char buff[MAX_BUF_SIZE];
	int rc, wc;
	char *end;

	printf("OUTPUT OF THE EXECUTABLE:\n");
	while ((rc = read(sd, buff, MAX_BUF_SIZE)) > 0) // reads from the server
	{
		if (rc == -1)
		{
			perror("Reading from the server");
			exit(-1);
		}
		// checks if the transmission ended
		end = strstr(buff, "END_TRANSMISSION");
		if (end != NULL)
			memset(end, 0, sizeof("END_TRANSMISSION"));
		// writes to the terminal the output
		wc = write(STDOUT_FILENO, buff, rc);
		printf("\n");
		if (wc < rc)
		{
			printf("Not all of it was written!\n");
			exit(-1);
		}
		if (wc == -1)
		{
			perror("Writing to terminal");
			exit(-1);
		}
		if (end != NULL)
		{
			break;
		}
	}
	if (rc == 0)
	{
		printf("End of data transmission from the worker\n");
	}
	else
	{
		perror("Reading");
	}
}

int main()
{
	int sd;
	char buf[MAX_BUF_SIZE];
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
		scanf("%s", buf);			 // reads the filepath
		if (checkIfExists(buf) == 0) // checks the existance of the file
		{
			if (transferData(buf, sd) == 0) // transfers the executable to the server
			{
				printf("S-a tranferat catre worker!\n");
				printf("SUCCESS TRANSFER OF DATA TO THE SERVER!\n");
				printf("WAITING FOR THE RESULTS...\n");
				reciveData(sd); // get the data from the server
				printf("\n");
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