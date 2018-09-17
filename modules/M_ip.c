#include <vbar.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#define IP_MAX 32
#define IP_DEVICE_NAME_MAX 128

typedef struct ip{
	char selected[IFNAMSIZ];
}ip_s;

__ef_private void ipv4_name(char* out, char* dev){
	__ef_fd_autoclose int fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, dev);
	ioctl(fd, SIOCGIFADDR, &ifr);
	strcpy(out, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

__ef_private void ipv6_name(char* out, char* dev){
	__ef_fd_autoclose int fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET6;
	strcpy(ifr.ifr_name, dev);
	ioctl(fd, SIOCGIFADDR, &ifr);
	strcpy(out, inet_ntop(AF_INET6,&((struct sockaddr_in6 *)&ifr.ifr_addr)->sin6_addr, out, 32));
}

__ef_private int ip_mod_refresh(__ef_unused module_s* mod){
	return 0;
}

__ef_private int ip_mod_env(module_s* mod, int id, char* dest){
	ip_s* ip = mod->data;
	*dest = 0;
	switch( id ){
		case 0:
			ipv4_name(dest, ip->selected);
		break;

		case 1:
			ipv6_name(dest, ip->selected);
		break;

		case 2:
			/*TODO remote ip*/
		break;

		default:
			dbg_error("index to large");
		return -1;
	}
	return 0;
}

__ef_private int ip_mod_free(module_s* mod){
	free(mod->data);
	return 0;
}

int ip_mod_load(module_s* mod, char* path){
	ip_s* ip = ef_mem_new(ip_s);
	ip->selected[0] = 0;

	mod->data = ip;
	mod->refresh = ip_mod_refresh;
	mod->getenv = ip_mod_env;
	mod->free = ip_mod_free;

	strcpy(mod->att.longunformat, "ip $0");
	strcpy(mod->att.shortunformat, "$0");
	strcpy(mod->att.name, "generic");
	strcpy(mod->att.instance, "ip");
	modules_icons_init(mod, 1);
	modules_icons_set(mod, 0, "🖧");

	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "device", CNF_S, ip->selected, IFNAMSIZ, 0, NULL);
	config_load(&conf, path);
	config_destroy(&conf);

	return 0;
}


