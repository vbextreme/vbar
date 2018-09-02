#include <vbar.h>
#include <sys/socket.h>
#include <sys/un.h>

typedef struct uxsocket{
	int fd;
	struct sockaddr_un addr;
	char* name;
	int rpid;
	module_s* this;
}uxsocket_s;


__ef_private void ux_close(uxsocket_s* ux){
	if( ux->fd == -1 ){
		dbg_warning("try to close closed socket");
		return;
	}
	close(ux->fd);
}

__ef_private int ux_open(uxsocket_s* ux, char* name){

	ux->fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if( ux->fd == -1 ){
		dbg_error("socket");
		dbg_errno();
		return -1;
	}
	
	memset(&ux->addr, 0, sizeof(ux->addr));
	ux->addr.sun_family = AF_UNIX;
	ux->name = name;
	
	iassert( ux->name != NULL );
	if( *ux->name == 0 ){
		dbg_info("global name %s", ux->name + 1);
		*ux->addr.sun_path = 0;
		strcpy(ux->addr.sun_path + 1, ux->name + 1);
	}
	else{
		dbg_info("file name %s", ux->name);
		strcpy(ux->addr.sun_path, ux->name);
		unlink(ux->name);
	}

	return 0;
}
	
__ef_private int ux_listen(uxsocket_s* ux){
	if( bind(ux->fd, (struct sockaddr*)&ux->addr, sizeof(ux->addr)) == -1 ){
		dbg_error("bind");
		dbg_errno();
		return -1;
	}

	if( listen(ux->fd, 10) == -1 ){
		dbg_error("listen");
		dbg_errno();
		return -1;
	}

	return 0;
}

__ef_private int ux_accept(uxsocket_s* ux){
	int fd = accept(ux->fd, NULL, NULL);
	if( fd == -1 ){
		dbg_error("accept");
		dbg_errno();
		return -1;
	}
	return fd;
}


__ef_private int ipc_mod_refresh(__ef_unused module_s* mod){
	return 0;
}

__ef_private int ipc_mod_env(__ef_unused module_s* mod, __ef_unused int id, char* dest){
	*dest = 0;
	return 0;
}

__ef_private int ipc_mod_free(module_s* mod){
	uxsocket_s* ux = mod->data;
	ux_close(ux);
	free(ux);
	return 0;
}

__ef_private char* ipc_event_parse(event_s* ev, char* parse){
	char* eName = strchr(parse, ':');
	if( eName == NULL || *eName == 0) return NULL;
	char* value = eName + 1;
	char* eValue = strchr(value, '\n');
	if( eValue == NULL ) return NULL;

	if( !str_len_cmp(parse, eName - parse, "instance", strlen("instance")) ){
		str_nncpy_src(ev->instance, ATTRIBUTE_TEXT_MAX, value, (eValue - value));
		return eValue + 1;
	}

	if( !str_len_cmp(parse, eName - parse, "name", strlen("name")) ){
		str_nncpy_src(ev->name, ATTRIBUTE_TEXT_MAX, value, (eValue - value));
		return eValue + 1;
	}
	
	dbg_warning("ipc not recognize %.*s:%.*s", (int)(eName-parse), parse, (int)(eValue - value), value);
	return NULL;
}

__ef_private void cbk_event(void* arg){
	uxsocket_s* ux = arg;
	
	int cli = ux_accept(ux);
	if( cli == -1 ){
		return;
	}
	
	event_s ev = { .instance[0] = 0, .name[0] = 0 };
	
	char buf[4096];
	ssize_t nr;
	nr = read( cli, buf, 4096 );
	if( nr > 0 ){
		char* parse = buf;
		while( (parse = ipc_event_parse(&ev, parse)) );
	}
	close(cli);
	if( ev.instance[0] && ev.name[0] ){
		modules_dispatch(ux->this->parent, &ev);
	}
}

int ipc_mod_load(module_s* mod, char* path){
	uxsocket_s* ux = ef_mem_new(uxsocket_s);
	ux->this = mod;

	mod->data = ux;
	mod->refresh = ipc_mod_refresh;
	mod->getenv = ipc_mod_env;
	mod->free = ipc_mod_free;

	strcpy(mod->att.longunformat, "ipc");
	strcpy(mod->att.shortunformat, "ipc");
	strcpy(mod->att.name, "generic");
	strcpy(mod->att.instance, "ipc");
	mod->att.hide = 1;
	mod->att.reftime = -1;

	//char fname[PATH_MAX] = "\0/vbar/ipc";
	char fname[PATH_MAX] = "/tmp/vbar.ipc.socket";

	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "port", CNF_S, fname, PATH_MAX, 0, NULL);
	config_load(&conf, path);
	config_destroy(&conf);

	if(	ux_open(ux, fname) ){
		return -1;
	}
	if( ux_listen(ux) ){
		return -1;
	}
	
	(void)ipc_register_callback(ux->fd, cbk_event, ux);	
	
	return 0;
}


