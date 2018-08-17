#include <vbar.h>

#define SYSINFO_ELEMS 14
typedef struct mem{
	struct sysinfo si;
	unsigned long used;
	unsigned long unit;
	unsigned long toblink;
}mem_s;

__ef_private int mem_mod_refresh(module_s* mod){
	mem_s* mem = mod->data;
	sysinfo(&mem->si);
	mem->used = mem->si.totalram - mem->si.freeram;
	module_set_urgent(mod, mem->si.freeram < mem->toblink);
	return 0;
}

__ef_private int mem_mod_env(module_s* mod, int id, char* dest){
	mem_s* mem = mod->data;
	switch( id ){
		case 0:
			sprintf(dest, modules_format_get(mod, id, "ld"), mem->si.uptime);
		break;
		case 1:
			sprintf(dest, modules_format_get(mod, id, "lu"), mem->si.loads[0]);
		break;
		case 2:
			sprintf(dest, modules_format_get(mod, id, "lu"), mem->si.loads[1]);
		break;
		case 3:
			sprintf(dest, modules_format_get(mod, id, "lu"), mem->si.loads[2]);
		break;
		case 4:
			sprintf(dest, modules_format_get(mod, id, "lu"), (double)mem->si.totalram / mem->unit);
		break;
		case 5:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->si.freeram / mem->unit);
		break;
		case 6:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->si.sharedram / mem->unit);
		break;
		case 7:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->si.bufferram / mem->unit);
		break;
		case 8:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->si.totalswap / mem->unit);
		break;
		case 9:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->si.freeswap / mem->unit);
		break;
		case 10:
			sprintf(dest, modules_format_get(mod, id, "d"), mem->si.procs);
		break;
		case 11:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->si.totalhigh / mem->unit);
		break;
		case 12:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->si.freehigh / mem->unit);
		break;
		case 13:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)mem->used / (double)mem->unit);
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
	modules_format_init(mod, SYSINFO_ELEMS);
	for( size_t i = 0; i < 4; ++i){
		modules_format_set(mod, i, "3");
	}
	for( size_t i = 4; i < SYSINFO_ELEMS; ++i){
		modules_format_set(mod, i, "5.2");
	}
	modules_format_set(mod, 10, "3");

	mem_mod_refresh(mod);

	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "blink.on", CNF_LU, &mem->toblink, 0, 0);
	config_add(&conf, "unit", CNF_LU, &mem->unit, 0, 0);
	config_load(&conf, path);
	config_destroy(&conf);
	
	return 0;
}



