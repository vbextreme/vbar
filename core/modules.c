#include <vbar.h>

#define PHQ_PARENT(I) ((I)/2)
#define PHQ_LEFT(I) ((I)*2)
#define PHQ_RIGHT(I) ((I)*2+1)

module_s* modules_pop(modules_s* mods) {
	if( mods->mod[1]->att.tick > (long)time_ms() ){
		return NULL;
	}
	
	module_s* ret = mods->mod[1];
	mods->mod[1] = mods->mod[mods->count];
	--mods->count;
	
	size_t parent = 1;
    size_t child;
    
    for (;;) {
        child = parent * 2;
        if( child > mods->count ){
            break;
        }
        if( child < mods->count) {
            if( mods->mod[child]->att.tick > mods->mod[child+1]->att.tick ){
                ++child;
            }
        }
        
        if( mods->mod[child]->att.tick < mods->mod[parent]->att.tick ){
            module_s* tmp = mods->mod[child];
            mods->mod[child] = mods->mod[parent];
            mods->mod[parent] = tmp;
            parent = child;
        } else {
            break;
        }
    }
	return ret;
}

void modules_insert(modules_s* mods, module_s* mod){

	if( mod->att.blinkstatus ){
		mod->att.tick = mod->att.blinktime;
		mod->att.urgent = (mod->att.urgent + 1) & 1;
	}
	else{
		mod->att.tick = mod->att.reftime;
	}
	
	mod->att.tick += time_ms();

	size_t child = ++mods->count;
	iassert(mods->count < MODULES_MAX);

	size_t parent = PHQ_PARENT(child);
	while( parent && mod->att.tick < mods->mod[parent]->att.tick ){
		mods->mod[child] = mods->mod[parent];
		child = parent;
		parent = PHQ_PARENT(child);
	}
	
	mods->mod[child] = mod;
}

long modules_next_tick(modules_s* mods){
	return mods->mod[1]->att.tick;
}

__ef_private void module_reform(module_s* mod, char* dst, size_t len, char* src){
	--len;
	while( *src ){
		while( *src && *src != '$' && len-->0 ) *dst++ = *src++;
		if( 0 == src ){
			break;
		}
		++src;
		switch( *src ){
			case '$':
				--len;
				*dst++ = *src++;
			break;
			
			case '@':{
				if( mod->att.iconcount == 0 ){
					dbg_warning("no icons");
					break;
				}
				strcpy(dst, mod->att.icons[mod->att.icoindex]);
				size_t l = strlen(dst);
				dst += l;
				if( len < l ){
					dbg_fail("get env bo");
				}
				len -= l;
				++src;
			}break;	

			default: {
				char* chk;
				size_t id = strtoul(src, &chk, 10);
				if( chk == src ){
					*dst = 0;
					return;
				}
				src = chk;
				mod->getenv(mod, id, dst);
				size_t l = strlen(dst);
				dst += l;
				if( len < l ){
					dbg_fail("get env bo");
				}
				len -= l;
			}break;
		}
	}
	*dst = 0;
}

void modules_reformatting(module_s* mod){
	module_reform(mod, mod->att.longformat, ATTRIBUTE_TEXT_MAX, mod->att.longunformat);
	module_reform(mod, mod->att.shortformat, ATTRIBUTE_TEXT_MAX, mod->att.shortunformat);
}

void modules_refresh_output(modules_s* mods){
	ipc_begin_elements();
	for(size_t i = 0; i < mods->used - 1; ++i){
		ipc_write_element( &mods->rmod[i].att, TRUE);
	}
	ipc_write_element( &mods->rmod[mods->used - 1].att, FALSE);
	ipc_end_elements();
}

