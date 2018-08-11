#include <vbar.h>
#include <time.h>

typedef enum{ DT_DY, DT_DM, DT_DD, DT_TH, DT_TM, DT_TS, DT_COUNT } datetime_e;

typedef struct datetime{
	unsigned long toblink;
	datetime_e dt[DT_COUNT];
	char format[DT_COUNT][MAX_FORMAT];
}datetime_s;

__ef_private int datetime_mod_refresh(module_s* mod){
	datetime_s* dt = mod->data;
	time_t t = time(NULL);
	struct tm* tm = localtime(&t);
	dt->dt[DT_DY] = tm->tm_year;
	dt->dt[DT_DM] = tm->tm_mon;
	dt->dt[DT_DD] = tm->tm_mday;
	dt->dt[DT_TH] = tm->tm_hour;
	dt->dt[DT_TM] = tm->tm_min;
	dt->dt[DT_TS] = tm->tm_sec;

	module_set_urgent(mod, (time_t)dt->toblink == t );	
	return 0;
}

__ef_private int datetime_mod_env(module_s* mod, int id, char* dest){
	datetime_s* dt = mod->data;
	if( (unsigned)id >= DT_COUNT ){
		dbg_error("index to large");
		return 0;
	}
	sprintf(dest, dt->format[id], dt->dt[id]);	
	return 0;
}

__ef_private int datetime_mod_free(module_s* mod){
	free(mod->data);
	return 0;
}

int datetime_mod_load(module_s* mod, char* path){
	datetime_s* dt = ef_mem_new(datetime_s);
	dt->toblink = 0;
	for( size_t i = 0; i < DT_COUNT; ++i){
		strcpy(dt->format[i], "%d");
	}
	mod->data = dt;
	mod->refresh = datetime_mod_refresh;
	mod->getenv = datetime_mod_env;
	mod->free = datetime_mod_free;
	mod->blink = FALSE;
	mod->blinktime = 500;
	mod->blinkstatus = 0;
	strcpy(mod->longformat, "date $2/$01/$0");
	strcpy(mod->shortformat, "$2/$1/$0");
	mod->reftime = 1000;
	strcpy(mod->i3.name, "generic");
	strcpy(mod->i3.instance, "datetime");
	modules_icons_init(mod, 1);
	modules_icons_set(mod, 0, "⌚");

	datetime_mod_refresh(mod);

	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "blink.on", CNF_LF, &dt->toblink, 0, 0);
	config_add(&conf, "format", CNF_S, dt->format, MAX_FORMAT, DT_COUNT);
	config_load(&conf, path);
	config_destroy(&conf);

	mod->tick = mod->reftime + time_ms();
	return 0;
}


