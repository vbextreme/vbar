#include <vbar.h>

//TODO 
//riordinare
//event shell software

int main(__ef_unused int argc, __ef_unused char** argv)
{
	modules_s mods;
	modules_load(&mods);
	
	i3bar_init(TRUE);
	while(1){
		i3event_s ev;
		int ret = i3bar_wait(&ev, modules_next_tick(&mods));
		if( ret & I3BAR_EVENT ){
			//TODO
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
