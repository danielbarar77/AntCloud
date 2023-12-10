#include "todo.h"


sem_t sem_todo;
pthread_mutex_t mtx_todo = PTHREAD_MUTEX_INITIALIZER;
todo_info_t todo_queue[MAX_TODO_NR] = {0};
int todo_index = -1;

int push_todo(todo_info_t todo) {
	pthread_mutex_lock(&mtx_todo);
	if ( todo_index == MAX_TODO_NR ){
		printf("Todo queue is full!\n");
		pthread_mutex_unlock(&mtx_todo);
		return -1;
	}

	todo_queue[++todo_index] = todo;

	pthread_mutex_unlock(&mtx_todo);
	sem_post(&sem_todo);
	return 0;
}

/// This functioon blocks on semaphore wait
int pop_todo(todo_info_t* todo_out) {
	sem_wait(&sem_todo);
	pthread_mutex_unlock(&mtx_todo);
	if ( todo_index == -1 ){
		printf("Todo queue is empty!\n");
		pthread_mutex_unlock(&mtx_todo);
		return -1;
	}

	*todo_out = todo_queue[todo_index--];
	pthread_mutex_lock(&mtx_todo);

	return 0;
}