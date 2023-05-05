#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include "../Logger/logger.h"
#include "../macros.h"
#include "prompt.h"

#define MAX_SYS_NAME_LEN 20

extern uid_t user_id;
extern char* user_name;

// Would later include the path required.
char * display_prompt( char *display , char* rel_path, int success, int time_ )
{
    char system_name[MAX_SYS_NAME_LEN];
    // char display[MAX_PROMPT_LEN];

    int succ = gethostname(system_name,MAX_SYS_NAME_LEN);
    // if succ == -1, then an error has occured.
    if ( succ == -1 )
    {
        // Error getting the hostname.
        system_name[0] = '\0';
        Error(" Error Retrieving System Name. ");
    }
    char curr_dir[MAX_PATH_LENGTH] = "\0";
    getcwd(curr_dir,MAX_PATH_LENGTH);

    char relative_path[MAX_PATH_LENGTH] = "\0";

    strncat(relative_path,rel_path,MAX_PATH_LENGTH - 1 );
    
    // Main printing part(for now).
    // printf( "-> <%s@",user_name);
    fprintf(stdout ,CYAN);
    if ( ! success ) printf(PURPLE);
    fprintf(stdout , "\r<");
    // printf(CYAN);
    fprintf(stdout,"%s@",user_name);
    fprintf(stdout,"%s:", system_name);
    fprintf(stdout, BLUE);
    fprintf(stdout, "%s ", relative_path);
    fprintf(stdout, CYAN);
    if ( !success ) fprintf(stdout,PURPLE);
    if ( time_ > 1 )
        fprintf(stdout,"%stook:%ds%s", GREY, time_, RESET);
    fprintf( stdout, CYAN );
    if ( !success ) fprintf(stdout, PURPLE);
    fprintf(stdout, "> ");
    fprintf(stdout, RESET);
    // Add colors to these.
    fflush(stdout);
    snprintf(display,MAX_PROMPT_LENGTH,"<%s@%s %s>", user_name, system_name, curr_dir);
    
    time_ = -1;
    return display;
}
