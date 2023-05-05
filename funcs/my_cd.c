#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "my_cd.h"
#include "../Logger/logger.h"
#include "../utils.h"
#include "../macros.h"
int cd( char *target, char* curr_path, char* prev_path, const char* home )
{
    char * next = target;
    
    while ( *next == ' ' || *next == '\t' )
        next++;
    
    rel_to_abs( next, home);
    rel_to_abs( curr_path, home);
    rel_to_abs( prev_path, home);
    if ( next[0] == '\0' )
    {
        int r = chdir( home );
        if ( r == -1 )
        {
            char buff[100];
            sprintf(buff,"The path %s does not exist anymore.\n", home);
            Error(buff);
            return -1;
        }
    }
    else if ( next[0] == '-')
    {
        int r_p = chdir( prev_path );
        if ( r_p == -1 )
        {
            char buff[100];
            sprintf(buff,"The path %s does not exist anymore.\n", prev_path);
            Error(buff);
            return -1;
        }
        else
        {
            printf(YELLOW);
            printf("%s\n", prev_path );
            printf(RESET);
        }
    }
    else
    {
        int ret_chdir = chdir( next );
        if ( ret_chdir == -1 )\
        {
            char buff[100];
            sprintf(buff,"The path %s does not exist.\n", next);
            Error(buff);
            return -1;
            // Error("The directory %s.\n");
        }
    }
    // Correcting the previous path.
    memmove(prev_path,curr_path,MAX_PATH_LENGTH);
    // Getting the new path.
    char buff[MAX_PATH_LENGTH];
    getcwd(buff,MAX_PATH_LENGTH);
    // Changing the current path.
    memmove(curr_path,buff,MAX_PATH_LENGTH);
    curr_path[strlen(buff)] = '\0'; 
    // printf("%s\n", curr_path);
    return 0;
}
