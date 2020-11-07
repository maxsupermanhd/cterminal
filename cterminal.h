#ifndef CTERMINAL_H
#define CTERMINAL_H

#if !defined(unix) && !defined(__unix__) && !defined(__unix)
# error not supported other than unix
#endif

extern const size_t CTmaxBufferSize;  // Default command buffer size (excluding 0)
extern const char* CTdefaultPromt;   // Default promt

/* CTinit( Commander function )
 *  - Starts reader thread
 *  - Sets terminal state
 *  - Sets commander (calls CTsetCommander)
 * Return: 0 - success
 *         1 - fail */
int CTinit(void (*cmd)(char*));

/* CTexit()
 *  = Requires init.
 *  - Restoring terminal state back to normal
 * Return: 0 - success
 *         1 - fail */
int CTexit();

/* CTsetCommander( Commander function )
 *  - Sets commander function
 * NOTE: you can use NULL to disable commander at all
 * NOTE: will be overrided by init operation if done before init
 * Return: 0 - success
 *         1 - fail */
void CTsetCommander(void (*cmd)(char*));

/* CTsetPromt( Null-terminated string )
 * - Sets promt (char sequence before command field) */
int CTsetPromt(char*);

/* CTpromtUpdate()
 *  = Requires init.
 *  - Updates command promt entierly, cleaning and reprinting both command
 *     buffer and promt before. Should be called after CTsetPromt. */
void CTpromtupdate();

/* CTprintf( printf formatting )
 *  = Requires init.
 *  - Prints a line into console without breaking user input. Passes all
 *     arguments to vprintf. */
void CTprintf(const char* format, ...);

#endif
