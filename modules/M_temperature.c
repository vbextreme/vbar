#include <vbar.h>

#ifndef SYS_CLASS_THERMAL
	#define SYS_CLASS_THERMAL "/sys/class/thermal/thermal_zone0/temp"
#endif
#ifndef SYS_CLASS_THERMAL_CRITIC
	#define SYS_CLASS_THERMAL_CRITIC "/sys/class/hwmon/hwmon0/temp1_crit"
#endif


typedef struct temperature{
	size_t temp;
	size_t crit;
	size_t blinkon;
	size_t unit;
}temperature_s;

__ef_private int temp_mod_refresh(module_s* mod){
	temperature_s* tm = mod->data;
	tm->temp = os_read_lu(SYS_CLASS_THERMAL);
	tm->crit = os_read_lu(SYS_CLASS_THERMAL_CRITIC);
	module_set_urgent(mod, tm->temp >= tm->blinkon);
	return 0;
}

__ef_private int temp_mod_env(module_s* mod, int id, char* dest){
	temperature_s* tm = mod->data;
	*dest = 0;
	switch( id ){
		case 0:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)tm->temp / (double)tm->unit);
		break;
		case 1:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)tm->crit / (double)tm->unit);
		break;

		default:
			dbg_error("index to large");
		return -1;
	}
	return 0;
}

__ef_private int temp_mod_free(module_s* mod){
	free(mod->data);
	return 0;
}

int temp_mod_load(module_s* mod, char* path){
	temperature_s* tm = ef_mem_new(temperature_s);
	tm->unit = 1000;
	tm->blinkon = 80000;

	mod->data = tm;
	mod->refresh = temp_mod_refresh;
	mod->getenv = temp_mod_env;
	mod->free = temp_mod_free;

	strcpy(mod->att.longunformat, "temp $0");
	strcpy(mod->att.shortunformat, "$0");
	strcpy(mod->att.name, "generic");
	strcpy(mod->att.instance, "temperature");
	modules_icons_init(mod, 1);
	modules_icons_set(mod, 0, "🌡");

	modules_format_init(mod, 2);
	modules_format_set(mod, 0, "6.2");
	modules_format_set(mod, 1, "6.2");

	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "unit", CNF_LU, &tm->unit, 0, 0);
	config_add(&conf, "blink.on", CNF_LU, &tm->blinkon, 0, 0);
	config_load(&conf, path);
	config_destroy(&conf);
	
	temp_mod_refresh(mod);

	return 0;
}

