/*
 * Terminal safer v1.0
 *  - Your input got screwed because of multithreaded IO operations?
 *  - This is for you!
 * Not breakig user input.
 * Promt for an imput.
 * Command interpreter.
 * UNIX ONLY!
 * Compile with: gcc ts1.c -o ts1 -lpthread
*/
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <termios.h>

#if !defined(unix) && !defined(__unix__) && !defined(__unix)
# error not supported other than unix
#endif

#define BUFSIZE 1024
#define PROMT "->:"

struct termios tattr_saved;
char cmdbuf[BUFSIZE] = {0};
size_t cmdbufo = 0;
const char *cmdpromt = PROMT;
const size_t cmdpromtl = strlen(PROMT);
pthread_mutex_t iom = PTHREAD_MUTEX_INITIALIZER;

int needexit = 0;
pthread_mutex_t exitm = PTHREAD_MUTEX_INITIALIZER;

void mpromtupdate() {
	pthread_mutex_lock(&iom);
	printf("\r%*s\r", (int)cmdbufo+(int)cmdpromtl, "");
	printf("%s%s", cmdpromt, cmdbuf);
	pthread_mutex_unlock(&iom);
	return;
}

void mprintf(const char* format, ...) {
	pthread_mutex_lock(&iom);
	printf("\r%*s\r", (int)cmdbufo+(int)cmdpromtl, "");
	va_list args;
	va_start(args, format);
	size_t used = vprintf(format, args);
	va_end(args);
	printf("\n%s%s", cmdpromt, cmdbuf);
	pthread_mutex_unlock(&iom);
	return;
}

void terminit() {
	setbuf(stdout, 0);
	setbuf(stdin, 0);
	struct termios tattr;
	char *name;
	if(!isatty(STDIN_FILENO)) {
		fprintf(stderr, "Not a terminal.\n");
		exit(EXIT_FAILURE);
	}
	tcgetattr(STDIN_FILENO, &tattr_saved);
	tcgetattr(STDIN_FILENO, &tattr);
	tattr.c_lflag &= ~(ICANON|ECHO);
	tattr.c_cc[VMIN] = 1;
	tattr.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
	return;
}

void termexit() {
	tcsetattr(STDIN_FILENO, TCSANOW, &tattr_saved);
	printf("\n");
	return;
}

void commander() {
	char cmdebuf[BUFSIZE];
	strncpy(cmdebuf, cmdbuf, strlen(cmdbuf));
	cmdebuf[strlen(cmdbuf)] = '\0';

	memset(cmdbuf, 0, BUFSIZE);
	mpromtupdate();
	cmdbufo = 0;
	if(strcmp(cmdebuf, "exit") == 0) {
		pthread_mutex_lock(&exitm);
		needexit = 1;
		pthread_mutex_unlock(&exitm);
		mprintf("Exiting...");
	}
	return;
}

void* termreader(void* a) {
	char c;
	while(read(STDIN_FILENO, &c, 1) == 1) {
		switch(c) {
			case '\n':
			commander();
			break;
			case 127:
			cmdbuf[strlen(cmdbuf)-1] = '\0';
			cmdbufo--;
			printf("\b \b");
			break;
			default:
			cmdbuf[strlen(cmdbuf)] = c;
			cmdbufo++;
			printf("%c", c);
			break;
		}
	}
	return NULL;
}

void* spammer(void* a) {
	for(int i=0; ; i++) {
		mprintf("I'm random log string, blah. (%d)", i);
		usleep(rand()%900000);
		pthread_mutex_lock(&exitm);
		if(needexit) {
			pthread_mutex_unlock(&exitm);
			break;
		}
		pthread_mutex_unlock(&exitm);
	}
	return NULL;
}

int main(int argc, char const *argv[]) {
	terminit();

	pthread_t termt;
	pthread_create(&termt, NULL, termreader, NULL);

	pthread_t spamt;
	pthread_create(&spamt, NULL, spammer, NULL);

	pthread_join(spamt, NULL);

	pthread_mutex_destroy(&iom);
	pthread_mutex_destroy(&exitm);
	termexit();
	return 0;
}
