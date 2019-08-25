#include <ef/strong.h>
#include <ef/memory.h>

char* str_dup(const char* src, size_t optlen){
	if( optlen == 0 ) optlen = strlen(src);
	dbg_info("dup(%lu)", optlen);
	char* ret = mem_many(char, optlen+1);
	iassert(ret != NULL);
	memcpy(ret, src, optlen + 1);
	return ret;
}

int str_equal(char const* a, size_t lena, char const* b, size_t lenb){
	if( lena != lenb ) return lena - lenb;
	return memcmp(a,b,lena);
}

char* str_skip_h(char* str) {
	while( *str && (*str == ' ' || *str == '\t') ) ++str;
	return str;
}

char* str_skip_hn(char*str){
	while( *str && (*str == ' ' || *str == '\t' || *str == '\n') ) ++str;
	return str;
}

char* str_copy_to_str_ifsize(char* dst, size_t len, char* src, char* find){
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

char* str_copy_to_ch(char* dst, size_t len, char* src, char ch){
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
	return str_equal(a->begin, substr_len(a), b, strlen(b));
}

int substr_cmp(substr_s* a, substr_s* b){
	return str_equal(a->begin, substr_len(a), b->begin, substr_len(b));
}


