#include <signal.h>
#include "macros.h"

/* Logger */
#ifndef __LOGGER_
#define __LOGGER_

#define BLACK "\033[0;30m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"
#define CYAN "\033[0;36m"
#define BLUE "\033[0;34m"
#define GREY "\033[0;90m"

int Error( const char *s );
int Warning( const char *s );
int Success( const char *s );

#endif



/* Prompt */
#ifndef __PROMPT_
#define __PROMPT_

char * display_prompt( char *, char *, int, int);

#endif



/* Change Directory */
#ifndef __MY_CD_
#define __MY_CD_

int cd( char *target, char *curr_dir , char *prev_dir, const char *home);

#endif



/* Clearing the Console */
#ifndef __MY_CLEAR_
#define __MY_CLEAR_

int clear ( char * );

#endif



/* Echo Function */
#ifndef __MY_ECHO__
#define __MY_ECHO__

int echo( const char *s );

#endif




/* Present Working Directory */

#ifndef __MY_PWD_
#define __MY_PWD_

int pwd( char *curr_path, char* command);

#endif

/* Jobs : Printing the process that are active/ stopped. */
#ifndef __JOBS_
#define __JOBS_
int jobs ( char *command );
#endif

/* Level Shift ( ls )*/
#ifndef __LS_
#define __LS_

int ls ( char *command, char *current_path, const char *home );

#endif



/* History */
#ifndef __HISTORY_
#define __HISTORY_

/*
 * Using HISTORY:
 * 1. Read the last line from the history file.
 * 2. Check if it matches with the given command.
 * 3. If no, append it to the file, else move on.
 * 4. Close the file.
*/

int history( char* command , int num);
void push_history( char* command );

#endif


/* Discover */
#ifndef __DISCOVER_
#define __DISCOVER_

int discover( char* command , const char* curr_path, const char* home );

#endif
/* Discover */




/*Pinfo ( Process Info )*/
#ifndef __PINFO_
#define __PINFO_

typedef struct proc
{
    int pid;
    char Status;
    char mode;
    int memory;
    char *exec_path;

}Proc;

int pinfo( char *command, const char *home ); 
#endif



/* General Utilities  */
#ifndef __UTILS_H_

#define __UTILS_H_

typedef struct proces
{
    char proc_name[MAX_FILE_NAME];
    char comm_name[MAX_FILE_NAME];
    pid_t proc_id;
    int valid;
    int run;
    int num;
}Process;

char *filter( char* );
// Removes trailing and leading spaces & tablines in a one line command.
int choose( char* , int ,char* , char * , const char* , int * , int *);
// The function that chooses what to do based on the input commands
void abs_to_rel( char* , const char *);
//Updates the current path and the previous path based on the target given by the user.
char* rel_to_abs( char *, const char * );

int redirect( char*, int , char *, char* ,const char*, int *, int * );
#endif



/* FOREGROUND */

#ifndef __FOREGROUND_
#define __FOREGROUND_

int sys_commands( char *command, const char *curr_path, const char *home, int *eta);

#endif



/* BACKGROUND */

#ifndef __BACKGROUND_
#define __BACKGROUND_

int sys_commands_b( char *command, const char *curr_path, const char *home, int *eta);
#endif


/* SIGNAL HANDLING*/
int sig( char *command );

int bg( char *command );

int fg( char *command, int *eta );

/* Autocompletion */
int autoComplete( char *command, char *modif, int addNumRead ,char * currPath, const char *home );


/* Pipelining */

int doPipeing( char* command, int p_mode, char *curr_path, char* prev_path ,const char* home, int *zen_mode, int *eta );

