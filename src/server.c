#define _XOPEN_SOURCE 700

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
#include <signal.h>

#include <string.h>

#include "common.h"
#include "svnet.h"

#define MAX_EVENTS 10
#define SERVER_PORT 1101

int listen_sock;

void signal_termination_handler(int signum){
    printf("Signal handler reached!\n");
    close(listen_sock);

    for (int i = 0; i < MAX_HOST_NR; i++){
        if (hosts[i].type != HOST_TYPE_NULL){
            close(i);
        }
    }

    exit(0);
}

int main()
{
    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, epollfd;
    

	char buf[MAX_BUF_SIZE] = "", fname[10];
	struct sockaddr_in ser;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_flags = SA_RESETHAND;
    sa.sa_handler = signal_termination_handler;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGKILL, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

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
                    ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP; // level-triggered

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

                if (hosts[cd].type != HOST_TYPE_NULL){
                    if (hosts[cd].type == HOST_TYPE_CLIENT) {
                        if (hosts[cd].pConnection == NULL) {
                            int rc = assignWorker(cd);
                            
                            if (rc == 0){
                                printf("Linking client: successful to host cd: %d!\n", hosts[cd].pConnection->cd_right);
                            }
                        }
                    }

                    if (hosts[cd].pConnection != NULL) {
                        if (events[n].events & EPOLLIN) {
                            if (hosts[cd].pConnection->hasMsgToRead == 0) {
                                memset(buf, 0, MAX_BUF_SIZE);
                                int rc = read(cd, buf, MAX_BUF_SIZE);

                                if (rc < 0) {
                                    perror("read");
                                } else {
                                    if (rc > 0) {
                                        write(STDOUT_FILENO, "connWriteMsg: ", sizeof("connReadMsg: "));
                                        write(STDOUT_FILENO, buf, rc);
                                        write(STDOUT_FILENO, "\n", sizeof("\n"));
                                        connWriteMsg(buf, rc, hosts[cd].pConnection);
                                    }
                                }
                                
                            }
                        } else if (events[n].events & EPOLLOUT) {
                            if (hosts[cd].pConnection->hasMsgToRead == 1) {
                                memset(buf, 0, MAX_BUF_SIZE);
                                int crm = connReadMsg(buf, hosts[cd].pConnection);

                                if (crm > 0) {
                                    write(STDOUT_FILENO, "connReadMsg: ", sizeof("connReadMsg: "));
                                    write(STDOUT_FILENO, buf, crm);
                                    write(STDOUT_FILENO, "\n", sizeof("\n"));
                                    int wc = write(cd, buf, crm);

                                    if (wc < 0){
                                        perror("write");
                                    }
                                }
                            }
                        }
                    }
                } else if (hosts[cd].type == HOST_TYPE_NULL) {
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
                } else {
                    printf("Invalid host type for socket: %d\n", cd);
                }

                if (events[n].events & EPOLLRDHUP) {
                    printf("Host disconnected\n"); // TODO: print host address
                    deleteHostConnection(cd);
                    deleteHost(cd);

                    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, cd, &ev) == -1) {
                        perror("epoll_ctl: cd");
                        exit(EXIT_FAILURE);
                    }
                    
                    close(cd);
                }
            }
        }
	}

	close(listen_sock);

	return 0;
}