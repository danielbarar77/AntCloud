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
#include "base64.h"

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
	char *temp = NULL;
	int wc; // wc = write count
	int rc; // rc = read count

	memset(buff, 0, MAX_BUF_SIZE);
	while ((rc = read(fd, buff, MAX_BUF_SIZE)) > 0) // reads from the source file
	{
		if (rc == -1)
		{
			perror("Reading");
			close(fd);
			free(buff);
			return -1;
		}

		// strcpy(temp, base64_encode(buff));
		temp = base64_encode(buff);
		wc = write(sd, temp, strlen(temp)); // wites to the server
		free(temp);

		if (wc == -1)
		{
			perror("Writing");
			close(fd);
			return -1;
		}
		if (wc < rc)
		{
			printf("Unable to write all the data through the socket!\n");
			close(fd);
			return 11;
		}
		memset(buff, 0, MAX_BUF_SIZE);
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
	char *temp = NULL;
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

		temp = base64_decode(buff);
		// writes to the terminal the output
		wc = write(STDOUT_FILENO, temp, strlen(temp));
		free(temp);
		printf("\n");

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
	printf("Connecting to server...\n");
	connect(sd, (struct sockaddr *)&ser, sizeof(ser));
	printf("Connected to server!\n");
	printf("Greeting server...\n");
	write(sd, CLIENT_GREETING, sizeof(CLIENT_GREETING));
	printf("Server greeted!\n");

	// printf("Waiting 5 seconds...\n");
	// sleep(5);

	printf("Writing test string to server...\n");
	// Testing message writing to worker
	const char testStr[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\n";
	int wc = write(sd, testStr, sizeof(testStr));
	
	if (wc < 0){
		perror("write");
		exit(EXIT_FAILURE);
	}

	printf("Test str written to the server!\n");

	while(1){
		sleep(1);
	}

	close(sd);

	return 0;
}