#ifndef __UTF8_H__
#define __UTF8_H__

#include "type.h"
#include "memory.h"
#include <locale.h>
#include <ctype.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <unistr.h>
#include <uniconv.h>
#include <unistdio.h>
#include <uniname.h>
#include <unictype.h>
#include <uniwidth.h>
#include <unigbrk.h>
#include <uniwbrk.h>
#include <unilbrk.h>
#include <uninorm.h>
#include <unicase.h>

typedef uint8_t utf8_t; 
typedef uint16_t utf16_t; 
typedef uint32_t utf32_t; 
typedef ucs4_t utf_t;

extern const char* lc_charset;

typedef struct utf8Iterator{
	utf8_t* begin;
	utf8_t* str;
	size_t id;
}utf8Iterator_s;

#define UTF_NOT_VALID 0xFFFD
#define UTF_LINE_SIZE 80

#define utf_validate_n(STR,NCH) _Generic((STR),\
		utf8_t*: u8_check,\
		utf16_t*: u16_check,\
		utf32_t*: u32_check\
	)(STR,NCH)
		
#define utf8_cast_n(STR8,STR,LEN) _Generic((STR),\
		utf16_t*: u8_to_u16,\
		utf32_t*: u8_to_u32\
	)(STR8,STR,LEN)

#define utf16_cast_n(STR16,STR,LEN) _Generic((STR),\
		utf8_t*: u16_to_u8,\
		utf32_t*: u16_to_u32\
	)(STR8,STR,LEN)

#define utf32_cast_n(STR8,STR,LEN) _Generic((STR),\
		utf8_t*: u32_to_u8,\
		utf16_t*: u32_to_u16\
	)(STR8,STR,LEN)

#define utf_cast_n(STRSRC,STRDEST,LEN) _Generic((STRSRC),\
		utf8_t*: utf8_cast_n,\
		utf16_t*: utf16_cast_n,\
		utf32_t*: utf32_cast_n\
	)(STRSRC,STRDEST,LEN)

#define utf_unit_count_n(STR,NCH) _Generic((STR),\
		utf8_t*: u8_mblen,\
		utf16_t*: u16_mblen,\
		utf32_t*: u32_mblen\
	)(STR,NCH)

#define utf_get_unit_n(RET,STR,NCH) _Generic((STR),\
		utf8_t*: u8_mbtouc_unsafe,\
		utf16_t*: u16_mbtouc_unsafe,\
		utf32_t*: u32_mbtouc_unsafe\
	)(RET,STR,NCH)

#define utf_get_unit_and_check_n(RET,STR,NCH) _Generic((STR),\
		utf8_t*: u8_mbtouc,\
		utf16_t*: u16_mbtouc,\
		utf32_t*: u32_mbtouc\
	)(RET,STR,NCH)

#define utf_get_unit_and_check_and_error_n(RET,STR,NCH) _Generic((STR),\
		utf8_t*: u8_mbtoucr,\
		utf16_t*: u16_mbtoucr,\
		utf32_t*: u32_mbtoucr\
	)(RET,STR,NCH)

#define utf_putu_n(STR,UTF,NCH) _Generic((STR),\
		utf8_t*: u8_uctomb,\
		utf16_t*: u16_uctomb,\
		utf32_t*: u32_uctomb\
	)(STR,UTF,NCH)

#define utf_putch(STR, UTF) utf_putu_n(STR, UTF, sizeof(utf_t))

#define utf_cpy_n(DST,SRC,NCH) _Generic((DST),\
		utf8_t*: u8_cpy,\
		utf16_t*: u16_cpy,\
		utf32_t*: u32_cpy\
	)(DST,SRC,NCH)

#define utf_move_n(DST,SRC,NCH) _Generic((DST),\
		utf8_t*: u8_move,\
		utf16_t*: u16_move,\
		utf32_t*: u32_move\
	)(DST,SRC,NCH)

