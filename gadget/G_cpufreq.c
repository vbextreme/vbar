#include <vbar.h>
#include <ef/sysclass.h>

__private size_t TYPE = 0; 

__private size_t wrap_cpufreq_fq_get(unsigned mode, size_t idcpu){
	__private const char* pcfn[] = { SYSCLASS_CPUFREQ_CURFQ, SYSCLASS_CPUFREQ_MINFQ, SYSCLASS_CPUFREQ_MAXFQ};
	char fname[PATH_MAX];
	if( mode > 3 ) return -1;
	cpufreq_filename(fname, pcfn[mode], idcpu);
	return cpufreq_read_value(fname);
}	

__private char** wrap_cpufreq_available_governor_get(size_t* count){
	return cpufreq_available_governor(count);
}

__private void release_available_governor(char** list, size_t count){
	for( size_t i = 0; i < count; ++i ){
		free(list[i]);
	}
	free(list);
}

__private char* wrap_cpufreq_current_governor_get(size_t idcpu){
	char fname[PATH_MAX];
	cpufreq_filename(fname, SYSCLASS_CPUFREQ_GOV, idcpu);
	return cpufreq_current_governor(fname);
}

__private void release_current_governor(char* name){
	free(name);
}

int gadget_cpufreq_load(gadget_s* g){
	g->data = NULL;
	g->ellapse = NULL;
	g->free = NULL;
	return 0;
}

void gadget_cpufreq_register(vbar_s* vb){
	dbg_info("register cpu");
	TYPE = gadget_type_get(vb, "cpufreq");
	config_add_symbol(vb, "gadget_cpufreq_fq_get", wrap_cpufreq_fq_get);
	config_add_symbol(vb, "gadget_cpufreq_available_governor_get", wrap_cpufreq_available_governor_get);
	config_add_symbol(vb, "gadget_cpufreq_available_governor_free", release_available_governor);
	config_add_symbol(vb, "gadget_cpufreq_current_governor_get", wrap_cpufreq_current_governor_get);
	config_add_symbol(vb, "gadget_cpufreq_current_governor_free", release_current_governor);
}

