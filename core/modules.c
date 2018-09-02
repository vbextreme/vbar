#define PRIVATE_MODULES
#include <vbar.h>
#include <config_mod.h>
#include "intp.h"

#define PHQ_PARENT(I) ((I)/2)
#define PHQ_LEFT(I) ((I)*2)
#define PHQ_RIGHT(I) ((I)*2+1)

int file_exists(char* fname){
	FILE* fd = fopen(fname,"r");
	if( fd == NULL ){
		dbg_warning("file %s not exists", fname);
		return 0;
	}
	fclose(fd);
	return 1;
}

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
			
			case '{':
				src = intp_interpretate(src, mod);
				if( NULL == src ){
					*dst = 0;   
					return;
				}
			break;

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
	if( mod->att.useshort ){
		module_reform(mod, mod->att.longformat, ATTRIBUTE_TEXT_MAX, mod->att.shortunformat);
	}
	else{
		module_reform(mod, mod->att.longformat, ATTRIBUTE_TEXT_MAX, mod->att.longunformat);
	}
	module_reform(mod, mod->att.shortformat, ATTRIBUTE_TEXT_MAX, mod->att.shortunformat);
}

void modules_refresh_output(modules_s* mods){
	ipc_begin_elements();
	
	module_s* last;
	module_s* it;
	it = last = mods->rmod;
	for(; it; it = it->next ){
		if( !it->att.hide ){
			last = it;
		}
	}
	for(it = mods->rmod; it != last; it = it->next){
		if( !it->att.hide )
			ipc_write_element( &it->att, TRUE);
	}
	ipc_write_element( &last->att, FALSE);

	ipc_end_elements();
}

__ef_private void modules_insert_inhash(modules_s* mods, module_s* mod){
	size_t h = hash_mods(mod->att.instance, strlen(mod->att.instance));
	mod->hnext = mods->hmod[h];
	mods->hmod[h] = mod;
	dbg_info("hash %lu insert '%s'", h, mod->att.instance);
}

__ef_private void module_load(modules_s* mods, char* name, char* path){
	AUTO_PROTO_MODULE
	
	__ef_private struct selective {
		char* name;
		int(*modload)(module_s*, char*);
		char* conf;
	} modsconf[] = { AUTO_VECTOR_MODULE {NULL, NULL, NULL}	};

	dbg_info("load module %s", name);

	for(size_t i = 0; modsconf[i].name; ++i){
		if( 0 == strcmp(name, modsconf[i].name) ){
			iassert(mods->used < MODULES_MAX);
			module_s* mod = ef_mem_new(module_s);
			mod->parent = mods;
			mod->next = mods->rmod;
			mods->rmod = mod;
			mod->att = mods->def;
			mod->att.onevent[0] = 0;
			int ok;
			if( *path ){
				ok = modsconf[i].modload(mod, path);
			}
			else{
				ok = modsconf[i].modload(mod, modsconf[i].conf);
			}
			if( !ok ){
				if( mod->att.reftime > 0) modules_insert(mods, mod);
				modules_insert_inhash(mods, mod);
				++mods->used;
			}
			else{
				free(mod);
				dbg_warning("module %s fail initialization",modsconf[i].name);
			}
		}
	}
}

__ef_private module_s* modules_search(modules_s* mods, char* instance, size_t lenI, char* name, size_t lenN){
	size_t h = hash_mods(instance, lenI);
	if( h > HMODS_MAX_HASH_VALUE ) return NULL;
	for(module_s* it = mods->hmod[h]; it; it = it->hnext){
		if( !str_len_cmp(it->att.name, strlen(it->att.name), name, lenN) ){
			return it;
		}
	}
	return NULL;
}

__ef_private void icmd_module_toggle(modules_s* mods, __ef_unused module_s* cl, size_t argc, char* argv[], size_t* argl){
	if( argc != 3 ){
		dbg_warning("wrong args %lu", argc);
		return;
	}

	module_s* mod = modules_search(mods, argv[0], argl[0], argv[1], argl[1]);
	if( mod ){
		char name[ATTRIBUTE_TEXT_MAX];
		str_nncpy_src(name, ATTRIBUTE_TEXT_MAX, argv[2], argl[2]);
		ipc_toggle_attribute_byname(&mod->att, name);
	}
	else{
		dbg_warning("no module %.*s::%.*s", (int)argl[0], argv[0], (int)argl[1], argv[1]);
	}
}

