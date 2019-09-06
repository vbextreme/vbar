#include <vbar.h>
#include <ef/spawn.h>

//
//txt:text
//cmd:cmd
//cmd:cmd

__private size_t TYPE = 0;

typedef struct script{
	char* out;
	char* err;
	char* cmd;
	int exitcode;
}script_s;

__private err_t script_shell(gadget_s* g, char* cmd){
	if( g->type != TYPE ) return -1;
	script_s* s = g->data;
	if( s->err ) free(s->err);
	if( s->out ) free(s->out);
	return spawn_shell_slurp(&s->out, &s->err, &s->exitcode, cmd);
}

__private err_t script_shell_event(gadget_s* g, char* cmd, gadgetEventType_e ev, void* arg){
	if( g->type != TYPE ) return -1;

	static const char* evname[] = {
		"GADGET_EVENT_REFRESH",
		"GADGET_EVENT_EXTEND_OPEN",
		"GADGET_EVENT_EXTEND_CLOSE",
		"GADGET_EVENT_EXTEND_REFRESH",
		"GADGET_EVENT_MOUSE_RELEASE", 
		"GADGET_EVENT_MOUSE_PRESS", 
		"GADGET_EVENT_MOUSE_MOVE", 
		"GADGET_EVENT_MOUSE_ENTER", 
		"GADGET_EVENT_MOUSE_LEAVE", 
		"GADGET_EVENT_MOUSE_CLICK", 
		"GADGET_EVENT_MOUSE_DBLCLICK"
	};

	if( ev >= 0 && ev <= GADGET_EVENT_MOUSE_DBLCLICK )
		dbg_info("shell '%s' event:%s arg:%p", cmd, evname[ev], arg);
	else
		dbg_info("shell '%s' event:-1 arg:%p", cmd, arg);
	

	size_t lencmd = strlen(cmd);
	if( lencmd + 150 > 4096 ){
		dbg_warning("cmd to long");
		return -1;
	}

	char opt[4096];
	strcpy(opt, cmd);
	char* args = opt + lencmd;
	*args++ = ' ';
	*args = 0;

	switch( ev ){
		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_REFRESH:
			strcpy(args, evname[ev]);
		break;
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		case GADGET_EVENT_MOUSE_CLICK:
		case GADGET_EVENT_MOUSE_LEAVE:
		case GADGET_EVENT_MOUSE_ENTER:
		{
			vbarMouse_s* m = arg;
			sprintf(args,"%s %u %u %u %u %u %u",evname[ev], m->x, m->y, m->button, m->icon, m->extend, m->line);
		}
		break;
	}

	script_s* s = g->data;
	if( s->err ) free(s->err);
	if( s->out ) free(s->out);
	dbg_info("spawn %s", opt);
	return spawn_shell_slurp(&s->out, &s->err, &s->exitcode, opt);
}

__private const char* script_raw_out_get(gadget_s* g){
	if( g->type != TYPE ) return "error gadget";
	script_s* s = g->data;
	return s->out;
}

__private const char* script_raw_err_get(gadget_s* g){
	if( g->type != TYPE ) return "error gadget";
	script_s* s = g->data;
	return s->err;
}

__private int script_exit_code_get(gadget_s* g){
	script_s* s = g->data;
	return s->exitcode;
}

__private const char* script_txt_get(gadget_s* g){
	if( g->type != TYPE ) return "error gadget";
	script_s* s = g->data;
	char* txt = strstr(s->out, "txt:");
	if( !txt ){
		dbg_warning("no text");
		return NULL;
	}
	char* rmn;
	if( (rmn=strchr(txt, '\n')) ) *rmn = 0;
	return txt+strlen("txt:");
}

__private const char* script_cmd_get(gadget_s* g){
	if( g->type != TYPE ) return "error gadget";
	script_s* s = g->data;
	if( s->cmd == NULL ) s->cmd = s->out;
	do{
		if( !strncmp(s->cmd, "cmd:", strlen("cmd:")) ){
			s->cmd += strlen("cmd:");
			return s->cmd;
		}
	}while( (s->cmd = strchr(s->cmd, '\n')) );
	return s->cmd;
}

__private int script_free(gadget_s* g){
	script_s* s = g->data;
	if( s->err ) free(s->err);
	if( s->out ) free(s->out);
	free(g->data);
	return 0;
}

__private void script_label_reset(gadget_s* g){
	g->redraw = GADGET_NOREDRAW;
	vbar_label_reset(g);
}

int gadget_script_load(gadget_s* g){
	dbg_info("load script");
	script_s* s = mem_new(script_s);
	s->cmd = s->err = s->out = NULL;
	s->exitcode = -1;

	g->data = s;
	g->ellapse = NULL;
	g->free = script_free;

	return 0;
}

void gadget_script_register(vbar_s* vb){
	dbg_info("register label");
	TYPE = gadget_type_get(vb, "script");
	config_add_symbol(vb, "gadget_script_shell", script_shell);
	config_add_symbol(vb, "gadget_script_shell_event", script_shell_event);
	config_add_symbol(vb, "gadget_script_out_raw", script_raw_out_get);
	config_add_symbol(vb, "gadget_script_err_raw", script_raw_err_get);
	config_add_symbol(vb, "gadget_script_exitcode", script_exit_code_get);
	config_add_symbol(vb, "gadget_script_txt", script_txt_get);
	config_add_symbol(vb, "gadget_script_cmd", script_cmd_get);
	config_add_symbol(vb, "gadget_script_label_reset", script_label_reset);
}

