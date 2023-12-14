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

void readCommand()
{
	setvbuf(stdout, NULL, _IONBF, 0);
	char *command = (char *)malloc(128 * sizeof(char));

	printf("		");
	for (int i = 0; i < 24; i++)
	{
		printf("#");
		usleep(25000);
	}
	usleep(75000);
	printf("\n			  100% 				\n");
	usleep(125000);
	printf("\n		****Welcome to Agora****		\n\n");

	while (1)
	{
		printf(">");
		scanf("%s", command);
		if ((strcmp(command, "?") == 0) || (strcmp(command, "help") == 0) || (strcmp(command, "-h") == 0))
		{
			printHelp();
		}
		if ((strcmp(command, "exit") == 0) || (strcmp(command, "q") == 0))
		{
			exit(0);
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

	// writes a message the signals the end of the transmission
	if (write(sd, END_TRANSMISSION_SIGNAL, sizeof(END_TRANSMISSION_SIGNAL)) == -1)
	{
		perror("Shutdown");
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
	char buf[MAX_BUF_SIZE];
	char *command = (char *)malloc(128 * sizeof(command));
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
		readCommand();
		scanf("%s", buf); // reads the filepath
		printf("ENTER THE EXECUTABLE: \n");
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
