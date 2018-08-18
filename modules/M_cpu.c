#include <vbar.h>

#ifndef PROC_STAT
	#define PROC_STAT "/proc/stat"
#endif

#ifndef NCORES_MAX
	#define NCORES_MAX 64
#endif
typedef enum { CPU_USER, CPU_NICE, CPU_SYSTEM, CPU_IDLE, CPU_IOWAIT, CPU_IRQ, CPU_SOFTIRQ, CPU_STEAL, CPU_GUEST, CPU_GUEST_NICE, CPU_TIME_COUNT } cputime_e;

typedef struct procCpu{
	double toblink;
	int ncores;
	size_t current;
	uint64_t tick[2][NCORES_MAX][CPU_TIME_COUNT];
}procCpu_s;

__ef_private int cpu_count(void){
	return sysconf(_SC_NPROCESSORS_ONLN);
}

__ef_private void cpu_time(procCpu_s* cpu) {
	char in[1024];
	FILE* fps = fopen(PROC_STAT, "r");
	if( fps == NULL ) {
		dbg_error("%s not available", PROC_STAT);
		dbg_errno();
		return;
	}

	int i,k;
	for( i = 0; i < cpu->ncores; ++i ){
		if( NULL == fgets(in, 1024, fps) ){
			dbg_error("fgets");
			fclose(fps);
			return;
		}
		
		char* parse;
		if( (parse = strpbrk(in, " \t")) == NULL){
			dbg_error("on skip cpu");
			fclose(fps);
			return;
		}

		for(k = 0; k < CPU_TIME_COUNT; ++k){
			char* en;
			parse = str_skip_h(parse);
			cpu->tick[cpu->current][i][k] = strtol(parse, &en, 10);
			parse = en;
		}
	}
	
	fclose(fps);
}

__ef_private uint64_t cpu_time_tick(uint64_t* st){
	int i;
	uint64_t full = 0;
	for( i = 0; i < CPU_TIME_COUNT; ++i ){
		full += *st++;
	}
	return full;
}

__ef_private uint64_t cpu_tick_elapse(uint64_t* st, uint64_t* en, cputime_e ct){
	return en[ct] - st[ct];
}

__ef_private double cpu_load_average(uint64_t tick, uint64_t idle){
	return ((double)(tick - idle) / (double)tick) * 100.00; 
}

__ef_private double cpu_average(procCpu_s* cpu, int id){
	size_t cur = cpu->current;
	size_t old = (cur - 1) & 1;
	uint64_t tick = cpu_time_tick(cpu->tick[cur][id]) - cpu_time_tick(cpu->tick[old][id]);
	uint64_t idle = cpu_tick_elapse(cpu->tick[old][id], cpu->tick[cur][id], CPU_IDLE);
	return cpu_load_average(tick, idle);
}

__ef_private int cpu_mod_refresh(module_s* mod){
	procCpu_s* cpu = mod->data;
	cpu->current = (cpu->current + 1) & 1;
	cpu_time(cpu);
	module_set_urgent(mod, cpu_average(cpu, 0) > cpu->toblink );
	return 0;
}

__ef_private int cpu_mod_env(module_s* mod, int id, char* dest){
	procCpu_s* cpu = mod->data;
	if( (unsigned)id >= NCORES_MAX ){
		dbg_error("index to large");
	}
	sprintf(dest, modules_format_get(mod, id, "lf"), cpu_average(cpu, id));	
	return 0;
}

__ef_private int cpu_mod_free(module_s* mod){
	free(mod->data);
	return 0;
}

int cpu_mod_load(module_s* mod, char* path){
	procCpu_s* cpu = ef_mem_new(procCpu_s);
	cpu->ncores = cpu_count();
	cpu->toblink = 99.0;

	mod->data = cpu;
	mod->refresh = cpu_mod_refresh;
	mod->getenv = cpu_mod_env;
	mod->free = cpu_mod_free;

	strcpy(mod->att.longunformat, "cpu $0%");
	strcpy(mod->att.shortunformat, "$0%");
	mod->att.reftime = 1000;
	strcpy(mod->att.name, "generic");
	strcpy(mod->att.instance, "cpu");
	modules_icons_init(mod, 1);
	modules_icons_set(mod, 0, "💻");
	modules_format_init(mod, NCORES_MAX);
	for( size_t i = 0; i < NCORES_MAX; ++i){
		modules_format_set(mod, i, "6.2");
	}

	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "blink.on", CNF_LF, &cpu->toblink, 0, 0);
	config_load(&conf, path);
	config_destroy(&conf);

	cpu_mod_refresh(mod);
	cpu_mod_refresh(mod);

	return 0;
}


