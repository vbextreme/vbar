#include <vbar.h>
#include <time.h>

__private size_t TYPE = 0;

typedef enum{ DT_DY, DT_DM, DT_DD, DT_TH, DT_TM, DT_TS, DT_COUNT } datetime_e;

__private int clock_ellapse(gadget_s* g){
	time_t t = time(NULL);
	struct tm* tm = g->data;
	*tm	= *localtime(&t);
	return 0;
}

__private int clock_free(gadget_s* g){
	free(g->data);
	return 0;
}

__private int clock_years(gadget_s* g){
	if( g->type != TYPE ) return -1;
	return ((struct tm*)g->data)->tm_year + 1900;
}

__private int clock_month(gadget_s* g){
	if( g->type != TYPE ) return -1;
	return ((struct tm*)g->data)->tm_mon + 1;
}

__private int clock_day(gadget_s* g){
	if( g->type != TYPE ) return -1;
	return ((struct tm*)g->data)->tm_mday;
}

__private int clock_hour(gadget_s* g){
	if( g->type != TYPE ) return -1;
	return ((struct tm*)g->data)->tm_hour;
}

__private int clock_minutes(gadget_s* g){
	if( g->type != TYPE ) return -1;
	return ((struct tm*)g->data)->tm_min;
}

__private int clock_seconds(gadget_s* g){
	if( g->type != TYPE ) return -1;
	return ((struct tm*)g->data)->tm_sec;
}

int gadget_clock_load(gadget_s* g){
	struct tm* tm = mem_new(struct tm);
	g->data = tm;
	g->ellapse = clock_ellapse;
	g->free = clock_free;
	return 0;
}

void gadget_clock_register(vbar_s* vb){
	dbg_info("register clock");
	TYPE = gadget_type_get(vb, "clock");
	config_add_symbol(vb, "gadget_clock_years", clock_years);
	config_add_symbol(vb, "gadget_clock_month", clock_month);
	config_add_symbol(vb, "gadget_clock_day", clock_day);
	config_add_symbol(vb, "gadget_clock_hour", clock_hour);
	config_add_symbol(vb, "gadget_clock_minutes", clock_minutes);
	config_add_symbol(vb, "gadget_clock_seconds", clock_seconds);
}