__ef_private void icmd_module_attribute_set(modules_s* mods, __ef_unused module_s* cl, size_t argc, char* argv[], size_t* argl){
	if( argc != 4 ){
		dbg_warning("wrong args %lu", argc);
		return;
	}
	
	module_s* mod = modules_search(mods, argv[0], argl[0], argv[1], argl[1]);
	if( mod ){
		char name[ATTRIBUTE_TEXT_MAX];
		char val[ATTRIBUTE_TEXT_MAX];
		str_nncpy_src(name, ATTRIBUTE_TEXT_MAX, argv[2], argl[2]);
		str_nncpy_src(val, ATTRIBUTE_TEXT_MAX, argv[3], argl[3]);

		ipc_set_attribute_byname(&mod->att, name, val);
	}
	else{
		dbg_warning("no module %.*s::%.*s", (int)argl[0], argv[0], (int)argl[1], argv[1]);
	}
}

__ef_private void icmd_module_attribute_reg(modules_s* mods, __ef_unused module_s* cl, size_t argc, char* argv[], size_t* argl){
	if( argc != 4 ){
		dbg_warning("wrong args %lu", argc);
		return;
	}
	
	module_s* mod = modules_search(mods, argv[0], argl[0], argv[1], argl[1]);
	if( mod ){
		char name[ATTRIBUTE_TEXT_MAX];
		size_t reg = strtoul(argv[3],  NULL, 10);
		str_nncpy_src(name, ATTRIBUTE_TEXT_MAX, argv[2], argl[2]);

		ipc_set_attribute_byreg(&mod->att, name, reg);
	}
	else{
		dbg_warning("no module %.*s::%.*s", (int)argl[0], argv[0], (int)argl[1], argv[1]);
	}
}

__ef_private void icmd_module_attribute_store(modules_s* mods, __ef_unused module_s* cl, size_t argc, char* argv[], size_t* argl){
	if( argc != 4 ){
		dbg_warning("wrong args %lu", argc);
		return;
	}
	
	module_s* mod = modules_search(mods, argv[0], argl[0], argv[1], argl[1]);
	if( mod ){
		char name[ATTRIBUTE_TEXT_MAX];
		size_t reg = strtoul(argv[3],  NULL, 10);
		str_nncpy_src(name, ATTRIBUTE_TEXT_MAX, argv[2], argl[2]);

		ipc_reg_store_attribute_byname(&mod->att, name, reg);
	}
	else{
		dbg_warning("no module %.*s::%.*s", (int)argl[0], argv[0], (int)argl[1], argv[1]);
	}
}

__ef_private void icmd_modules_refresh(modules_s* mods, module_s* mod, __ef_unused size_t argc, __ef_unused char* argv[], __ef_unused size_t* argl){
	modules_reformatting(mod);	
	modules_refresh_output(mods);
}

__ef_private void cbk_module_load(void* arg, __ef_unused char* name, __ef_unused size_t lenName, char* value, size_t lenValue){
	modules_s* mods = arg;
	char nn[ATTRIBUTE_TEXT_MAX];
	sprintf(nn,"%.*s", (int)lenValue, value);
	module_load(mods, nn, mods->generic);
	*((char*)mods->generic) = 0;
}

