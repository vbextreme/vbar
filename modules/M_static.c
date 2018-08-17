#include <vbar.h>

__ef_private int static_mod_refresh(__ef_unused module_s* mod){
	return 0;
}

__ef_private int static_mod_env(__ef_unused module_s* mod, __ef_unused int id, char* dest){
	*dest = 0;
	return 0;
}

__ef_private int static_mod_free(__ef_unused module_s* mod){
	return 0;
}

int static_mod_load(module_s* mod, char* path){
	mod->data = NULL;

	mod->refresh = static_mod_refresh;
	mod->getenv = static_mod_env;
	mod->free = static_mod_free;

	strcpy(mod->att.longunformat, "long format");
	strcpy(mod->att.shortunformat, "short");
	mod->att.reftime = -1;
	strcpy(mod->att.name, "generic");
	strcpy(mod->att.instance, "static");	
	modules_icons_init(mod, 1);
	modules_icons_set(mod, 0, "⊶");
	
	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_load(&conf, path);
	config_destroy(&conf);
	
	return 0;
}



