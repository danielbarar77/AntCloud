#include "todo.h"


sem_t sem_todo;
todo_info_t todo_queue[MAX_TODO_NR] = {0};
int todo_index = 0;

int push_todo(todo_info_t todo) {
	if ( todo_index == MAX_TODO_NR ){
		printf("Todo queue is full!\n");
		return -1;
	}

	todo_queue[todo_index++] = todo;
	sem_post(&sem_todo);
	return 0;
}

/// This functioon blocks on semaphore wait
int pop_todo(todo_info_t* todo_out) {
	sem_wait(&sem_todo);
	if ( todo_index == -1 ){
		printf("Todo queue is empty!\n");
		return -1;
	}

	*todo_out = todo_queue[todo_index--];

	return 0;
}