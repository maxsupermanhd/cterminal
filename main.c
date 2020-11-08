#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "cterminal.h"

bool needexit = false;
pthread_mutex_t needexitmutex = PTHREAD_MUTEX_INITIALIZER;

void commdr(char* command) {
	if(strcmp(command, "lolkekexitme") == 0) {
		pthread_mutex_lock(&needexitmutex);
		needexit = 1;
		pthread_mutex_unlock(&needexitmutex);
	}
	return;
}

void *WorkerThreadFunction(void*) {
	for(int i=0; ; i++) {
		usleep(rand()%900000);
		CTprintf("I'm random log string, blah. (%d)", i);
		pthread_mutex_lock(&needexitmutex);
		if(needexit) {
			pthread_exit(0);
		}
		pthread_mutex_unlock(&needexitmutex);
	}
	return NULL;
}

int main() {
	CTinit(&commdr);
	pthread_t WorkerThread;
	pthread_create(&WorkerThread, NULL, &WorkerThreadFunction, NULL);
	pthread_join(WorkerThread, NULL);
	CTexit();
	return 0;
}
