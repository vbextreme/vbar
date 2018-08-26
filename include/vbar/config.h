#ifndef __VBAR_CONFIG_H__
#define __VBAR_CONFIG_H__

#include <vbar/type.h>
#include <vbar/memory.h>
#include <sys/sysinfo.h>

#ifndef VBAR_CONFIG
	#define VBAR_CONFIG "~/.config/vbar/config"
#endif

typedef enum { CNF_D, CNF_U, CNF_LD, CNF_LU, CNF_F, CNF_LF, CNF_S, CNF_C } config_e;

typedef struct configElement{
	struct configElement* next;
	char* name;
	void* ptr;
	size_t isvector;
	size_t maxlen;
	config_e type;
}configElement_s;

typedef struct config{
	configElement_s** elems;
	size_t count;
}config_s;

size_t kr_hash(char*s, size_t size);
size_t kr_nhash(char*s, size_t len, size_t size);
void config_init(config_s* cf, size_t maxhash);
void config_destroy(config_s* cf);
void config_add(config_s* cf, char* name, config_e type, void* ptr, size_t maxlen, size_t isvector);
void config_load(config_s* cf, char* fconf);

#endif
