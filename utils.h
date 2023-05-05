#ifndef __UTILS_H_
#define __UTILS_H_

#include <signal.h>
#include "macros.h"


typedef struct Proces Process;
struct Proces
{
    char proc_name[MAX_FILE_NAME];
    pid_t proc_id;
    int valid;
};

char *filter( char* s );
// Removes trailing and leading spaces & tablines in a one line command.

int choose( char* command, int p_mode, char* curr_path, char * prev_path, const char* home, int *zen_mode );
// The function that chooses what to do based on the input commands

void abs_to_rel( char* curr_path, const char* home);
// Updates the current path and the previous path based on the target given by the user.

char* rel_to_abs( char *, const char * );
#endif
