#include <ef/optex.h>
#include <vbar.h>
#include <ef/file.h>

typedef enum{ 
	a_help, 
	a_config,
	a_monitor,
	a_debug,
	a_count
}arg_e;

long topSpacingOpt = 0;

argdef_s args[] = {
	{0, 'h', "help",    ARGDEF_NOARG, NULL, "display this help"},
	{0, 'c', "config",  ARGDEF_STR,   NULL, "set config file"},
	{0, 'm', "monitor", ARGDEF_STR,   NULL, "set monitor name"},
	{0, 't', "top",     ARGDEF_SIGNED,&topSpacingOpt, "set top spacing"}, 
	{0, 'd', "debug",   ARGDEF_STR,   NULL, "set file to output debug"},
	{0, 0  , NULL   ,   ARGDEF_NOARG, NULL , NULL}
};	

//err_t vbar_deadline(__unused int type, void* vbar);
#include <ef/spawn.h>

int main(int argc, char* argv[]){
	__mem_autofree char* config = NULL;

	utf_init();
	g2d_begin();

	if( opt_parse(args, argv, argc) < 0 ){
		dbg_error("parse command");
		return 1;
	}

	if( args[a_debug].hasset ){
		vbar_change_ferr(args[a_debug].autoset);
	}

	if( args[a_help].hasset ){
		opt_usage(args, argv[0]);
		return 1;
	}

	if( args[a_config].hasset ){
		dbg_info("use config %s", (char*)args[a_config].autoset);
		config = path_resolve(args[a_config].autoset);
	}
	else{
		dbg_info("use config %s", VBAR_CONFIG);
		config = path_resolve(VBAR_CONFIG);
	}
	if( !config ){
		dbg_error("bad path");
		return 1;
	}
	dbg_info("config resolve to %s", config);

	vbar_s vbar;
	memset(&vbar, 0, sizeof(vbar_s));
	
	if( args[a_monitor].hasset ){
		vbar.monitorName = args[a_monitor].autoset;
		dbg_info("default monitor name::%s", vbar.monitorName);
	}

	vbar_begin(&vbar);
	if( vbar_script_load(&vbar, config) ){
		const char* errd = vbar_script_error();
		iassert( errd );
		bar_simple_setting(&vbar);
		bar_start(&vbar);
		bar_error(&vbar, (utf8_t*)errd);
		bar_show(&vbar);
		vbar_loop(&vbar);
		return 1;
	}
	
	vbar_register_gadget(&vbar);
	vbar_register_symbol(&vbar);

	if( vbar_script_run(&vbar) ){
		const char* errd = vbar_script_error();
		bar_simple_setting(&vbar);
		bar_start(&vbar);
		bar_error(&vbar, (utf8_t*)errd);
		bar_show(&vbar);
		vbar_loop(&vbar);
		return 1;
	}

	bar_start(&vbar);
	bar_show(&vbar);
	vbar_gadget_refresh_all(&vbar);	
	vbar_loop(&vbar);	
	vbar_end(&vbar);

	return 0;
}