__ef_private void module_load(modules_s* mods, char* name, char* path){
	int cpu_mod_load(module_s* mod, char* path);
	int mem_mod_load(module_s* mod, char* path);
	int datetime_mod_load(module_s* mod, char* path);
	int static_mod_load(module_s* mod, char* path);
	int power_mod_load(module_s* mod, char* path);
	int net_mod_load(module_s* mod, char* path);

	__ef_private struct selective {
		char* name;
		int(*modload)(module_s*, char*);
		char* conf;
	} modsconf[] = {
		{"cpu",      cpu_mod_load,      "~/.config/vbar/cpu/config"},
		{"memory",   mem_mod_load,      "~/.config/vbar/memory/config"},
		{"datetime", datetime_mod_load, "~/.config/vbar/datetime/config"},
		{"static",   static_mod_load,   "~/.config/vbar/static/config"},
		{"power",    power_mod_load,    "~/.config/vbar/power/config"},
		{"network",  net_mod_load,      "~/.config/vbar/network/config"},
		{NULL, NULL, NULL}
	};


	dbg_info("load module %s", name);

	for(size_t i = 0; modsconf[i].name; ++i){
		if( 0 == strcmp(name, modsconf[i].name) ){
			iassert(mods->used < MODULES_MAX);
			module_s* mod = &mods->rmod[mods->used++];
			mod->att = mods->def;
			mod->att.onevent[0] = 0;
			if( *path ){
				modsconf[i].modload(mod, path);
			}
			else{
				modsconf[i].modload(mod, modsconf[i].conf);
			}
			if( mod->att.reftime > 0) modules_insert(mods, mod);
		}
	}
}

void modules_load(modules_s* mods){
	mods->used = 0;
	for( size_t i = 0; i < MODULES_MAX; ++i ){
		mods->mod[i] = NULL;
	}	
	
	mods->count = 0;
	mods->def.align = 0;
	mods->def.background = 0;
	mods->def.color = 0xFFFFFF;
	mods->def.border = 0x0000FF;
	strcpy(mods->def.longunformat, "error text unset");
	strcpy(mods->def.longformat, "error text unset");
	mods->def.shortunformat[0] = 0;
	mods->def.shortformat[0] = 0;
	mods->def.instance[0] = 0;
	mods->def.markup = 0;
	mods->def.min_width = -1;
	mods->def.name[0] = 0;
	mods->def.seaparator = 1;
	mods->def.separator_block_width = -1;
	mods->def.urgent = -1;
	mods->def.blink = 1;
	mods->def.blinkstatus = 0;
	mods->def.blinktime = 400;
	mods->def.format = NULL;
	mods->def.formatcount = 0;
	mods->def.icons = NULL;
	mods->def.iconcount = 0;
	mods->def.icoindex = 0;
	mods->def.onevent[0] = 0;
	mods->def.reftime = 1000;
	mods->def.tick = 0;

	char** listModules = ef_mem_matrix_new(MODULES_MAX, sizeof(char) * ATTRIBUTE_TEXT_MAX);
	char** listModulesDir = ef_mem_matrix_new(MODULES_MAX, sizeof(char) * PATH_MAX);
	for( size_t i = 0; i < MODULES_MAX; ++i){
		listModules[i][0] = 0;
		listModulesDir[i][0] = 0;
	}

	config_s conf;
	config_init(&conf, 256);
	config_add(&conf, "module.type", CNF_S, listModules, ATTRIBUTE_TEXT_MAX, MODULES_MAX);
	config_add(&conf, "module.path", CNF_S, listModulesDir, PATH_MAX, MODULES_MAX);
	config_add(&conf, "color", CNF_U, &mods->def.color, 0, 0);
	config_add(&conf, "background", CNF_U, &mods->def.background, 0, 0);
	config_add(&conf, "border", CNF_U, &mods->def.border, 0, 0);
	config_add(&conf, "min_width", CNF_U, &mods->def.min_width, 0, 0);
	config_add(&conf, "align", CNF_U, &mods->def.color, 0, 0);
	config_add(&conf, "seaparator", CNF_U, &mods->def.seaparator, 0, 0);
	config_add(&conf, "separator_block_width", CNF_U, &mods->def.separator_block_width, 0, 0);
	config_add(&conf, "markup", CNF_U, &mods->def.markup, 0, 0);
	config_load(&conf, VBAR_CONFIG);
	config_destroy(&conf);
	
	for( size_t i = 0; i < MODULES_MAX; ++i ){
		if( listModules[i][0] ){
			module_load(mods, listModules[i], listModulesDir[i]);
		}
	}
	
	ef_mem_matrix_free(listModules, MODULES_MAX);
	ef_mem_matrix_free(listModulesDir, MODULES_MAX);
	
	if( mods->used == 0 ){
		dbg_fail("need set module to run");
	}
}

