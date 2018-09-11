#include <vbar.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/wireless.h>

#define RETRY 2

#ifndef NET_DEVICES_NAME_MAX
	#define NET_DEVICES_NAME_MAX 128
#endif

typedef struct wireless{
	char selected[NET_DEVICES_NAME_MAX];
	int socket;
	struct iwreq rqsk;
	char essid[IW_ESSID_MAX_SIZE];
}wireless_s;

__ef_private int wireless_ssid_refresh(wireless_s* wi){
	if( wi->socket == -1 ){
		dbg_info("init socket");	
		if( (wi->socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
			dbg_error("socket");
			dbg_errno();
			return -1;
		}
		memset(&wi->rqsk, 0, sizeof(struct iwreq));
		strcpy(wi->rqsk.ifr_name, wi->selected);
		wi->rqsk.u.essid.length = IW_ESSID_MAX_SIZE;
		wi->rqsk.u.essid.pointer = wi->essid;
	}
	
	wi->essid[0] = 0;
    if( ioctl(wi->socket, SIOCGIWESSID, &wi->rqsk) == -1 ){
		int er = errno;
        dbg_warning("ioctl");
		dbg_errno();
		close(wi->socket);
		wi->socket = -1;
		return er == 19 ? -2 : -1;
    }
	wi->essid[wi->rqsk.u.essid.length] = 0;
	return 0;
}

__ef_private int wireless_mod_refresh(module_s* mod){
	for( size_t i = 0; i < RETRY && wireless_ssid_refresh(mod->data); ++i );
	return 0;
}

__ef_private int wireless_mod_env(module_s* mod, int id, char* dest){
	wireless_s* wi = mod->data;
	*dest = 0;
	switch( id ){
		case 0:
			sprintf(dest, modules_format_get(mod, id, "s"), wi->essid);
		break;

		default:
			dbg_error("index to large");
		return -1;
	}
	return 0;
}

__ef_private int wireless_mod_free(module_s* mod){
	wireless_s* wi = mod->data;
	close(wi->socket);
	wi->socket = -1;
	free(wi);
	return 0;
}

int wireless_mod_load(module_s* mod, char* path){
	wireless_s* wi = ef_mem_new(wireless_s);
	wi->selected[0] = 0;
	wi->socket = -1;
	wi->essid[0] = 0;

	mod->data = wi;
	mod->refresh = wireless_mod_refresh;
	mod->getenv = wireless_mod_env;
	mod->free = wireless_mod_free;

	strcpy(mod->att.longunformat, "essid $0");
	strcpy(mod->att.shortunformat, "$0");
	strcpy(mod->att.name, "generic");
	strcpy(mod->att.instance, "wireless");
	modules_icons_init(mod, 1);
	modules_icons_set(mod, 0, "🖧");

	modules_format_init(mod, 1);
	modules_format_set(mod, 0, "");

	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "device", CNF_S, wi->selected, NET_DEVICES_NAME_MAX, 0, NULL);
	config_load(&conf, path);
	config_destroy(&conf);
	
	if( wireless_ssid_refresh(wi) == -2 ){
		free(wi);
		return -1;
	}


	return 0;
}


