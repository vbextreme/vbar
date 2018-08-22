#include <vbar.h>

int main(__ef_unused int argc, __ef_unused char** argv)
{
	spawn_init();
	ipc_init(TRUE);

	modules_s mods;
	modules_load(&mods);
		
	for(size_t i = 0; i < mods.used; ++i){
		modules_reformatting(&mods.rmod[i]);
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
				mod->refresh(mod);
				modules_reformatting(mod);
				modules_insert(&mods, mod);
			}
			modules_refresh_output(&mods);
		}
	}

	return 0;
}
