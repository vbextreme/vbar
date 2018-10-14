#ifndef __VBAR_STRING_H__
#define __VBAR_STRING_H__

#include <vbar/type.h>
#include <vbar/memory.h>
#include <sys/sysinfo.h>

typedef struct substr{
	char* begin;
	char* end;
}substr_s;

#define substr_len(S) ((*(S)).end - (*(S)).begin)

/*** string.c ***/
int str_len_cmp(char* a, size_t la, char* b, size_t lb);
char* str_skip_h(char* str);
char* str_skip_hn(char*str);
char* str_copy_to_str_ifsize(char* dst, size_t len, char* src, char* find);
char* str_copy_to_ch(char* dst, size_t len, char* src, char ch);
char* str_nncpy_src(char* dst, size_t lend, char* src, size_t lens);
char* str_ncpy(char* dst, size_t lend, char* src);
char* str_find_num(char* str);
int substr_end(substr_s* out, char* when);
int substr_end_ch(substr_s* out, char when);
int substr_cmp_str(substr_s* a, char* b);
int substr_cmp(substr_s* a, substr_s* b);

#endif
