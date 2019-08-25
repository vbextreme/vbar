#include <ef/proc.h>
#include <ef/file.h>
#include <ef/strong.h>
#include <ef/memory.h>

#define PROC_STAT "/proc/stat"

#define SYS_DEVICES_SYSTEM_CPU "/sys/devices/system/cpu/cpu"
#define CPUFREQ "/cpufreq"
#define SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors"
#define SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV_MAX 128

#define PROC_MEM "/proc/meminfo"
#define MEMINFO_ELEMS 8

#define PROC_NET_DEV "/proc/net/dev"
#define NET_DEV_NAME_MAX 256

int cpu_core_count(void){
	return sysconf(_SC_NPROCESSORS_ONLN);
}

err_t cpu_tick_get(size_t* tick, int ncores){
	if( ncores < 0 ) ncores = cpu_core_count();
	++ncores;

	char in[1024];
	__file_autoclose file_t* fps = fopen(PROC_STAT, "r");
	if( fps == NULL ) {
		dbg_error("%s not available", PROC_STAT);
		dbg_errno();
		return -1;
	}

	for( size_t i = 0; (int)i < ncores; ++i ){
		if( NULL == fgets(in, 1024, fps) ){
			dbg_error("fgets");
			fclose(fps);
			return -1;
		}
		
		char* parse;
		if( (parse = strpbrk(in, " \t")) == NULL){
			dbg_error("on skip cpu");
			fclose(fps);
			return -1;
		}
		
		size_t it = i * CPU_TIME_COUNT;
		for(size_t k = 0; k < CPU_TIME_COUNT; ++k){
			char* en = NULL;
			parse = str_skip_h(parse);
			tick[it++] = strtol(parse, &en, 10);
			parse = en;
		}
	}
	return 0;
}

size_t cpu_time_tick(size_t* tick){
	size_t full = 0;
	for(size_t i = 0; i < CPU_TIME_COUNT; ++i ){
		full += tick[i];
	}
	return full;
}

double cpu_load_average(size_t* tickS, size_t* tickE, unsigned core, int ncores){
	if( ncores < 0 ) ncores = cpu_core_count();
	++ncores;

	if( core > (unsigned)ncores ){
		dbg_warning("no core %u", core);
		return 0.0;
	}

	core *= CPU_TIME_COUNT; 
	size_t tick = cpu_time_tick(&tickE[core]) - cpu_time_tick(&tickS[core]);
	size_t idle = tickE[core+CPU_IDLE] - tickS[core+CPU_IDLE];
	double average = ((double)(tick - idle) / (double)tick) * 100.00; 
	dbg_info("average %u %lf", core, average);
	return average;
}

/* sysinfo return incorrect value and not return cached, used /proc/meminfo */

__private size_t meminfo_parse(char* line){
	while( *line && (*line < '0' || *line > '9') ) ++line;
	iassert( *line );
	return strtoul(line, NULL, 10);
}

void meminfo_read(memInfo_s* mem){
	__private char* col[] = {
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

	__file_autoclose file_t * fm = fopen(PROC_MEM, "r");
	if( fm == NULL ) {
		dbg_error("%s not available", PROC_MEM);
		dbg_errno();
		return;
	}
	
	char inp[1024];
	while( fgets(inp, 1024, fm) ){
		for( size_t i = 0; col[i]; ++i ){
			if( !strncmp(inp, col[i], strlen(col[i])) ){
				*memptr[i] = meminfo_parse(inp);
			}
		}
	}

	mem->used = mem->total - (mem->free + mem->buffers + mem->cached + mem->SReclaimable);
}

err_t net_device(netDev_s* net, char* device){
	__file_autoclose file_t* fn = fopen(PROC_NET_DEV, "r");
	if( NULL == fn ) {
		dbg_error("on open %s", PROC_NET_DEV);
		dbg_errno();
		return -1;
	}

	char in[1024];
	if( NULL == fgets(in, 1024, fn) ){
		dbg_warning("no skip1");
		dbg_errno();
		return -1;
	}
	if( NULL == fgets(in, 1024, fn) ){
		dbg_warning("no skip2");
		dbg_errno();
		return -1;
	}
	
	while( fgets(in, 1024, fn) ){
		char* parse = str_skip_h(in);
		char nname[NET_DEV_NAME_MAX];
		parse = str_copy_to_ch(nname, NET_DEV_NAME_MAX, parse, ':');
		if( parse == NULL || strcmp(nname, device) ){
			continue;
		}
		++parse;
		parse = str_skip_h(parse);
		size_t i;
		for( i = 0; i < ND_COUNT; ++i){
			char* toked;
			net->receive[i] = strtol(parse, &toked, 10);
			parse = str_skip_h(toked);
		}
		for( i = 0; i < ND_COUNT; ++i){
			char* toked;
			net->transmit[i] = strtol(parse, &toked, 10);
			parse = str_skip_h(toked);
		}
		break;
	}
	return 0;
}

