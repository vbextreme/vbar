#include <vbar.h>
#include "intp.h"

#define ARGS_MAX 4
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
	char* arg[ARGS_MAX];
	size_t lenArg[ARGS_MAX];
	size_t count;
}blkel_s;

__ef_private char* intp_parse(blkel_s* blk, char* line){
#ifdef EF_DEBUG_ENABLE
	char* stline = line;
#endif

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
		
		for( blk->count = 0; blk->count < ARGS_MAX; ++blk->count ){
			while( *line && (*line == ' ' || *line == '\t') ) ++line;
			blk->arg[blk->count] = line;
			while( *line && *line != ')' && *line != ',') ++line;
			if( !*line ){
				dbg_warning("no close call");
				dbg_warning("%s",stline);
				return NULL;
			}
			blk->lenArg[blk->count] = line - blk->arg[blk->count];
			if( *line == ')' ) break;
		}
		++blk->count;

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

	if( !(line = intp_parse(&blk, line)) ){
		return NULL;
	}

	intpcmd_s* cmd = intp_find(blk.name, blk.lenName);
	if( !cmd ){
		dbg_warning("function %.*s not exists", (int)blk.lenName, blk.name);
		return NULL;
	}

	cmd->call(cmd->autoarg, blk.count, blk.arg, blk.lenArg);
	return line+1;
}

