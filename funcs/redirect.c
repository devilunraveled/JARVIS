#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../stark.h"
#include "../Logger/logger.h"

extern int jarvis_terminal; 
extern int errno;
extern int defInp;
extern int defOut;

char* filterCMDs( char *command )
{
    while ( *command == ' ' || *command == '\t' )
        command++;
    
    int len = strlen( command );
    
    char *end = ( command + len );
    while ( *end == ' ' || *end == '\t' || *end == '\0' )
    {
        *end = '\0';
        end--;
    }

    char filteredCMD[COMMAND_LENGTH];
    int j = 0;

    for( int i = 0; i < len ; i++ )
    {
        while ( ( command[i] == command[i + 1] ) && ( command[i] == ' ' || command[i] == '\t' || command[i] == '\0') )
            i++;    

        filteredCMD[j++] = command[i];
    }
    filteredCMD[j++] = '\0';
    // printf("filter : %s\n", filteredCMD);

    memcpy( command, filteredCMD, j + 1 );

    return command;
}

int setInp( int file_desc )
{
    // Changes STD_IN to file_desc 
    dup2( file_desc, STDIN_FILENO );
    // The descroptor file_desc is closed before opening.
    if ( errno == EBADF )
    {
        close(file_desc);
        return -1;
    }

    return 0;
}

int setOut( int file_desc )
{
    // Changes STD_IN to file_desc 
    dup2( file_desc, STDOUT_FILENO );
    // The descroptor file_desc is closed before opening.
    if ( errno == EBADF )
    {
        close(file_desc);
        return -1;
    }

    return 0;
}

int resetIO()
{
    if ( dup2(defOut, STDOUT_FILENO ) < 0) 
        return -1;
    if ( dup2(defInp, STDIN_FILENO ) < 0 )
        return -1;
    
    return 0;
}

int redirect( char* command, int p_mode, char *curr_path, char* prev_path ,const char* home, int *zen_mode, int *eta )
{
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

    /* Parsing the command. */
    int rArrow = 0, lArrow = 0;
    int n = strlen(command);
    
    char outputFile[MAX_FILE_NAME] = {'\0'};
    int j_out = 0;

    char inputFile[MAX_FILE_NAME] = {'\0'};
    int j_in = 0;
    
    for ( int i = 0; i < n; i++ )
    {
        if ( command[i] == '>' && (!rArrow) )
        {
            rArrow++;
            command[i] = '\0';

            if ( i < n - 1 && command[i + 1] == '>' )
            {
                i++;
                rArrow++;
            }

            i++;

            while ( i < n  && ( command[i] != '<' && command[i] != '\0' ) )
                outputFile[j_out++] = command[++i];
        }
        
        if ( command[i] == '<' && (!lArrow) )
        {
            command[i] = '\0';

            lArrow++;
            
            i++;

            while ( i < n  && ( command[i] != '>' && command[i] != '\0' ) )
                inputFile[j_in++] = command[++i];
            i--;
        }
    }
    outputFile[j_out - 1] = '\0';
    inputFile[j_in - 1] = '\0';
    
    // Since the each command ends with a null character, 
    // three different strings are generated.
    filterCMDs( command );
    filterCMDs( outputFile );
    filterCMDs( inputFile );

    // printf("Command : %s Files ==> (O) : |%s| & (I) : |%s|\n", command, outputFile, inputFile);

    int inpFile = -1, outFile = -1;

    if ( rArrow == 0 && lArrow == 0 )
    {
        int r =  choose( command, p_mode, curr_path, prev_path, home, zen_mode, eta);
        return r;
    }
    else if ( rArrow > 2 || lArrow > 1 )
    {
        Error("Invalid Arguments.\n");
    }
    else 
    {
        if ( lArrow == 1 )
        {
            // Open the input file and assign a file descriptor to it.
            inpFile = open( inputFile, O_RDONLY ,S_IRUSR | S_IRGRP | S_IROTH );
            if ( inpFile == -1 )
            {
                perror("Error Opening The Input File.");
                return -1;
            }
        }

        if ( rArrow == 1 )
        {
            // Open the output file in truncate mode and assign a file descriptor to it.
            outFile = open( outputFile, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ) ;
            if ( outFile == -1 )
            {
                perror("Error opening the output file in truncate mode.");
                return -1;
            }
        }
        else if ( rArrow == 2 )
        {
            // Open the output file in append mode and assign a file descriptor to it.
            outFile = open( outputFile, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
            if ( outFile == -1 )
            {
                perror("Error opening the output file in append mode.");
                return -1;
            }
        }
    }
    
    if ( lArrow == 1 ) 
    {
        int er = setInp( inpFile );
        if ( er == -1 )
           Error("Could not set the input descroptor.\n");
    }
    if ( rArrow >= 1 ) 
    {
        int er = setOut( outFile );
        if ( er == -1 )
            Error("Could not set the output descriptor.\n");
    }
    int k = choose( command, p_mode, curr_path, prev_path, home, zen_mode, eta);
    resetIO();
    if ( lArrow != 0 ) close(inpFile);
    if ( rArrow != 0 ) close(outFile);

    return k;
}
