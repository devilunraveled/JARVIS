#include <stdio.h>
#include <unistd.h>
#include "my_pwd.h"
#include "../Logger/logger.h"
#include "../macros.h"

int pwd( char *s, char *command )
{
    // Checking if command is valid.
    if ( *s == '\0' )
        return -1;
    while ( *command != '\0' )
    {
        if ( *command != ' ' && *command != '\t' )
        {
            Error("Too many Arguments for 'pwd' \n");
            return -1;
        }
        command++;
    }


    char cwd[MAX_PATH_LENGTH];
    printf(BLUE);
    printf("%s\n", getcwd(cwd,MAX_PATH_LENGTH));
    printf(RESET);
    return 0;
}
