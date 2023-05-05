#ifndef __LOG_H_
#define __LOG_H_

#define BLACK "\033[0;30m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"
#define CYAN "\033[0;36m"
#define BCYAN "\033[0;96m"
#define BLUE "\033[0;34m"
#define PURPLE "\033[0;35m"
#define GREY "\033[0;90m"

int Error( const char *s );
int Warning( const char *s );
int Success( const char *s );

#endif
