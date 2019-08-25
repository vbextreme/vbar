#include <vbar.h>
#include <ef/proc.h>

typedef struct procCpu{
	size_t current;
	size_t ncores;
	size_t* tick[2];
}procCpu_s;

__private int cpu_ellapse(gadget_s* g){
	procCpu_s* cpu = g->data;
	cpu->current = (cpu->current + 1) & 1;
	cpu_tick_get(cpu->tick[cpu->current], cpu->ncores);
	return 0;
}

__private double wrap_cpu_load_average(gadget_s* g, unsigned core){
	procCpu_s* cpu = g->data;
	if( core > cpu->ncores+1 ){
		dbg_warning("no core %u", core);
		return 0.0;
	}
	size_t cur = cpu->current;
	size_t old = (cur - 1) & 1;
	return cpu_load_average(cpu->tick[old], cpu->tick[cur], core, cpu->ncores);
}

__private int cpu_free(gadget_s* g){
	procCpu_s* cpu = g->data;
	free(cpu->tick[0]);
	free(cpu->tick[1]);
	free(cpu);
	g->data = NULL;
	return 0;
}

int gadget_cpu_load(gadget_s* g){
	procCpu_s* cpu = mem_new(procCpu_s);
	cpu->current = 0;
	cpu->ncores = cpu_core_count();
	dbg_info("load cpu %lu", cpu->ncores);
	cpu->tick[0] = mem_many(size_t, CPU_TIME_COUNT * (cpu->ncores+1));
	cpu->tick[1] = mem_many(size_t, CPU_TIME_COUNT * (cpu->ncores+1));

	g->data = cpu;
	g->ellapse = cpu_ellapse;
	g->free = cpu_free;

	cpu_ellapse(g);
	cpu_ellapse(g);
	
	return 0;
}

void gadget_cpu_register(vbar_s* vb){
	dbg_info("register cpu");
	config_add_symbol(vb, "gadget_cpu_load_average", wrap_cpu_load_average);
	config_add_symbol(vb, "gadget_cpu_count", cpu_core_count);
}

