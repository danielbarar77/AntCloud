#ifndef TODO_H
#define TODO_H

#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <semaphore.h>
#include "common.h"

#define MAX_TODO_NR 10

struct todo_info {
    int client_tid;
    char code[MAX_CODE_SIZE];
};

typedef struct todo_info todo_info_t;

extern sem_t sem_todo;
extern todo_info_t todo_queue[MAX_TODO_NR];

int push_todo(todo_info_t todo);
int pop_todo(todo_info_t *todo_out);

#endif