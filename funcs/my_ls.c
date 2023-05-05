#include "my_ls.h"
#include "../Logger/logger.h"
#include "../Prompt/prompt.h"
#include "../utils.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <errno.h>
#include <time.h>
#include <grp.h>
#include "../macros.h"

#define NUM_FLAGS 4
#define MAX_FILES 100000  /* To be put in struct dirent. */
#define SYM_BUFF 200
#define MAX_ARGS 100
typedef struct stat Stat;

char* flags[NUM_FLAGS];

int tokenize( char* commands[MAX_ARGS], char* input );

int do_it_for_dirs( struct dirent* files[], int num_files, char *curr_path, const char *home, char* next, int mode );

int do_it_for_files( char *file, int mode );

int cmp( const void *f1, const void* f2);

int display_details( char* perms, Stat* status, char *file_name, int mode , char* file_path );

int flag_mode( char* command );

char* go_down( char* path, const char* next );

int do_it( char* commands[MAX_ARGS], char* curr_path, const char* home, int num_files, int mode, int i, int print_mode, int another);

int ls( char *command, char* curr_path, const char* home )
{
    // Checking for no arguments.
    char *next = command;
    
    // printf("This dir : %s\n", curr_path );

    // Error rectification for commands of the form lsx.
    if ( *next != '\0' && *next != ' ' && *next != '\t' )
    {
        char buffer[256];
        sprintf( buffer, "Command ls%s not found.\n", next );
        Error(buffer);
        return -1;
    }
    while( *next == ' ' || *next == '\t' )
        next++;
    // Removes the leading tabs and spaces

    int num_files = 0, mode = 0;
    mode = flag_mode( next );
    // filters out the flags and finds out the bitmasks corresponding to it.

    while( *next == ' ' || *next == '\t' )
        next++;

    char *end = (next + strlen(next) - 1);
    // Points to end of the string.

    while( *end == ' ' || *end == '\t' )
    {
        *end = '\0';
        end--;
    }


    rel_to_abs( curr_path, home );
    // General Code upto this point, now comes the file specific code.

    char *commands[MAX_COMMANDS_PER_LINE];

    int token_num = tokenize( commands , next );

    if ( token_num < 0 )
    {
        return -1;
    }
    else
    {
        int bak = 0, another = 0;

        if ( token_num >= 2 )
            bak = 1;
        for ( int i = 0; i < token_num; i++)
        {
            if ( i == token_num - 1) another = 1;
            // printf("C : %s\n", commands[i]);
            int error_val = do_it( commands, curr_path, home, num_files, mode, i, bak, another);
            
            if ( error_val == -1 )
                perror("Error");

            errno = 0;
        }
    }// Extracting directories and files.
    return 0; 
    // Same flags for all the directories.
}


int do_it( char* commands[MAX_ARGS], char* curr_path, const char* home, int num_files, int mode, int i, int print_mode, int another)
{
    char *next = commands[i];
    
    char pathway[MAX_PATH_LENGTH] = "\0" ;
    
    int l = strlen( next );
    
    for ( int i = 0; i < l; i++ )
        pathway[i] = *(next + i);
    pathway[l] = '\0';
    
    DIR *directory;
    
    if ( *next == '\0' )
    {
        // No input path is specified.
        directory = opendir( curr_path );
    }
    else if ( *next == '~' )
    {
        //Input path is relative to the custom home directory.
        rel_to_abs( pathway, home );
        directory = opendir( pathway );
    }
    else
    {
        if ( *next != '/' )
        {
            pathway[0] = '\0';
            // Absolute path is not provided.
            
            // Constructig the path for the directory to open.
            strncat( pathway, curr_path, strlen( curr_path ) );
            
            strcat( pathway, "/");
        
            strncat( pathway, next, strlen( next ) );
            pathway[ strlen(pathway) + strlen(home) + 1] = '\0';
            
            directory = opendir( pathway );
            next = pathway;
        }
        else
        {
            directory = opendir( pathway );
        }
    }
    // Possible path specification.
    
    if ( pathway[0] == '\0')
        strncat( pathway, curr_path, strlen(curr_path) ) ;
   

    Stat* opening = (Stat*)( malloc( sizeof(Stat) ) );
    int er = lstat( pathway, opening);

    if ( er == -1 )
    {
        printf("%sErrenous Path : %s%s\n", YELLOW, pathway, RESET);
        Error("Error Opening the given path.\n");
        free( opening );
        return -1;
    }

    struct dirent *files[MAX_FILES]; 
    struct dirent *dir;
    
    if ( S_ISDIR( opening->st_mode ) && !S_ISLNK( opening->st_mode ) )
    {
        // The given file is a directory, not a symbolik link.
        // lstat is ised for this purpose only, stat follows a symbolik
        // to its memory location, treating it as a directory.
        
        char *temp = strrchr( next, '/');
        // Gets the name of the file from the path.

        if ( print_mode == 1 )
        {
            if ( *temp == '\0' )
                printf("/:\n");
            else
                printf("%s:\n", ++temp);
        }
        while ( 1 )
        {
            dir = readdir( directory );
            // dir contains a pointer to the struct dirent struture.
            if ( dir == NULL || num_files == MAX_FILES )
                break;
            else
                files[ num_files++ ] = dir;
            // files is an array of pointers that will store the 
            // file structure for all the files.
            // This is not required by the current approach and
            // is to be removed.
        }
    }
    else
    {
        if ( pathway[0] == '~' )
            rel_to_abs( pathway, home );
        
        // for symbolik links and files, this code is same.
        do_it_for_files( pathway, mode );
        if (print_mode == 1 && another != 1)
            printf("\n");
        
        return 0;
    }

    qsort( files, num_files, sizeof( struct dirent* ), cmp );
    // Sorts the directories which are to be printed.

    do_it_for_dirs( files, num_files, curr_path, home, pathway , mode );
    if ( print_mode == 1 && another != 1 )
        printf("\n");
    
    closedir(directory);
    
    //Freeing the structures.
    free( dir );
    free( opening );
    // for ( int i = 0; i < num_files; i++ ) 
    //     free( files[ i ] );
    return 0;
    
}

