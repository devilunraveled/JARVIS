#include <stdio.h>       
#include <stdlib.h>     
#include <string.h>     
#include <sys/types.h>    // OpenDir
#include <dirent.h>     // OpenDir
#include <sys/stat.h>   // Status 
#include <fcntl.h> 

#include "discover.h"   
#include "../Logger/logger.h"
#include "../utils.h"
#include "../macros.h"

#define E_DIRS 1
#define E_FILE 2

void cut_out( char* target );
int err_check( char* command );
int look_through( const char* path, const char* home, const char* target, char *suffix, int flag );
int parse_modes( char* command );
int get_args( char *command, char* dir, char* target );

typedef struct stat Stat;
typedef struct dirent Dirent;

int discover( char* command, const char* curr_path, const char* home )
{
    // Function being called from jarvis.c .
    int parse_error = err_check( command );
    
    int mode = 0;

    if ( parse_error != 1)
    {
        mode = parse_modes( command );
        // printf("Mode_Code : %d\n", mode);
    }
    else
    {
        Error("Invalid instruction.\n");
        return -1;
    }

    char target[MAX_FILE_NAME] = "\0", working_direc[MAX_FILE_NAME] = "\0", working_path[MAX_PATH_LENGTH] = "\0" ;

    int args = get_args( command, working_direc, target );
    
    // The given path is relative if :
    // '/' is not the first character.
    if ( ( args & 1 ) && working_direc[0] != '/')
    {
        char *slash = "/";
        // Given input path is relative.
        if ( working_direc[0] == '~' )
        {
            memmove( working_path, working_direc, strlen( working_direc ) + 1);
            rel_to_abs( working_path, home );
        }
        else
        {
            memmove( working_path, curr_path, strlen(curr_path) + 1);
            strncat( working_path, slash, 2 );
            strncat( working_path, working_direc, strlen( working_direc) + 1 );
        }
    }
    else if ( !( args & 1 ) ) 
    {
        // If no argument is supplied, the source directory is
        // the current directory.
        memmove( working_path, curr_path, strlen( curr_path ) );
    }
    else if ( ( args & 1 ) && working_direc[0] == '/')
    {
        // Absolute path is given as input.
        memmove( working_path, working_direc, strlen( working_direc) + 1);
    }
    
    rel_to_abs( working_path, home );
    
    if ( args & 2 )
        cut_out( target );
    
    // printf("%s, %s\n",working_path , target);
    // return 0;
    if ( working_direc[0] != '\0' )
        printf("%s%s:%s\n", YELLOW, working_direc, RESET);
    char *sff = strchr( working_direc, '/');
    
    if ( sff != NULL) 
        sff++;
    else
        sff = working_direc;
    // If sff is NULL, then the '/' character is not present.
    int ret_look = look_through( working_path, working_path, target, sff, mode );
     
    return ret_look;
}
int err_check( char* command )
{
    // Checks the command and analyzes if it is valid or not.
    // 0 : Valid
    // -1 : Invalid
    if ( command[0] != ' ' && command[0] != '\t' && command[0] != '\0' )
    {
        // The command is of the form discoverX.
        return -1;
    }

    return 0;
}
int look_through( const char* curr_path, const char* home, const char* target, char *sff, int flag )
{
    // Function being called recursively.
    int len = strlen( curr_path );
    char path_copy[ len + 5];
    char *slash = "/";
    // Address now stored in a modifiable manner.
    // This is done to avoid change to crucial data : path.

    memmove( path_copy , curr_path, len + 1 );

    Stat status;

    int stat_ret = lstat( path_copy , &status );
    
    if ( stat_ret == -1 )
    {
        perror("Error Opening the file.");
        printf("Error Path : %s\n", path_copy);
        return -1;
    }
    
    Dirent *direc;
    DIR* d = opendir( path_copy );
   
    if ( d == NULL)
    {
        Error("Could Not Open the directory.\n");
        return -1;
    }

    Stat file_status ;
    if ( S_ISDIR( status.st_mode ) )
    {
        char mod_path[MAX_PATH_LENGTH];
        char *file_name;
        while ( ( direc = readdir( d ) ) != NULL )
        {
            // Points to null when the entire directory is read.
            mod_path[0] = '\0';
            file_name = direc->d_name;

            if ( strlen(file_name) == 2 && strcmp( file_name, ".." ) == 0)
                continue;
            else if ( strlen(file_name) == 1 && file_name[0] == '.')
                continue;
            memmove( mod_path, path_copy, len + 1 );
            strncat( mod_path, slash, 1 );
            strncat( mod_path, file_name, strlen(file_name));
            
            int s_file = lstat( mod_path, &file_status );
            if ( s_file == -1 )
            {
                perror("Lstat Error.");
                continue;
            }
            
            // Printing the file/folder in any case. 
            if ( S_ISDIR( file_status.st_mode) )
            {
                if ( ( *target == '\0') && ( flag & E_DIRS || !flag ) ) 
                    printf("%s%s%s%s\n",BLUE, sff ,&mod_path[strlen(home)], RESET);
                else if ( ( flag & E_DIRS ) || !flag )
                {
                    if ( strcmp( target, file_name ) == 0 )
                    {
                        printf("%s%s%s%s\n",PURPLE,sff,file_name,RESET);
                    }
                }
                look_through( mod_path, home, target, sff, flag );
            }
            else
            {
                abs_to_rel( mod_path, home);
                if ( ( *target == '\0') && ( flag & E_FILE  || !flag ) )
                    printf("%s%s\n", sff ,&mod_path[1]);
                else if ( ( flag & E_FILE ) || !flag )
                {
                    if ( strcmp( target, file_name ) == 0 )
                    {
                        printf("%s%s%s%s\n",PURPLE,sff,&mod_path[1],RESET);
                    }
                }
            }
        }
    }

    closedir(d);
    return 0;
}

