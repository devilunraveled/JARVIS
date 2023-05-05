#include <stdio.h>
#include "my_clear.h"
#include "../Logger/logger.h"
#include "../macros.h"

int clear( char* command )
{
    char *next = command;

    while( *next != '\0' )
    {
        if ( *next != ' ' && *next != '\t' )
        {
            char buffer[COMMAND_LENGTH];
            sprintf(buffer, "Commnad 'clear' has no argument '%s'.\n", next);
            Error(buffer);
            
            return -1;
        }
        next++;
    }
    
    int p = printf("\e[1;1H\e[2J");
    
    return p;
}
