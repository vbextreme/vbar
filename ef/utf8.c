#include <ef/type.h>
#include <ef/utf8.h>
/*
typedef struct regex{
	pcre2_code* rx;
	pcre2_match_data* match;
	PCRE2_SIZE* offv;
	size_t offc;
	char* regex;
	char* text;
	size_t lent;
	unsigned int newline;
	int utf8;
	int crlf;
}regex_s;

void regex_init(regex_s* rx){
	rx->rx = NULL;
	rx->match = NULL;
	rx->offv = NULL;
	rx->offc = 0;
	rx->regex = NULL;
	rx->text = NULL;
	rx->lent = 0;
	rx->newline = 0;
	rx->utf8 = -1;
	rx->crlf = -1;
}

void regex_set(regex_s* rx, char* match){
	rx->regex = match;
}

void regex_text(regex_s* rx, char* txt, size_t len){
	rx->text = txt;
	rx->lent = len;
}

void regex_match_free(regex_s* rx){
	pcre2_match_data_free(rx->match);
}

void regex_free(regex_s* rx){
	regex_match_free(rx);
	pcre2_code_free(rx->rx);
}

int regex_build(regex_s* rx){
	int err;
	PCRE2_SIZE errof;
	rx->rx = pcre2_compile((PCRE2_SPTR)rx->regex, PCRE2_ZERO_TERMINATED, 0, &err, &errof, NULL);
	if( rx->rx == NULL ){
		PCRE2_UCHAR buffer[256];
		pcre2_get_error_message(err, buffer, sizeof(buffer));
		dbg_error("compilation failed at offset %d: %s\n", (int)errof,buffer);
		return -1;
	}
	return 0;
}

int regex_match(regex_s* rx){
	rx->match = pcre2_match_data_create_from_pattern(rx->rx, NULL);
	
	int rc = pcre2_match( rx->rx, (PCRE2_SPTR)rx->text, rx->lent, 0, 0, rx->match, NULL);

	if (rc < 0){
		switch(rc){
			case PCRE2_ERROR_NOMATCH: dbg_warning("no match"); break;
			default: dbg_error("matching error %d", rc); break;
		}
		return -1;
	}
	rx->offc = rc;
	
	rx->offv = pcre2_get_ovector_pointer(rx->match);
	if( rx->offv[0] > rx->offv[1] ){
		dbg_error("\\K");
		return -1;
	}

	return 0;
}

void _regex_match_before_continue(regex_s* rx){
	uint32_t obits;
	pcre2_pattern_info(rx->rx, PCRE2_INFO_ALLOPTIONS, &obits);
	rx->utf8 = (obits & PCRE2_UTF) != 0;
	pcre2_pattern_info(rx->rx, PCRE2_INFO_NEWLINE, &rx->newline);
	rx->crlf = rx->newline == PCRE2_NEWLINE_ANY || rx->newline == PCRE2_NEWLINE_CRLF || rx->newline == PCRE2_NEWLINE_ANYCRLF;
}

int regex_match_continue(regex_s* rx){
	if( rx->utf8 == 1 && rx->crlf == -1 )
		_regex_match_before_continue(rx);

	for(;;){
		uint32_t options = 0;
		PCRE2_SIZE start_offset = rx->offv[1];
		
		if( rx->offv[0] == rx->offv[1]){
			if( rx->offv[0] == rx->lent) return -1;
			options = PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED;
		}
		else{
			PCRE2_SIZE startchar = pcre2_get_startchar(rx->match);
			if( start_offset <= startchar ){
				if (startchar >= rx->lent) return -1;
				start_offset = startchar + 1;
				if( rx->utf8){
				for (; start_offset < rx->lent; start_offset++)
					if ((rx->text[start_offset] & 0xc0) != 0x80) return -1;
				}
			}
		}

		int rc = pcre2_match(rx->rx, (PCRE2_SPTR)rx->text, rx->lent, start_offset, options, rx->match, NULL);

		if( rc == PCRE2_ERROR_NOMATCH){
		    if (options == 0) return -1;                    
			rx->offv[1] = start_offset + 1;
		    if( rx->crlf && start_offset < rx->lent - 1 && rx->text[start_offset] == '\r' && rx->text[start_offset + 1] == '\n'){
				rx->offv[1] += 1;
			}
			else if( rx->utf8 ){
				while( rx->offv[1] < rx->lent){
					if( (rx->text[rx->offv[1]] & 0xc0) != 0x80) return -1;
					rx->offv[1] += 1;
				}
			}
			continue;
		}

		if (rc < 0){
			dbg_error("matching error %d", rc);
		    return -1;
		}
		
		rx->offc = rc;
		if( rx->offv[0] > rx->offv[1] ){
			dbg_error("\\K");
			return -1;
		}
		break;
	}

	return 0;
}

size_t regex_match_count(regex_s* rx){
	return rx->offc;
}

char* regex_match_get(size_t* lenout, regex_s* rx, size_t index){
	iassert(index < rx->offc);
	char* start = rx->text + rx->offv[2*index];
	*lenout = rx->offv[2*index+1] - rx->offv[2*index];
	return start;
}

string_s* string_regex_build(size_t* out, string_s* str, regex_s* rx, int global){
	regex_text(rx, str->caret, string_len(str));
	*out = 0;

	if( regex_match(rx) ){
		return NULL;
	}

	size_t count = regex_match_count(rx);
	string_s* subs = ef_mem_many(string_s, count);
	for(size_t i = 0; i < count; ++i){
		size_t lo;
		subs[i].begin = subs[i].caret = regex_match_get(&lo, rx, i);
		subs[i].end = subs[i].begin + lo;
		subs[i].block = 0;
		subs[i].size = 0;
	}
	*out = count;

	if( global ){
		while( !regex_match_continue(rx) ){
			count = regex_match_count(rx);
			subs = realloc(subs, sizeof(string_s) * (count + *out));
			for(size_t i = 0; i < count; ++i){
				size_t lo;
				subs[i+*out].begin = subs[i+*out].caret = regex_match_get(&lo, rx, i);
				subs[i+*out].end = subs[i+*out].begin + lo;
				subs[i+*out].block = 0;
				subs[i].size = 0;
			}
			*out += count;
		}	
	}
	return subs;
}

string_s* string_regex_char(size_t* out, string_s* str, char* reg, int global){
	regex_s rx;
	regex_init(&rx);
	regex_set(&rx, reg);
	if( regex_build(&rx) ){
		*out = 0;
		return NULL;
	}
	string_s* ret = string_regex_build(out, str, &rx, global);
	regex_free(&rx);
	return ret;
}

#define string_regex(OUT, PSTR, RX, G) _Generic((RX),\
		char*: string_regex_char,\
		regex_s*: string_regex_build\
	)(OUT,PSTR,RX,G)

*/









