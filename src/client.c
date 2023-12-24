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

void allocMemory(char **buffer, char **command, arguments **args)
{
	*args = (arguments *)malloc(sizeof(arguments));
	*buffer = (char *)malloc(256 * sizeof(char));
	*command = (char *)malloc(64 * sizeof(char));
	(*args)->argc = 0;
	(*args)->argv = (char **)malloc(MAX_ARGS * sizeof(char *));
	for (int i = 0; i < MAX_ARGS; i++)
	{
		(*args)->argv[i] = (char *)malloc(ARGS_LENGTH * sizeof(char));
	}
}

void resetMemory(char **buffer, char **command, arguments **args)
{
	memset(*command, 0, 64);
	memset(*buffer, 0, 256);
	for (int i = 0; i < (*args)->argc; i++)
	{
		memset((*args)->argv[i], 0, ARGS_LENGTH);
	}
	(*args)->argc = 0;
}

void freeMemory(char **buffer, char **command, arguments **args)
{
	free(*buffer);
	free(*command);
	for (int i = 0; i < MAX_ARGS; i++)
	{
		free((*args)->argv[i]);
	}
	free((*args)->argv);
	free(*args);
}

void printHelp()
{
	int fd = open("help.txt", O_RDONLY);
	if (fd == -1)
	{
		perror("Opening help.txt");
		return;
	}
	char buf[MAX_BUF_SIZE];
	int rc, wc;
	while ((rc = read(fd, buf, MAX_BUF_SIZE)) > 0)
	{
		wc = write(STDOUT_FILENO, buf, rc);
	}
}

void tokenizeInput(char **buffer, char **command, arguments **args)
{
	if (strlen(*buffer) == 1)
	{
		strncpy(*command, *buffer, 1);
		(*command)[1] = '\0';
		return;
	}
	char *temp = (char *)malloc(256 * sizeof(char));
	strcpy(temp, *buffer);
	char *token;
	token = strtok(temp, " \n\r");
	if (token != NULL)
	{
		strncpy(*command, token, strlen(token));
		command[strlen(token)] = '\0';
	}

	(*args)->argc = 0;
	while ((token = strtok(NULL, " \n\r\0")))
	{
		if ((*args)->argc < MAX_ARGS)
		{
			strncpy((*args)->argv[(*args)->argc], token, strlen(token));
			(*args)->argv[(*args)->argc][strlen(token)] = '\0';
			(*args)->argc++;
		}
		else
		{
			printf("Maximum arguments is 16 !\n");
			break;
		}
	}
	free(temp);
}

void loadingScreen()
{
	setvbuf(stdout, NULL, _IONBF, 0);
	printf("		");
	for (int i = 0; i < 27; i++)
	{
		printf("#");
		usleep(25000);
	}
	usleep(75000);
	printf("\n\t\t\t   100%%\n");
	usleep(125000);
	printf("\n\t\t****Welcome to AntCloud****\t\t\n\n");
}

void readCommand(char **buffer, char **command, arguments **args)
{
	char c;
	int index = -1;
	while (1)
	{
		index = -1;
		memset(*buffer, 0, 256);
		memset(*command, 0, 64);
		for (int i = 0; i < MAX_ARGS; i++)
		{
			memset((*args)->argv[i], 0, ARGS_LENGTH);
		}
		printf(">");
		while ((c = getchar()) != '\n')
		{
			(*buffer)[++index] = c;
		}
		tokenizeInput(buffer, command, args);
		if ((strcmp(*buffer, "?") == 0) || (strcmp(*buffer, "help") == 0) || (strcmp(*buffer, "-h") == 0))
		{
			printHelp();
		}
		else if ((strcmp(*buffer, "exit") == 0) || (strcmp(*buffer, "q") == 0))
		{
			exit(0);
		}
		else if (strcmp(*command, "run") == 0)
		{
			if ((*args)->argc > 0)
				return;
			else
				printf("Incorrect syntax! Try \"help\" for more info.\n");
		}
		else if ((*buffer)[0] != 0)
		{
			printf("Command \'%s\' not found!\n", *buffer);
			printf("Try \'help\' for documentation.\n");
		}
	}
}

int checkIfExists(char *buf)
{
	// checks the existance of the file and if is readable
	if (access(buf, F_OK | R_OK))
		return -1;

	return 0;
}

