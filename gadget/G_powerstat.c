#include <vbar.h>
#include <ef/sysclass.h>
#include <ef/strong.h>

typedef struct wrapPowerStat{
	char* device;
	powerstat_s ps;
}wrapPowerStat_s;

__private int powerstat_ellapse(gadget_s* g){
	wrapPowerStat_s* wps = g->data;
	if( !wps->device ) return 0;
	powerstate_get(&wps->ps, wps->device);
	return 0;
}

__private int powerstat_free(gadget_s* g){
	wrapPowerStat_s* wps = g->data;
	free(wps->device);
	free(wps);
	g->data = NULL;
	return 0;
}

// BAT0
__private void powerstat_device_set(gadget_s* g, const char* device){
	wrapPowerStat_s* wps = g->data;
	wps->device = str_dup(device, 0);
}

// /1000000 V
__private double powerstat_voltage_min_get(gadget_s* g, unsigned unit){
	if( unit == 0 ) ++unit;
	wrapPowerStat_s* wps = g->data;
	return (double)(wps->ps.voltageMin) / (double)unit;
}

// /1000000 V
__private double powerstat_voltage_now_get(gadget_s* g, unsigned unit){
	if( unit == 0 ) ++unit;
	wrapPowerStat_s* wps = g->data;
	return (double)(wps->ps.voltageNow) / (double)unit;
}

// /1000000 W/h
__private double powerstat_energy_full_get(gadget_s* g, unsigned unit){
	if( unit == 0 ) ++unit;
	wrapPowerStat_s* wps = g->data;
	return (double)(wps->ps.energyFull) / (double)unit;
}

// /1000000 W/h
__private double powerstat_energy_now_get(gadget_s* g, unsigned unit){
	if( unit == 0 ) ++unit;
	wrapPowerStat_s* wps = g->data;
	return (double)(wps->ps.energyNow) / (double)unit;
}

//%
__private size_t powerstat_capacity_get(gadget_s* g){
	wrapPowerStat_s* wps = g->data;
	return wps->ps.capacity;
}

__private size_t powerstat_timeleft_hours_get(gadget_s* g){
	wrapPowerStat_s* wps = g->data;
	return wps->ps.timeleft;
}

__private size_t powerstat_timeleft_minutes_get(gadget_s* g){
	wrapPowerStat_s* wps = g->data;
	return (size_t)(wps->ps.timeleft*60.0) % 60;
}

__private const char* powerstat_status_get(gadget_s* g){
	wrapPowerStat_s* wps = g->data;
	return wps->ps.status;
}

int gadget_powerstat_load(gadget_s* g){
	wrapPowerStat_s* wps = mem_new(wrapPowerStat_s);
	wps->ps.powersupply[0] = 0;
	wps->device = NULL;
	g->data = wps;
	g->ellapse = powerstat_ellapse;
	g->free = powerstat_free;
	
	return 0;
}

void gadget_powerstat_register(vbar_s* vb){
	dbg_info("register powerstat");
	config_add_symbol(vb, "gadget_powerstat_device_set", powerstat_device_set);
	config_add_symbol(vb, "gadget_powerstat_voltage_min_get", powerstat_voltage_min_get);
	config_add_symbol(vb, "gadget_powerstat_voltage_now_get", powerstat_voltage_now_get);
	config_add_symbol(vb, "gadget_powerstat_energy_full_get", powerstat_energy_full_get);
	config_add_symbol(vb, "gadget_powerstat_energy_now_get", powerstat_energy_now_get);
	config_add_symbol(vb, "gadget_powerstat_capacity_get", powerstat_capacity_get);
	config_add_symbol(vb, "gadget_powerstat_timeleft_hours_get", powerstat_timeleft_hours_get);
	config_add_symbol(vb, "gadget_powerstat_timeleft_minutes_get", powerstat_timeleft_minutes_get);
	config_add_symbol(vb, "gadget_powerstat_status_get", powerstat_status_get);
}


