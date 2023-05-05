#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>
#include <termios.h>
#include "stark.h"

#define MAX_BACK_PROCESSES 512

extern struct termios jarvis_tmodes; 
extern int jarvis_terminal;
extern void handle_sigs( int sig, siginfo_t* S, void *ucontext );
extern int need_to_print;

Process BackProc[MAX_BACK_PROCESSES];
int num_processes = 0;

int extract_args( char *command, char *args[MAX_COMMANDS_PER_LINE] );

int choose( char* command, int p_mode ,char* curr_path, char* prev_path, const char* home, int *zen_mode, int *eta )
{
    // redirect( command, p_mode, curr_path, prev_path, home, zen_mode, eta);

    while ( *command == '\n' )
        command++;
    int len = strlen(command);
    int ret_val = 0;
    
    // Every command, once successfully parsed is stored in the history.
    
    // printf("P_mode : %d\n", p_mode);
    // Breaking the loop
    // filter( command );

    if ( len == 0 )
        return 1;
        
    // printf("Cum :|%s|\n", command);
    if ( len == 4 && memcmp(command, "exit", 4 ) == 0 )
        return 0;    
    else if ( len >= 5 && memcmp(command, "__zen", 5) == 0 )
    {
        *zen_mode = 1 - *zen_mode;
        if (*zen_mode == 1) Success("Zen mode activated.\n");
        else Success("Zen mode de-activated.\n");
        return 1;
    }
    // Checking for the functions.
    // A function is allowed to have empty arguments as 
    // the strings are terminated with the null character.
    else if ( ( len >= 4 ) && ( memcmp( "echo", command, 4 ) == 0 ) )
        ret_val = echo( &command[4] );
    else if ( ( len >=  3) && ( memcmp( "pwd", command, 3) == 0) )
    {
        ret_val =  pwd( curr_path, &command[3] );
    }
    else if ( ( len >=  4) && ( memcmp( "jobs", command, 4) == 0) )
    {
        // printf("%s--%s\n", command, &command[4]);
        ret_val =  jobs( &command[4] );
    }
    else if ( ( len >= 2 ) && ( memcmp( "cd", command, 2) == 0 ) )
    {
        ret_val = cd( &command[2], curr_path, prev_path, home ); 
    }
    else if ( ( len >= 5 ) && ( memcmp( "clear", command, 5) == 0 ) )
    {
        ret_val = clear( &command[5] );
    }
    else if ( ( len >= 2) && ( memcmp( "ls", command, 2) == 0 ) )
    {
        ret_val = ls ( &command[2], curr_path, home );
    }
    else if ( ( len >= 7 ) && ( memcmp( "history", command, 7 ) == 0 ) )
    {
        ret_val = history( &command[7] , MAX_HISTORY );
    }
    else if ( ( len >= 8 ) && ( memcmp( "discover", command, 8 ) == 0 ) )
    {
        ret_val = discover( &command[8], curr_path, home );
    }
    else if ( ( len >= 5 ) && ( memcmp( "pinfo", command, 5) == 0 ) )
    {
        ret_val = pinfo( &command[5], home );
    }
    else if ( ( len >=  3) && ( memcmp( "sig", command, 3) == 0) )
    {
        need_to_print = 1;
        ret_val =  sig( &command[3] );
    }
    else if ( ( len >= 2 ) && ( memcmp( "bg", command, 2) == 0 ) )
    {
        need_to_print = 1;
        ret_val = bg( &command[2] );
    }
    else if ( ( len >= 2 ) && ( memcmp( "fg", command, 2) == 0 ) )
    {
        need_to_print = 0;
        ret_val = fg( &command[2], eta ); 
    }
    else
    {
        need_to_print = 1;
        while ( *command == ' ' || *command == '\t' )
            command++;

        if ( *command == '\0' )
        {
            Warning("Empty Instruction.\n");
            exit(EXIT_FAILURE);
        }

        char *args[MAX_COMMANDS_PER_LINE];
    
        char actualcomm[COMMAND_LENGTH];
        memcpy( actualcomm, command, strlen(command) + 1 );
        int num_args = extract_args( command, args );

        args[num_args] = NULL;
        
        int f_val = fork();
        // Creates a copy of the current process.
        
        if ( f_val != 0 )
        {
            if( p_mode == 0 )
            {
                setpgid(f_val, f_val);

                tcsetpgrp(jarvis_terminal, f_val);

                int wstat = 1;

                time_t s = time(NULL);
                
                signal( SIGCHLD, SIG_IGN );
                waitpid( f_val, &wstat, WUNTRACED);
                tcsetpgrp( jarvis_terminal, getpid() );

                struct sigaction act;
                sigemptyset( &act.sa_mask );

                act.sa_flags = SA_SIGINFO | SA_RESTART ;
                act.sa_sigaction = handle_sigs;

                int sex = sigaction( SIGCHLD, &act, NULL );

                if ( sex == -1 )
                    perror("Signal");
                /* End of sigaction handling.*/
                // tcsetattr( jarvis_terminal, 0, &jarvis_tmodes);
                *eta = time(NULL) - s;

                if ( WIFSTOPPED( wstat ) )
                {
                    // Ctrl + Z is pressed.
                    // Process is sent to the background.
                    // Needs to be added in the background list.
                    
                    for ( int i = 0; i < MAX_BACK_PROCESSES; i++)
                    {
                        if ( BackProc[i].valid == 0 )
                        {
                            memcpy( BackProc[i].proc_name, actualcomm, strlen(actualcomm) + 1 );
                            memcpy( BackProc[i].comm_name, command, strlen(command) + 1);
                            BackProc[i].proc_id = f_val;
                            BackProc[i].valid = 1;
                            BackProc[i].run = 0;
                            BackProc[i].num = i + 1;
                            if ( i == num_processes )
                                num_processes++;
                            printf("[%d] %d\n", i + 1, f_val);
                            break;
                        }
                    }
                }

                if( wstat == -1 )
                {
                    Error("Child process ended with errors.\n");
                    exit(EXIT_FAILURE);
                }

                return 1;
            }
            else if ( p_mode == 1 )
            {
                setpgid(f_val, f_val);
                // Parent process with a background command.
                for ( int i = 0; i < MAX_BACK_PROCESSES; i++)
                {
                    if ( BackProc[i].valid == 0 )
                    {
                        memcpy( BackProc[i].proc_name, actualcomm, strlen(actualcomm) + 1 );
                        memcpy( BackProc[i].comm_name, command, strlen(command) + 1);
                        BackProc[i].proc_id = f_val;
                        BackProc[i].valid = 1;
                        BackProc[i].run = 1;
                        BackProc[i].num = i + 1;
                        if ( i == num_processes )
                            num_processes++;
                        printf("[%d] %d\n", i + 1, f_val);
                        break;
                    }
                }
            }
        }
        else if ( f_val == 0 )
        {
            // Process is to be ran in foreground or background.
            signal (SIGTSTP, SIG_DFL);   
            signal (SIGTTIN, SIG_DFL);
            signal (SIGTTOU, SIG_DFL);
            signal (SIGINT, SIG_DFL);
            signal (SIGQUIT, SIG_DFL);
            if(p_mode == 0) tcsetpgrp(jarvis_terminal, f_val);
            
            execvp( args[0], args);
            
            Error("No Command found.");
            printf(": %s\n", command);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }


    if ( ret_val != 0 )
        return ret_val;
    else 
        return 1;
}

char* filter( char *command )
{
    char *start = command;
    char *end = (command + strlen(command) - 1 );
    
    while ( *start == ' ' || *start == '\t' )
        start++;
    
    if ( start == end && ( *start != '\0' ) )
    {
        command[1] = '\0';
        return command;
    }

    for (int i = 0;;i++)
    {
        // fgets reads the newline character, so we override that with the null chr.
        if ( start + i == end )
        {
            //Single character strings would have just the null character.
            command[i] = '\0';
            break;
        }
        command[i] = start[i];
    }

    return command;
}

void abs_to_rel( char* curr_path, const char* home)
{
    int home_len = strlen(home);

    if ( ( (int)strlen(curr_path) >= home_len ) && memcmp(home,curr_path, home_len) == 0 )
    {
        curr_path[0] = '~';
        for( int i = 1; ;i++)
        {
            curr_path[i] = curr_path[i + home_len - 1];
            if ( curr_path[i] == '\0' )
                break;
        }
    }
}


char* rel_to_abs( char *s , const char* home)
{
    char temp[MAX_PATH_LENGTH] = "";
    int n = strlen(s),j = 0;

    for( int i = 0; i < n; i++ )
    {
        if ( s[i] == '~')
        {
            j += strlen(home); 
            strncat( temp, home, strlen(home) );
        }
        else
        {
            temp[j++] = s[i];
        }
    }

    memmove(s,temp,j);
    s[j++] = '\0';
    return s;
}

int extract_args( char *command, char *args[MAX_COMMANDS_PER_LINE] )
{
    // A modification to the inbuilt strtok/strtok_r function.
    int i = 1, j = 0, len = strlen(command);
    command[len] = '\0';

    args[0] = command;
    for ( j = 0; j < len ;  j++ )
    {
        while( j < len && ( command[j] == ' ' || command[j] == '\t' ) )
        {
            command[j] = '\0'; 
            if ( j < len - 1 && ( command[j+1] != ' ' && command[j+1] != '\t' ) )
                args[i++] = &command[j+1];
        }
    }
    args[i] = command + strlen(args[ i - 1 ]) + 1; 
    return i;
}