char* go_down( char* path, const char* next )
{
    // Assumes the string path has enough space
    // and ends in the null character.
    char *slash = "/" ;
    strncat( path, slash, 1);
    strncat( path, next, strlen(next) );
    // strncat adds the null character by itself.
    return path;
}

int tokenize( char *commands[MAX_ARGS], char *input )
{
    // A modification to the inbuilt strtok/strtok_r function.
    int i = 1, j = 0, len = strlen(input);

    commands[0] = input;
    for ( j = 0; j < len ;  j++ )
    {
        while( j < len && ( input[j] == ' ' || input[j] == '\t' ) )
        {
            input[j] = '\0'; 
            if ( j < len - 1 && ( input[j+1] != ' ' && input[j+1] != '\t' ) )
                commands[i++] = &input[j+1];
        }
    }
    commands[i] = input + strlen(commands[ i - 1 ]) + 1; 

    return i;
}


int do_it_for_dirs( struct dirent* files[], int num_files, char *curr_path, const char *home, char *next, int mode )
{
    int count = 0;
    for ( int i = 0; i < num_files; i++ )
    {
        char file_name[MAX_FILE_NAME];

        memcpy( file_name, files[i]->d_name, strlen( files[i]->d_name ) + 1 );

        char path[MAX_PATH_LENGTH] = "";

        rel_to_abs( curr_path, home);

        // Constructing the path.
        if ( *next != '/' ) 
        {
            memcpy( path, curr_path, strlen(curr_path) );
        }
        else 
        {
            memcpy( path, next, strlen( next ) );
            path[ strlen( next ) ] = '\0';
        }

        go_down( path, file_name );
        
        if ( file_name[0] == '.' && ( mode == 0 || mode == 2 ) )
            continue;

        Stat* status = ( Stat* ) malloc( sizeof( Stat ) );
        int err = lstat( path, status );
        
        if ( err != -1)
            count += status->st_blocks;
        
        free( status );
    }
    
    if ( mode & 2)
        printf("%stotal : %d%s\n", PURPLE ,count/2,RESET);
    
    for ( int i = 0; i < num_files; i++ )
    {
        char file_type = files[i]->d_type;
        char file_name[MAX_FILE_NAME];

        memcpy( file_name, files[i]->d_name, strlen( files[i]->d_name ) + 1 );

        char path[MAX_PATH_LENGTH] = "";

        rel_to_abs( curr_path, home);

        // Constructing the path.
        if ( *next != '/' ) 
        {
            memcpy( path, curr_path, strlen(curr_path) );
        }
        else 
        {
            memcpy( path, next, strlen( next ) );
            path[ strlen( next ) ] = '\0';
        }

        go_down( path, file_name );

        char perms[11] = "----------";
        perms[10] = '\0';

        if ( file_name[0] == '.' && ( mode == 0 || mode == 2 ) )
            continue;

        Stat* status = ( Stat* ) malloc( sizeof( Stat ) );
        int err = lstat( path, status );
        
        if ( ( S_ISDIR( file_type ) ) && err == -1 )
        {
            // To be modified in the future.
            perror("Error Getting status of the file.");
            free( status );
            return -1;
        }
        
        display_details( perms, status, file_name, mode, path );

        free( status );
    }

    return 0;
    // Returns the mode of ls, and modifies command to remove the flags.
}


int cmp( const void *f1, const void* f2)
{
    // const struct dirent *a = *( const struct dirent**)f1, *b = *( const struct dirent**)f2;
    if ( (*( const struct dirent**)f1)->d_name[0] == '.' &&  (*( const struct dirent**)f1)->d_name[0] == '.' )
        return ( strcasecmp( (& (*( const struct dirent**)f1 )->d_name[1] ) , &( (*( const struct dirent**)f2)->d_name[1] ) ) );
    return ( strcasecmp( (*( const struct dirent**)f1 )->d_name , (*( const struct dirent**)f2)->d_name ) );
}

