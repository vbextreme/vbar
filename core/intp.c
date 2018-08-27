#include <vbar.h>
#include "intp.h"

#define HASH_CMD 256

typedef struct intpcmd{
	struct intpcmd* next;
	char* name;
	intpcall_f call;
	void* autoarg;
}intpcmd_s;

typedef struct intpapi{
	intpcmd_s* cmd[HASH_CMD];
}intpapi_s;

__ef_private intpapi_s intp;

void intp_register_command(char* name, intpcall_f call, void* autoarg){
	dbg_info("%s", name);
	size_t hash = kr_hash(name, HASH_CMD);
	intpcmd_s* cmd = ef_mem_new(intpcmd_s);
	cmd->name = name;
	cmd->call = call;
	cmd->autoarg = autoarg;
	cmd->next = intp.cmd[hash];
	intp.cmd[hash] = cmd;
}

typedef struct blkel{
	char* name;
	size_t lenName;
	char* arg0;
	size_t lenArg0;
	char* arg1;
	size_t lenArg1;
}blkel_s;

__ef_private char* intp_parse(blkel_s* blk, char* line){
#ifdef EF_DEBUG_ENABLE
	char* stline = line;
#endif
	blk->lenName = 0;
	blk->lenArg0 = 0;
	blk->lenArg1 = 0;

	while( *line && (*line == '{' || *line == ' ' || *line == '\t') ) ++line;
	
	blk->name = line;
	while( *line && *line != '(' && *line != '}' ) ++line;

	if( !(blk->lenName = line - blk->name) ){
		dbg_warning("no function name");
		dbg_warning("%s", stline);
		return NULL;
	}

	if( *line == '(' ){
		++line;
		while( *line && (*line == ' ' || *line == '\t') ) ++line;
		blk->arg0 = line;
		while( *line && *line != ')' && *line != ',') ++line;
		if( !*line ){
			dbg_warning("no close call");
			dbg_warning("%s",stline);
			return NULL;
		}
		blk->lenArg0 = line - blk->arg0;

		if( *line == ',' ){
			++line;
			while( *line && (*line == ' ' || *line == '\t') ) ++line;
			blk->arg1 = line;
			while( *line && *line != ')') ++line;
				if( !*line ){
				dbg_warning("no close call");
				dbg_warning("%s",stline);
				return NULL;
			}
			blk->lenArg1 = line - blk->arg1;
		}

		if( *line != ')' ){
			dbg_warning("no close call");
			dbg_warning("%s",stline);
			return NULL;
		}
		while(*line && *line != '}') ++line;
	}

	if( *line != '}' ){
		dbg_warning("no close command");
		dbg_warning("%s",stline);
		return NULL;
	}

	dbg_info("parse: %.*s %.*s %.*s", (int)blk->lenName, blk->name, (int)blk->lenArg0, blk->arg0, (int)blk->lenArg1, blk->arg1);
	return line;
}

__ef_private intpcmd_s* intp_find(char* name, size_t len){
	size_t hash = kr_nhash( name, len, HASH_CMD );
	intpcmd_s* find = intp.cmd[hash];
	for(; find && str_len_cmp(find->name, strlen(find->name), name, len);	find = find->next);
	return find;
}

char* intp_interpretate(char* line){
	blkel_s blk;
	blk.arg1 = NULL;
	blk.arg0 = NULL;

	if( !(line = intp_parse(&blk, line)) ){
		return NULL;
	}

	intpcmd_s* cmd = intp_find(blk.name, blk.lenName);
	if( !cmd ){
		dbg_warning("function %.*s not exists", (int)blk.lenName, blk.name);
		return NULL;
	}

	cmd->call(cmd->autoarg, blk.arg0, blk.lenArg0, blk.arg1, blk.lenArg1);
	return line+1;
}

