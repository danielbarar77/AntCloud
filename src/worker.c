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

void reciveData(int cd)
{
	char buff[MAX_BUF_SIZE];
	int rc, wc;
	// opens or create the file in which the recived data will be stored
	int fd = open("../temp/source.c", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1)
		perror("Creating source.c file");
	char *end;

	// reads from the socket
	while ((rc = read(cd, buff, MAX_BUF_SIZE)) > 0)
	{
		printf("buff: %s\n", buff);
		if (rc == -1)
		{
			perror("Reading from the server");
			exit(-1);
		}
		// checks if the transmission was ended
		end = strstr(buff, "END_TRANSMISSION");

		// memmove(buff, buff, buff - end);
		if (end != NULL)
			memset(end, 0, sizeof("END_TRANSMISSION"));

		// writes the data in the file
		wc = write(fd, buff, rc);
		if (wc < rc)
		{
			printf("Couldn't write all the data in source.c!\n");
			exit(-1);
		}
		if (wc == -1)
		{
			perror("Writing in source.c");
			exit(-1);
		}
		if (end != NULL)
		{
			break;
		}

		memset(buff, 0, MAX_BUF_SIZE);
	}

	if (rc == 0)
	{
		printf("End of data transmission from the client\n");
	}
	else
	{
		perror("Reading");
	}
	close(fd);
}

void compile()
{
	// creates a child process that compiles the source file
	int pid = fork();
	if (pid == -1)
	{
		perror("Creating child");
		exit(-1);
	}
	if (pid == 0)
	{
		if (execl("/bin/sh", "sh", "-c",
				  "gcc ../temp/source.c -o ../temp/executable", (char *)NULL) == -1)
		{
			perror("Compilation");
			exit(-1);
		}
	}
	else
	{
		// the parent waits for the child to finish
		int status;
		waitpid(pid, &status, 0);
	}
}

void runExecutable()
{
	// creates a child process that executes the executable
	int pid = fork();
	if (pid == -1)
	{
		perror("Creating child");
		exit(-1);
	}
	if (pid == 0)
	{
		// opens or create the file in which the output will be saved
		int fd = open("../temp/output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (fd == -1)
		{
			perror("Creating output for the executable");
			exit(-1);
		}
		// duplicates the STDOUT file descriptor to the output.txt
		dup2(fd, STDOUT_FILENO);
		close(fd);
		// runs the code
		if (execl("../temp/./executable", "executable", (char *)NULL) == -1)
		{
			perror("Running the executable");
			exit(-1);
		}
	}
	else
	{
		// waits for the child to finish
		int status;
		waitpid(pid, &status, 0);
	}
}

void transferData(int cd)
{
	int rc, wc;
	char buff[MAX_BUF_SIZE];
	// opens the output.txt for read
	int fd = open("../temp/output.txt", O_RDONLY);
	if (fd == -1)
	{
		perror("Reading from output.txt");
		exit(-1);
	}
	// reads from the output.txt
	while ((rc = read(fd, buff, MAX_BUF_SIZE)) > 0)
	{
		if (rc == -1)
		{
			perror("Reading from output.txt");
			exit(-1);
		}
		// writes to the server
		wc = write(cd, buff, rc);
		if (wc < rc)
		{
			printf("Couldn't write all the data to the socket!\n");
			exit(-1);
		}
		if (wc == -1)
		{
			perror("Writing in socket");
			exit(-1);
		}
	}

	if (write(cd, "END_TRANSMISSION", strlen("END_TRANSMISSION")) == -1)
	{
		perror("Shutdown");
	}

	if (rc == 0)
	{
		printf("End of data transmission to the client\n");
	}
	else
	{
		perror("Reading");
	}
	close(fd);
}

void removeFiles()
{
	// removes all the temporary files that were needed
	int status;
	if ((status = unlink("../temp/output.txt")) == -1)
	{
		perror("Deleting \"output.txt\"");
		exit(-1);
	}
	if ((status = unlink("../temp/source.c")) == -1)
	{
		perror("Deleting \"source.c\"");
		exit(-1);
	}
	if ((status = unlink("../temp/executable")) == -1)
	{
		perror("Deleting \"executable\"");
		exit(-1);
	}
	printf("Successfuly removed all temporary files!\n");
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
	while (b == -1)
	{
		b = bind(sd, (struct sockaddr *)&ser, sizeof(ser));
	}
	printf("BIND VALUE: %d\n", b);
	if (b == -1)
	{
		perror("BIND");
		exit(-1);
	}
	listen(sd, 5);
	printf("Connecting to client...\n");

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
		reciveData(cd);
		compile();
		runExecutable();
		transferData(cd);
		removeFiles();
	}
	////////////////////////

	// Connect to the server
	// printf("Connecting to server...\n");
	// connect(sd, (struct sockaddr *)&ser, sizeof(ser));
	// printf("Connection successful!\n");

	// Greet the server
	// printf("Greeting server...\n");
	// write(sd, WORKER_GREETING, sizeof(WORKER_GREETING));

	// close(sd);

	return 0;
}