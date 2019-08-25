#ifndef __EF_SYSCLASS_H__
#define __EF_SYSCLASS_H__

#include <ef/type.h>

#define SYSCLASS_CPUFREQ_CURFQ "/scaling_cur_freq"
#define SYSCLASS_CPUFREQ_MINFQ "/scaling_min_freq"
#define SYSCLASS_CPUFREQ_MAXFQ "/scaling_max_freq"
#define SYSCLASS_CPUFREQ_GOV "/scaling_governor"

#define SYSCLASS_POWERSTATE_STATUS_MAX 32
#define SYSCLASS_POWERSTATE_STATUS_ERROS "error"

typedef struct powerstat{
	size_t voltageMin; /* uV */
	size_t voltageNow; /* uV */
	size_t energyFull; /* uW/h */
	size_t energyNow; /* uW/h */
	size_t powerNow; /* uW */
	size_t capacity; /* % */
	double timeleft;
	char powersupply[PATH_MAX];
	char status[SYSCLASS_POWERSTATE_STATUS_MAX];
}powerstat_s;


void cpufreq_filename(char* out, const char* fname, size_t idcpu);
size_t cpufreq_read_value(char* filename);
char** cpufreq_available_governor(size_t* count);
char* cpufreq_current_governor(const char* filename);

err_t powerstate_get(powerstat_s* ps, const char* device);


#endif
