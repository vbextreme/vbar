#include <vbar.h>
#include "intp.h"
#include "hash_intp.h"

#define ARGS_MAX 4

typedef struct intpcmd{
	char* name;
	intpcall_f call;
	void* autoarg;
}intpcmd_s;

typedef struct intpapi{
	intpcmd_s cmd[HINTP_MAX_HASH_VALUE + 1];
}intpapi_s;

__ef_private intpapi_s intp;

void intp_register_command(char* name, intpcall_f call, void* autoarg){
	size_t hash = hash_intp(name, strlen(name));
	dbg_info("%s [%lu]", name, hash);

	if( intp.cmd[hash].call ){
		dbg_fail("hash collision on %s hash %lu", name, hash);
	}
	intp.cmd[hash].name = name;
	intp.cmd[hash].call = call;
	intp.cmd[hash].autoarg = autoarg;
}

typedef struct blkel{
	char* name;
	size_t lenName;
	char* arg[ARGS_MAX];
	size_t lenArg[ARGS_MAX];
	size_t count;
}blkel_s;

__ef_private __ef_can_null char* intp_parse(blkel_s* blk, char* line){
#ifdef EF_DEBUG_ENABLE
	char* stline = line;
#endif

	while( *line && (*line == '{' || *line == ' ' || *line == '\t') ) ++line;
	
	blk->name = line;
	blk->count = 0;
	while( *line && *line != '(' && *line != '}' ) ++line;

	if( !(blk->lenName = line - blk->name) ){
		dbg_warning("no function name");
		dbg_warning("%s", stline);
		return NULL;
	}

	if( *line == '(' ){
		++line;
		
		for(; blk->count < ARGS_MAX; ++blk->count ){
			while( *line && (*line == ' ' || *line == '\t') ) ++line;
			blk->arg[blk->count] = line;
			while( *line && *line != ')' && *line != ',') ++line;
			if( !*line ){
				dbg_warning("no close call in loop arguments");
				dbg_warning("line::%s",stline);
				dbg_warning("parsed::%s", line);
				return NULL;
			}
			blk->lenArg[blk->count] = line - blk->arg[blk->count];
			if( *line == ')' ) break;
			++line;
		}
		if( blk->count ) ++blk->count;

		if( *line != ')' ){
			dbg_warning("no close call");
			dbg_warning("line::%s", stline);
			dbg_warning("parsed::%s", line);
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

__ef_private __ef_can_null intpcmd_s* intp_find(char* name, size_t len){
	size_t hash = hash_intp( name, len );
	dbg_info("%.*s hash %lu/%d", (int)len, name, hash,HINTP_MAX_HASH_VALUE);
	if( hash > HINTP_MAX_HASH_VALUE ){
		return NULL;
	}
	intpcmd_s* find = &intp.cmd[hash];
	return find->call ? find : NULL;
}

__ef_can_null char* intp_interpretate(char* line, module_s* th){
	blkel_s blk;

	if( !(line = intp_parse(&blk, line)) ){
		return NULL;
	}

	intpcmd_s* cmd = intp_find(blk.name, blk.lenName);
	if( !cmd ){
		dbg_warning("function %.*s not exists", (int)blk.lenName, blk.name);
		return NULL;
	}
	
	if( strncmp(cmd->name, blk.name, blk.lenName) ){
		dbg_warning("function %.*s not exists", (int)blk.lenName, blk.name);
		return NULL;
	}

	dbg_info("%.*s(%lu)", (int)blk.lenName, blk.name, blk.count);
	cmd->call(cmd->autoarg, th, blk.count, blk.arg, blk.lenArg);
	return line+1;
}

