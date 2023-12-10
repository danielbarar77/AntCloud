#ifndef DONE_H
#define DONE_H

#include <pthread.h>
#define MAX_DONE_NR 10

struct done_info {
    pthread_t client_tid;
    int return_value;
};

typedef struct done_info done_info_t;

extern pthread_mutex_t mtx_done;
extern done_info_t done_list[MAX_DONE_NR];

int push_done(done_info_t done);
int try_pop_done(pthread_t client_tid, done_info_t *output);

#endif