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
#include "base64.h"

void allocMemory(char **command, arguments **args)
{
	*args = (arguments *)malloc(sizeof(arguments));
	*command = (char *)malloc(64 * sizeof(char));
	(*args)->argc = 0;
	(*args)->argv = (char **)malloc(MAX_ARGS * sizeof(char *));
	for (int i = 0; i < MAX_ARGS; i++)
	{
		(*args)->argv[i] = (char *)malloc(ARGS_LENGTH * sizeof(char));
	}
}

void resetMemory(char **command, arguments **args)
{
	memset(*command, 0, 64);
	(*args)->argv[(*args)->argc] = (char *)malloc(ARGS_LENGTH * sizeof(char));
	for (int i = 0; i < (*args)->argc; i++)
	{
		memset((*args)->argv[i], 0, ARGS_LENGTH);
	}
	(*args)->argc = 0;
}

void freeMemory(char **command, arguments **args)
{
	free(*command);
	for (int i = 0; i < MAX_ARGS; i++)
	{
		free((*args)->argv[i]);
	}
	free((*args)->argv);
	free(*args);
}

void reciveData(int cd, arguments **args)
{
	char *buff = (char *)malloc(MAX_BUF_SIZE * sizeof(char));
	char *temp;
	int rc, wc;
	int buff_length = 0, temp_length = 0;
	// opens or create the file in which the recived data will be stored
	int fd = open("../temp/source.c", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1)
	{
		perror("Creating source.c file");
		exit(1);
	}

	memset(buff, 0, MAX_BUF_SIZE);
	// waitng for CMD_RUN signal
	rc = read(cd, buff, strlen(CMD_RUN));
	if (rc == -1)
	{
		perror("reading CMD_RUN");
		exit(1);
	}
	while (strcmp(buff, CMD_RUN) != 0)
	{
		rc = read(cd, buff, strlen(CMD_RUN));
		if (rc == -1)
		{
			perror("reading CMD_RUN");
			exit(1);
		}
	}

	// reading the buff length
	rc = read(cd, &buff_length, 4);
	if (rc == -1)
	{
		perror("reading buff length");
		exit(1);
	}

	memset(buff, 0, MAX_BUF_SIZE);
	// reading the buffer of buff_length size
	rc = read(cd, buff, buff_length);
	if (rc == -1)
	{
		perror("reading buff length");
		exit(1);
	}
	buff[buff_length] = '\0';

	// decoding from b64
	temp = base64_decode(buff);
	temp_length = strlen(temp);

	// writing in source.c
	wc = write(fd, temp, temp_length);
	if (wc == -1)
	{
		perror("writing in source.c");
		exit(1);
	}
	close(fd);
	memset(buff, 0, MAX_BUF_SIZE);
	// waiting for ARGUMENTS_SIGNAL
	rc = read(cd, buff, strlen(ARGUMENTS_SIGNAL));
	if (rc == -1)
	{
		perror("reading ARGUMENTS_SIGNAL");
		exit(1);
	}
	while (strcmp(buff, ARGUMENTS_SIGNAL) != 0)
	{
		rc = read(cd, buff, strlen(ARGUMENTS_SIGNAL));
		if (rc == -1)
		{
			perror("reading ARGUMENTS_SIGNAL");
			exit(1);
		}
	}

	// read argc
	if ((rc = read(cd, &((*args)->argc), 4)) == -1)
	{
		perror("read argc");
		exit(1);
	}
	int argv_len = 0;
	// read argvs
	memset(buff, 0, MAX_BUF_SIZE);
	for (int i = 0; i < (*args)->argc; i++)
	{
		read(cd, &argv_len, 4);
		read(cd, buff, argv_len);
		buff[argv_len] = '\0';
		strncpy((*args)->argv[i], buff, argv_len);
		memset(buff, 0, MAX_BUF_SIZE);
	}
	(*args)->argv[(*args)->argc] = NULL;

	memset(buff, 0, MAX_BUF_SIZE);
	// waiting for END_TRANSMISSION_SIGNAL
	rc = read(cd, buff, strlen(END_TRANSMISSION_SIGNAL));
	if (rc == -1)
	{
		perror("reading END_TRANSMISSION_SIGNAL");
		exit(1);
	}
	while (strcmp(buff, END_TRANSMISSION_SIGNAL) != 0)
	{
		rc = read(cd, buff, strlen(END_TRANSMISSION_SIGNAL));
		if (rc == -1)
		{
			perror("reading END_TRANSMISSION_SIGNAL");
			exit(1);
		}
	}

	free(buff);
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

void runExecutable(arguments *args)
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

		// runs the executable
		if (execv("../temp/./executable", args->argv) == -1)
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
	char *temp;
	int temp_length = 0;
	// opens the output.txt for read
	int fd = open("../temp/output.txt", O_RDONLY);
	if (fd == -1)
	{
		perror("Reading from output.txt");
		exit(-1);
	}

	// informs the server that I will send a the return value
	wc = write(cd, CMD_RETURN, strlen(CMD_RETURN));
	if (wc == -1)
	{
		perror("Writing CMD_RETURN to the server");
		exit(-1);
	}
	else if (wc == 0)
	{
		printf("Couldn't send CMD_RETURN to the server!");
		exit(-1);
	}
	memset(buff, 0, MAX_BUF_SIZE);

	// reads from the output.txt
	rc = read(fd, buff, MAX_BUF_SIZE);
	if (rc == -1)
	{
		perror("Reading from output.txt");
		exit(-1);
	}
	close(fd);

	temp = base64_encode(buff);
	temp_length = strlen(temp);

	// sending the buffer length
	if ((wc = write(cd, &temp_length, 4)) == -1)
	{
		perror("sending buffer length");
		exit(1);
	}

	// sending the buffer
	if ((wc = write(cd, temp, strlen(temp))) == -1)
	{
		perror("sending the buffer");
		exit(1);
	}
	free(temp);

	if (write(cd, END_TRANSMISSION_SIGNAL, strlen(END_TRANSMISSION_SIGNAL)) == -1)
	{
		perror("Shutdown");
		exit(1);
	}
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
	int sd, cd, rc;
	char *command = NULL;
	arguments *args = NULL;
	struct sockaddr_in ser;

	// Create a socket
	cd = socket(AF_INET, SOCK_STREAM, 0);
	if (cd < 0)
		printf("SOCKET NOT CREATED\n");

	bzero(&ser, sizeof(struct sockaddr_in));
	ser.sin_family = AF_INET;
	ser.sin_port = htons(1101);
	inet_aton("localhost", &ser.sin_addr);

	// //////////////////////// for testing
	// int b = bind(sd, (struct sockaddr *)&ser, sizeof(ser));
	// while (b == -1)
	// {
	// 	sleep(1);
	// 	printf("Bind value: %d!\n", b);
	// 	b = bind(sd, (struct sockaddr *)&ser, sizeof(ser));
	// }
	// printf("BIND VALUE: %d\n", b);
	// if (b == -1)
	// {
	// 	perror("BIND");
	// 	exit(-1);
	// }
	// listen(sd, 5);
	// printf("Connecting to client...\n");

	// cd = accept(sd, NULL, NULL);
	// if (cd == -1)
	// {
	// 	printf("Coudn't accept connection!\n");
	// 	return -1;
	// }
	// else
	// {
	// 	printf("Connection successful!\n");
	// }

	rc = connect(cd, (struct sockaddr *)&ser, sizeof(ser));

	if (rc < 0)
	{
		perror("connect");
		exit(1);
	}

	printf("Connection to server successful!\n");

	write(cd, WORKER_GREETING, GREETING_SIZE);

	allocMemory(&command, &args);
	while (1)
	{
		reciveData(cd, &args);
		compile();
		runExecutable(args);
		transferData(cd);
		removeFiles();
		resetMemory(&command, &args);
	}
	freeMemory(&command, &args);
	////////////////////////

	// // Connect to the server
	// printf("Connecting to server...\n");
	// connect(sd, (struct sockaddr *)&ser, sizeof(ser));
	// printf("Connection successful!\n");

	// // Greet the server
	// printf("Greeting server...\n");
	// write(sd, WORKER_GREETING, sizeof(WORKER_GREETING));

	// // TESTING : read testStr from client
	// read(sd, buf, MAX_BUF_SIZE);
	// printf("testStr: %s", buf);

	// while (1)
	// {
	// 	sleep(1);
	// }

	close(sd);

	return 0;
}