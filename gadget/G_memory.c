#include <vbar.h>
#include <ef/proc.h>

__private int gmem_ellapse(gadget_s* g){
	memInfo_s* mem = g->data;
	meminfo_read(mem);
	return 0;
}

__private int gmem_free(gadget_s* g){
	free(g->data);
	return 0;
}

__private size_t gmem_total_get(gadget_s* g){
	return ((memInfo_s*)g->data)->total;
}

__private size_t gmem_free_get(gadget_s* g){
	return ((memInfo_s*)g->data)->free;
}

__private size_t gmem_shared_get(gadget_s* g){
	return ((memInfo_s*)g->data)->shared;
}

__private size_t gmem_buffers_get(gadget_s* g){
	return ((memInfo_s*)g->data)->buffers + ((memInfo_s*)g->data)->cached + ((memInfo_s*)g->data)->SReclaimable;
}

__private size_t gmem_available_get(gadget_s* g){
	return ((memInfo_s*)g->data)->available;
}

__private size_t gmem_totalswap_get(gadget_s* g){
	return ((memInfo_s*)g->data)->totalswap;
}

__private size_t gmem_freeswap_get(gadget_s* g){
	return ((memInfo_s*)g->data)->freeswap;
}

__private size_t gmem_used_get(gadget_s* g){
	return ((memInfo_s*)g->data)->used;
}

int gadget_memory_load(gadget_s* g){
	dbg_info("load memory");
	g->data = mem_new(memInfo_s);
	g->ellapse = gmem_ellapse;
	g->free = gmem_free;
	gmem_ellapse(g);
	
	return 0;
}

void gadget_memory_register(vbar_s* vb){
	dbg_info("register memory");
	config_add_symbol(vb, "gadget_memory_total_get", gmem_total_get);
	config_add_symbol(vb, "gadget_memory_free_get", gmem_free_get);
	config_add_symbol(vb, "gadget_memory_shared_get", gmem_shared_get);
	config_add_symbol(vb, "gadget_memory_buffers_get", gmem_buffers_get);
	config_add_symbol(vb, "gadget_memory_available_get", gmem_available_get);
	config_add_symbol(vb, "gadget_memory_totalswap_get", gmem_totalswap_get);
	config_add_symbol(vb, "gadget_memory_freeswap_get", gmem_freeswap_get);
	config_add_symbol(vb, "gadget_memory_used_get", gmem_used_get);
}

