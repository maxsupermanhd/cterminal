#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "cterminal.h"

struct termios CTattr;

const size_t CTmaxBufferSize = 2048;  // Default command buffer size (excluding 0)
char *CTcmdbuf = NULL;
size_t CTcmdbufUsed = 0;
size_t CTcmdbufLen = 0;

const char* CTdefaultPromt = "[~>";   // Default promt
char *CTcmdpromt = NULL;
size_t CTcmdpromtLen = 0;

pthread_mutex_t CTiom = PTHREAD_MUTEX_INITIALIZER;
int CTwidth = 0;

void (*CTcommander)(char*) = NULL;

pthread_t CTreaderT;
int CTreaderControll = 0;
pthread_mutex_t CTreaderControllMutex = PTHREAD_MUTEX_INITIALIZER;
void* CTreader(void*);

bool CTinitDone = false;

int CTGetReaderControllState() {
	int r = -1;
	pthread_mutex_lock(&CTreaderControllMutex);
	r = CTreaderControll;
	pthread_mutex_unlock(&CTreaderControllMutex);
	return r;
}
void CTSetReaderControllState(int s) {
	pthread_mutex_lock(&CTreaderControllMutex);
	CTreaderControll = s;
	pthread_mutex_unlock(&CTreaderControllMutex);
	return;
}

void CTsetCommander(void (*cmd)(char*)) {
	CTcommander = cmd;
	return;
}

int CTsetPromt(char* np) {
	char* topromt = (char*)CTdefaultPromt;
	if(np != NULL) {
		topromt = np;
	}
	pthread_mutex_lock(&CTiom);
	char* newpromt = (char*) realloc(CTcmdpromt, strlen(topromt)+1);
	if(newpromt == NULL) {
		return 1;
	}
	CTcmdpromt = newpromt;
	strncpy(CTcmdpromt, topromt, strlen(topromt));
	CTcmdpromt[strlen(topromt)] = '\0';
	CTcmdpromtLen = strlen(topromt);
	pthread_mutex_unlock(&CTiom);
	return 0;
}

int CTtermsize() {
	struct winsize size;
	if(ioctl(0, TIOCSWINSZ, (char *)&size)) {
		return 1;
	}
	CTwidth = size.ws_row;
	return 0;
}

void CTpromtupdate() {
	pthread_mutex_lock(&CTiom);
	printf("\r%*s\r", (int)CTcmdbufUsed+(int)CTcmdpromtLen, "");
	printf("%s%s", CTcmdpromt, CTcmdbuf);
	pthread_mutex_unlock(&CTiom);
	return;
}

void CTpromtupdateNM() {
	printf("\r%*s\r", (int)CTcmdbufUsed+(int)CTcmdpromtLen, "");
	printf("%s%s", CTcmdpromt, CTcmdbuf);
	return;
}

void CTprintf(const char* format, ...) {
	if(CTinitDone) {
		pthread_mutex_lock(&CTiom);
		printf("\r%*s\r", (int)CTcmdbufUsed+(int)CTcmdpromtLen, "");
		va_list args;
		va_start(args, format);
		size_t used = vprintf(format, args);
		va_end(args);
		printf("\n%s%s", CTcmdpromt, CTcmdbuf);
		pthread_mutex_unlock(&CTiom);
	} else {
		va_list args;
		va_start(args, format);
		size_t used = vprintf(format, args);
		va_end(args);
	}
	return;
}

int CTinit(void (*cmd)(char*)) {
	CTsetCommander(cmd);
	CTsetPromt(NULL);
	setbuf(stdout, 0);
	setbuf(stdin, 0);
	struct termios tattr;
	if(!isatty(STDIN_FILENO)) {
		return 1;
	}
	if(tcgetattr(STDIN_FILENO, &CTattr)) {
		return 1;
	}
	if(tcgetattr(STDIN_FILENO, &tattr)) {
		return 1;
	}
	tattr.c_lflag &= ~(ICANON|ECHO|ISIG);
	tattr.c_cc[VMIN] = 0;
	tattr.c_cc[VTIME] = 1;
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr)) {
		CTinitDone = false;
		return 1;
	}
	CTcmdbuf = (char*)malloc((CTmaxBufferSize+1)*sizeof(char));
	CTcmdbufLen = CTmaxBufferSize+1;
	memset(CTcmdbuf, 0, CTcmdbufLen);
	CTcmdbufUsed = 0;
	pthread_create(&CTreaderT, NULL, &CTreader, NULL);
	CTinitDone = true;
	return 0;
}

int CTexit() {
	if(CTinitDone) {
		CTprintf("CT shutdown.\n");
		CTSetReaderControllState(-1);
		pthread_join(CTreaderT, NULL);
		free(CTcmdbuf);
		CTcmdbufUsed = 0;
		CTcmdbufLen = 0;
		if(tcsetattr(STDIN_FILENO, TCSANOW, &CTattr)) {
			return 1;
		}
		printf("\n");
		CTinitDone = false;
	}
	return 0;
}

void* CTreader(void* a) {
	while(1) {
		switch(CTGetReaderControllState()) {
			case 0:
			{
				char c;
				if(read(STDIN_FILENO, &c, 1) == 1) {
					switch(c) {
						case '\n':
						{
							pthread_mutex_lock(&CTiom);
							if(CTcommander != NULL) {
								CTcommander(strdup(CTcmdbuf));
							}
							memset(CTcmdbuf, 0, CTcmdbufLen);
							CTpromtupdateNM();
							CTcmdbufUsed = 0;
							pthread_mutex_unlock(&CTiom);
						}
						break;
						case 127:
						pthread_mutex_lock(&CTiom);
						if(CTcmdbufUsed > 0) {
							CTcmdbuf[strlen(CTcmdbuf)-1] = '\0';
							CTcmdbufUsed--;
							printf("\b \b");
						}
						pthread_mutex_unlock(&CTiom);
						break;
						default:
						pthread_mutex_lock(&CTiom);
						CTcmdbuf[strlen(CTcmdbuf)] = c;
						CTcmdbufUsed++;
						printf("%c", c);
						pthread_mutex_unlock(&CTiom);
						break;
					}
				}
			}
			break;
			case -1:
			pthread_exit(0);
			break;
		}
	}
	return NULL;
}
