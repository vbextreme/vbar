#include <vbar.h>

#ifndef SYS_DEVICES_SYSTEM_CPUFREQ_CURFQ
	#define SYS_DEVICES_SYSTEM_CPUFREQ_CURFQ "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"
#endif
#ifndef SYS_DEVICES_SYSTEM_CPUFREQ_MINFQ
	#define SYS_DEVICES_SYSTEM_CPUFREQ_MINFQ "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq"
#endif
#ifndef SYS_DEVICES_SYSTEM_CPUFREQ_MAXFQ
	#define SYS_DEVICES_SYSTEM_CPUFREQ_MAXFQ "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#endif

#ifndef SYS_DEVICES_SYSTEM_CPUFREQ_GOV
	#define SYS_DEVICES_SYSTEM_CPUFREQ_GOV "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
#endif

#ifndef SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV
	#define SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors"
#endif

#ifndef GOVERNOR_MAX
	#define GOVERNOR_MAX 12
#endif

#ifndef GOVERNOR_NAME_MAX
	#define GOVERNOR_NAME_MAX 24
#endif

typedef struct cpufreq{
	size_t minfq;
	size_t maxfq;
	size_t curfq;
	char governor[GOVERNOR_MAX][GOVERNOR_NAME_MAX];
	size_t curgovernor;
	size_t countgovernor;
	size_t unit;
}cpufreq_s;

__ef_private void cpufreq_read_gov(cpufreq_s* cf){
	FILE* fd = fopen( SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV, "r");
	if( fd == NULL ){
		dbg_error("fopen");
		dbg_errno();
		return;
	}

	char inp[4096];
	inp[0] = 0;
	fgets(inp, 4096, fd);

	char* parse = inp;
	cf->countgovernor = 0;
	while( *parse ){
		size_t cp = 0;
		while( *parse && *parse != ' ' && *parse != '\t' && *parse != '\n' ){
			cf->governor[cf->countgovernor][cp++] = *parse++;
		}
		cf->governor[cf->countgovernor++][cp] = 0;

		while( *parse && (*parse == ' ' || *parse == '\t' || *parse == '\n') ){
			++parse;
		}
	}
	fclose(fd);
}

__ef_private void cpufreq_select_gov(cpufreq_s* cf){
	FILE* fd = fopen( SYS_DEVICES_SYSTEM_CPUFREQ_GOV, "r");
	if( fd == NULL ){
		dbg_error("fopen");
		dbg_errno();
		return;
	}

	char inp[80];
	inp[0] = 0;
	fgets(inp, 80, fd);
	size_t len = strlen(inp);
	if( inp[len-1] == '\n' ){
	   inp[len-1] = 0;
	}	   
	fclose(fd);

	for(cf->curgovernor = 0; cf->curgovernor < cf->countgovernor && strcmp(cf->governor[cf->curgovernor], inp); ++cf->curgovernor);
}

__ef_private int cpufreq_mod_refresh(module_s* mod){
	cpufreq_s* cf = mod->data;
	cf->minfq = os_read_lu(SYS_DEVICES_SYSTEM_CPUFREQ_MINFQ);
	cf->maxfq = os_read_lu(SYS_DEVICES_SYSTEM_CPUFREQ_MAXFQ);
	cf->curfq = os_read_lu(SYS_DEVICES_SYSTEM_CPUFREQ_CURFQ);
	cpufreq_read_gov(cf);
	cpufreq_select_gov(cf);
	return 0;
}

__ef_private int cpufreq_mod_env(module_s* mod, int id, char* dest){
	cpufreq_s* cf = mod->data;
	switch( id ){
		case 0:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)cf->minfq / (double)cf->unit);	
		break;

		case 1:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)cf->maxfq / (double)cf->unit);	
		break;
		
		case 2:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)cf->curfq / (double)cf->unit);
		break;
		
		case 3:
			for( size_t i = 0; i < cf->countgovernor; ++i ){
				sprintf(dest, modules_format_get(mod, id, "s"), cf->governor[i]);
				dest += strlen(cf->governor[i]);
				*dest++ = ' ';
			}
			--dest;
			*dest = 0;
		break;
		
		case 4:
			sprintf(dest, modules_format_get(mod, id, "s"), cf->governor[cf->curgovernor]);	
		break;

		default:
			dbg_error("index to large");
		return -1;
	}
	return 0;
}

__ef_private int cpufreq_mod_free(module_s* mod){
	free(mod->data);
	return 0;
}

int cpufreq_mod_load(module_s* mod, char* path){
	if( !file_exists(SYS_DEVICES_SYSTEM_CPUFREQ_MINFQ) ||
		!file_exists(SYS_DEVICES_SYSTEM_CPUFREQ_MAXFQ) ||
		!file_exists(SYS_DEVICES_SYSTEM_CPUFREQ_CURFQ) ||
		!file_exists(SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV) ||
		!file_exists(SYS_DEVICES_SYSTEM_CPUFREQ_GOV)
	){
		return -1;
	}

	cpufreq_s* cf = ef_mem_new(cpufreq_s);
	cf->minfq = 0;
	cf->maxfq = 0;
	cf->countgovernor = 0;
	cf->curfq = 0;
	cf->curgovernor = 0;
	cf->governor[0][0] = 0;
	cf->unit = 1000000;

	mod->data = cf;
	mod->refresh = cpufreq_mod_refresh;
	mod->getenv = cpufreq_mod_env;
	mod->free = cpufreq_mod_free;

	strcpy(mod->att.longunformat, "freq $2 $4%");
	strcpy(mod->att.shortunformat, "$2 $4%");
	mod->att.reftime = 1000;
	strcpy(mod->att.name, "generic");
	strcpy(mod->att.instance, "cpufreq");
	modules_icons_init(mod, 1);
	modules_icons_set(mod, 0, "💻");
	modules_format_init(mod, 5);
	for( size_t i = 0; i < 3; ++i){
		modules_format_set(mod, i, "4.2");
	}
	for( size_t i = 3; i < 5; ++i){
		modules_format_set(mod, i, "");
	}

	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "unit", CNF_LU, &cf->unit, 0, 0, NULL);
	config_load(&conf, path);
	config_destroy(&conf);
	
	if( cf->unit < 1 ) cf->unit = 1;
	cpufreq_mod_refresh(mod);

	return 0;
}


