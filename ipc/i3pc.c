#include <vbar/ipc.h>
#include <vbar/string.h>
//#include <vbar/delay.h>
//#include <sys/time.h>
//#include <sys/epoll.h>
//#include <sys/inotify.h>
//#include <fcntl.h>

#define json_begin_block() putchar('{')
#define json_end() putchar('\n')
#define json_end_block() putchar('}')

__ef_private inline void json_next(bool_t next){
	if( next ){
		putchar(',');
	}
}

__ef_private void json_write_str(char* name, char* value, bool_t next){
	if( !*value ) return;
	putchar('"');
	fputs(name, stdout);
	fputs("\": \"", stdout);
	fputs(value, stdout);
	putchar('"');
	json_next(next);
}

__ef_private void json_write_i3color(char* name, int value, bool_t next){
	if( value < 0 ) return;
	static char* hex = "0123456789ABCDEF"; 
	char out[] = "00000000";
	size_t j = 7;
	while( value > 0 ){
		out[j--] = hex[value & 15];
		value >>= 4;
	}
	printf("\"%s\":\"#%s\"", name, &out[2]);
	json_next(next);
}

__ef_private void json_write_i3align(char* name, align_e value, bool_t next){
	__ef_private char* align[] = {
		"center",
		"right",
		"left"
	};

	if( value < ALIGN_CENTER || value > ALIGN_LEFT ) return;
	
	printf("\"%s\": \"%s\"", name, align[value]);
	json_next(next);
}

__ef_private void json_write_int(char* name, int value, bool_t next){
	if( value < 0 ) return;
	printf("\"%s\": %d", name, value);
	json_next(next);
}

__ef_private void json_write_bool(char* name, int value, bool_t next){
	if( value < 0 ) return;
	printf("\"%s\": %s", name, value ? "true" : "false" );
	json_next(next);
}

__ef_private void i3bar_event_parser(event_s* ev, char* name, size_t lenName, char* value, size_t lenValue){	
	static char* elname[] = {
		"name",
		"instance",
		"x",
		"y",
		"button",
		"relative_x",
		"relative_y",
		"width",
		"height",
		NULL
	};

	static int eltype[] = {
		0,
		0,
		1,
		1,
		1,
		1,
		1,
		1,
		1
	};

	void* elptr[] = {
		ev->name,
		ev->instance,
		&ev->x,
		&ev->y,
		&ev->button,
		&ev->relative_x,
		&ev->relative_y,
		&ev->width,
		&ev->height
	};

	size_t i;
	for( i = 0; elname[i]; ++i){
		if( 0 == str_len_cmp(elname[i], strlen(elname[i]), name, lenName) ){
			if( eltype[i] == 0 ){
				dbg_info("copy %s is string = %.*s", elname[i], (int)lenValue, value);
				str_nncpy_src(elptr[i], ATTRIBUTE_TEXT_MAX, value, lenValue);
			}
			else if( eltype[i] == 1){
				dbg_info("copy %s is long '%.*s'", elname[i], (int)lenValue, value);
				*((int*)elptr[i]) = strtol(value, NULL, 10);
			}
			return;
		}
	}
	dbg_warning("no element '%.*s'", (int)lenName, name); 
}

__ef_private int i3bar_event_lexer(event_s* ev, char* line){
	dbg_info("line:%s",line);
	char* parse = line;

	while( (parse = strchr(parse, '"')) ){
		char* name = parse + 1;
		parse = strchr(name, '"');
		if( NULL == parse ){
			dbg_warning("name not terminated");
			return -1;
		}
		size_t lenName = parse - name;

		parse = strchr(parse, ':');
		++parse;
		parse = str_skip_h(parse);
		char* entok;
		if( *parse == '"'){
			++parse;
			entok = "\"";
		}
		else{
			entok = " \t\n,}";
		}
		char* value = parse;
		parse = strpbrk(parse, entok);
		if( NULL == parse ){
			dbg_warning("value not terminate");
			return -1;
		}
		size_t lenValue = parse - value;
		i3bar_event_parser(ev, name, lenName, value, lenValue);
		++parse;
	}

	return 0;
}

int ipc_onstdin(event_s* ev){
	char inp[2048];
	while( fgets(inp, 2048, stdin) ){
		char* begin = strchr(inp, '{');
		if( !begin ){
			dbg_info("garbage %s",inp);
			continue;
		}
		return i3bar_event_lexer(ev, begin++);
	}
	return -1;
}

void ipc_custom_init(bool_t clickevents){
	dbg_info("ipc i3wm init");
	
	fputs("{ \"version\": 1", stdout);
	if( clickevents ){
		fputs(", \"click_events\": true", stdout);
	}
	puts("}");
	puts("[");
}

void ipc_begin_elements(){
	putchar('[');
}

void ipc_end_elements(){
	puts("],");
	fflush(stdout);
}

void ipc_write_element(attribute_s* el, bool_t next){
	ipc_set_blink_mode(el);

	json_begin_block();
	if( el->longformat[0] == 0 ){
		json_write_str("full_text", " ", TRUE );
	}
	else{
		json_write_str("full_text", el->longformat, TRUE );
	}
	json_write_str("short_text", el->shortformat, TRUE);
	json_write_i3color("color", el->color, TRUE);
	json_write_i3color("background", el->background, TRUE);
	json_write_i3color("border", el->border, TRUE);
	json_write_int("min_width", el->min_width, TRUE);
	json_write_i3align("align", el->align, TRUE);
	json_write_str("name", el->name, TRUE);
	json_write_str("instance", el->instance, TRUE);
	json_write_bool("urgent", el->urgent, TRUE);
	json_write_bool("separator", el->separator, TRUE);
	json_write_int("separator_block_width", el->separator_block_width, TRUE);
	json_write_bool("markup", el->markup, FALSE);
	json_end_block();
	json_next(next);
}


