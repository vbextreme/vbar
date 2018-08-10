#include <vbar.h>

int main(__ef_unused int argc, __ef_unused char** argv)
{
	spawn_init();
	modules_s mods;
	modules_load(&mods);
		
	i3bar_init(TRUE);
	while(1){
		i3event_s ev;
		int ret = i3bar_wait(&ev, modules_next_tick(&mods));
		if( ret & I3BAR_EVENT ){
			dbg_info("event %s %s", ev.instance, ev.name); 
			modules_dispatch(&mods, &ev);
		}

		if( ret & I3BAR_TIMEOUT ){
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
