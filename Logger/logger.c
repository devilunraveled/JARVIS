#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include"logger.h" 

int Error( const char *s )
{
    fprintf(stdout,RED);
    int n = fprintf(stdout,"jarvis: %s", s);
    fprintf(stdout,RESET);
    
    fflush(stdout);
    if ( n == -1 )
        return 0;
    else 
        return 1;
}

int Warning( const char *s )
{
    fprintf(stdout,YELLOW);
    int n = fprintf(stdout,"jarvis: %s", s);
    fprintf(stdout,RESET);
    fflush(stdout);

    if ( n == -1 )
        return 0;
    else 
        return 1;
}

int Success( const char *s )
{
    fprintf(stdout,GREEN);
    int n = fprintf(stdout,"jarvis: %s", s);
    fprintf(stdout,RESET);
    fflush(stdout);

    if ( n == -1 )
        return 0;
    else 
        return 1;
}
