#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <sys/wait.h>
#include <ctype.h>
#include <pwd.h>
#include "stark.h"

#define EXIT -69
#define MAX_BACK_PROCESSES 4096

extern struct proces BackProc[ MAX_BACK_PROCESSES ];
extern int num_processes;

pid_t jarvis_pgid;
struct termios jarvis_tmodes;
int jarvis_terminal;
int jarvis_is_interactive;
int defInp;
int defOut;
int zen_mode = 0;
int success, lastCmdTime;
char* user_name;
uid_t user_id;

extern void handle_sigs( int sig, siginfo_t* S, void *ucontext );

int need_to_print = 1;

char * username( uid_t user_id);
int getInput( char *command );

char init_prompt[MAX_PROMPT_LENGTH]={'\0'}, curr_path[MAX_PATH_LENGTH] = "~", prev_path[MAX_PATH_LENGTH] = "~", home[MAX_PATH_LENGTH] = "";
int main()
{
    /*Saving The Terminal Attributes.*/
    jarvis_terminal = STDIN_FILENO;
    jarvis_is_interactive = isatty( jarvis_terminal );
    
    user_id = geteuid();
    user_name = username(user_id);
    
    defOut = dup( STDOUT_FILENO );
    defInp = dup( STDIN_FILENO );

    if ( jarvis_is_interactive )
    {
        // checks if shell_terminal refers to a terminal or not.
        signal(SIGINT, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        jarvis_pgid = getpid();

        tcsetpgrp( jarvis_terminal, jarvis_pgid);

        // The termios structure stores the attributes.
        tcgetattr( jarvis_terminal, &jarvis_tmodes);
    }

    getcwd( home, MAX_PATH_LENGTH );
   
    /*Implementing the stuff for background processes.*/
    struct sigaction act;
    sigemptyset( &act.sa_mask );

    act.sa_flags = SA_SIGINFO | SA_RESTART ;
    act.sa_sigaction = handle_sigs;
    
    int s = sigaction( SIGCHLD, &act, NULL );
    
    if ( s == -1 )
        perror("Signal");
    /* End of sigaction handling.*/
    
    int cool_mode = 0; 
    if ( cool_mode ) Success("J.A.R.V.I.S at your Service\n");
    
    int loop = 1;
    success = 1;
    lastCmdTime = -1;
    
    while( loop )
    {
        // Changes the curr_path to absolute if it is possible
        abs_to_rel( curr_path, home);

        char command[COMMAND_LENGTH] = {'\0'};
        
        if ( !zen_mode ) display_prompt(init_prompt, curr_path, success, lastCmdTime);
        else if ( zen_mode )
        {
            printf(GREEN);
            if ( !success )
                printf(RED);
            printf("-> ");
            printf(RESET);
        }

        success = 1;
        lastCmdTime = -1;

        // fgets(command,COMMAND_LENGTH,stdin);
        // In raw mode, fgets cannot be used.
        getInput( command );
        // printf("len command : %d\n", lenCommand);
        char* tokens[MAX_COMMANDS_PER_LINE] = {NULL};
        int command_id[MAX_COMMANDS_PER_LINE] = {0}; 
        // int dp[MAX_COMMANDS_PER_LINE] = {0}; 
        // Removes leading spaces and tab characters.
        filter( command );

        int i = 0, j = 0, k = 0;
        loop = 1;
        
        while ( 1 )
        {
            if ( command[0] == '\0' )
            {
                // No commands input by the user.
                break;
            }
            else if ( strlen(command) == 1 && *command != '\n')
            {
                //Single Character strings.
                loop = choose(command,0,curr_path,prev_path,home,&zen_mode,&lastCmdTime);
                if ( loop == -1 )
                    success = 0;
                loop = 1;
                break;
            }

            if ( j >= (int)strlen(command) )
                break;

            tokens[i] = (char *)malloc( SIZE_OF_COMMAND * sizeof(char) +  1 );

            if ( tokens[i] == NULL )
                Error("Token Limit Exceeded.\n");

            for ( k = 0; ; j++ )
            {
                if ( command[j] == ';' || command[j] == '&' || command[j] == '\0' )
                {
                    if( command[j] == '&' )
                        command_id[i] = 1;
                    j++;
                    break;
                }
                else 
                    tokens[i][k++] = command[j];
            }
            
            tokens[i][k++] = '\n';
            tokens[i][k] = '\0';
            // This is done so that filter can have a similar input to work with
            // as the one it gets from fgets.

            if ( k == 1 )
            {
                Warning("Empty Instruction.\n");
                i++;
                continue;
            }
            // printf("i : %s\n", tokens[i]);
            filter( tokens[i] );

            //Chooses which function to call.
            // printf("p : %d\n", command_id[i]);

            i++;
        }

        for ( int n_comm = 0; n_comm < i; n_comm++ )
        {
            // printf("Token %d: %s\n", n_comm, tokens[n_comm]); 
            // loop = redirect( tokens[n_comm], command_id[n_comm], curr_path, prev_path, home, &zen_mode, &lastCmdTime );
            if ( strlen( tokens[n_comm]) == 1 )
            {
                loop = 1;
                continue;
            }
            loop = doPipeing(  tokens[n_comm], command_id[n_comm], curr_path, prev_path, home, &zen_mode, &lastCmdTime );
            if ( loop == -1 )
                success = 0;

            free( tokens[n_comm] );
            // Avoid the snarls of valgrind.
        }

    }

    if ( cool_mode ) Success("J.A.R.V.I.S bades thy farewell.\n");
    // do something.
    return 0;
}

void disableRawMode()
{
    if ( tcsetattr(STDIN_FILENO, TCSAFLUSH, &jarvis_tmodes) == -1 )
        exit(1);
}

void enableRawMode()
{
    atexit( disableRawMode );
    struct termios raw_mode = jarvis_tmodes;
    raw_mode.c_lflag &= ~( ICANON | ECHO );
    // Selective flags are set.
    if ( tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_mode) == -1 )
    {
        Error("Error Setting Raw Mode.\n");
        exit(1);
    }
}

