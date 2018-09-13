#include <vbar.h>

#ifndef NCORES_MAX
	#define NCORES_MAX 64
#endif

#define SYS_DEVICES_SYSTEM_CPU "/sys/devices/system/cpu/cpu"
#define CPUFREQ "/cpufreq"

#ifndef CPUFREQ_CURFQ
	#define CPUFREQ_CURFQ "/scaling_cur_freq"
#endif
#ifndef CPUFREQ_MINFQ
	#define CPUFREQ_MINFQ "/scaling_min_freq"
#endif
#ifndef CPUFREQ_MAXFQ
	#define CPUFREQ_MAXFQ "/scaling_max_freq"
#endif
#ifndef CPUFREQ_GOV
	#define CPUFREQ_GOV "/scaling_governor"
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
	size_t minfq[NCORES_MAX];
	size_t maxfq[NCORES_MAX];
	size_t curfq[NCORES_MAX];
	char governor[GOVERNOR_MAX][GOVERNOR_NAME_MAX];
	size_t countgovernor;
	size_t curgovernor[NCORES_MAX];
	size_t unit;
}cpufreq_s;

__ef_private char* cpufreq_filename(char* fname, size_t idcpu){
	static char cf[PATH_MAX] = SYS_DEVICES_SYSTEM_CPU;
	sprintf(&cf[strlen(SYS_DEVICES_SYSTEM_CPU)], "%lu" CPUFREQ "%s", idcpu, fname);
	return cf;
}

__ef_private void cpufreq_read_gov(cpufreq_s* cf){
	__ef_file_autoclose file_t* fd = fopen( SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV, "r");
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
}

__ef_private void cpufreq_select_gov(cpufreq_s* cf, size_t idcore){
	__ef_file_autoclose file_t* fd = fopen( cpufreq_filename(CPUFREQ_GOV, idcore), "r");
	if( fd == NULL ){
		dbg_error("fopen %s", cpufreq_filename(CPUFREQ_GOV, idcore));
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

	for(cf->curgovernor[idcore] = 0; cf->curgovernor[idcore] < cf->countgovernor && strcmp(cf->governor[cf->curgovernor[idcore]], inp); ++cf->curgovernor[idcore]);
}

__ef_private int cpufreq_mod_refresh(module_s* mod){
	cpufreq_s* cf = mod->data;
	cpufreq_read_gov(cf);
	for(size_t i = 0; i < NCORES_MAX; ++i){
		cf->minfq[i] = os_read_lu(cpufreq_filename(CPUFREQ_MINFQ,i));
		cf->maxfq[i] = os_read_lu(cpufreq_filename(CPUFREQ_MAXFQ,i));
		cf->curfq[i] = os_read_lu(cpufreq_filename(CPUFREQ_CURFQ,i));
		cpufreq_select_gov(cf,i);
	}
	return 0;
}

__ef_private int cpufreq_mod_env(module_s* mod, int id, char* dest){
	cpufreq_s* cf = mod->data;
	if( id == 0 ){ 
		for( size_t i = 0; i < cf->countgovernor; ++i ){
			sprintf(dest, modules_format_get(mod, id, "s"), cf->governor[i]);
			dest += strlen(cf->governor[i]);
			*dest++ = ' ';
		}
		--dest;
		*dest = 0;
		return 0;
	}

	size_t idcore = ((id-1) / 4);

	if( idcore >= NCORES_MAX ){
		dbg_error("index > ncores");
		return -1;
	}
	
	id = id % 4;
	switch( id ){
		case 1:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)cf->minfq[idcore] / (double)cf->unit);	
		break;

		case 2:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)cf->maxfq[idcore] / (double)cf->unit);	
		break;
		
		case 3:
			sprintf(dest, modules_format_get(mod, id, "lf"), (double)cf->curfq[idcore] / (double)cf->unit);
		break;
		
		case 0:
			sprintf(dest, modules_format_get(mod, id, "s"), cf->governor[cf->curgovernor[idcore]]);	
		break;

		default:
			dbg_error("index to large %d", id);
		return -1;
	}
	return 0;
}

__ef_private int cpufreq_mod_free(module_s* mod){
	free(mod->data);
	return 0;
}

int cpufreq_mod_load(module_s* mod, char* path){
	if( !file_exists(cpufreq_filename(CPUFREQ_MINFQ,0)) ||
		!file_exists(cpufreq_filename(CPUFREQ_MAXFQ,0)) ||
		!file_exists(cpufreq_filename(CPUFREQ_CURFQ,0)) ||
		!file_exists(SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV) ||
		!file_exists(cpufreq_filename(CPUFREQ_GOV,0))
	){
		return -1;
	}

	cpufreq_s* cf = ef_mem_new(cpufreq_s);
	ef_mem_clear(cpufreq_s, cf);	
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
	modules_format_init(mod, NCORES_MAX * 4 + 1);
	modules_format_set(mod, 0, "");

	for( size_t i = 1; i < NCORES_MAX * 4; i+=4){
		modules_format_set(mod, i, "");
		modules_format_set(mod, i, "4.2");
		modules_format_set(mod, i, "4.2");
		modules_format_set(mod, i, "4.2");
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


