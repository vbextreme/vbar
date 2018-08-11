#ifndef __VBAR_MODULES_H__
#define __VBAR_MODULES_H__

#include <vbar.h>

/*** modules.c ***/
#ifndef MODULES_MAX
	#define MODULES_MAX 32
#endif

#define ICONS_SIZE 8
#define MAX_FORMAT 24
#define MODULE_NAME_MAX 32
#define MODULE_SPAWN_MAX 1024

typedef struct module module_s;

typedef int (*modself_f)(module_s*);
typedef int (*modselfds_f)(module_s*,int,char*);

typedef struct module{
	char longformat[I3BAR_TEXT_MAX];
	char shortformat[I3BAR_TEXT_MAX];
	long blinktime;
	int blink;
	int blinkstatus;
	long reftime;
	long tick;
	char** icons;
	size_t iconcount;
	size_t icoindex;
	char onevent[MODULE_SPAWN_MAX];

	i3element_s i3;
	void* data;
	modself_f refresh;
	modself_f free;
	modselfds_f getenv;
}module_s;

typedef struct modules{
	module_s rmod[MODULES_MAX];
	size_t used;
	module_s* mod[MODULES_MAX];
	size_t count;
	i3element_s def;
}modules_s;

module_s* modules_pop(modules_s* mods);
void modules_insert(modules_s* mods, module_s* mod);
void modules_refresh_tick(modules_s* mods, long ellapsems);
void modules_load(modules_s* mod);
long modules_next_tick(modules_s* mods);
void modules_reformatting(module_s* mod);
void modules_refresh_output(modules_s* mods);
void modules_default_config(module_s* mod, config_s* conf);
void modules_icons_init(module_s* mod, size_t count);
void modules_icons_set(module_s* mod, size_t id, char* ico);
void modules_dispatch(modules_s* mods, i3event_s* ev);

#endif