#define U8 (uint8_t*)

const char* lc_charset = NULL;

void utf_init(void){
	setlocale (LC_ALL, "");
	lc_charset = locale_charset();
}

utf8_t* utf8_to(const utf8_t* str, size_t n){
	const utf8_t* prev = str;
	while( n-->0 && str ){
		prev = str;
		utf_t unused;
		str = utf_next(&unused, str);
	}
	return str ? U8 str : U8 prev;
}	

void utf8_ins_n(utf8_t* dst, size_t dnch, utf8_t* src, size_t snch){
	utf_move_n(dst+snch, dst, dnch);
	utf_cpy_n(dst,src,snch);
}

void utf8_ins(utf8_t* dst, utf8_t* src){
	utf8_ins_n(dst, utf_len(dst), src, utf_len(src));
}

void utf8_replace(utf8_t* dst, const utf8_t* src){
	utf_t chs;
	utf_t chd;
	utf8_t* prev = dst;

	while( (src = utf_next(&chs, src)) && dst ){
		utf_putch(dst, chs);
		prev = dst;
		dst = U8 utf_next(&chd,dst);
	}
	if( !dst ) utf_cpy(prev, src);
}

void utf8_del_n(utf8_t* dst, size_t dnch, size_t ndel){
	utf_move_n(dst, dst + ndel, dnch - ndel );
}

