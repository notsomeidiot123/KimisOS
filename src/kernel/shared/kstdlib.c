#include "kstdlib.h"
#include <stdarg.h>

char* log_info_types[] = {
    "\b",
    "| DEBUG",
    "\b",
    "| WARNING",
    "| ERROR"
};

void mlog(char *module, char *str, uint32_t type, ...){
    va_list vars;
    va_start(vars, type);
    if(!type || type > 4){
        return;
    }
    printf("[ %s %s ]", module, log_info_types[type]);
    vprintf(str, vars);
}