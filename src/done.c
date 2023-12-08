#include "done.h"
#include <stdio.h>

pthread_mutex_t mtx_done = PTHREAD_MUTEX_INITIALIZER;
done_info_t done_list[MAX_DONE_NR] = {0};
int doneIndex = -1;

int push_done(done_info_t done) {
	pthread_mutex_lock(&mtx_done);
	if( doneIndex == MAX_DONE_NR ) {
		printf("Cannot push: the done list is full!\n");
		pthread_mutex_unlock(&mtx_done);
		return -1;
	}

	done_list[doneIndex++] = done;
	pthread_mutex_unlock(&mtx_done);

	return 0;
}

int try_pop_done(int client_tid, done_info_t *output) {
	pthread_mutex_lock(&mtx_done);
	if ( doneIndex == -1){
		printf("Cannot pop: the done list is empty!\n");
		pthread_mutex_unlock(&mtx_done);
		return -1;
	}

	*output = done_list[doneIndex--]; // TODO: modify this to compare client tid
	pthread_mutex_unlock(&mtx_done);

	return 0;
}