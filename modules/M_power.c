#include <vbar.h>

#define POWER_STAT_ERROR "ERR"
#define STATUS_MAX 32
typedef struct powerstat{
	size_t voltageMin; /* uV */
	size_t voltageNow; /* uV */
	size_t energyFull; /* uW/h */
	size_t energyNow; /* uW/h */
	size_t powerNow; /* uW */
	size_t capacity; /* % */
	size_t toblink;
	double timeleft;
	char powersupply[PATH_MAX];
	char status[STATUS_MAX];
}powerstat_s;

#define POWER_SUPPLY "/sys/class/power_supply/BAT0/uevent"

__ef_private void power_stat(powerstat_s* pw){
	struct mapname{
		char* name;
		size_t* ptr;
	}map[] = { 
		{ "VOLTAGE_MIN_DESIGN=" , &pw->voltageMin },
		{ "VOLTAGE_NOW=" , &pw->voltageNow },
		{ "ENERGY_FULL=" , &pw->energyFull },
		{ "ENERGY_NOW=" , &pw->energyNow },
		{ "POWER_NOW=" , &pw->powerNow },
		{ "CAPACITY=" , &pw->capacity },
		{ NULL, NULL }
	};

	FILE* fn = fopen(pw->powersupply, "r");
	if( NULL == fn ) {
		strcpy(pw->status, POWER_STAT_ERROR);
		dbg_warning("error on file %s", pw->powersupply);
		dbg_errno();
		return;
	}

	char in[1024];
	while( fgets(in, 1024, fn) ){
		char* parse = in + strlen("POWER_SUPPLY_");
		if( 0 == strncmp(parse, "STATUS=", strlen("STATUS=")) ){
			parse += strlen("STATUS=");
			str_copy_to_ch(pw->status, STATUS_MAX, parse, '\n');
			continue;
		}
		unsigned i;
		for(i = 0; map[i].name; ++i){
			if( 0 == strncmp(parse, map[i].name, strlen(map[i].name)) ){
				parse = strchr(parse, '=');
				++parse;
				*map[i].ptr = strtol(parse, NULL, 10);
				break;
			}
		}
	}

	fclose(fn);
}

__ef_private int power_mod_refresh(module_s* mod){
	powerstat_s* pw = mod->data;
	power_stat(pw);
	pw->timeleft = (double)pw->energyNow / (double)pw->powerNow;
	if( strcmp(pw->status, "Discharging") ) {	
		modules_icons_select(mod, mod->att.iconcount-1);
	}
	else{
		modules_icons_select(mod,(unsigned)((double)pw->capacity / (100.0 / (double)(mod->att.iconcount-1))));
	}
	module_set_urgent(mod, (pw->capacity < pw->toblink) );	
	return 0;
}

__ef_private int power_mod_env(module_s* mod, int id, char* dest){
	powerstat_s* pw = mod->data;

	switch( id ){
		case 0:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)pw->voltageMin/1000000.0);
		break;
		case 1:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)pw->voltageNow/1000000.0);
		break;
		case 2:
			sprintf(dest, modules_format_get(mod, id, "lu"), pw->capacity);
		break;
		case 3:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)pw->energyFull/1000000.0);
		break;
		case 4:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)pw->energyNow/1000000.0);
		break;
		case 5:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)pw->powerNow/1000000.0);
		break;
		case 6:
			sprintf(dest, modules_format_get(mod, id, "s"), pw->status);
		break;
		case 7:
			sprintf(dest, modules_format_get(mod, id, "u"), (unsigned)pw->timeleft);
		break;
		case 8:
			sprintf(dest, modules_format_get(mod, id, "u"), (unsigned)(pw->timeleft*60)%60);
		break;
		default:
			dbg_error("index to large");
		return -1;
	}
	return 0;
}

__ef_private int power_mod_free(module_s* mod){
	free(mod->data);
	return 0;
}

int power_mod_load(module_s* mod, char* path){
	powerstat_s* pw = ef_mem_new(powerstat_s);
	pw->toblink = 10;
	
	mod->data = pw;
	mod->refresh = power_mod_refresh;
	mod->getenv = power_mod_env;
	mod->free = power_mod_free;

	strcpy(mod->att.longunformat, "power $2% $7:$8");
	strcpy(mod->att.shortunformat, "$2% $7:$8");
	strcpy(mod->att.name, "generic");
	strcpy(mod->att.instance, "power");
	modules_icons_init(mod, 9);
	modules_icons_set(mod, 0, "▁");
   	modules_icons_set(mod, 1, "▂");
   	modules_icons_set(mod, 2, "▃");
   	modules_icons_set(mod, 3, "▄");
   	modules_icons_set(mod, 4, "▅");
	modules_icons_set(mod, 5, "▆");
	modules_icons_set(mod, 6, "▇");
	modules_icons_set(mod, 7, "█");
	modules_icons_set(mod, 8, "🔋");

	modules_format_init(mod, 9);
	for( size_t i = 3; i < 6; ++i){
		modules_format_set(mod, i, "7.2");
	}
	modules_format_set(mod, 0, "5.2");
	modules_format_set(mod, 1, "5.2");
	modules_format_set(mod, 2, "3");
	modules_format_set(mod, 6, "");
	modules_format_set(mod, 7, "2");
	modules_format_set(mod, 8, "2");

	char fname[PATH_MAX] = "BAT0";

	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "blink.on", CNF_LU, &pw->toblink, 0, 0, NULL);
	config_add(&conf, "powersupply", CNF_S, fname, PATH_MAX-33, 0, NULL);
	config_load(&conf, path);
	config_destroy(&conf);
	
	UNSAFE_BEGIN("-Wformat-truncation");
		snprintf(pw->powersupply, PATH_MAX, "/sys/class/power_supply/%s/uevent", fname);
	UNSAFE_END;
	
	if( !file_exists(pw->powersupply) ){
		free(pw);
		return -1;
	}

	power_mod_refresh(mod);
	return 0;
}


