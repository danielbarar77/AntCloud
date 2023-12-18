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
#include "svnet.h"

#define MAX_EVENTS 10
#define SERVER_PORT 1101

int main()
{

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
                    //fcntl(conn_sock, F_SETFL, fcntl(conn_sock, F_GETFL) | O_NONBLOCK); // set nonblocking
                    //ev.events = EPOLLIN | EPOLLOUT | EPOLLET; // edge-triggered
                    ev.events = EPOLLIN | EPOLLOUT; // level-triggered

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
                    // if(events[n].events & EPOLLHUP){
                    //     printf("Client %d: closed connection.\n", cd);
                    //     epoll_ctl(epollfd, EPOLL_CTL_DEL, cd, NULL);
                    //     break;
                    // }
                    if (hosts[cd].pConnection == NULL) {
                        assignWorker(hosts, cd);
                    }
                    if ((events[n].events & EPOLLIN) && hosts[cd].pConnection != NULL) {
                        if (isSender(cd, *(hosts[cd].pConnection)) && hosts[cd].pConnection->hasMsgToRead == 0) {
                            memset(buf, 0, MAX_BUF_SIZE);
                            int rc = read(cd, buf, MAX_BUF_SIZE); // read msg from client

                            if (rc < 0) {
                                perror("read");
                            } else {
                                printf("%d connWriteMsg: %s\n", cd, buf);
                                connWriteMsg(buf, hosts[cd].pConnection); // write msg to worker
                            }
                            
                        }
                    }
                    break;
                case HOST_TYPE_WORKER:
                    // if ((events[n].events & EPOLLOUT))
                    //     printf("Can write to worker!\n");
                    if (hosts[cd].pConnection != NULL) {
                        if ((events[n].events & EPOLLOUT)) {
                            if (isReceiver(cd, *(hosts[cd].pConnection))) {
                                memset(buf, 0, MAX_BUF_SIZE);
                                int crm = connReadMsg(buf, hosts[cd].pConnection); // read msg from client

                                if (crm == 0) {
                                    printf("%d connReadMsg: %s\n", cd, buf);
                                    int wc = write(cd, buf, sizeof(buf)); // send msg to worker

                                    if (wc < 0){
                                        perror("write");
                                    }
                                }
                            }
                        }
                    }
                    // if(events[n].events & EPOLLHUP){
                    //     printf("Client %d: closed connection.\n", cd);
                    //     epoll_ctl(epollfd, EPOLL_CTL_DEL, cd, NULL);
                    // }
                    break;
                case HOST_TYPE_NULL:
                    if (events[n].events & EPOLLIN) {
                        memset(buf, 0, MAX_BUF_SIZE);
                        if ( read(cd, buf, GREETING_SIZE) < 0 ) {
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
                    printf("Invalid host type for socket: %d\n", cd);
                    break;
                }
            }
        }
	}

	close(listen_sock);

	return 0;
}