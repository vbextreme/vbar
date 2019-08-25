#include <ef/sysclass.h>
#include <ef/file.h>
#include <ef/strong.h>
#include <ef/memory.h>


#define SYS_DEVICES_SYSTEM_CPU "/sys/devices/system/cpu/cpu"
#define CPUFREQ "/cpufreq"
#define SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors"
#define SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV_MAX 128

void cpufreq_filename(char* out, const char* fname, size_t idcpu){
	strcpy(out, SYS_DEVICES_SYSTEM_CPU);
	sprintf(&out[strlen(SYS_DEVICES_SYSTEM_CPU)], "%lu" CPUFREQ "%s", idcpu, fname);
}

size_t cpufreq_read_value(char* filename){
	__file_autoclose file_t* fd = fopen(filename, "r");
	if( fd == NULL ){
		dbg_error("fopen %s", filename);
		dbg_errno();
		return 0;
	}
	char buf[80];
	if( !fgets(buf, 80, fd) ) return 0;
	return strtoul(buf, NULL, 10);
}

__private size_t cpufreq_available_governor_count(void){
	__file_autoclose file_t* fd = fopen(SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV, "r");
	if( fd == NULL ){
		dbg_error("fopen");
		dbg_errno();
		return 0;
	}
	
	size_t count = 1;
	int ch;
	while( (ch=fgetc(fd)) != EOF ) 
		if( ch == ' ' ) ++ count;
	return count;
}

char** cpufreq_available_governor(size_t* count){
	*count = cpufreq_available_governor_count();
	if( *count == 0 ) return NULL;
	
	char** list = mem_many(char*, *count);
	iassert(list);

	__file_autoclose file_t* fd = fopen(SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV, "r");
	if( fd == NULL ){
		dbg_error("fopen");
		dbg_errno();
		free(list);
		return 0;
	}
	
	char buffer[SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV_MAX];
	size_t id = 0;
	for( id = 0; id < *count; ++id){
		int ch;
		char* d = buffer;
		while( (ch=fgetc(fd)) != EOF && ch != ' ' && ch != '\n' ) 
			*d++ = ch;
		*d = 0;
		list[id] = mem_many(char, d - buffer);
		strcpy(list[id], buffer);
	}
	return list;
}

char* cpufreq_current_governor(const char* filename){
	__file_autoclose file_t* fd = fopen(filename, "r");
	if( fd == NULL ){
		dbg_error("fopen %s", filename);
		dbg_errno();
		return 0;
	}
	
	char buffer[SYS_DEVICES_SYSTEM_CPUFREQ_GOV_AV_MAX];
	int ch;
	char* d = buffer;
	while( (ch=fgetc(fd)) != EOF && ch != ' ' && ch != '\n' ) 
		*d++ = ch;
	*d++ = 0;
	char* name = mem_many(char, d - buffer);
	iassert(name);
	strcpy(name, buffer);
	return name;
}

err_t powerstate_get(powerstat_s* ps, const char* device){
	iassert(device != NULL);
	
	struct mapname{
		char* name;
		size_t* ptr;
	}map[] = { 
		{ "VOLTAGE_MIN_DESIGN=" , &ps->voltageMin },
		{ "VOLTAGE_NOW=" , &ps->voltageNow },
		{ "ENERGY_FULL=" , &ps->energyFull },
		{ "ENERGY_NOW=" , &ps->energyNow },
		{ "POWER_NOW=" , &ps->powerNow },
		{ "CAPACITY=" , &ps->capacity },
		{ NULL, NULL }
	};

	snprintf(ps->powersupply, PATH_MAX, "/sys/class/power_supply/%s/uevent", device);

	dbg_info("file: %s", ps->powersupply);

	__file_autoclose file_t* fn = fopen(ps->powersupply, "r");
	if( NULL == fn ) {
		strcpy(ps->status, SYSCLASS_POWERSTATE_STATUS_ERROS);
		dbg_error("error on file %s", ps->powersupply);
		dbg_errno();
		return -1;
	}

	char in[1024];
	while( fgets(in, 1024, fn) ){
		char* parse = in + strlen("POWER_SUPPLY_");
		if( 0 == strncmp(parse, "STATUS=", strlen("STATUS=")) ){
			parse += strlen("STATUS=");
			str_copy_to_ch(ps->status, SYSCLASS_POWERSTATE_STATUS_MAX, parse, '\n');
			//dbg_info("set status:%s", ps->status); 
			continue;
		}
		unsigned i;
		for(i = 0; map[i].name; ++i){
			if( 0 == strncmp(parse, map[i].name, strlen(map[i].name)) ){
				//dbg_info("PARSE:%s",parse);
				parse = strchr(parse, '=');
				++parse;
				*map[i].ptr = strtol(parse, NULL, 10);
				//dbg_info("set %s: %lu", map[i].name, *map[i].ptr);
				break;
			}
		}
	}
	
	ps->timeleft = ps->powerNow == 0 ? 0 : (double)ps->energyNow / (double)ps->powerNow;
	//dbg_info("timeleft: %lf", ps->timeleft);
	return 0;
}
