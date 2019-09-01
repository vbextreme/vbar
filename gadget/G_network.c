#include <vbar.h>
#include <ef/proc.h>

__private size_t TYPE = 0;

typedef struct gnetwork{
	char* device;
	char essid[SOCKET_WIFI_ESSID_SIZE];
	int dbm;
	int bitrate;
	int statusok;
	unsigned long speed;
	netDev_s nd[2];
	unsigned increment;
}gnetwork_s;

__private int network_free(gadget_s* g){
	free(g->data);
	return 0;
}

__private int network_ellapse(gadget_s* g){
	gnetwork_s* n = g->data;
	if( socket_wifi_info(n->device, n->essid, &n->dbm, &n->bitrate) ){
		n->statusok = 0;
	}
	else{
		n->statusok = 1;
	}
	if( net_device(&n->nd[n->increment&1], n->device) ){
		dbg_error("on read net devices");
	}
	++n->increment;
	return 0;
}

__private void network_device_set(gadget_s* g, char* device){
	if( g->type != TYPE ) return;
	gnetwork_s* n = g->data;
	n->device = device;
}

__private const char* network_essid_get(gadget_s* g){
	if( g->type != TYPE ) return "error gadget";
	gnetwork_s* n = g->data;
	return n->essid;
}

__private int network_dbm_get(gadget_s* g){
	if( g->type != TYPE ) return -1;
	gnetwork_s* n = g->data;
	return n->dbm;
}

__private int network_bitrate_get(gadget_s* g){
	if( g->type != TYPE ) return -1;
	gnetwork_s* n = g->data;
	return n->bitrate;
}
	
__private int network_ok(gadget_s* g){
	if( g->type != TYPE ) return -1;
	gnetwork_s* n = g->data;
	return n->statusok;
}

__private unsigned long network_receive_speed(gadget_s* g){
	if( g->type != TYPE ) return 0;
	gnetwork_s* n = g->data;
	if( n->increment < 2 ) return 0;
	size_t old = n->increment & 1;
	size_t now = (n->increment-1)&1;
	old = n->nd[old].receive[ND_BYTES];
	now = n->nd[now].receive[ND_BYTES];
	return now-old;
}

__private unsigned long network_transmit_speed(gadget_s* g){
	if( g->type != TYPE ) return 0;
	gnetwork_s* n = g->data;
	if( n->increment < 2 ) return 0;
	size_t old = n->increment & 1;
	size_t now = (n->increment-1)&1;
	old = n->nd[old].transmit[ND_BYTES];
	now = n->nd[now].transmit[ND_BYTES];
	return now-old;
}

int gadget_network_load(gadget_s* g){
	gnetwork_s* gn = mem_new(gnetwork_s);
	gn->essid[0] = 0;
	gn->increment = 0;
	gn->speed = 0;
	g->data = gn;
	g->ellapse = network_ellapse;
	g->free = network_free;
	return 0;
}

void gadget_network_register(vbar_s* vb){
	dbg_info("register network");
	TYPE = gadget_type_get(vb, "network");
	config_add_symbol(vb, "gadget_network_device_set", network_device_set);
	config_add_symbol(vb, "gadget_network_essid_get", network_essid_get);
	config_add_symbol(vb, "gadget_network_dbm_get", network_dbm_get);
	config_add_symbol(vb, "gadget_network_bitrate_get", network_bitrate_get);
	config_add_symbol(vb, "gadget_network_receive_speed", network_receive_speed);
	config_add_symbol(vb, "gadget_network_transmit_speed", network_transmit_speed);
	config_add_symbol(vb, "gadget_network_ok", network_ok);
}
