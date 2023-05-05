#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "pinfo.h"
#include "../Logger/logger.h"
#include "../utils.h"

typedef struct proc
{
    int pid;
    char Status;
    char mode;
    int memory;
    char *exec_path;

}Proc;


int pinfo( char* command, const char *home )
{
    if ( *command != ' ' && *command != '\t' && *command != '\0' )
    {
        Error( "Invalid Command " );
        return -1;
    }
    while( *command == ' ' || *command == '\t' )
        command++;

    Proc *Process = ( Proc * )malloc( sizeof( Proc ) );

    if ( *command == '\0' )
    {
        // Command was entered without any arguments.
        // Self pinfo is required.
        char *self = "self";
        memmove( command, self, strlen(self) + 1 );
    }

    // At this point either the pid is set or self is replaced
    char address[100] = "/proc/";
    char stat[8] = "/stat";
    strncat( address, command, strlen(command) );
    strncat( address, stat, strlen(stat));

    FILE *pr = fopen( address, "r" );
    char f_name[1024] = "\0";

    if ( pr == NULL )
    {
        printf("The process does not exist.\n");
        free(Process);
        return -1;
    }
    
    int a,b,c,d,e;

    fscanf( pr, "%d %s %c %d %d %d %d %d ", &( Process->pid ), f_name ,&( Process->Status ), &a, &b, &c, &d, &e );

    // printf( "Foreground process ID : %d\nGroup ID : %d", b, e);
    // gets the process id of the given process.
    
    if ( e == b )
    {
        Process->mode = '+';
    }
    else
    {
        Process->mode = '\0';
    }

    char address2[100] = "/proc/";
    char statm[8] = "/statm";
    strncat( address2, command, strlen(command) );
    strncat( address2, statm, strlen(statm) );
    FILE *mem = fopen( address2, "r" );

    if ( pr == NULL )
    {
        printf("The process does not exist.\n");
        free(Process);
        return -1;
    }
    fscanf( mem, "%d", &( Process->memory ) );
    
    char address3[100] = "/proc/";

    char exe[8] = "/exe";
    strncat( address3, command, strlen(command) );
    strncat( address3, exe, strlen(exe) );
    // printf("Address 3 Calculated : %s\n", address3); 
    
    struct stat *S = ( struct stat* )malloc( sizeof( struct stat ) );
    int l = lstat( address3, S );

    if ( l == -1 )
    {
        perror("Could not read the sym-link");
        free( S );
        return -1;
    }

    char buffer[256];
    if ( !S_ISLNK( S->st_mode ) )
    {
        // The path pointed is not a regular link.
        Error("The sym-link for the process doesn't exist.");
        free(S);
        return -1;
    }
    else
    {
        // The sym-link is correctly located.
        int r = readlink( address3, buffer, 256 );
        if ( r == -1 )
        {
            perror("Could not retrieve the executable from the sym-link.");
            free(S);
            return -1;
        }
        else
        {
            buffer[r] = '\0'; 
        }
    }

    abs_to_rel( buffer, home );

    printf("pid : %d\n", Process->pid);
    printf("process status : %c%c\n", Process->Status, Process->mode);
    printf("memory : %d\n", Process->memory);
    printf("executable path : %s\n", buffer);
    
    free(S);
    free(Process);

    fclose( mem );
    fclose( pr );
    return 0; 
}