#define utf_set_n(DST,UTF,NCH) _Generic((DST),\
		utf8_t*: u8_set,\
		utf16_t*: u16_set,\
		utf32_t*: u32_set\
	)(DST,UTF,NCH)

#define utf_fixed_cmp_n(SA,SB,NCH) _Generic((SA),\
		utf8_t*: u8_cmp,\
		utf16_t*: u16_cmp,\
		utf32_t*: u32_cmp\
	)(SA,SB,NCH)

#define utf_cmp_n(SA,SB,NCH) _Generic((SA),\
		utf8_t*: u8_cmp2,\
		utf16_t*: u16_cmp2,\
		utf32_t*: u32_cmp2\
	)(SA,SB,NCH)

#define utf_chr_n(STR,NCH,UTF) _Generic((STR),\
		utf8_t*: u8_chr,\
		utf16_t*: u16_chr,\
		utf32_t*: u32_chr\
	)(STR,NCH,UTF)

#define utf_len_n(STR,NCH) _Generic((STR),\
		utf8_t*: u8_mbsnlen,\
		utf16_t*: u16_mbsnlen,\
		utf32_t*: u32_mbsnlen\
	)(STR,NCH)

#define utf_cpy_n_alloc(SRC,NCH) _Generic((SRC),\
		utf8_t*: u8_cpy_alloc,\
		utf16_t*: u16_cpy_alloc,\
		utf32_t*: u32_cpy_alloc\
	)(DST,SRC,NCH)

#define utf_unit_count(STR) _Generic((STR),\
		utf8_t*: u8_strmblen,\
		utf16_t*: u16_strmblen,\
		utf32_t*: u32_strmblen\
	)(STR)

#define utf_get_unit(RET,STR,NCH) _Generic((STR),\
		utf8_t*: u8_strmbtouc,\
		utf16_t*: u16_strmbtouc,\
		utf32_t*: u32_strmbtouc\
	)(RET,STR)

#define utf_next(RET,STR) _Generic((STR),\
		utf8_t*: u8_next,\
		const utf8_t*: u8_next,\
		utf16_t*: u16_next,\
		utf32_t*: u32_next\
	)(RET,STR)

#define utf_prev(RET,STR,START) _Generic((STR),\
		utf8_t*: u8_prev,\
		utf16_t*: u16_prev,\
		utf32_t*: u32_prev\
	)(RET,STR, START)

#define utf_len(STR) _Generic((STR),\
		utf8_t*: u8_strlen,\
		utf16_t*: u16_strlen,\
		utf32_t*: u32_strlen\
	)(STR)

#define utf_len_max(STR,MAX) _Generic((STR),\
		utf8_t*: u8_strnlen,\
		utf16_t*: u16_strnlen,\
		utf32_t*: u32_strnlen\
	)(STR,MAX)

#define utf_cpy(DST,SRC) _Generic((DST),\
		utf8_t*: u8_stpcpy,\
		utf16_t*: u16_stpcpy,\
		utf32_t*: u32_stpcpy\
	)(DST,SRC)

#define utf_cpy_max(DST,SRC,MAX) _Generic((DST),\
		utf8_t*: u8_strncpy,\
		utf16_t*: u16_stpncpy,\
		utf32_t*: u32_stpncpy\
	)(DST,SRC,MAX)

#define utf_cat(DST,SRC) _Generic((DST),\
		utf8_t*: u8_strcat,\
		utf16_t*: u16_strcat,\
		utf32_t*: u32_strcat\
	)(DST,SRC)

#define utf_cat_max(DST,SRC,MAX) _Generic((DST),\
		utf8_t*: u8_strncat,\
		utf16_t*: u16_strncat,\
		utf32_t*: u32_strncat\
	)(DST,SRC,MAX)

#define utf_coll(SA,SB) _Generic((SA),\
		utf8_t*: u8_strcoll,\
		utf16_t*: u16_strcoll,\
		utf32_t*: u32_strcoll\
	)(SA,SB)

