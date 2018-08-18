#include <vbar.h>

/* sysinfo return incorrect value and not return cached, used /proc/meminfo */

#define PROC_MEM "/proc/meminfo"
#define MEMINFO_ELEMS 8
typedef struct mem{
	size_t total;
	size_t free;
	size_t available;
	size_t buffers;
	size_t cached;
	size_t totalswap;
	size_t freeswap;
	size_t shared;
	size_t SReclaimable;
	size_t SUnreclaim;
	size_t used;
	size_t unit;
	size_t toblink;
}mem_s;

__ef_private size_t mem_parse(char* line){
	while( *line && (*line < '0' || *line > '9') ) ++line;
	iassert( *line );
	size_t ret = strtoul(line, NULL, 10);
	return ret;
}

__ef_private void mem_read(mem_s* mem){
	__ef_private char* col[] = {
		"MemTotal:",
		"MemFree:",
		"MemAvailable:",
		"Buffers:",
		"Cached:",
		"SwapTotal:",
		"SwapFree:",
		"Shmem:",
		"SReclaimable:",
		"SUnreclaim:",
		NULL
	};

	size_t* memptr[] = {
		&mem->total,
		&mem->free,
		&mem->available,
		&mem->buffers,
		&mem->cached,
		&mem->totalswap,
		&mem->freeswap,
		&mem->shared,
		&mem->SReclaimable,
		&mem->SUnreclaim
	};

	FILE* fm = fopen(PROC_MEM, "r");
	if( fm == NULL ) {
		dbg_error("%s not available", PROC_MEM);
		dbg_errno();
		return;
	}
	
	char inp[1024];
	while( fgets(inp, 1024, fm) ){
		for( size_t i = 0; col[i]; ++i ){
			if( !strncmp(inp, col[i], strlen(col[i])) ){
				*memptr[i] = mem_parse(inp);
			}
		}
	}

	fclose(fm);
}

__ef_private int mem_mod_refresh(module_s* mod){
	mem_s* mem = mod->data;
	mem_read(mem);
	mem->used = mem->total - (mem->free + mem->buffers + mem->cached + mem->SReclaimable);
	module_set_urgent(mod, (mem->total - mem->used) < mem->toblink);
	return 0;
}

__ef_private int mem_mod_env(module_s* mod, int id, char* dest){
	mem_s* mem = mod->data;
	switch( id ){
		case 0:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->total / mem->unit);
		break;
		case 1:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->free / mem->unit);
		break;
		case 2:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->shared / mem->unit);
		break;
		case 3:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)(mem->buffers+mem->cached + mem->SReclaimable) / mem->unit);
		break;
		case 4:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->available / mem->unit);
		break;
		case 5:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->totalswap / mem->unit);
		break;
		case 6:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->freeswap / mem->unit);
		break;
		case 7:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->used / (double)mem->unit);
		break;
		default:
			dbg_error("index to large");
		return -1;

	}
	return 0;
}

__ef_private int mem_mod_free(module_s* mod){
	free(mod->data);
	return 0;
}

int mem_mod_load(module_s* mod, char* path){
	mem_s* mem = ef_mem_new(mem_s);
	mem->toblink = 1024*1024*500;
	mem->unit = 1024*1024*1024;

	mod->data = mem;
	mod->refresh = mem_mod_refresh;
	mod->getenv = mem_mod_env;
	mod->free = mem_mod_free;

	strcpy(mod->att.longunformat, "mem $13GiB");
	strcpy(mod->att.shortformat, "$13GiB");
	strcpy(mod->att.name, "generic");
	strcpy(mod->att.instance, "memory");	
	modules_icons_init(mod, 1);
	modules_icons_set(mod, 0, "🌀");
	modules_format_init(mod, MEMINFO_ELEMS);
	for( size_t i = 0; i < MEMINFO_ELEMS; ++i){
		modules_format_set(mod, i, "5.2");
	}

	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "blink.on", CNF_LU, &mem->toblink, 0, 0);
	config_add(&conf, "unit", CNF_LU, &mem->unit, 0, 0);
	config_load(&conf, path);
	config_destroy(&conf);
	
	mem_mod_refresh(mod);

	return 0;
}



