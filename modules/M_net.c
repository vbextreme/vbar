#include <vbar.h>
//#include <math.h>

#ifndef NET_DEVICES_NAME_MAX
	#define NET_DEVICES_NAME_MAX 128
#endif

#ifndef PROC_NET_DEV
	#define PROC_NET_DEV "/proc/net/dev"
#endif

typedef enum { ND_BYTES, ND_PACKETS, ND_ERRS, ND_DROP, ND_FIFO, ND_FRAME, ND_COMPRESSED, ND_MULTICAST, ND_COUNT } netdev_e;

typedef struct netdev {
	size_t receive[ND_COUNT];
	size_t transmit[ND_COUNT];
}netdev_s;

typedef struct nets{
	netdev_s devs[2];
	double rx;
	double tx;
	size_t scaler;
	size_t scalet;
	char selected[NET_DEVICES_NAME_MAX];
	size_t ref[2];
	size_t current;
	size_t unit;
}nets_s;

__ef_private void net_device(nets_s* net){
	net->ref[net->current] = 1;

	FILE* fn = fopen(PROC_NET_DEV, "r");
	if( NULL == fn ) {
		dbg_warning("no %s", PROC_NET_DEV);
		dbg_errno();
		return;
	}

	char in[1024];
	if( NULL == fgets(in, 1024, fn) ){
		dbg_warning("no skip1");
		dbg_errno();
		fclose(fn);
		return;
	}
	if( NULL == fgets(in, 1024, fn) ){
		dbg_warning("no skip2");
		dbg_errno();
		fclose(fn);
		return;
	}
	
	while( fgets(in, 1024, fn) ){
		char* parse = str_skip_h(in);
		char nname[NET_DEVICES_NAME_MAX];
		parse = str_copy_to_ch(nname, NET_DEVICES_NAME_MAX, parse, ':');
		if( parse == NULL || strcmp(nname, net->selected) ){
			continue;
		}
		++parse;
		parse = str_skip_h(parse);
		size_t i;
		for( i = 0; i < ND_COUNT; ++i){
			char* toked;
			net->devs[net->current].receive[i] = strtol(parse, &toked, 10);
			parse = str_skip_h(toked);
		}
		for( i = 0; i < ND_COUNT; ++i){
			char* toked;
			net->devs[net->current].transmit[i] = strtol(parse, &toked, 10);
			parse = str_skip_h(toked);
		}
		net->ref[net->current] = 1;
	}

	fclose(fn);
}

__ef_private size_t net_transmit(nets_s* net){
	if( !net->ref[0] || !net->ref[1] ){
		dbg_warning("no refresh device %s", net->selected);
		return 0;
	}

	size_t cur = net->current;
	size_t old = (cur - 1) & 1;
	return net->devs[cur].transmit[ND_BYTES] - net->devs[old].transmit[ND_BYTES];
}

__ef_private size_t net_receive(nets_s* net){
	if( !net->ref[0] || !net->ref[1] ){
		dbg_warning("no refresh device %s", net->selected);
		return 0;
	}

	size_t cur = net->current;
	size_t old = (cur - 1) & 1;
	return net->devs[cur].receive[ND_BYTES] - net->devs[old].receive[ND_BYTES];
}

__ef_private size_t net_scale(double* po, double val, double unit){
	size_t s = 0;
	*po = 1;
	while( val > unit ){
		val /= unit;
		++s;
		*po *= unit;
	}
	return s;
}

__ef_private int net_mod_refresh(module_s* mod){
	nets_s* net = mod->data;
	net->current = (net->current + 1) & 1;
	net_device(net);
	double time = (double)mod->att.reftime / 1000.0;
	net->rx = (double)net_receive(net) / time;
	net->tx = (double)net_transmit(net) / time;
	double po;
	net->scaler = net_scale(&po, net->rx, (double)net->unit);
	net->rx /= po;
	net->scalet = net_scale(&po, net->tx, (double)net->unit);
	net->tx /= po;
	return 0;
}

__ef_private int net_mod_env(module_s* mod, int id, char* dest){
	__ef_private char* dspunit[] = {
		"b/s ",
		"kb/s",
		"mb/s",
		"gb/s"
	};

	nets_s* net = mod->data;
	*dest = 0;
	switch( id ){
		case 0:
			sprintf(dest, modules_format_get(mod, id, "lf"), net->rx);
		break;
		
		case 1:
			if( net->scaler > 3 ){
				//dbg_error("rx out of scale");
				return -1;
			}
			sprintf(dest, modules_format_get(mod, id, "s"), dspunit[net->scaler]);	
		break;

		case 2:
			sprintf(dest, modules_format_get(mod, id, "lf"), net->tx);
		break;
		
		case 3:
			if( net->scalet > 3 ){
				//dbg_error("tx out of scale");
				return -1;
			}
			sprintf(dest, modules_format_get(mod, id, "s"), dspunit[net->scalet]);	
		break;

	}
	return 0;
}

__ef_private int net_mod_free(module_s* mod){
	free(mod->data);
	return 0;
}

int net_mod_load(module_s* mod, char* path){
	nets_s* net = ef_mem_new(nets_s);
	net->ref[0] = 0;
	net->ref[1] = 0;
	net->unit = 1000;
	net->current = 0;
	net->selected[0] = 0;
	net->scaler = 1;
	net->scalet = 1;

	mod->data = net;
	mod->refresh = net_mod_refresh;
	mod->getenv = net_mod_env;
	mod->free = net_mod_free;

	strcpy(mod->att.longunformat, "net ⇩$1 $0 ⇧$2 $0");
	strcpy(mod->att.shortunformat, "$1$0 $1$0");
	strcpy(mod->att.name, "generic");
	strcpy(mod->att.instance, "network");
	modules_icons_init(mod, 1);
	modules_icons_set(mod, 0, "🖧");

	modules_format_init(mod, 4);
	modules_format_set(mod, 0, "7.2");
	modules_format_set(mod, 1, "");
	modules_format_set(mod, 2, "7.2");
	modules_format_set(mod, 3, "");
	
	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "device", CNF_S, net->selected, NET_DEVICES_NAME_MAX, 0);
	config_add(&conf, "unit", CNF_LU, &net->unit, 0, 0);
	config_load(&conf, path);
	config_destroy(&conf);
	
	net_mod_refresh(mod);
	net_mod_refresh(mod);

	return 0;
}


