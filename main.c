/*
	This file is part of cterminal.

    cterminal is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cterminal is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cterminal.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "cterminal.h"

// exit flag
bool needexit = false;
pthread_mutex_t needexitmutex = PTHREAD_MUTEX_INITIALIZER;

// commander function, recieves null-terminated commands
// called with pthread_create inside cterminal
void commdr(char* command) {
	if(strcmp(command, "lolkekexitme") == 0) {
		CTprintf("Exit scheduled.");
		pthread_mutex_lock(&needexitmutex);
		needexit = 1;
		pthread_mutex_unlock(&needexitmutex);
	}
	return;
}

// simple spammer example, will not respond to exit flag until finished work (sleep)
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
	// init
	CTinit(&commdr);

	// create workload
	pthread_t WorkerThread;
	pthread_create(&WorkerThread, NULL, &WorkerThreadFunction, NULL);

	// wait till we are ready to exit
	pthread_join(WorkerThread, NULL);

	// shutdown & cleanup
	CTexit();
	return 0;
}