void modules_load(modules_s* mods, char* config){
	intp_register_command("toggle", icmd_module_toggle, mods);
	intp_register_command("set", icmd_module_attribute_set, mods);
	intp_register_command("reg", icmd_module_attribute_reg, mods);
	intp_register_command("store", icmd_module_attribute_store, mods);
	intp_register_command("refresh", icmd_modules_refresh, mods);

	mods->used = 0;
	mods->rmod = NULL;
	mods->generic = ef_mem_many(char, PATH_MAX);
	*((char*)mods->generic)=0;
	for( size_t i = 0; i < HMODS_MAX_HASH_VALUE + 1; ++i ){
		mods->hmod[i] = NULL;
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
	mods->def.separator = 1;
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
	mods->def.hide = 0;
	mods->def.useshort = 0;

	config_s conf;
	config_init(&conf, 256);
	config_add(&conf, "module.load", CNF_CBK, cbk_module_load, 0, 0, mods);
	config_add(&conf, "module.path", CNF_S, mods->generic, PATH_MAX, 0, NULL);
	config_add(&conf, "color", CNF_U, &mods->def.color, 0, 0, NULL);
	config_add(&conf, "background", CNF_U, &mods->def.background, 0, 0, NULL);
	config_add(&conf, "border", CNF_U, &mods->def.border, 0, 0, NULL);
	config_add(&conf, "min_width", CNF_U, &mods->def.min_width, 0, 0, NULL);
	config_add(&conf, "align", CNF_U, &mods->def.color, 0, 0, NULL);
	config_add(&conf, "separator", CNF_U, &mods->def.separator, 0, 0, NULL);
	config_add(&conf, "separator_block_width", CNF_U, &mods->def.separator_block_width, 0, 0, NULL);
	config_add(&conf, "markup", CNF_U, &mods->def.markup, 0, 0, NULL);
	config_load(&conf, config);
	config_destroy(&conf);
	
	free(mods->generic);
	mods->generic = NULL;
	if( mods->used == 0 ){
		dbg_fail("need set module to run");
	}
}

void modules_default_config(module_s* mod, config_s* conf){
	config_add(conf, "name", CNF_S, mod->att.name, ATTRIBUTE_TEXT_MAX, 0, NULL);
	config_add(conf, "blink", CNF_D, &mod->att.blink, 0, 0, NULL);
	config_add(conf, "blink.time", CNF_LD, &mod->att.blinktime, 0, 0, NULL);
	config_add(conf, "text.full", CNF_S, &mod->att.longunformat, ATTRIBUTE_TEXT_MAX, 0, NULL);
	config_add(conf, "text.short", CNF_S, &mod->att.shortunformat, ATTRIBUTE_TEXT_MAX, 0, NULL);
	config_add(conf, "text.short.enable", CNF_D, &mod->att.useshort, 0, 0, NULL);
	config_add(conf, "refresh", CNF_LD, &mod->att.reftime, 0, 0, NULL);
	config_add(conf, "color", CNF_D, &mod->att.color, 0, 0, NULL);
	config_add(conf, "background", CNF_D, &mod->att.background, 0, 0, NULL);
	config_add(conf, "border", CNF_D, &mod->att.border, 0, 0, NULL);
	config_add(conf, "min_width", CNF_D, &mod->att.min_width, 0, 0, NULL);
	config_add(conf, "align", CNF_D, &mod->att.color, 0, 0, NULL);
	config_add(conf, "separator", CNF_D, &mod->att.separator, 0, 0, NULL);
	config_add(conf, "separator_block_width", CNF_D, &mod->att.separator_block_width, 0, 0, NULL);
	config_add(conf, "markup", CNF_U, &mod->att.markup, 0, 0, NULL);
	config_add(conf, "icon", CNF_S, mod->att.icons, ATTRIBUTE_ICONS_SIZE, mod->att.iconcount, NULL);
	config_add(conf, "format", CNF_S, mod->att.format, ATTRIBUTE_FORMAT_MAX, mod->att.formatcount, NULL);
	config_add(conf, "event", CNF_S, mod->att.onevent, ATTRIBUTE_SPAWN_MAX, 0, NULL);
	config_add(conf, "hide", CNF_D, &mod->att.hide, 0, 0, NULL);
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
	size_t h = hash_mods(ev->instance, strlen(ev->instance));
	dbg_info("hash %lu name %s", h, ev->name);
	if( h > HMODS_MAX_HASH_VALUE ) return;
	for( module_s* it = mods->hmod[h]; it; it = it->hnext ){
		if( !it->att.hide && it->att.onevent[0] && !strcmp(it->att.name, ev->name)){
			dbg_info("dispatch mod %s", it->att.name);
			char cmd[2048];
			module_reform(it, cmd, 2048, it->att.onevent);
			if(*cmd) spawn_shell(cmd);
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

size_t os_read_lu(char* fname){
	FILE* fd = fopen(fname, "r");
	if( fd == NULL ){
		dbg_error("fopen %s", fname);
		dbg_errno();
		return 0;
	}

	char inp[64];
	inp[0] = 0;
	fgets(inp, 64, fd);
	fclose(fd);

	return strtoul(inp, NULL, 10);
}

