#ifndef __HISTORY_
#define __HISTORY_

/*
 * Using HISTORY:
 * 1. Read the last line from the history file.
 * 2. Check if it matches with the given command.
 * 3. If yes, append it to the file, else move on.
 * 4. Close the file.
*/

int history( char* command, int len );
void push_history( char* command );
#endif