void utf8_del(utf8_t* dst, size_t ndel){
	utf8_del_n(dst, utf_len(dst), ndel);
}

void utf8_delu(utf8_t* dst, size_t ndel){
	iassert(dst);
	const utf8_t* src = dst;
	utf_t ch;
	while( ndel --> 0 && src ){
		src = utf_next(&ch, src);
	}
	if( src == NULL ){
		*dst = 0;
		return;
	}

	while( (src = utf_next(&ch,src)) ){
		utf_putch(dst,ch);
		dst =(utf8_t*)utf_next(&ch, dst);
	}
	*dst = 0;
}

int utf8_resize(utf8_t** str, size_t element){
	utf8_t* mem = U8 realloc(*str, sizeof(utf8_t) * element);
	if( !mem ) return -1;
	*str = mem;
	return 0;
}

//TODO DA FARE HEADER

void utf8_fputchar(FILE* fd, utf_t ch){
	utf8_t pch[8] = {0};
	utf_putch(pch, ch);
	utf_fprintf(fd, "%U", pch);
}

utf8Iterator_s utf8_iterator(utf8_t* begin, size_t index){
	utf8Iterator_s it = { .begin = begin, .str = begin, .id = index };
	return it;
}

void utf8_iterator_rewind(utf8Iterator_s* it){
	it->str = it->begin;
	it->id = 0;
}

size_t utf8_iteretor_position(utf8Iterator_s* it){
	return it->id;
}

utf_t utf8_iterator_next(utf8Iterator_s* it){
	utf_t ret;
	if( *it->str == 0 ) return 0;
	utf8_t* next = (utf8_t*)utf_next(&ret, it->str);
	if( next ){
		it->str = next;
	}
	else{
		while( *it->str ) ++it->str;
	}
	++it->id;
	return ret;
}

utf_t utf8_iterator_next_to(utf8Iterator_s* it, size_t count){
	utf_t ret;
	do{		
		ret = utf8_iterator_next(it);
	}while( ret && --count > 0 );
	return ret;
}

utf_t utf8_iterator_last(utf8Iterator_s* it){
	while( utf8_iterator_next(it) );
	return 0;
}

utf_t utf8_iterator_last_valid(utf8Iterator_s* it){
	utf_t ret = 0;
	utf_t tmp = 0;
	do{
		ret = tmp;
		tmp = utf8_iterator_next(it);
	}while( tmp );

	utf8_iterator_prev(it);
	return ret;
}

utf_t utf8_iterator_prev(utf8Iterator_s* it){
	utf_t ret;
	if( it->str == it->begin ){
		return 0;
	}
	utf8_t* prev = (utf8_t*)utf_prev(&ret, it->str, it->begin);
	if( prev ){
		it->str = prev;
	}
	--it->id;
	return ret;
}

utf_t utf8_iterator_prev_to(utf8Iterator_s* it, size_t count){
	utf_t ret;
	do{		
		ret = utf8_iterator_prev(it);
	}while( ret && --count > 0 );
	return ret;
}

void utf8_iterator_replace(utf8Iterator_s* it, utf_t ch){
	if( *it->str == 0 ){
		utf_putch(it->str, ch);
		utf8_iterator_next(it);
		*it->str=0;
	}
	else{
		utf_putch(it->str, ch);
		utf8_iterator_next(it);
	}
}

void utf8_iterator_replace_str(utf8Iterator_s* it, utf8_t* str, size_t width){
	while( width --> 0 && *str ){
		utf_t ch;
		iassert(str);
		str = (utf8_t*)utf_next(&ch,str);
		utf8_iterator_replace(it, ch);
	}
}

void utf8_iterator_insert(utf8Iterator_s* it, utf_t ch){
	utf8Iterator_s lc = *it;
	utf8Iterator_s bk = *it;
	while( ch != 0 ){
		utf_t mem = utf8_iterator_next(&lc);
		utf8_iterator_replace(&bk,ch);
		dbg_info("mem:%c ch:(%d)%c",mem,ch,ch);
		ch = mem;
	}
	utf8_iterator_next(it);
	dbg_info("string8 '%s'", it->begin);
}

