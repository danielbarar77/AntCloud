#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>

#include "common.h"
#include "done.h"
#include "todo.h"

#define MAX_NR_THREADS 10

struct parameters
{
	int cd; // connection descriptor;
};

typedef struct parameters parameters_t;

int cmd_client_run(char program[MAX_PROGRAM_SIZE]) {
    todo_info_t todo;
    memset(&todo, 0, sizeof(todo));
    todo.client_tid = pthread_self();
    strcpy(todo.program, program);

    while(push_todo(todo) == -1){
        printf("Retrying to push the task\n");
        sleep(1);
    }

    while(1){
        done_info_t doneInfo;
        memset(&doneInfo, 0, sizeof(doneInfo));

        int tryRet = try_pop_done(pthread_self(), &doneInfo);

        if (tryRet == 0) {
            printf("Executable finished with return value: %d\n", doneInfo.return_value);
            return doneInfo.return_value;
        }

        sleep(1);
    }
}

void* client_routine(parameters_t *params){
    char buf[MAX_BUF_SIZE];
    int rc = 0, wc = 0;
    int cd = params->cd;
    int isRunning = 1;

    printf("Client thread, cd: %d, tid: %lu\n", cd, pthread_self());

    while(isRunning){
        rc = read(cd, buf, MAX_BUF_SIZE);
        if (rc == -1){
            printf("In client tid: %lu ", pthread_self());
            perror("couldn't read from socket");
        } else {
            //;
            if( strstr(buf, CMD_RUN) != NULL ) {
                char *program = buf + sizeof(CMD_RUN);
                int result = cmd_client_run(program);

                memset(buf, 0, MAX_BUF_SIZE);
                sprintf(buf, "return %d", result);
                wc = write(cd, buf, strlen(buf)); // send back result to client

                if ( wc == -1 ){
                    printf("In client tid: %lu, ", pthread_self());
                    perror(" error sending result");
                }
            }
            //char program[MAX_PROGRAM_SIZE];
            
        }
    }


	free(params);

	return NULL;
}

void* worker_routine(parameters_t *params){
    printf("Worker thread, cd: %d, tid: %lu\n", params->cd, pthread_self());
    char buf[MAX_BUF_SIZE];
    int rc = 0, wc = 0;

    /// ? nu-s sigur de codul asta daca e safe
    todo_info_t todo;
    int popValue = pop_todo(&todo); // waits on semaphore
    if (popValue == 0){
        // run executable
        done_info_t done;
        memset(&done, 0, sizeof(done));
        done.client_tid = todo.client_tid;
        done.return_value = 5;

        while( push_done(done) == -1) {
            printf("Retrying to push the task results...\n");
            sleep(1);
        }
        
        printf("pushed the task result...\n");
    }

	free(params);

	return NULL;
}

int main()
{
	int sd;
	char buf[MAX_BUF_SIZE] = "", fname[10];
	struct sockaddr_in ser;

    pthread_t threads[MAX_NR_THREADS];
    int nrOfThreads = 0;

    sem_init(&sem_todo, 0, 0);

	// Create a socket
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0)
		printf("SOCKET NOT CREATED\n");

	bzero(&ser, sizeof(struct sockaddr_in));
	ser.sin_family = AF_INET;
	ser.sin_port = htons(1101);
	inet_aton("localhost", &ser.sin_addr);

    int b = bind(sd, (struct sockaddr *)&ser, sizeof(ser));

    while( b == -1 ){
        perror("Error binding to socket");
        printf("BIND VALUE: %d\n", b);
        printf("Retrying to bind to socket...\n");
        sleep(1);
        b = bind(sd, (struct sockaddr *)&ser, sizeof(ser));
    }

    printf("BIND VALUE: %d\n", b);
    printf("Binding successful!\n");


	listen(sd, 5);

	while (1)
	{
		if (nrOfThreads != MAX_NR_THREADS)
		{
			parameters_t *params = malloc(sizeof(parameters_t));

			if (params == NULL)
			{
				perror("Failed to allocate thread parameters!\n");
				exit(1);
			}

			memset(params, 0, sizeof(*params));

			params->cd = accept(sd, NULL, NULL);

			if (params->cd == -1)
			{
				perror("Could not accept connection!");
				free(params);
			}
			else
			{ // Create thread

				int rc = read(params->cd, buf, MAX_BUF_SIZE); // Read entity greeting

				if (rc == -1 || rc == 0)
				{
					perror("Error reading greeting!");
					exit(1);
				}

				int errorCode = -1;
				int isValidEntity = 1;
				if (strcmp(buf, CLIENT_GREETING) == 0)
				{
					errorCode = pthread_create(&(threads[nrOfThreads]), NULL, client_routine, (void *)params); // Make client routine thread
				}
				else if (strcmp(buf, WORKER_GREETING) == 0)
				{
					errorCode = pthread_create(&(threads[nrOfThreads]), NULL, worker_routine, (void *)params); // Make worker routine thread
				}
				else
				{
					isValidEntity = 0;
				}

				if (isValidEntity == 0)
				{
					printf("Invalid entity type!\n");
					memset(params, 0, sizeof(*params));
					free(params);
				}
				else
				{
					if (errorCode != 0)
					{
						perror("Error creating thread!\n");
						memset(params, 0, sizeof(*params));
						free(params);
					}
					else
					{
						nrOfThreads++;
					}
				}
			}
		}
	}

	close(sd);

	return 0;
}