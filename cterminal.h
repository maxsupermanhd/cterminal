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
