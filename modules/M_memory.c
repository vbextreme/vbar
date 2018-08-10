#include <vbar.h>

#define MAX_FORMAT 24
#define SYSINFO_ELEMS 14
typedef struct mem{
	struct sysinfo si;
	unsigned long used;
	unsigned long unit;
	unsigned long toblink;
	char format[SYSINFO_ELEMS][MAX_FORMAT];
}mem_s;

__ef_private int mem_mod_refresh(module_s* mod){
	mem_s* mem = mod->data;
	sysinfo(&mem->si);
	mem->used = mem->si.totalram - mem->si.freeram;
	mod->blinkstatus = ( mod->blink &&  mem->si.freeram < mem->toblink) ? TRUE : FALSE;
	
	return 0;
}

__ef_private int mem_mod_env(module_s* mod, int id, char* dest){
	mem_s* mem = mod->data;
	switch( id ){
		case 0:
			sprintf(dest, mem->format[id], mem->si.uptime);
		break;
		case 1:
			sprintf(dest, mem->format[id], mem->si.loads[0]);
		break;
		case 2:
			sprintf(dest, mem->format[id], mem->si.loads[1]);
		break;
		case 3:
			sprintf(dest, mem->format[id], mem->si.loads[2]);
		break;
		case 4:
			sprintf(dest, mem->format[id], (double)mem->si.totalram / mem->unit);
		break;
		case 5:
			sprintf(dest, mem->format[id], (double)mem->si.freeram / mem->unit);
		break;
		case 6:
			sprintf(dest, mem->format[id], (double)mem->si.sharedram / mem->unit);
		break;
		case 7:
			sprintf(dest, mem->format[id], (double)mem->si.bufferram / mem->unit);
		break;
		case 8:
			sprintf(dest, mem->format[id], (double)mem->si.totalswap / mem->unit);
		break;
		case 9:
			sprintf(dest, mem->format[id], (double)mem->si.freeswap / mem->unit);
		break;
		case 10:
			sprintf(dest, mem->format[id], mem->si.procs);
		break;
		case 11:
			sprintf(dest, mem->format[id], (double)mem->si.totalhigh / mem->unit);
		break;
		case 12:
			sprintf(dest, mem->format[id], (double)mem->si.freehigh / mem->unit);
		break;
		case 13:
			sprintf(dest, mem->format[id], (double)mem->used / (double)mem->unit);
		break;
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
	strcpy(mem->format[0], "%ld");
	strcpy(mem->format[10], "%d");
	for( size_t i = 1; i < 4; ++i){
		strcpy(mem->format[i], "%lu");
	}
	for( size_t i = 4; i < SYSINFO_ELEMS; ++i){
		strcpy(mem->format[i], "%.2lf");
	}

	mod->data = mem;
	mod->refresh = mem_mod_refresh;
	mod->getenv = mem_mod_env;
	mod->free = mem_mod_free;
	mod->blink = TRUE;
	mod->blinktime = 500;
	mod->blinkstatus = 0;
	strcpy(mod->longformat, "mem $13GiB");
	strcpy(mod->shortformat, "$13GiB");
	mod->reftime = 400;
	strcpy(mod->i3.name, "generic");
	strcpy(mod->i3.instance, "memory");	
	modules_icons_init(mod, 1);
	modules_icons_set(mod, 0, "🌀");

	mem_mod_refresh(mod);

	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "blink.on", CNF_LF, &mem->toblink, 0, 0);
	config_add(&conf, "format", CNF_S, mem->format, MAX_FORMAT, SYSINFO_ELEMS);
	config_load(&conf, path);
	config_destroy(&conf);
	mod->tick = mod->reftime + time_ms();
	
	return 0;
}



