#include <vbar.h>

#ifndef SYS_CLASS_THERMAL
	#define SYS_CLASS_THERMAL "/sys/class/thermal/thermal_zone0/temp"
#endif
#ifndef SYS_CLASS_HWMON_CRITIC
	#define SYS_CLASS_HWMON_CRITIC "/sys/class/hwmon/hwmon0/temp1_crit"
#endif

typedef struct temperature{
	size_t temp;
	size_t crit;
	size_t blinkon;
	size_t unit;
	char pthermal[PATH_MAX];
	char pcritic[PATH_MAX];
}temperature_s;

__ef_private int temp_mod_refresh(module_s* mod){
	temperature_s* tm = mod->data;
	tm->temp = os_read_lu(tm->pthermal);
	tm->crit = os_read_lu(tm->pcritic);
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

int temperature_mod_load(module_s* mod, char* path){
	
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
	
	int sensor = 0;
	int temp = 1;
	int sensor_critic = 0;
	int temp_critic = 1;

	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "unit", CNF_LU, &tm->unit, 0, 0, NULL);
	config_add(&conf, "blink.on", CNF_LU, &tm->blinkon, 0, 0, NULL);
	config_add(&conf, "sensor", CNF_D, &sensor, 0, 0, NULL);
	config_add(&conf, "sensor.temp", CNF_D, &sensor, 0, 0, NULL);
	config_add(&conf, "sensor.critic", CNF_D, &sensor_critic, 0, 0, NULL);
	config_add(&conf, "sensor.critic.temp", CNF_D, &temp_critic, 0, 0, NULL);
	config_load(&conf, path);
	config_destroy(&conf);
	
	if( sensor < 0 ){
		sprintf(tm->pthermal, "/sys/class/thermal/thermal_zone%d/temp", temp);
	}
	else{
		sprintf(tm->pthermal, "/sys/class/hwmon/hwmon%d/temp%d_input", sensor, temp);
	}
	if( sensor_critic < 0 ){
		sprintf(tm->pcritic, "/sys/class/thermal/thermal_zone%d/temp", temp_critic);
	}
	else{
		sprintf(tm->pcritic, "/sys/class/hwmon/hwmon%d/temp%d_crit", sensor_critic, temp_critic);
	}
	dbg_info("temp %s", tm->pthermal);
	dbg_info("crit %s", tm->pcritic);

	if( !file_exists(tm->pthermal) || !file_exists(tm->pcritic) ){
		free(tm);
		return -1;
	}

	if( tm->unit < 1 ) tm->unit = 1;	
	temp_mod_refresh(mod);

	return 0;
}