int get_args( char *command, char* dir, char* target )
{
    // Will parse the command and find the dir and target.
    int d = 0,f = 0, i_f = 0, i_d = 0;
    char temp_target[MAX_FILE_NAME] = "\0";
    char temp_dir[MAX_FILE_NAME] = "\0";
    
    while( *command == ' ' || *command == '\t' )
        command++;
        
    int len = strlen( command );
    for( int i = 0; i <= len; i++ )
    {
        if ( command[i] == '"' && !f )
        {
            // Quotes are not yet parsed.
            f = 1;
            d = 0;
        }
        else if ( command[i] == '"' && f )
        {
            f = 0;
            temp_target[i_f] = '\0';
        }
        else if ( ( command[i] != ' ' && command[i] != '\t' ) && ( f == 0 ) )
        {
            // Marks start of reading the directory.
            d = 1;
            temp_dir[i_d++] = command[i];
        }
        else if ( ( command[i] == ' ' || command[i] == '\t' || command[i] == '\0') && ( d == 1 ) )
        {
            // The file is parsed.
            d = 0;
            temp_dir[i_d] = '\0';
        }
        else if ( d == 1 )
            temp_dir[i_d++] = command[i];
        else if ( f == 1 )
            temp_target[i_f++] = command[i];
        
        if ( i == len && d == 1)
        {
            temp_dir[i_d] = '\0';
            d = 0;
        }
    }
    
    if ( d != 0 || f != 0 )
    {
        // The input could not be parsed and is going to given an error.
        Error("Invalid input supplied.\n");
        // printf("f : |%c| , d: |%c|\n", temp_target[i_f], temp_dir[i_d]);
    }

    memmove( dir, temp_dir, i_d );
    memmove( target, temp_target, i_f);

    // printf("Dir : |%s|\nTarget : |%s|\n", dir, target);
    
    int val = 0;
    if ( i_d != 0 )
        val++;
    else if ( i_f != 0 )
        val+=2;
    return val;
}


void cut_out( char * target )
{
    // int len = strlen(target);
    char *tar = strrchr( target, '/');
    memmove( target, tar, strlen(tar) );
    return;
}

int parse_modes( char* command )
{
    int len_command = strlen(command);

    char new_command[ len_command + 5 ];
    int list_mode = 0, hidden_mode = 0;
    int j = 0;

    for( int i = 0; i < len_command ; i++ )
    {
        if ( ( i == 0 || command[i-1] == ' ' || command[i-1] == '\t' ) && command[i] == '-' )
        {
            if ( command[ i + 1 ] == 'd' )
            {
                hidden_mode = 1;
                if ( ( i == len_command - 3 && command[ i + 2 ] == 'f' )  || ( i < len_command - 3 && command[ i + 2 ] == 'f' && ( command[ i + 3] == ' ' || command[ i + 3 ] == '\t' )))
                {
                    i++;
                    list_mode = 1;
                }
                i++; // Ignoring the next element.
            }
            else if ( command[ i + 1 ] == 'f')
            {
                list_mode = 1;
                if ( ( i == len_command - 3 && command[ i + 2 ] == 'd' )  || ( i < len_command - 3 && command[ i + 2 ] == 'd' && ( command[ i + 3] == ' ' || command[ i + 3 ] == '\t' )) )
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

