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

#define MAX_NR_THREADS 5
#define MAX_EVENTS 10
#define MAX_CONNECTION_QUEUE 5

struct parameters
{
	int cd; // connection descriptor;
};

typedef struct parameters parameters_t;

void cmd_client_run(char program[MAX_PROGRAM_SIZE], char output[MAX_OUTPUT_SIZE]) {
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
            printf("Executable finished with return value: %s\n", doneInfo.output);
            return 0;
        }

        sleep(1);
    }

    return 0;
}

int main()
{
    struct epoll_event ev, events[MAX_EVENTS];
    int listen_sock, conn_sock, nfds, epollfd;

	char buf[MAX_BUF_SIZE] = "", fname[10];
	struct sockaddr_in ser;

    pthread_t threads[MAX_NR_THREADS];
    int nrOfThreads = 0;

    sem_init(&sem_todo, 0, 0);

	// Create a socket
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0)
		printf("SOCKET NOT CREATED\n");

	bzero(&ser, sizeof(struct sockaddr_in));
	ser.sin_family = AF_INET;
	ser.sin_port = htons(1101);
	inet_aton("localhost", &ser.sin_addr);

    int b = bind(listen_sock, (struct sockaddr *)&ser, sizeof(ser));

    while( b == -1 ){
        perror("Error binding to socket");
        printf("BIND VALUE: %d\n", b);
        printf("Retrying to bind to socket...\n");
        sleep(1);
        b = bind(listen_sock, (struct sockaddr *)&ser, sizeof(ser));
    }

    printf("BIND VALUE: %d\n", b);
    printf("Binding successful!\n");

	listen(listen_sock, MAX_CONNECTION_QUEUE);

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

	for (;;)
	{
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) { // accept new connections on listen socket
                conn_sock = accept(listen_sock, (struct sockaddr *) &addr, &addrlen);
                if (conn_sock == -1) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                setnonblocking(conn_sock);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                    perror("epoll_ctl: conn_sock");
                    exit(EXIT_FAILURE);
                }
            } else { // handle event
                
            }
        }
	}

	close(listen_sock);

	return 0;
}