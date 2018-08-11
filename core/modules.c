#include <vbar.h>

#define PHQ_PARENT(I) ((I)/2)
#define PHQ_LEFT(I) ((I)*2)
#define PHQ_RIGHT(I) ((I)*2+1)

module_s* modules_pop(modules_s* mods){
	if( mods->mod[1]->tick > (long)time_ms() ){
		return NULL;
	}

	module_s* ret = mods->mod[1];
	mods->mod[1] = mods->mod[mods->count];
	mods->mod[mods->count--] = NULL;
	size_t bubble = 1;
	
	while( bubble < mods->count ){
		size_t left = PHQ_LEFT(bubble);
		size_t right = PHQ_RIGHT(bubble);
		
		if( mods->mod[left] && mods->mod[bubble]->tick > mods->mod[left]->tick ){
			module_s* tmp = mods->mod[left];
			mods->mod[left] = mods->mod[bubble];
			mods->mod[bubble] = tmp;
			bubble = left;
		}
		else if( mods->mod[right] && mods->mod[bubble]->tick > mods->mod[right]->tick ){
			module_s* tmp = mods->mod[right];
			mods->mod[right] = mods->mod[bubble];
			mods->mod[bubble] = tmp;
			bubble = right;
		}
		else{
			break;
		}
	}
	return ret;
}

void modules_insert(modules_s* mods, module_s* mod){
	++mods->count;
	iassert(mods->count < MODULES_MAX);
	
	if( mod->blinkstatus ){
		mod->tick = mod->blinktime;
		mod->i3.urgent = (mod->i3.urgent + 1) & 1;
	}
	else{
		mod->tick = mod->reftime;
	}
	
	mod->tick += time_ms();
	mods->mod[mods->count] = mod;

	size_t child = mods->count;
	size_t parent = PHQ_PARENT(child);
	while( parent && mods->mod[child]->tick < mods->mod[parent]->tick ){
		mods->mod[child] = mods->mod[parent];
		child = parent;
		parent = PHQ_PARENT(child);
	}
	mods->mod[child] = mod;
}

long modules_next_tick(modules_s* mods){
	return mods->mod[1]->tick;
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
				if( mod->iconcount == 0 ){
					dbg_warning("no icons");
					break;
				}
				strcpy(dst, mod->icons[mod->icoindex]);
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
	module_reform(mod, mod->i3.full_text, I3BAR_TEXT_MAX,mod->longformat);
	module_reform(mod, mod->i3.short_text, I3BAR_TEXT_MAX,mod->shortformat);
}

void modules_refresh_output(modules_s* mods){
	i3bar_begin_elements();
	for(size_t i = 0; i < mods->used - 1; ++i){
		i3bar_write_element( &mods->rmod[i].i3, TRUE);
	}
	i3bar_write_element( &mods->rmod[mods->used - 1].i3, FALSE);
	i3bar_end_elements();
	fflush(stdout);
}

__ef_private void module_load(modules_s* mods, char* name, char* path){
	static char* modsname[] = {
		"cpu",
		"memory",
		"datetime",
		NULL
	};

	typedef int(*modload_f)(module_s*,char*);
	static modload_f modsload[] = {
		cpu_mod_load,
		mem_mod_load,
		datetime_mod_load
	};
	
	static char* modsconf[] = {
		"~/.config/vbar/cpu/config",
		"~/.config/vbar/memory/config",
		"~/.config/vbar/datetime/config"
	};
	
	dbg_info("load module %s", name);

	for(size_t i = 0; modsname[i]; ++i){
		if( 0 == strcmp(name, modsname[i]) ){
			iassert(mods->used < MODULES_MAX);
			module_s* mod = &mods->rmod[mods->used++];
			mod->i3 = mods->def;
			mod->onevent[0] = 0;
			if( *path ){
				modsload[i](mod, path);
			}
			else{
				modsload[i](mod, modsconf[i]);
			}
			modules_insert(mods, mod);
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
	strcpy(mods->def.full_text, "err");
	mods->def.short_text[0] = 0;
	mods->def.instance[0] = 0;
	mods->def.markup = 0;
	mods->def.min_width = -1;
	mods->def.name[0] = 0;
	mods->def.seaparator = 1;
	mods->def.separator_block_width = -1;
	mods->def.urgent = -1;

	char** listModules = ef_mem_matrix_new(MODULES_MAX, sizeof(char) * I3BAR_TEXT_MAX);
	char** listModulesDir = ef_mem_matrix_new(MODULES_MAX, sizeof(char) * PATH_MAX);
	for( size_t i = 0; i < MODULES_MAX; ++i){
		listModules[i][0] = 0;
		listModulesDir[i][0] = 0;
	}

	config_s conf;
	config_init(&conf, 256);
	config_add(&conf, "module.type", CNF_S, listModules, I3BAR_TEXT_MAX, MODULES_MAX);
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
	config_add(conf, "blink", CNF_D, &mod->blink, 0, 0);
	config_add(conf, "blink.time", CNF_LD, &mod->blinktime, 0, 0);
	config_add(conf, "text.full", CNF_S, &mod->longformat, I3BAR_TEXT_MAX, 0);
	config_add(conf, "text.short", CNF_S, &mod->shortformat, I3BAR_TEXT_MAX, 0);
	config_add(conf, "refresh", CNF_LD, &mod->reftime, 0, 0);
	config_add(conf, "color", CNF_U, &mod->i3.color, 0, 0);
	config_add(conf, "background", CNF_U, &mod->i3.background, 0, 0);
	config_add(conf, "border", CNF_U, &mod->i3.border, 0, 0);
	config_add(conf, "min_width", CNF_U, &mod->i3.min_width, 0, 0);
	config_add(conf, "align", CNF_U, &mod->i3.color, 0, 0);
	config_add(conf, "seaparator", CNF_U, &mod->i3.seaparator, 0, 0);
	config_add(conf, "separator_block_width", CNF_U, &mod->i3.separator_block_width, 0, 0);
	config_add(conf, "markup", CNF_U, &mod->i3.markup, 0, 0);
	config_add(conf, "icon", CNF_S, mod->icons, ICONS_SIZE, mod->iconcount);
	config_add(conf, "event", CNF_S, mod->onevent, MODULE_SPAWN_MAX, 0);
}

void modules_icons_init(module_s* mod, size_t count){
	mod->icons = ef_mem_matrix_new(count, sizeof(char) * ICONS_SIZE);
	mod->iconcount = count;
}

void modules_icons_set(module_s* mod, size_t id, char* ico){
	if( id >= mod->iconcount){
		dbg_warning("id icon out of range");
		return;
	}
	strcpy(mod->icons[id],ico);
}

void modules_dispatch(modules_s* mods, i3event_s* ev){
	for( size_t i = 0; i < mods->used; ++i ){
		if( mods->rmod[i].onevent[0] && !strcmp(mods->rmod[i].i3.instance, ev->instance) && !strcmp(mods->rmod[i].i3.name, ev->name)){
			char cmd[2048];
			module_reform(&mods->rmod[i], cmd, 2048, mods->rmod[i].onevent);
			spawn_shell(cmd);
		}
	}
}

void module_set_urgent(module_s* mod, int enable){
	if( mod->blink ){
		mod->blinkstatus =  enable;
		if( !enable ) mod->i3.urgent = 0;
	}
	else{
		mod->i3.urgent = enable;
	}

}

//TODO free modules??????
