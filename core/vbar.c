#include <vbar.h>
#include <vbar/os.h>
#include "optex.h"

__ef_private argdef_s args[] = {
	{ 0, 'c', "config", ARGDEF_STR, VBAR_CONFIG, "path to config file, default ~/.config/vbar/config"},
#ifdef EF_DEBUG_ENABLE
	{ 0, 'd', "debug", ARGDEF_STR, NULL, "path to output debug"},
#endif
	{ 0, 0  , NULL    , 0         , NULL, NULL }
};

int main(__ef_unused int argc, __ef_unused char** argv)
{
	int ret = opt_parse(args, argv, argc);
	if( ret == -1 ){
		opt_usage(args, argv[0]);
		return -1;
	}

#ifdef EF_DEBUG_ENABLE 
	__ef_file_autoclose file_t * ferr;
	if( args[1].hasset ){
		ferr = fopen(args[1].autoset, "w+");
		if( ferr ){
			fclose(stderr);
			stderr = ferr;
		}
	}
#endif

	spawn_init();
	ipc_init(TRUE);

	ef_os_segfault_report(NULL);

	modules_s mods;
	modules_load(&mods, args[0].autoset);
	
	for(module_s* it = mods.rmod; it; it = it->next){
		modules_reformatting(it);
		if( it->att.scrolltime > 0 ){
			strcpy(it->att.scrollformat, it->att.longformat);
		}
	}
	modules_refresh_output(&mods);

	while(1){
		event_s ev;
		int ret = ipc_wait(&ev, modules_next_tick(&mods));
		if( ret == -1 ){
			dbg_error("ipc_wait");
			continue;
		}

		if( ret & IPC_EVENT ){
			dbg_info("event %s %s", ev.instance, ev.name); 
			modules_dispatch(&mods, &ev);
		}

		if( ret & IPC_TIMEOUT ){
			module_s* mod;
			while( (mod = modules_pop(&mods)) ){
				if( !mod->att.hide ){
					if( mod->att.scrolltime < 0 ){
						mod->refresh(mod);
						modules_reformatting(mod);	
					}
					else{
						ipc_set_scroll(&mod->att);
						if( mod->att.doubletick <= 0 ){
							mod->att.doubletick = mod->att.reftime;
							mod->refresh(mod);
							modules_reformatting(mod);
						}
					}
				}
				module_time_set(mod);
				modules_insert(&mods, mod);
			}
			modules_refresh_output(&mods);
		}
	}

	return 0;
}