int transferData(arguments *args, int sd)
{
	// opens the source file that needs to be sent to the sever
	int fd = open(args->argv[0], O_RDONLY);

	if (fd == -1)
	{
		perror("Opening the file");
		return -1;
	}

	char buff[MAX_BUF_SIZE];
	char *temp = NULL;
	int wc;			// wc = write count
	int rc;			// rc = read count
	int length = 0; // length of the buffer to be send

	// informs the server that I will send a file that needs to be run
	wc = write(sd, CMD_RUN, strlen(CMD_RUN));
	if (wc == -1)
	{
		perror("Writing CMD_RUN to the server");
		exit(-1);
	}
	else if (wc == 0)
	{
		printf("Couldn't send CMD_RUN to the server!");
		exit(-1);
	}

	memset(buff, 0, MAX_BUF_SIZE);
	// reads from the source file
	rc = read(fd, buff, MAX_BUF_SIZE);
	if (rc == -1)
	{
		perror("reading from file");
		exit(1);
	}

	// encoding in b64 and setting the length
	temp = base64_encode(buff);
	length = strlen(temp);
	// writing the length of the buffer
	wc = write(sd, &length, 4);
	if (wc == -1)
	{
		perror("Sending buff length");
		exit(1);
	}

	// writing the b64 encoded buffer
	wc = write(sd, temp, length);
	if (wc == -1)
	{
		perror("Sending the buffer");
		exit(1);
	}

	// close the file descriptor of the source file
	if (close(fd) == -1)
	{
		perror("Closing");
		return -1;
	}

	// sending the argument signal
	if (write(sd, ARGUMENTS_SIGNAL, strlen(ARGUMENTS_SIGNAL)) == -1)
	{
		perror("Arguments signal");
		return -1;
	}
	// sending argc
	if ((wc = write(sd, &(args->argc), 4) == -1))
	{
		perror("Sending argc");
		return -1;
	}

	// sending the argvs
	int index = 0;
	int argv_len = 0;
	while (index < args->argc)
	{
		argv_len = strlen(args->argv[index]);
		wc = write(sd, &argv_len, 4);

		wc = write(sd, args->argv[index], strlen(args->argv[index]));
		if (wc == -1)
		{
			perror("Sending argvs");
			return -1;
		}
		index++;
	}

	// writes a message the signals the end of the transmission
	if (write(sd, END_TRANSMISSION_SIGNAL, strlen(END_TRANSMISSION_SIGNAL)) == -1)
	{
		perror("on END_TRANSMISSION_SIGNAL");
		return -1;
	}

	return 0;
}

void reciveData(int sd)
{
	char *buff = (char *)malloc(MAX_BUF_SIZE * sizeof(char));
	char *temp = NULL;
	int rc, wc;
	int buff_length = 0, temp_length = 0;
	memset(buff, 0, MAX_BUF_SIZE);

	// waitng for CMD_RUN signal
	rc = read(sd, buff, strlen(CMD_RETURN));
	if (rc == -1)
	{
		perror("reading CMD_RETURN");
		exit(1);
	}
	while (strcmp(buff, CMD_RETURN) != 0)
	{
		rc = read(sd, buff, strlen(CMD_RETURN));
		if (rc == -1)
		{
			perror("reading CMD_RETURN");
			exit(1);
		}
	}

	// reading buff_length
	rc = read(sd, &buff_length, 4);
	if (rc == -1)
	{
		perror("reading buff_length");
		exit(1);
	}
	memset(buff, 0, MAX_BUF_SIZE);
	// reading the buffer
	rc = read(sd, buff, buff_length);
	if (rc == -1)
	{
		perror("reading buffer");
		exit(1);
	}

	// decode buffer
	temp = base64_decode(buff);
	temp_length = strlen(temp);

	memset(buff, 0, MAX_BUF_SIZE);
	// waiting for END_TRANSMISSION_SIGNAL
	rc = read(sd, buff, strlen(END_TRANSMISSION_SIGNAL));
	if (rc == -1)
	{
		perror("reading END_TRANSMISSION_SIGNAL");
		exit(1);
	}
	while (strcmp(buff, END_TRANSMISSION_SIGNAL) != 0)
	{
		rc = read(sd, buff, strlen(END_TRANSMISSION_SIGNAL));
		if (rc == -1)
		{
			perror("reading END_TRANSMISSION_SIGNAL");
			exit(1);
		}
	}

	// writing to console
	wc = write(STDOUT_FILENO, temp, temp_length);
	if (wc == -1)
	{
		perror("writing output to console");
		exit(1);
	}
	write(STDOUT_FILENO, "\n", 1);
	free(buff);
	free(temp);
}

int main()
{
	int sd;
	char *buff = NULL, *command = NULL;
	arguments *args = NULL;
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
	loadingScreen();
	allocMemory(&buff, &command, &args);
	for (;;)
	{
		readCommand(&buff, &command, &args);
		if ((strcmp(command, "run") == 0) && (checkIfExists(args->argv[0]) == 0)) // checks the existance of the source file
		{
			if (transferData(args, sd) == 0) // transfers the executable to the server
			{
				printf("SUCCESS TRANSFER OF DATA TO THE SERVER!\n");
				printf("WAITING FOR THE RESULTS...\n");
				printf("OUTPUT OF THE EXECUTABLE:\n");
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
			printf("INVALID SOURCE FILE OR CANNOT READ FROM IT!\n");
		}
		resetMemory(&buff, &command, &args);
	}
	freeMemory(&buff, &command, &args);

	close(sd);

	return 0;
}
