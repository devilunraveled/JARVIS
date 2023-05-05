#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <termios.h>

#include "../stark.h"
#include "../Logger/logger.h"
#define MAX_BACKGROUND_PROCESS 200

extern int errno;
extern Process BackProc[MAX_BACKGROUND_PROCESS];
extern int num_processes;
extern int jarvis_terminal;
extern struct termios jarvis_tmodes;

int giveNums( char *command, int *first, int *second, int req);

int sig( char *command )
{
    int p_errno = errno;
    while( *command == ' ' || *command == '\t' )
        command++;
    
    int procNum = 0, sigId = 0;
    int er = giveNums( command, &procNum, &sigId, 2);
    if ( er == -1 )
        return -1;
    
    for( int i = 0; i < num_processes; i++ )
    {
        if ( BackProc[i].valid == 1 )
        {
            if ( BackProc[i].num == procNum )
            {
                int k_er = kill( BackProc[i].proc_id, sigId);
                if ( k_er == -1 )
                {
                    perror("Could Not send the signal");
                    errno = p_errno;
                    return -1;
                }
                // errno = p_errno;
                return 0;
            }
        }
    }

    Error("NO process with that Process-Number exists.\n");
    // errno = p_errno;
    return -1;
}

int giveNums( char *command, int *first, int *second, int req)
{ 
    int len = strlen( command );
    char *second_arg="\0", *first_arg = command;
    
    for ( int i = 1; i < len - 1; i++ )
    {
        if ( command[i] == ' ' || command[i] == '\0' )
        {
            command[i] = '\0';
            second_arg = &command[i + 1];
        }
    }

    *first = (int)strtol( first_arg, NULL, 10);
    if ( req == 2 )
        *second = (int)strtol( second_arg, NULL, 10);

    if ( req >= 1 && *first_arg == '\0' )
    {
        Error("Missing Arguments.");
        return -1;
    }
    else if ( req == 2 && *second_arg == '\0' )
    {
        Error("Invalid Arguments.\n");
        return -1;
    }
    
    return 0;
}

int bringforw( int pr_id , int *eta )
{
    tcsetpgrp(jarvis_terminal, pr_id);

    int wstat = 1;

    time_t s = time(NULL);
   
    kill( pr_id, SIGCONT);
    // Sends the continue signal to the process if it was stopped.
    signal( SIGCHLD, SIG_IGN );
    waitpid( pr_id, &wstat, WUNTRACED );
    signal( SIGCHLD, SIG_DFL );
        
    *eta = time(NULL) - s;
    
    tcsetpgrp(jarvis_terminal, getpid());
    
    if ( WIFSTOPPED( wstat ) )
        return 2;

    if( wstat == -1 )
    {
        Error("Child process ended with errors.\n");
        return -1;
    }

    tcsetattr( jarvis_terminal, 0, &jarvis_tmodes );
    return 1;
}

int fg( char *command, int *eta )
{
    if ( *command != '\0' && *command != ' ' && *command != '\t' )
    {
        Error("Invalid Command.\n");
        return -1;
    }
    while ( *command == ' ' || *command == '\t' )
        command++;
    
    int proc_num = 0;

    int er = giveNums( command, &proc_num, NULL, 1);
    
    if ( er == -1 )
        return -1;
    
    for( int i = 0; i < num_processes; i++ )
    {
        if ( BackProc[i].valid == 1 )
        {
            if ( BackProc[i].num == proc_num )
            {
                int bf_er = bringforw( BackProc[i].proc_id, eta);
                
                BackProc[i].valid = 0;
                if ( bf_er == -1 )
                    return -1;

                else if ( bf_er == 2 )
                {
                    BackProc[i].valid = 1;
                    BackProc[i].run = 0;
                }
                return 0;
            }
        }
    }
    
    Error("No Process with that Process Number exists.\n");
    return 0;
}

int bg( char *command )
{
    if ( *command != '\0' && *command != ' ' && *command != '\t' )
    {
        Error("Invalid Command.\n");
        return -1;
    }
    while ( *command == ' ' || *command == '\t' )
        command++;
    
    int proc_num = 0;

    int er = giveNums( command, &proc_num, NULL, 1);
    
    if ( er == -1 )
        return -1;
    
    for( int i = 0; i < num_processes; i++ )
    {
        if ( BackProc[i].valid == 1 )
        {
            if ( BackProc[i].num == proc_num )
            {
                int k_er = kill( BackProc[i].proc_id, SIGCONT);
                
                if ( k_er == 1 )
                    return -1;
                
                BackProc[i].run = 1;
                return 0;
            }
        }
    }
    
    
    Error("No Process with that Process Number exists.\n");
    return 0;
}
