#include <vbar/string.h>

int str_len_cmp(char* a, size_t la, char* b, size_t lb){
	for(; la > 0 && lb > 0; ++a, ++b, --la, --lb ){
		if( *a != *b ) return -1;
	}
	if( la || lb ) return 1;
	return 0;
}	

char* str_skip_h(char* str) {
	while( *str && (*str == ' ' || *str == '\t') ) ++str;
	return str;
}

char* str_skip_hn(char*str){
	while( *str && (*str == ' ' || *str == '\t' || *str == '\n') ) ++str;
	return str;
}

__ef_can_null char* str_copy_to_str_ifsize(char* dst, size_t len, char* src, char* find){
	char* to = strpbrk(src, find);
	if( to == NULL ){
		return NULL;
	}

	size_t max = to - src;
	if( max > len ){
		return NULL;
	}

	memcpy(dst, src, sizeof(char) * max);
	dst[max] = 0;

	return src + max;
}

__ef_can_null char* str_copy_to_ch(char* dst, size_t len, char* src, char ch){
	char* to = strchr(src, ch);
	if( to == NULL ){
		return NULL;
	}

	size_t max = to - src;
	if( max > len ){
		return NULL;
	}

	memcpy(dst, src, sizeof(char) * max);
	dst[max] = 0;

	return src + max;
}

char* str_nncpy_src(char* dst, size_t lend, char* src, size_t lens){
	iassert(lend > 1);
	--lend;
	while( lens-->0 && lend-->0 && *src){
		*dst++=*src++;
	}
	*dst = 0;
	return src;
}

char* str_ncpy(char* dst, size_t lend, char* src){
	iassert(lend > 1);
	--lend;
	while( lend-->0 && *src ){
		*dst++=*src++;
	}
	*dst = 0;
	return dst;
}

char* str_find_num(char* str){
	while( *str && (*str < '0' || *str > '9') ) ++str;
	return str;
}


int substr_end(substr_s* out, char* when){
	out->end = strpbrk(out->begin, when);
	return !out->end ? -1 : 0;
}

int substr_end_ch(substr_s* out, char when){
	out->end = strchr(out->begin, when);
	return !out->end ? -1 : 0;
}

int substr_cmp_str(substr_s* a, char* b){
	return str_len_cmp(a->begin, substr_len(a), b, strlen(b));
}

int substr_cmp(substr_s* a, substr_s* b){
	return str_len_cmp(a->begin, substr_len(a), b->begin, substr_len(b));
}


