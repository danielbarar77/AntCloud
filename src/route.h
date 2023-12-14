#ifndef ROUTE_H
#define ROUTE_H

#include <fcntl.h>
#include <sys/types.h>
#include "common.h"
#include "networking.h"

#define MAX_ROUTE_NR 10

struct route_info {
    pthread_t client_tid;
    connection_t conn;
    char buf[MAX_BUF_SIZE];
};

typedef struct route_info route_info_t;

extern route_info_t route_queue[MAX_ROUTE_NR];

int push_route(route_info_t route);
int pop_route(route_info_t *route_out);

#endif