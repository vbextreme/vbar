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

typedef enum { APS_LOAD, APS_OUT, APS_REF } appstatus_e;
__ef_private appstatus_e app_status;

__ef_private void main_crash(__ef_unused void* arg){
	dbg_error("application stop working");
	switch( app_status ){
		case APS_LOAD:
			dbg_error("fatal error on load modules");
		break;
		case APS_OUT:
			dbg_error("fatal error on set enviroment");
		break;
		case APS_REF:
			dbg_error("fatal error on refresh module");
		break;
	}
}

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

	ef_os_segfault_report(main_crash);

	app_status = APS_LOAD;
	modules_s mods;
	modules_load(&mods, args[0].autoset);
	
	app_status = APS_OUT;
	for(module_s* it = mods.rmod; it; it = it->next){
		modules_reformatting(it);
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
					app_status = APS_REF;
				   	mod->refresh(mod);
					app_status = APS_OUT;
					modules_reformatting(mod);
				}
				modules_insert(&mods, mod);
			}
			modules_refresh_output(&mods);
		}
	}

	return 0;
}
