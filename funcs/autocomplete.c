#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

#include "../stark.h"
#include "../macros.h"

void constructPath( char *path, char* fileName );

extern char init_prompt[MAX_PROMPT_LENGTH];
extern char curr_path[MAX_PATH_LENGTH];
extern int success;
extern int lastCmdTime;

int autoComplete( char *command, char *modif, int addNumRead ,char * currPath, const char *home )
{
    int numRead = addNumRead;
    int commLen = strlen(command);
    
    char *filterComm = NULL , *temp = command + commLen;
    
    while ( *temp != ' ' )
        filterComm = temp--;

    char pathCopy[MAX_PATH_LENGTH] = {'\0'};
    
    // printf("%s\n", filterComm);

    int i = numRead - 1;
    for ( i = numRead - 1; i >= 0; i-- )
    {
        if ( command[i] == ' ' || command[i] == '\t' ) break;
        // Shifting the end pointer.
        // Now points to the file nume.
        if ( i == 0 )
            return numRead;
    }
    filterComm = &command[ i + 1 ];
    int fileLen = strlen( filterComm );

    // if ( filterComm[0] == '/' )
    // {
    //     // Absolute path is given.
    //     memcpy( pathCopy, filterComm, strlen(filterComm) + 1 );
    //     int lengthOfPath = strlen(filterComm);
    //
    //     while ( lengthOfPath > 0 &&  pathCopy[lengthOfPath] != '/' )
    //         pathCopy[lengthOfPath--] = '\0';
    // }
    // else if ( filterComm[0] == '~' )
    // {
    //     // Path is given, but relative.
    //     memcpy( pathCopy, filterComm, fileLen + 1 );
    //     rel_to_abs( pathCopy, home );
    // }
    // else if ( filterComm[0] == '.' )
    // {
    //     // Path is given relative to curr directory.
    //     memcpy( pathCopy, currPath, strlen(currPath) + 1 );
    //     strcat( pathCopy, "/" );
    //     strcat( pathCopy, filterComm );
    // }
    // // In all the above cases path was given so path was constructed.
    // else
    // {
        memcpy( pathCopy, currPath, strlen(currPath) + 1 );
    // }
    // Path in which to search the files for is created in pathCopy.
    
    // printf("\n Path : |%s| \n", pathCopy);

    char *slash = "/";
    char modPath[MAX_PATH_LENGTH] = {'\0'};

    rel_to_abs( pathCopy, home);
    DIR *thisDirectory = opendir( pathCopy );

    if ( thisDirectory == NULL )
    {
        printf("\a");
        return commLen;
    }

    struct dirent *contents;

    char *fileName = NULL, next_char[2] = "\0\0";
    int numSuggestions = 0, fileNamelen = 0;
    char completed[COMMAND_LENGTH] = {'\0'};
    int dirExists = 0;

    char *fileList[10000] = {NULL};

    while ( ( contents = readdir( thisDirectory ) ) != NULL )
    {
        fileName = contents->d_name;
        fileNamelen = strlen(fileName);
        
        if ( fileName[0] == '.' )
            continue;

        int predictLen = fileNamelen > fileLen ? fileLen : fileNamelen ;

        if ( fileLen < fileNamelen && ( strncmp(filterComm, fileName, predictLen ) == 0 ) )
        {
            memcpy( modPath, pathCopy, strlen(pathCopy) );
            strcat( modPath, slash );
            strcat( modPath, fileName );

            fileList[ numSuggestions++ ] = fileName;

            struct stat S;
            int statErr = lstat(modPath, &S);
            
            if ( statErr != -1 )
            {
                if ( fileLen > 0 && S_ISDIR( S.st_mode ) )
                {
                    //printf("FileLength : %d\n", fileLen);
                    next_char[0] = '/';
                    dirExists = 1;
                }
                else
                    next_char[0] = '\0';
                memmove( completed, fileName, fileNamelen + 1 );
                continue;
            }
            
            if ( numSuggestions == 1 )
            {
                next_char[0] = ' ';
                memmove( completed, fileName, fileNamelen + 1 );
                continue;
            }
            
            if ( !dirExists )
            {
                next_char[0] = '\0';
            }
            
            for ( int i = 0; i < fileNamelen; i++ )
            {
                if ( completed[i] != fileName[i] )
                {
                    completed[i] = '\0';
                    break;
                }
            }

            completed[fileNamelen] = '\0';
        }
    }
    
    strcat(completed, next_char );
    memmove( filterComm, completed, strlen(completed) + 1 );

    int revisedLength = strlen(command);
   
    closedir(thisDirectory);

    if ( numSuggestions == 0 )
    {
        // User fucking around with autoComplete.
        // printf("\x1b[%dD", commLen);
        printf("\a");
        // printf("%s", command);
        fflush(stdout);
        return commLen;
    }
    else if ( numSuggestions == 1) printf("\x1b[%dD", commLen);
    else if ( numSuggestions > 1 )
    {
        printf("\n");fflush(stdout);
        for ( int k = 0; k < numSuggestions; k++ ) {printf("%s\n", fileList[k]); fflush(stdout); };
        display_prompt( init_prompt, curr_path, 1, -1 );
    }
    
    printf("%s", command);
    fflush(stdout);
    return revisedLength;
}
