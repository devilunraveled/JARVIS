#include <stdio.h>
#include "../stark.h"
#include <string.h>
#include <stdlib.h>

#define MAX_BACK_PROCESSES 512
#define RUNNING 1
#define STOPPED 2

extern Process BackProc[MAX_BACK_PROCESSES];
extern int num_processes;

int pModes( char* command );

int comparator( const void * a, const void * b )
{
    return strcasecmp( ((const Process *)a)->comm_name, ((const Process *)b)->comm_name );
}

int jobs( char *command )
{  
    // printf( "%s\n", command );
    while( *command == ' ' || *command == '\t' )
        command++;
    
    int flag_mode = pModes(command);
    if ( flag_mode == 0 ) flag_mode = 3; 
    while( *command == ' ' || *command == '\t' )
        command++;
    
    if ( *command != '\0' )
    {
        Error("Command 'jobs' does not take any arguments.\n");
        return -1;
    }
    int j = 0;
    
    Process instances[MAX_BACK_PROCESSES] = {'\0'};

    for(int i = 0; i <= num_processes; i++ )
        if ( BackProc[i].valid == 1 )
            instances[j++] = BackProc[i];
    
    qsort( instances, j, sizeof( Process ), comparator );

    for ( int k = 0; k < j; k++ )
    {
        if ( instances[k].run && ( flag_mode & RUNNING ) )
            printf("[%d] Running %s [%d]\n", instances[k].num, instances[k].proc_name, instances[k].proc_id);
        if ( (!instances[k].run) && (flag_mode & STOPPED ) )
            printf("[%d] Stopped %s [%d]\n", instances[k].num, instances[k].proc_name, instances[k].proc_id);
    }
    return 0;
}  

int pModes( char* command )
{
    int len_command = strlen(command);

    char new_command[ len_command + 5];
    int list_mode = 0, hidden_mode = 0;
    int j = 0;

    for( int i = 0; i < len_command ; i++ )
    {
        if ( ( i == 0 || command[i-1] == ' ' || command[i-1] == '\t' ) && command[i] == '-' )
        {
            if ( command[ i + 1 ] == 'r' )
            {
                hidden_mode = 1;
                if ( ( i == len_command - 3 && command[ i + 2 ] == 's' )  || ( i < len_command - 3 && command[ i + 2 ] == 's' && ( command[ i + 3] == ' ' || command[ i + 3 ] == '\t' )))
                {
                    i++;
                    list_mode = 1;
                }
                i++; // Ignoring the next element.
            }
            else if ( command[ i + 1 ] == 's')
            {
                list_mode = 1;
                if ( ( i == len_command - 3 && command[ i + 2 ] == 'r' )  || ( i < len_command - 3 && command[ i + 2 ] == 'r' && ( command[ i + 3] == ' ' || command[ i + 3 ] == '\t' )) )
                {
                    i++;
                    hidden_mode = 1;
                }
                i++;
            }
            else
            {
                if ( j > 0 && ( new_command[j - 1] == ' ' || new_command[j - 1] == ' ' ) && ( command[ i ] == '\t' || command[i] == ' ') )
                    continue;
                new_command[ j++ ] = command[ i ];
            }
        }
        else
        {
            if ( j > 0 && ( new_command[j - 1] == ' ' || new_command[j - 1] == ' ' ) && ( command[ i ] == '\t' || command[i] == ' ') )
                continue;
            new_command[ j++ ] = command[ i ];
        }
        // No changes to the command.
    }
    new_command[j] = '\0';
    memcpy( command, new_command, j + 1);
    //command[j] = '\0';

    return ( 2*list_mode + hidden_mode )%3;
}