#define utf_cmp_max(SA,SB,MAX) _Generic((SA),\
		utf8_t*: u8_strncmp,\
		utf16_t*: u16_strncmp,\
		utf32_t*: u32_strncmp\
	)(SA,SB,MAX)

#define utf_clone(STR) _Generic((STR),\
		utf8_t*: u8_strdup,\
		utf16_t*: u16_strdup,\
		utf32_t*: u32_strdup\
	)(STR)

#define utf_chr(STR,UTF) _Generic((STR),\
		utf8_t*: u8_strchr,\
		utf16_t*: u16_strchr,\
		utf32_t*: u32_strchr\
	)(STR,UTF)

#define utf_cspn(STR,REJECT) _Generic((STR),\
		utf8_t*: u8_strcspn,\
		utf16_t*: u16_strcspn,\
		utf32_t*: u32_strcspn\
	)(STR,REJECT)

#define utf_spn(STR,ACCEPT) _Generic((STR),\
		utf8_t*: u8_strspn,\
		utf16_t*: u16_strspn,\
		utf32_t*: u32_strspn\
	)(STR,ACCEPT)

#define utf_anyof(STR,ANY) _Generic((STR),\
		utf8_t*: u8_strpbrk,\
		utf16_t*: u16_strpbrk,\
		utf32_t*: u32_strpbrk\
	)(STR,ANY)

#define utf_search(STR,SEARCH) _Generic((STR),\
		utf8_t*: u8_strstr,\
		utf16_t*: u16_strstr,\
		utf32_t*: u32_strstr\
	)(STR,SEARCH)

#define utf_begin(STR,WITH) _Generic((STR),\
		utf8_t*: u8_startwith,\
		utf16_t*: u16_startwith,\
		utf32_t*: u32_startwith\
	)(STR,WITH)

#define utf_ends(STR,WITH) _Generic((STR),\
		utf8_t*: u8_endswith,\
		utf16_t*: u16_endswith,\
		utf32_t*: u32_endswith\
	)(STR,WITH)

#define utf_tok(STR,DEL,PTR) _Generic((STR),\
		utf8_t*: u8_strtok,\
		utf16_t*: u16_strtok,\
		utf32_t*: u32_strtok\
	)(STR,DEL,PTR)

