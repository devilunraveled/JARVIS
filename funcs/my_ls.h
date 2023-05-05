#ifndef __LS_
#define __LS_

#define NORMAL_MODE 0
#define HIDDEN_MODE 1
#define LIST_MODE 2
#define HIDDEN_LIST_MODE 3

int ls ( char *command, char *current_path, const char *home );

#endif
