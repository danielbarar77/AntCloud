#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>

#include "common.h"
#include "route.h"
#include "networking.h"

#define MAX_EVENTS 10
#define SERVER_PORT 1101

int main()
{
    host_t hosts[MAX_HOST_NR]; // hosts[socket_conn] = type of the host connected on the socket
    memset(hosts, 0, sizeof(hosts) * MAX_HOST_NR);

    struct epoll_event ev, events[MAX_EVENTS];
    int listen_sock, conn_sock, nfds, epollfd;
    int hostCount = 0;

	char buf[MAX_BUF_SIZE] = "", fname[10];
	struct sockaddr_in ser;

	// Create a socket
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0)
		printf("SOCKET NOT CREATED\n");

	bzero(&ser, sizeof(struct sockaddr_in));
	ser.sin_family = AF_INET;
	ser.sin_port = htons(SERVER_PORT);
	inet_aton("localhost", &ser.sin_addr);

    int b = bind(listen_sock, (struct sockaddr *)&ser, sizeof(ser));

    while( b == -1 ){
        perror("Error binding to socket");
        printf("BIND VALUE: %d\n", b);
        printf("Retrying to bind to socket...\n");
        sleep(1);
        b = bind(listen_sock, (struct sockaddr *)&ser, sizeof(ser));
    }

    printf("Binding successful on socket: %d!\n", listen_sock);

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

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) { // accept new connections on listen socket
                conn_sock = accept(listen_sock, NULL, NULL);
                if (conn_sock == -1) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                if (hostCount < MAX_HOST_NR) {
                    printf("Connection accepted: %d\n", conn_sock);
                    fcntl(conn_sock, F_SETFL, fcntl(conn_sock, F_GETFL) | O_NONBLOCK); // set nonblocking
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = conn_sock;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                        perror("epoll_ctl: conn_sock");
                        exit(EXIT_FAILURE);
                    }
                    hostCount++;
                } else {
                    printf("Maximum number of hosts reached!\n");
                }
            } else { // handle event
                int cd = events[n].data.fd;
                switch(hosts[cd].type){
                case HOST_TYPE_CLIENT:
                    if (hosts[cd].pConnection == NULL) {
                        assignWorker(hosts, cd);
                    }
                    break;
                case HOST_TYPE_WORKER:
                    break;
                case HOST_TYPE_NULL:
                    if (events[n].events & EPOLLIN) {
                        if ( read(cd, buf, MAX_BUF_SIZE) < 0 ) {
                            perror("read");
                            exit(EXIT_FAILURE);
                        }

                        if(strcmp(buf, CLIENT_GREETING) == 0){
                            printf("Added new client: %d\n", cd);
                            hosts[cd].type = HOST_TYPE_CLIENT;
                        } else if (strcmp(buf, WORKER_GREETING) == 0){
                            printf("Added new worker: %d\n", cd);
                            hosts[cd].type = HOST_TYPE_WORKER;
                        } else {
                            printf("%d: Invalid greeting!\n", cd);
                        }
                    }
                    break;
                default:
                    printf("Invalid host type for socket: %lu\n", cd);
                    break;
                }
            }
        }
	}

	close(listen_sock);

	return 0;
}