int getInput( char *command )
{
    enableRawMode();

    int numRead = 0;
        
    char input;
    int cont_read = 1;

    while (cont_read && read( STDIN_FILENO, &input, 1) == 1 )
    {
        if ( iscntrl( input ) )
        {
            switch( input )
            {
                case ( '\n' ) :
                {
                    // Input is read.
                    printf("\n");
                    fflush(stdout);
                    cont_read = 0;
                    break;
                }
                case ( 127 ) :
                {
                    if ( numRead > 0 ) 
                    {
                        command[--numRead] = '\0';
                        printf("\b \b");
                        fflush(stdout);
                    }
                    else if ( numRead == 0 )
                    {
                        printf("\a"); // Alarm Beep.
                        fflush(stdout);
                    }
                    break;
                }
                case ( 4 ) :
                {
                    disableRawMode();
                    exit(0);
                }
                case ( '\t' ) :
                {
                    char added[COMMAND_LENGTH] = "\0";
                    numRead = autoComplete( command, added, numRead, curr_path, home );
                    break; 
                }
                //case ( '' ) :
                default:
                    printf("\a");
            }
        }
        else
        {
            command[numRead++] = input;
            printf("%c", input);
            fflush(stdout);
            // Echo mode is disabled so
            // Input characters should be 
            // displayed manually.
        }
        fflush(stdout);
    }

    disableRawMode();
    
    command[numRead] = '\n';

    return numRead;
}


void handle_sigs( int sig, siginfo_t* S, void *ucontext )
{
    if ( S->si_errno != 0 )
    {
        perror("Child Process Error.");
        return;
    }
    else if ( sig < 0 )
    {
        Error("Invalid Signal returned by child process.\n");
    }
    if ( sig != SIGCHLD )
    {
        return;
    }
    
    pid_t id = S->si_pid;

    int wstat;
    

    waitpid( id, &wstat, WUNTRACED | WCONTINUED );
        
    if( wstat == -1 )
    {
        perror("");
        Error("Child process ended with errors.\n");
        return;
    }

    for ( int i = 0; i < num_processes; i++ )
    {
        if ( BackProc[i].proc_id == id )
        {
            char *returnStatus;

            switch ( S->si_code ) {
                case ( CLD_EXITED ) : returnStatus = "exited normally"; break;
                case ( CLD_KILLED ) : returnStatus = "killed";break;
                case ( CLD_DUMPED ) : returnStatus = "dumped";break;
                case ( CLD_TRAPPED ) : returnStatus = "trapped";break;
                case ( CLD_STOPPED ) : returnStatus = "stopped";break;
                case ( CLD_CONTINUED ) : returnStatus = "continued";break;
                default: returnStatus = "committed die abnormally.";break;
            }

            if ( need_to_print && tcgetpgrp( jarvis_terminal ) == getpid() )
                printf(BLUE"\n%s with pid : %d %s.\n"RESET, BackProc[i].proc_name, S->si_pid, returnStatus);
            fflush(stdout);
            if ( need_to_print )
            {
                // printf("LMAO\n");
                display_prompt(init_prompt, curr_path, success, lastCmdTime);
                // printf("LMAO2\n");
                fflush(stdout);
                // need_to_print = 0;
            }
            // Done to push the prompt on the terminal.
            if ( S->si_code != CLD_CONTINUED )
                BackProc[i].run = 0;
            if ( S->si_code == CLD_KILLED || S->si_code == CLD_EXITED ) 
                BackProc[i].valid = 0;
            return;
        }
    }
}

char * username( uid_t user_id)
{
    struct passwd *info = getpwuid(user_id);
    if ( info == NULL )
        return NULL;
    char *user = info->pw_name;
    return user;
}

