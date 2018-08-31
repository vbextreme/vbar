#ifndef __VBAR_STRING_H__
#define __VBAR_STRING_H__

#include <vbar/type.h>
#include <vbar/memory.h>
#include <sys/sysinfo.h>

/*** string.c ***/
int str_len_cmp(char* a, size_t la, char* b, size_t lb);
char* str_skip_h(char* str);
char* str_copy_to_str_ifsize(char* dst, size_t len, char* src, char* find);
char* str_copy_to_ch(char* dst, size_t len, char* src, char ch);
char* str_nncpy_src(char* dst, size_t lend, char* src, size_t lens);
char* str_ncpy(char* dst, size_t lend, char* src);
char* str_encpy(char* dst, size_t lend, char* src);

#endif
