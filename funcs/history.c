#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include "history.h"
#include "../Logger/logger.h"
#include "../macros.h"

#define SAME_ENTRY 0
#define SUCCESS 0
#define FAIL -1

static inline int min( int a, int b )
{
    return a < b ? a : b;
}

static inline int max( int a, int b)
{
    return a > b ? a : b;
}

char file_path[MAX_PATH_LENGTH];
char *file_name = ".jarvis_history";

int get_history( char stored_history[HISTORY_MEMORY+1][COMMAND_LENGTH] )
{
    file_path[0] = '\0';
    // Emptying the path.
    uid_t user_id = getuid();
    struct passwd *info = getpwuid( user_id );
    if ( info == NULL )
    {
        Error("Could not access system home directory.\n");
        return -1;
    }
    strcat( file_path, info->pw_dir );
    char *slash = "/";
    strcat( file_path, slash );
    strncat( file_path, file_name, strlen( file_name ) );
    
    // printf("File : %s\n", file_path );
    
    int fd = open( file_path, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR );

    if ( fd == FAIL )
    {
        printf("Invalid File\n");
        perror("Open");
    }

    ssize_t read_ret = 1;

    char c[5];
    int i = 0, j = 0;
    while ( read_ret >= 1 )
    {
        read_ret = read( fd, c, 1);
        
        if ( read_ret <= 0 )
        {
            if ( j != 0)
                stored_history[i++][j] = '\0';
            break;
        }
        
        if ( c[0] == '\n' )
        {
            stored_history[i++][j] = '\0';
            j = 0;
        }
        else
            stored_history[i][j++] = c[0];
    }   
    close( fd );
    
    return i;
}

int move_history( char* command )
{
    char stored_history[HISTORY_MEMORY + 1][COMMAND_LENGTH];
    int num_entries = 0, i = 0;
    // Stored history contains the history.
    num_entries = get_history( stored_history );
    if ( num_entries != 0 && strncmp( stored_history[ num_entries - 1 ], command, strlen(command) ) == 0 )
    {
        // The last entry was the same as this one.
        // printf("here\n");
        return SAME_ENTRY;
    }
    else if ( num_entries == HISTORY_MEMORY )
    {
        for ( i = 0; i < HISTORY_MEMORY ; i++ )
        {
            memmove( stored_history[i], stored_history[i+1], strlen( stored_history[i+1] ) );
            stored_history[i][ strlen( stored_history[i+1]) ] = '\0';
        }
    }
    
    memmove( stored_history[ min( num_entries, HISTORY_MEMORY - 1)], command, strlen(command) + 1);
    stored_history[ min( num_entries, HISTORY_MEMORY - 1)][ strlen( command ) ] = '\0';
    
    int fd = open( file_path , O_WRONLY | O_CREAT | O_TRUNC , 0600);
    
    for ( int k = 0; k <= min( num_entries, HISTORY_MEMORY - 1) ; k++ )
    {
        char buff[COMMAND_LENGTH + 1];
        sprintf(buff,"%s\n",stored_history[k]);
        write(fd,buff, strlen(stored_history[k]) + 1);
    }

    close(fd);

    return SUCCESS;
}

void push_history( char* command )
{
    // Actual function called from jarvis.c.
    // if ( command[0] != ' ' && command[0] != '\t' && command[0] != '\0' )
    //     return ;
    // else if ( strcmp( command, " " ) == 0 || strcmp( command, "\t" ) == 0 )
    //     return ;

    if ( strlen( command ) >= COMMAND_LENGTH )
    {
        Error("Command too big.\n");
        return ;
    }

    move_history( command );
    return;
}

int history( char* command, int len )
{
    while( *command == ' ' || *command == '\t' )
        command++;
    if ( *command != '\0' )
        return -1;
    char stored_history[HISTORY_MEMORY + 1][COMMAND_LENGTH];
    int num_entries = 0;
    // Stored history contains the history.
    num_entries = get_history( stored_history );
        
    for ( int k = 0; k < min( len, num_entries ); k++ )
    {
        printf("%s\n",stored_history[ max( 0, num_entries - len ) + k ]);
    }
    return num_entries;
}
