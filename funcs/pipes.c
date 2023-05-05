#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>

#include "../macros.h"
#include "../stark.h"

extern char* filterCMDs( char *command );
extern struct termios jarvis_tmodes;

int doPipeing( char* command, int p_mode, char *curr_path, char* prev_path ,const char* home, int *zen_mode, int *eta )
{
    // A function that takes input from a particular file-descriptor.
    // Gives output to a particular file descriptor.
    // Would call the next command recursively.
    // Null terminated string array.
    
    int len = strlen(command);

    char temp_command[ COMMAND_LENGTH + 10 ]={'\0'};
    if ( p_mode == 1 )
    {
        strcat(temp_command,command);
        temp_command[ len ] = '&';
        temp_command[ len + 1 ] = '\0';
        push_history( temp_command );
    }
    else
        push_history(command);

    int iPrev = 0;
    
    int fileDes[2] = {-1,-1};
    
    int origIN, origOUT;
    origIN = dup( STDIN_FILENO );
    origOUT = dup( STDOUT_FILENO );
/*
 * Consider the process a | b | c.
 * The logic to implement is as follows :
 * Till the first pipe is encountered,
 * continue reading the command as a single instance of pipe.
 * When a pipe is encountered, send the command before the pipe as a child process.
 * The pipe function, after the first call would give the file descriptors f1 and f2.
 * f1 being a read fd and f2 being a write fd.
 * After pipe1 is encountered, a has input from STDIN, so we close f1 in child.
 * The output is set to f2, since that stream is allocated for the pipe.
 * After the command a is processed in the child, 
 * the output filedescriptor is changed to STDOUT.
 * Meanwhile in the parent, the same f1 and f2 are allocated,
 * Since the parent does not need the output of this pipe, parent closes f2.
 * Now,the buffer allocated by pipe command has input in it.
 * To read this input, the parent would require f1.
 * The parent waits for a child to execute, then calls the dup2 function
 * The dup2 function would map STDIN to f1, which is used for the next process.
 * */


    for( int i = 0; i < len; i++ )
    {
        if ( command[i] == '|' )
        {
            if ( i == len - 1 )
            {
                Error("Invalid Command.\n");
                return -1;
            }
            command[i] = '\0';

            if ( pipe( fileDes ) == -1 )
            {
                Error("Too many pipes");
                perror("");
                return -1;
            }

            int getPID = fork();
            
            if ( getPID == -1 )
            {
                Error("Too many processes created.");
                perror("");
                return -1;
            }

            if ( getPID == 0 )
            {
                setpgid( getPID, getPID );
                // Creates the child process group.
                tcsetpgrp( STDIN_FILENO, getpid() );
                // Gives the control to child.
                
                close( fileDes[0] ); // The child
                dup2( fileDes[1], STDOUT_FILENO );
                // Takes the command input by default through STDIN.
                if ( command[iPrev] == '\0' )
                {
                    Warning("Skipping Empty Instriction.\n");
                    iPrev = i + 1;
                    dup2( origOUT, STDOUT_FILENO );
                    continue;
                }

                int r = redirect(&command[iPrev], p_mode, curr_path, prev_path, home, zen_mode, eta );
                
                dup2( origOUT, STDOUT_FILENO );
                
                if ( r == -1 )
                    exit(EXIT_FAILURE);
                else
                    exit(EXIT_SUCCESS);
            }
            else
            {
                setpgid( getPID, getPID );

                tcsetpgrp( STDIN_FILENO, getPID );
                
                close( fileDes[1] );
                
                int childStatus = 1;

                int waitErr = waitpid( getPID, &childStatus, WUNTRACED );
                // WCONTINUED flag is not raised since 
                // a process in pipeline is not continued.
                
                tcsetpgrp( STDIN_FILENO, getpid() );
                dup2( fileDes[0], STDIN_FILENO );
                
                // tcsetattr( STDIN_FILENO, 0, &jarvis_tmodes);
                
                if ( childStatus == -1 || waitErr == -1 )
                {
                    perror("");
                    Error("Something bad happened.\n");
                    dup2( origIN, STDIN_FILENO);
                    dup2( origOUT, STDOUT_FILENO);
                    return 1;
                }
            }

            iPrev = i + 1;
        }
        else if ( i == len - 1 )
        {
            // No Stress.
            int r = redirect(&command[iPrev], p_mode, curr_path, prev_path, home, zen_mode, eta );
            // tcsetattr( origIN, 0, &jarvis_tmodes);
            dup2( origIN, STDIN_FILENO);
            dup2( origOUT, STDOUT_FILENO);
            return r;
        }
    }
    
    return 1;
}
