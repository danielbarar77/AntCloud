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

	done_list[++doneIndex] = done;
	pthread_mutex_unlock(&mtx_done);

	return 0;
}

done_info_t pop_done(int index) {
	done_info_t done = done_list[index];

	for (int i = index; i < doneIndex - 1; i++){
		done_list[i] = done_list[i+1];
	}
	doneIndex--;

	return done;
}

int try_pop_done(pthread_t client_tid, done_info_t *output) {
	pthread_mutex_lock(&mtx_done);
	if ( doneIndex == -1){
		printf("Cannot pop: the done list is empty!\n");
		pthread_mutex_unlock(&mtx_done);
		return -1;
	}

	for (int i = 0; i <= doneIndex; i++){ // TODO: fix - it doesn't find the client's executable
		if (done_list[i].client_tid == client_tid){
			*output = pop_done(i);
			pthread_mutex_unlock(&mtx_done);
			return 0;
		}
	}
	

	printf("Cannot pop: client %d executable has not finished yet!\n", client_tid);
	pthread_mutex_unlock(&mtx_done);
	return -1;
}