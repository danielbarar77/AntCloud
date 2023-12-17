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

typedef struct arguments
{
	char **args;
	int argc;
} arguments;

void allocMemory(char **buffer, char **command, arguments **args)
{
	*args = (arguments *)malloc(sizeof(arguments));
	*buffer = (char *)malloc(256 * sizeof(char));
	*command = (char *)malloc(64 * sizeof(char));
	(*args)->argc = 0;
	(*args)->args = (char **)malloc(MAX_ARGS * sizeof(char *));
	for (int i = 0; i < MAX_ARGS; i++)
	{
		(*args)->args[i] = (char *)malloc(ARGS_LENGTH * sizeof(char));
	}
}

void freeMemory(char **buffer, char **command, arguments **args)
{
	free(*buffer);
	free(*command);
	for (int i = 0; i < MAX_ARGS; i++)
	{
		free((*args)->args[i]);
	}
	free((*args)->args);
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
			strncpy((*args)->args[(*args)->argc], token, strlen(token));
			(*args)->args[(*args)->argc][strlen(token)] = '\0';
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
			memset((*args)->args[i], 0, ARGS_LENGTH);
		}
		// memset(*args, 0, sizeof(*args));
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

int transferData(arguments args, int sd)
{
	// opens the source file that needs to be sent to the sever
	int fd = open(args.args[0], O_RDONLY);

	if (fd == -1)
	{
		perror("Opening the file");
		return -1;
	}

	char buff[MAX_BUF_SIZE];
	char *temp = NULL;
	int wc; // wc = write count
	int rc; // rc = read count

	// informs the server that I will send a file that needs to be run
	wc = write(sd, CMD_RUN, sizeof(CMD_RUN));
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
	while ((rc = read(fd, buff, MAX_BUF_SIZE)) > 0) // reads from the source file
	{
		if (rc == -1)
		{
			perror("Reading");
			close(fd);
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

	if (write(sd, ARGUMENTS_SIGNAL, sizeof(ARGUMENTS_SIGNAL)) == -1)
	{
		perror("Arguments signal");
		return -1;
	}
	if (write(sd, &(args.argc), sizeof(args.argc)) == -1)
	{
		perror("Sending argc");
		return -1;
	}
	int index = 0;
	while (index < args.argc)
	{
		wc = write(sd, args.args[index], sizeof(args.args[index]));
		if (wc == -1)
		{
			perror("Sending args");
			return -1;
		}
		index++;
	}

	// writes a message the signals the end of the transmission
	if (write(sd, END_TRANSMISSION_SIGNAL, sizeof(END_TRANSMISSION_SIGNAL)) == -1)
	{
		perror("Shutdown");
		return -1;
	}

	return 0;
}

void reciveData(int sd)
{
	char *buff = (char *)malloc(MAX_BUF_SIZE * sizeof(char));
	char *temp = NULL;
	int rc, wc;
	char *end, *type;

	printf("OUTPUT OF THE EXECUTABLE:\n");
	while ((rc = read(sd, buff, MAX_BUF_SIZE)) > 0) // reads from the server
	{
		if (rc == -1)
		{
			perror("Reading from the server");
			free(buff);
			exit(-1);
		}
		// checks if the transmission ended
		end = strstr(buff, END_TRANSMISSION_SIGNAL);
		if (end != NULL)
			memset(end, 0, sizeof(END_TRANSMISSION_SIGNAL));
		// checks the type of the data
		type = strstr(buff, CMD_RETURN);
		if (type != NULL)
		{
			memset(buff, 0, sizeof(type));
			buff += sizeof(type);
		}

		temp = base64_decode(buff);
		// writes to the terminal the output
		wc = write(STDOUT_FILENO, temp, strlen(temp));
		free(temp);
		printf("\n");

		if (wc == -1)
		{
			perror("Writing to terminal");
			free(buff);
			exit(-1);
		}
		if (end != NULL)
		{
			break;
		}
		free(buff);
	}
	free(buff);

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
	for (;;)
	{
		loadingScreen();
		allocMemory(&buff, &command, &args);
		readCommand(&buff, &command, &args);
		printf("argc = %d\n", args->argc);
		for (int i = 0; i < args->argc; i++)
		{
			printf("%s ", args->args[i]);
		}
		printf("\n");
		return 0;
		if ((strcmp(command, "run") == 0) && (checkIfExists(args->args[0]) == 0)) // checks the existance of the source file
		{
			if (transferData(*args, sd) == 0) // transfers the executable to the server
			{
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
			printf("INVALID SOURCE FILE OR CANNOT READ FROM IT!\n");
		}
		freeMemory(&buff, &command, &args);
	}

	close(sd);

	return 0;
}
