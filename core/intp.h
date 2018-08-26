#ifndef __INTP_H__
#define __INTP_H__

#include <vbar/type.h>

typedef void(*intpcall_f)(char* a0, size_t len0, char* a1, size_t len1);

void intp_register_command(char* name, intpcall_f call);
char* intp_interpretate(char* line);


#endif