int flag_mode( char* command )
{
    int len_command = strlen(command);

    char new_command[ len_command + 5 ];
    int list_mode = 0, hidden_mode = 0;
    int j = 0;

    for( int i = 0; i < len_command ; i++ )
    {
        if ( ( i == 0 || command[i-1] == ' ' || command[i-1] == '\t' ) && command[i] == '-' )
        {
            if ( command[ i + 1 ] == 'a' )
            {
                hidden_mode = 1;
                if ( ( i == len_command - 3 && command[ i + 2 ] == 'l' )  || ( i < len_command - 3 && command[ i + 2 ] == 'l' && ( command[ i + 3] == ' ' || command[ i + 3 ] == '\t' )))
                {
                    i++;
                    list_mode = 1;
                }
                i++; // Ignoring the next element.
            }
            else if ( command[ i + 1 ] == 'l')
            {
                list_mode = 1;
                if ( ( i == len_command - 3 && command[ i + 2 ] == 'a' )  || ( i < len_command - 3 && command[ i + 2 ] == 'a' && ( command[ i + 3] == ' ' || command[ i + 3 ] == '\t' )) )
                {
                    i++;
                    hidden_mode = 1;
                }
                i++;
            }
            else
                new_command[ j++ ] = command[ i ];
        }
        else
            new_command[ j++ ] = command[ i ];
        // No changes to the command.
    }
    new_command[j] = '\0';
    memcpy( command, new_command, j + 1);
    //command[j] = '\0';

    return 2*list_mode + hidden_mode;
}

int do_it_for_files( char *file, int mode )
{
    char perms[11] = "----------";
    perms[10] = '\0';

    Stat* status = ( Stat* ) malloc( sizeof( Stat ) );
    if ( status == NULL )
    {
        perror("Too many files open.");
    }
    int err = lstat( file, status );

    if ( err == -1 )
    {
        perror("Error Getting status of the file");
        return -1;
    }

    // Extracting the name of the file.
    char *temp; 
    temp = strrchr( file, '/');
    
    // In case the last character was '/'.
    if ( temp == NULL )
        display_details( perms, status, file, mode, file );
    else
    {
        temp++;
        display_details( perms, status, temp, mode, file );
    }

    free( status );

    return 0;
}

int display_details( char* perms, Stat* status, char* file_name, int mode, char* file_path )
{
    int file_mode = status->st_mode;

    int poss[] = {S_IRUSR, S_IWUSR, S_IXUSR, 
                  S_IRGRP, S_IWGRP, S_IXGRP, 
                  S_IROTH, S_IWOTH, S_IXOTH };

    char *rights = "rwx";
    
    char sym_buff[SYM_BUFF] = "";

    int sym_ret = 0;
    
    if ( S_ISLNK( file_mode ) )
        sym_ret = readlink( file_path, sym_buff, SYM_BUFF );
    if ( sym_ret == -1 )
    {
        Error( " Error Reading symbolink link. \n");
        return -1;
    }
    else
    {
        sym_buff[ sym_ret ] = '\0';
    }

    if ( mode & 2)
    {
        long long size_of_file = status->st_size;

        int user_id = status->st_uid;
        int grp_id = status->st_gid;
        int num_links = status->st_nlink;
        
        char date[21];
        long timed = status->st_mtime;
        if ( time(0) - timed < 15780000 )
        {
            // The file was last accessed in this year only.
            strftime(date, 20, "%b %d %H:%M", localtime( &timed ) );
        }
        else
        {
            // File was accessed more than an year before.
            strftime(date, 20, "%b %d  %Y", localtime( &timed ));
        }

        char user_name[100];

        struct passwd *user = getpwuid( user_id );
        if ( user == NULL )
            memcpy(user_name, "-not-available-", 16 );
        else
            memcpy(user_name, user->pw_name, strlen( user->pw_name ) + 1 );

        char group_name[100];
        struct group *grp = getgrgid( grp_id );
        if ( grp == NULL )
            memcpy(group_name, "-not-available-", 16 );
        else 
            memcpy(group_name, grp->gr_name, strlen( grp->gr_name ) + 1 );

        if ( S_ISDIR(file_mode) )
            perms[0] = 'd';

        for ( int i = 0; i < 9; i++ )
            if ( file_mode & poss[i] )
                perms[i + 1] = rights[ i % 3 ];

        if ( perms[0] == 'd' ) 
            printf( BLUE );
        else if ( perms[3] == 'x' ) 
            printf(GREEN);

        printf("%10s %3d %15s %15s %8lld ", perms, num_links, user_name, group_name, size_of_file);
        printf("%s ", date );
    }

    for ( int i = 0; i < 9; i++ )
        if ( file_mode & poss[i] )
            perms[i + 1] = rights[ i % 3 ];

    if ( S_ISDIR(file_mode) ) 
        printf(BLUE);
    else if ( S_ISLNK( file_mode ) ) 
        printf(CYAN);
    else if ( perms[3] == 'x' ) 
        printf(GREEN);
    // For executables.
    
    printf("%s", file_name );
    
    if ( S_ISLNK( file_mode ) && ( mode & 2 ) )
        printf(" %s-> %s%s", RESET, BLUE, sym_buff );
    
    printf("\n");
    printf(RESET);

    return 0;
}