#define utf8_f_sprintf(STR,FORM,arg...) _Generic((FORM),\
		char*: u8_sprintf,\
		utf8_t*: u8_u8_sprintf\
	)(STR,FORM, ## arg)
#define utf16_f_sprintf(STR,FORM,arg...) _Generic((FORM),\
		char*: u16_sprintf,\
		utf16_t*: u16_u16_sprintf\
	)(STR,FORM, ## arg)
#define utf32_f_sprintf(STR,FORM,arg...) _Generic((FORM),\
		char*: u32_sprintf,\
		utf32_t*: u32_u32_sprintf\
	)(STR,FORM, ## arg)

#define utf_sprintf(STR,FORM,arg...) _Generic((STR),\
		char*: ulc_sprintf,\
		utf8_t*: utf8_f_sprintf,\
		utf16_t*: utf16_f_sprintf,\
		utf32_t*: utf32_f_sprintf\
	)(STR,FORM, ## arg)

#define utf8_f_snprintf(STR,MAX,FORM,arg...) _Generic((FORM),\
		char*: u8_snprintf,\
		utf8_t*: u8_u8_snprintf\
	)(STR,MAX,FORM, ## arg)
#define utf16_f_snprintf(STR,MAX,FORM,arg...) _Generic((FORM),\
		char*: u8_snprintf,\
		utf16_t*: u16_u16_snprintf\
	)(STR,MAX,FORM, ## arg)
#define utf32_f_snprintf(STR,MAX,FORM,arg...) _Generic((FORM),\
		char*: u8_snprintf,\
		utf32_t*: u32_u32_snprintf\
	)(STR,MAX,FORM, ## arg)

#define utf_snprintf(STR,MAX,FORM,arg...) _Generic((STR),\
		char*: ulc_snprintf,\
		utf8_t*: utf8_f_snprintf,\
		utf16_t*: utf16_f_snprintf,\
		utf32_t*: utf32_f_snprintf\
	)(STR,MAX,FORM, ## arg)

#define utf8_f_sprintf_alloc(STR,FORM,arg...) _Generic((FORM),\
		char*: u8_asprintf,\
		utf8_t*: u8_u8_asprintf\
	)(STR,FORM, ## arg)
#define utf16_f_sprintf_alloc(STR,FORM,arg...) _Generic((FORM),\
		char*: u16_asprintf,\
		utf16_t*: u16_u16_asprintf\
	)(STR,FORM, ## arg)
#define utf32_f_sprintf_alloc(STR,FORM,arg...) _Generic((FORM),\
		char*: u32_asprintf,\
		utf32_t*: u32_u32_asprintf\
	)(STR,FORM, ## arg)

#define utf_sprintf_alloc(STR,FORM,arg...) _Generic((STR),\
		char*: ulc_asprintf,\
		utf8_t*: utf8_f_asprintf_alloc,\
		utf16_t*: utf16_f_asprintf_alloc,\
		utf32_t*: utf32_f_asprintf_alloc\
	)(STR,FORM, ## arg)

#define utf8_f_snprintf_alloc(STR,MAX,FORM,arg...) _Generic((FORM),\
		char*: u8_asnprintf,\
		utf8_t*: u8_u8_asnprintf,\
	)(STR,MAX,FORM, ## arg)
#define utf16_f_snprintf_alloc(STR,MAX,FORM,arg...) _Generic((FORM),\
		char*: u8_asnprintf,\
		utf16_t*: u16_u16_asnprintf,\
	)(STR,MAX,FORM, ## arg)
#define utf32_f_snprintf_alloc(STR,MAX,FORM,arg...) _Generic((FORM),\
		char*: u8_asnprintf,\
		utf32_t*: u32_u32_asnprintf,\
	)(STR,MAX,FORM, ## arg)

#define utf_snprintf_alloc(STR,MAX,FORM,arg...) _Generic((STR),\
		char*: ulc_asnprintf,\
		utf8_t*: utf8_f_asnprintf_alloc,\
		utf16_t*: utf16_f_asnprintf_alloc,\
		utf32_t*: utf32_f_asnprintf_alloc\
	)(STR,MAX,FORM, ## arg)

#define utf_fprintf ulc_fprintf
#define utf_printf(FORM,arg...) ulc_fprintf(stdout, FORM, ## arg)

#define UTF_NAME_MAX UNINAME_MAX
#define UTF_NAME_INVALID UNINAME_INVALID
#define utf_unicode_name unicode_character_name
#define utf_name_unicode unicode_name_character

#define utf_alnum uc_is_alnum
#define utf_alpha uc_is_alpha
#define utf_cntrl uc_is_cntrl
#define utf_digit uc_is_digit
#define utf_graph uc_is_graph
#define utf_lower uc_is_lower
#define utf_print uc_is_print
#define utf_punct uc_is_punct
#define utf_space uc_is_space
#define utf_upper uc_is_upper
#define utf_xdigit uc_is_xdigit
#define utf_blank uc_is_blank
#define utf_toupper uc_toupper
#define utf_tolower uc_tolower

#define utf_snprintf_alloc(STR,MAX,FORM,arg...) _Generic((STR),\
		char*: ulc_asnprintf,\
		utf8_t*: utf8_f_asnprintf_alloc,\
		utf16_t*: utf16_f_asnprintf_alloc,\
		utf32_t*: utf32_f_asnprintf_alloc\
	)(STR,MAX,FORM, ## arg)

#define utf_width(STR) _Generic((STR),\
		utf_t: uc_width,\
		utf8_t*: u8_strwidth,\
		utf16_t*: u16_strwidth,\
		utf32_t*: u32_strwidth\
	)(STR,lc_charset)

