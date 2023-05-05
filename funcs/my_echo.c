#include <stdio.h>
#include <string.h>
#include "my_echo.h"
#include "../Logger/logger.h"
#include "../utils.h"

// The recieved string s should be suffixed with a NULL character.

int echo( const char *s )
{
    // If all spaces or tabs, print newline character.
    // If there is atleast one non-space ascii, ptint the entire string.
    if ( s[0] == '\0' )
    {
        // The case of empty string.
        printf("\n");
        return 0;
    }
    if ( s[0] != '\0' && s[0] != ' ' && s[0] != '\t' )
    {
        char buff[1000];
        sprintf( buff, "Command echo%s not found.\n", s);
        Error( buff);
        return -1;
    }
    
    char temp[ strlen( s ) ];
    int j = 0;
    int i = 1;
    
    for ( ; s[i] != '\0'; i++)
    {
        if ( j >= 1 && ( s[i] == ' ' || s[i] == '\t' ) && ( temp[ j - 1 ] == ' ' || temp[ j - 1 ] == '\t') )
            continue;
        else if ( s[i] == ' ' || s[i] == '\t') 
            temp[j++] = ' ';
        else
            temp[j++] = s[i];
    }
    
    temp[j] = '\0';

    // Instead of returning, we will print the shit out.
    // return (s[i]);
    printf( "%s\n", temp );
    
    return 0;
}