void modules_default_config(module_s* mod, config_s* conf){
	config_add(conf, "name", CNF_S, mod->att.name, ATTRIBUTE_TEXT_MAX, 0);
	config_add(conf, "blink", CNF_D, &mod->att.blink, 0, 0);
	config_add(conf, "blink.time", CNF_LD, &mod->att.blinktime, 0, 0);
	config_add(conf, "text.full", CNF_S, &mod->att.longunformat, ATTRIBUTE_TEXT_MAX, 0);
	config_add(conf, "text.short", CNF_S, &mod->att.shortunformat, ATTRIBUTE_TEXT_MAX, 0);
	config_add(conf, "refresh", CNF_LD, &mod->att.reftime, 0, 0);
	config_add(conf, "color", CNF_D, &mod->att.color, 0, 0);
	config_add(conf, "background", CNF_D, &mod->att.background, 0, 0);
	config_add(conf, "border", CNF_D, &mod->att.border, 0, 0);
	config_add(conf, "min_width", CNF_D, &mod->att.min_width, 0, 0);
	config_add(conf, "align", CNF_D, &mod->att.color, 0, 0);
	config_add(conf, "seaparator", CNF_D, &mod->att.seaparator, 0, 0);
	config_add(conf, "separator_block_width", CNF_D, &mod->att.separator_block_width, 0, 0);
	config_add(conf, "markup", CNF_U, &mod->att.markup, 0, 0);
	config_add(conf, "icon", CNF_S, mod->att.icons, ATTRIBUTE_ICONS_SIZE, mod->att.iconcount);
	config_add(conf, "format", CNF_S, mod->att.format, ATTRIBUTE_FORMAT_MAX, mod->att.formatcount);
	config_add(conf, "event", CNF_S, mod->att.onevent, ATTRIBUTE_SPAWN_MAX, 0);
}

void modules_icons_init(module_s* mod, size_t count){
	mod->att.icons = ef_mem_matrix_new(count, sizeof(char) * ATTRIBUTE_ICONS_SIZE);
	mod->att.iconcount = count;
}

void modules_icons_set(module_s* mod, size_t id, char* ico){
	if( id >= mod->att.iconcount){
		dbg_warning("id icon out of range");
		return;
	}
	strcpy(mod->att.icons[id],ico);
}

void modules_icons_select(module_s* mod, size_t id){
	if( id >= mod->att.iconcount){
		dbg_warning("id icon out of range");
		return;
	}
	mod->att.icoindex = id;
}

void modules_format_init(module_s* mod, size_t count){
	mod->att.format = ef_mem_matrix_new(count, sizeof(char) * ATTRIBUTE_FORMAT_MAX);
	mod->att.formatcount = count;
}

void modules_format_set(module_s* mod, size_t id, char* format){
	if( id >= mod->att.formatcount){
		dbg_warning("id format out of range");
		return;
	}
	strcpy(mod->att.format[id],format);
}

char* modules_format_get(module_s* mod, size_t id, char* type){
	__ef_private char frm[ATTRIBUTE_FORMAT_MAX+5] = "%";
	strcpy(&frm[1], mod->att.format[id]);
	strcpy(&frm[strlen(frm)], type);
	return frm;
}

void modules_dispatch(modules_s* mods, event_s* ev){
	for( size_t i = 0; i < mods->used; ++i ){
		if( mods->rmod[i].att.onevent[0] && !strcmp(mods->rmod[i].att.instance, ev->instance) && !strcmp(mods->rmod[i].att.name, ev->name)){
			char cmd[2048];
			module_reform(&mods->rmod[i], cmd, 2048, mods->rmod[i].att.onevent);
			spawn_shell(cmd);
		}
	}
}

void module_set_urgent(module_s* mod, int enable){
	if( mod->att.blink ){
		mod->att.blinkstatus =  enable;
		if( !enable ) mod->att.urgent = 0;
	}
	else{
		mod->att.urgent = enable;
	}

}

//TODO free modules??????