#define utf_width_enc(STR,ENCODING) _Generic((STR),\
		utf_t: uc_width,\
		utf8_t*: u8_strwidth,\
		utf16_t*: u16_strwidth,\
		utf32_t*: u32_strwidth\
	)(STR,ENCODING)

#define utf_width_n(STR,NCH,ENCODING) _Generic((STR),\
		utf8_t*: u8_width,\
		utf16_t*: u16_width,\
		utf32_t*: u32_width\
	)(STR,NCH,ENCODING)


void utf_init(void);

utf8_t* utf8_to(const utf8_t* str, size_t n);
#define utf_to(STR,COUNT) _Generic((STR),\
		utf8_t*: utf8_to,\
		const uint8_t*: utf8_to\
	)(STR,COUNT)

void utf8_ins_n(utf8_t* dst, size_t dnch, utf8_t* src, size_t snch);
#define utf_insert_n(DST,DLEN,SRC,SLEN) _Generic((DST),\
		utf8_t*: utf8_ins_n\
	)(DST,DLEN,SRC,SLEN)

void utf8_ins(utf8_t* dst, utf8_t* src);
#define utf_insert(DST,SRC) _Generic((DST),\
		utf8_t*: utf8_ins\
	)(DST,SRC)

void utf8_replace(utf8_t* dst, const utf8_t* src);
#define utf_replace(DST,SRC) _Generic((DST),\
		utf8_t*: utf8_replace\
	)(DST,SRC)

void utf8_del_n(utf8_t* dst, size_t dnch, size_t ndel);
#define utf_del_n(STR,STRLEN,COUNT) _Generic((STR),\
		utf8_t*: utf8_del_n\
	)(STR,STRLEN,COUNT)

void utf8_del(utf8_t* dst, size_t ndel);
#define utf_del(STR,COUNT) _Generic((STR),\
		utf8_t*: utf8_del\
	)(STR,COUNT)

void utf8_delu(utf8_t* dst, size_t ndel);
#define utf_delu(STR,COUNT) _Generic((STR),\
		utf8_t*: utf8_delu\
	)(STR,COUNT)

int utf8_resize(utf8_t** str, size_t element);
#define utf_resize(PTRSTR,COUNT) _Generic((PTRSTR),\
		utf8_t**: utf8_resize\
	)(PTRSTR,COUNT)

void utf8_fputchar(FILE* fd, utf_t ch);
utf8Iterator_s utf8_iterator(utf8_t* begin, size_t index);
size_t utf8_iteretor_position(utf8Iterator_s* it);
void utf8_iterator_rewind(utf8Iterator_s* it);
utf_t utf8_iterator_next(utf8Iterator_s* it);
utf_t utf8_iterator_next_to(utf8Iterator_s* it, size_t count);
utf_t utf8_iterator_last(utf8Iterator_s* it);
utf_t utf8_iterator_last_valid(utf8Iterator_s* it);
utf_t utf8_iterator_prev(utf8Iterator_s* it);
utf_t utf8_iterator_prev_to(utf8Iterator_s* it, size_t count);
void utf8_iterator_replace(utf8Iterator_s* it, utf_t ch);
void utf8_iterator_replace_str(utf8Iterator_s* it, utf8_t* str, size_t width);
void utf8_iterator_insert(utf8Iterator_s* it, utf_t ch);
void utf8_iterator_insert_str(utf8Iterator_s* it, utf8_t* str, size_t width);
utf_t utf8_iterator_delete(utf8Iterator_s* it);
utf_t utf8_iterator_delete_to(utf8Iterator_s* it, size_t count);


#endif