void utf8_iterator_insert_str(utf8Iterator_s* it, utf8_t* str, size_t width){
	while( width --> 0 && *str ){
		utf_t ch;
		str = (utf8_t*)utf_next(&ch,str);
		utf8_iterator_insert(it, ch);
	}
}

utf_t utf8_iterator_delete(utf8Iterator_s* it){
	utf8Iterator_s lc = *it;
	utf8Iterator_s bk = *it;
	utf_t ch = utf8_iterator_next(&lc);
	utf_t ret = ch;
	while( ch != 0 ){
		ch = utf8_iterator_next(&lc);
		utf8_iterator_replace(&bk, ch);
	}
	return ret;
}

utf_t utf8_iterator_delete_to(utf8Iterator_s* it, size_t count){
	utf_t ret = 0;
	while( count --> 0 ){
		ret = utf8_iterator_delete(it);
	}
	return ret;
}













void utf8_chomp(utf8_t* str){
	size_t len = utf_len(str);
	if( str[len-1] == '\n' ){
		str[len-1] = 0;
	}
}

utf8_t* utf8_gets(utf8_t* line, size_t max, FILE* fd){
	if( !fgets((char*)line, max, fd) ){
		return NULL;
	}
	if( utf_validate_n(line, max) ){
		return NULL;
	}
	return line;
}

utf8_t* utf8_gets_alloc(size_t* outsize, int nl, FILE* fd){
	utf8_t* begin = mem_many(utf8_t, UTF_LINE_SIZE);
	utf8_t* in = begin;
	iassert(in != NULL);
	*outsize = UTF_LINE_SIZE;
	size_t wr = 0;
	int ch;
	while( (ch=fgetc(fd)) != EOF && ch != '\n' ){
		*in++ = ch;
		++wr;
		if( wr >= *outsize - 1 ){
			*outsize += UTF_LINE_SIZE;
			if( utf8_resize(&begin, *outsize) ){
				dbg_error("utf8_resize");
				free(begin);
				return NULL;
			}
		}
	}
	if( nl && ch == '\n' ){
		*in++ = '\n';
	}
	*in = 0;

	const utf8_t* che;
	if( (che=utf_validate_n(begin, *outsize)) ){
		dbg_error("invalid utf8 on ch number %ld, '%s'", che - begin, che);
		free(begin);
		return NULL;
	}
	return begin;
}

//fare regex

/*
#define TEST(EX, OP, V) do{\
	if( (EX) OP (V) ){\
		dbg_info("test %s successful", #EX);\
	}\
	else{\
		dbg_warning("test %s fail", #EX);\
	}\
}while(0)

int main(){
	utf_init();

	utf8_t str[1024] = "•hello world•";
	size_t len = utf_len(str);
	TEST( len, ==, 17);
	size_t width = utf_width(str);
	TEST( width, ==, 13);
	utf_printf("%s\nlen %lu\nwidth %lu\n", str,len,width);

	utf_t ch = U'•';
	utf_putch(str, ch);
	utf_printf("%s\n", str);

	utf8_ins(str,(utf8_t*)"ok");
	utf_printf("%s\n", str);

	//utf8_del(str, 2);
	//utf_printf("%s\n", str);

	utf8_delu(str, 2);
	utf_printf("%s\n", str);

	utf8_replace(str, U8"ciao mondo !");
	utf_printf("%s\n", str);

	utf8_t* ptr = utf8_to(str, 8);
	utf_putch(ptr, U'1');
	utf_printf("%s\n", str);

	utf_cpy(str,U8"•hello world•");
	utf_printf("%s\n", str);
	
	utf_printf("input: ");

	size_t size;
	utf8_t* in = utf8_gets_alloc(&size, 0, stdin);
	utf_printf("echo:%s\n", in);
	free(in);

	return 0;
}
*/
