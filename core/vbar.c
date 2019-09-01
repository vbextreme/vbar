#include <vbar.h>
#include <stdarg.h>
#include "gadget_autogen.h"

#define VBAR_IPC_BUFFER_SIZE 1024
#define VBAR_IPC_RAISE "raise "
#define VBAR_IPC_EVENT_REFRESH "refresh\n"

//TODO bar_draw_left/center/right to accelerate bar_draw

/****************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************/
/********************************************************************** GADGET **********************************************************************************/
/****************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************/

__private err_t vbar_gadget_insert(vbar_s* vb, gadget_s* g){
	dbg_info("insert %s", g->instance);
	
	if( chash_add_unique(&vb->coyote, g->instance, strlen(g->instance), 0, g) ){
		dbg_warning("%s already exists", g->instance);
		return -1;
	}

	if( vb->drawed ){
		vb->drawed->prev->next = g;
		g->next = vb->drawed;
		vb->drawed->prev = g;
	}
	else{
		vb->drawed = g->prev = g->next = g;
	}

	if( g->eventTimems > 0 ){
		dbg_info("add new runner %s", g->instance);
		g->selfE.priority = g->eventTimems + time_ms();
		phq_insert(&vb->runner, &g->selfE);
	}

	return 0;
}

size_t gadget_type_get(vbar_s* vbar, const char* name){
	chashElement_s* e = chash_find_raw(&vbar->list, name, strlen(name));
	if( !e ){
		return 0;
	}
	return (e->distance << 31) | e->hash;
}

__private gadget_s* gadget_new(vbar_s* vb, char const* class, char const* name){
	gadget_s* g = mem_zero_many(gadget_s,1);
	iassert(g);
	g->vbar = vb;
	g->status = -1;
	g->lenClass = strlen(class);
	g->lenName = strlen(name);
	g->instance = mem_many(char, g->lenClass+g->lenName+2);
	g->class = g->instance;
	g->name = stpcpy(g->class, class);
	*g->name++ = ':';
	strcpy(g->name, name);
	g->background = vb->bar.background;
	g->foreground = vb->bar.foreground;
	g->selfE.index = 0;
	g->selfE.priority = 0;
	g->selfE.data = g;
	g->selfE.free = NULL;
	g->type = gadget_type_get(vb, class);
	return g;
}

__private void gadget_free(gadget_s* g){
	if( g->free ) g->free(g);
	free(g->instance);
	free(g);
}

__private void gadget_tick(gadget_s* g){
	if( g->eventTimems > 0 ){
		g->selfE.priority = g->eventTimems + time_ms();
		dbg_info("%s is runner wake on %ld", g->instance, g->selfE.priority);
		phq_insert(&g->vbar->runner, &g->selfE);
	}
}

__private void gadget_change_tick(gadget_s* g, long newtime){
	if( newtime < 0 ) newtime = 0;
	if( g != g->vbar->active ){
		if( newtime > 0 ){
			dbg_info("%s change tick", g->instance);
			g->eventTimems = newtime;
			if( phq_peek(&g->vbar->runner) != &g->selfE ){
				phq_change_priority(&g->vbar->runner, newtime + time_ms(), &g->selfE);
			}
		}
	}
}

__private void gadget_interval(gadget_s* g, long newtime){
	//if( newtime < 100 ) newtime = 100;
	if( newtime < 0 ) newtime = 0;
	g->eventTimems = newtime;
}

__private void gadget_start(gadget_s* g){
	if( g != g->vbar->active ){
		gadget_tick(g);
	}
}

__private void gadget_stop(gadget_s* g){
	if( g != g->vbar->active ){
		phq_remove(&g->vbar->runner, &g->selfE);
	}
}

__private void gadget_event_register(gadget_s* g, gadgetScriptEvent_f event){
	g->event = event;
}

__private void gadget_background(gadget_s* g, g2dColor_t color){
	g->background = color;
	g->redraw = GADGET_REDRAW;
}

__private void gadget_foreground(gadget_s* g, g2dColor_t color){
	g->foreground = color;
	g->redraw = GADGET_REDRAW;
}

__private void gadget_border_color(gadget_s* g, g2dColor_t color){
	g->borderColor = color;
	g->redraw = GADGET_REDRAW;
}

__private void gadget_border(gadget_s* g, unsigned border){
	g->border = border;
	g->redraw = GADGET_REDRAW;
}

__private void gadget_align(gadget_s* g, vbarAligned_e align){
	g->align = align;
	g->position.x = 0;
	g->position.y = 0;
	g->position.w = 0;
	g->position.h = 0;	
}

__private void gadget_icon(gadget_s* g, char const* name){
	g->oldIcon = g->iconName;
	g->iconName = name;
	g->redraw = GADGET_REDRAW;
}

__private void gadget_hide(gadget_s* g, int hide){
	if( hide != g->hiden ) g->redraw = GADGET_FULL_REDRAW;
	g->hiden = hide;
}

__private int gadget_status_get(gadget_s* g){
	return g->status;
}

__private void gadget_status_set(gadget_s* g, int status){
	g->status = status;
}

__private size_t gadget_text_lenght(gadget_s* g, utf8_t* txt){
	return ft_line_lenght(&g->vbar->bar.fonts, txt);
}

__private void gadget_text_reset(gadget_s* g){
	vbar_label_reset(g);
}

__private void gadget_text(gadget_s* g, char const* format,  ...){
	va_list args;
	va_list aargs;
	va_start(args, format);
	va_start(aargs, format);

	if( g->redraw == GADGET_NOREDRAW ) g->redraw = GADGET_REDRAW_PARTIAL;

	size_t len = vsnprintf(NULL, 0, format, args);
	dbg_info("format \"%s\" required %lu spaces", format, len);

	if( g->label ){
		if( g->labelSize - g->labelLen < len + 1 ){
			dbg_info("resize");
			len = ROUND_UP(len, GADGET_TEXT_RESIZE);
			g->labelSize += len + 1;
			g->label = realloc(g->label, sizeof(utf8_t) * g->labelSize);
			iassert(g->label);
		}
	}
	else{
		dbg_info("alloc");
		len = ROUND_UP(len, GADGET_TEXT_RESIZE);
		g->label = mem_many(utf8_t, len);
		g->labelSize = len;
	}

	g->labelLen += vsnprintf((char*)&g->label[g->labelLen], g->labelSize - g->labelLen, format, aargs);
	dbg_info("text:\"%s\" partial len:%lu", g->label, g->labelLen);

	va_end(args);
	va_end(aargs);
}

//0 byte
//1 kilo
//2 mega
//3 giga
//4 tera
__private const char* gadget_tohuman(double* out, double value, int initial, double div){
	static char* hstr[] = {"b ","kb","mb","gb","tb"};
	while( value > div ){
		value /= div;
		++initial;
	}
	*out = value;
	return hstr[(unsigned)initial];
}

__private void gadget_redraw(gadget_s* g){
	if( !g->label ) return;
	switch( g->redraw ){
		default: case GADGET_NOREDRAW: break;
		case GADGET_REDRAW_PARTIAL: bar_gadget_draw(g->vbar, g, g->oldLabel); break;
		case GADGET_FULL_REDRAW:
		case GADGET_REDRAW: bar_gadget_draw(g->vbar, g, NULL); break;
	}
}

__private void gadget_extend_redraw(gadget_s* g){
	if( g->vbar->extend ){
		bar_draw_extend(g->vbar->extend);
	}
}

__private void vbar_icon_load(vbar_s* vb, char const* path, char const* name, unsigned bk){
	g2dImage_s png = {0};

	if( g2d_load(&png, path) ){
		dbg_warning("icon \"%s\" not exists", path);
		return;
	}

	icon_s* icon = mem_zero_many(icon_s, 1);
	icon->name = name;
	icon->path = path;
	if( chash_add_unique(&vb->icons, name, strlen(name), 0, icon) ){
		dbg_info("already loaded");
		free(icon);
		g2d_unload(&png);
		return;
	}

	g2dImage_s img = g2d_new(png.w, png.h, X_COLOR_MODE);
	g2dCoord_s dc = { .x = 0, .y = 0, .w = png.w, .h = png.h };	
	g2d_clear(&img, bk, &dc);
	g2d_bitblt_alpha(&img, &dc, &png, &dc);

	g2d_resize(&icon->img, &img, vb->bar.height, vb->bar.height - (vb->bar.topSpacing + vb->bar.bottomSpacing), 0);
	g2d_unload(&png);
	g2d_unload(&img);
	dbg_info("icon %s (%s) loaded", name, path);
}

__private void gadget_gline_free(gadget_s* g){
	for( size_t i = 0; i < g->extendCount; ++i ){
		free(g->extend[i].label);
		free(g->extend[i].oldLabel);
		free(g->extend[i].iconName);
		free(g->extend[i].oldIconName);
	}
	free(g->extend);
}

__private void gadget_extend_enable(gadget_s* g, size_t count){
	if( g->extend ) gadget_gline_free(g);
	g->extend = mem_zero_many(gLine_s, count);
	iassert(g->extend != NULL);
	g->extendCount = count;
	g->extendHeight = g->vbar->bar.height * count;
	for( size_t i = 0; i < count; ++i){
		g->extend[i].background = g->background;
		g->extend[i].foreground = g->foreground;
	}
}

__private void gadget_extend_background(gadget_s* g, size_t index, g2dColor_t color){
		if( index >= g->extendCount ){
		dbg_warning("out of bounds");
		return;
	}
	g->extend[index].background = color;
	g->redraw = GADGET_REDRAW;
}

__private void gadget_extend_foreground(gadget_s* g, size_t index, g2dColor_t color){
	if( index >= g->extendCount ){
		dbg_warning("out of bounds");
		return;
	}
	g->extend[index].foreground = color;
	g->redraw = GADGET_REDRAW;
}

__private void gadget_extend_border_color(gadget_s* g, size_t index, g2dColor_t color){
	if( index >= g->extendCount ){
		dbg_warning("out of bounds");
		return;
	}	
	g->extend[index].borderColor = color;
	g->extend[index].redraw = GADGET_REDRAW;
}

__private void gadget_extend_border(gadget_s* g, size_t index, unsigned border){
	if( index >= g->extendCount ){
		dbg_warning("out of bounds");
		return;
	}	
	g->extend[index].border = border;
	g->extend[index].redraw = GADGET_REDRAW;
}

__private void gadget_extend_label(gadget_s* g, size_t index, char const* format,  ...){ //TODO
	if( index >= g->extendCount ){
		dbg_warning("out of bounds");
		return;
	}
	
	va_list args;
	va_list aargs;
	va_start(args, format);
	va_start(aargs, format);

	gLine_s* gl = &g->extend[index];

	if( gl->type == GADGET_LINE_SEPARATOR ){
		gl->redraw = GADGET_REDRAW;
		gl->type = GADGET_LINE_LABEL;
	}
	if( gl->redraw == GADGET_NOREDRAW ) gl->redraw = GADGET_REDRAW_PARTIAL;

	size_t len = vsnprintf(NULL, 0, format, args);
	dbg_info("format \"%s\" required %lu spaces", format, len);
	
	if( gl->label ){
		if( gl->labelSize - gl->labelLen < len + 1 ){
			dbg_info("resize");
			len = ROUND_UP(len, GADGET_TEXT_RESIZE);
			gl->labelSize += len + 1;
			gl->label = realloc(gl->label, sizeof(utf8_t) * gl->labelSize);
			iassert(gl->label);
		}
	}
	else{
		dbg_info("alloc");
		len = ROUND_UP(len, GADGET_TEXT_RESIZE);
		gl->label = mem_many(utf8_t, len);
		gl->labelSize = len;
	}

	gl->labelLen += vsnprintf((char*)&gl->label[gl->labelLen], gl->labelSize - gl->labelLen, format, aargs);
	dbg_info("text:\"%s\" partial len:%lu", gl->label, gl->labelLen);

	va_end(args);
	va_end(aargs);
}

__private void gadget_extend_icons_enable(gadget_s* g, size_t index, size_t count){
	if( index >= g->extendCount ){
		dbg_warning("out of bounds");
		return;
	}

	gLine_s* gl = &g->extend[index];
	gl->type = GADGET_LINE_SELECTOR;
	gl->iconName = mem_zero_many(char*, count);
	gl->oldIconName = mem_zero_many(char*, count);
	gl->iconCount = count;	
}

__private void gadget_extend_icons_set(gadget_s* g, size_t index, size_t id, char* name){
	if( index >= g->extendCount ){
		dbg_warning("out of bounds");
		return;
	}

	gLine_s* gl = &g->extend[index];
	if( id >= gl->iconCount ){
		dbg_warning("icon out of bounds");
		return;
	}
	
	gl->redraw = GADGET_REDRAW;
	gl->oldIconName[id] = gl->iconName[id];
	gl->iconName[id] = name; 
}

__private void gadget_extend_toggle(gadget_s* g){
	if( g->vbar->extend == g ){
		dbg_info("extend close");
		g->vbar->extend = NULL;
		bar_extend_close(&g->vbar->bar);
		if( g->event ) g->event(g, GADGET_EVENT_EXTEND_CLOSE, NULL);
		return;
	}
	dbg_info("extend open");
	g->vbar->extend = g;
	for( size_t i = 0; i < g->extendCount; ++i ){
		g->extend[i].redraw = GADGET_REDRAW;
	}

	if( g->event ){
		g->event(g, GADGET_EVENT_EXTEND_OPEN, NULL);
		vbar_extend_reset(g);
		g->event(g, GADGET_EVENT_EXTEND_REFRESH, NULL);
	}
}

/**************************************************************************************************************************************************************/
/**************************************************************************************************************************************************************/
/********************************************************************** VBAR **********************************************************************************/
/**************************************************************************************************************************************************************/
/**************************************************************************************************************************************************************/

void vbar_label_reset(gadget_s* g){
	SWAP(g->oldLabel, g->label);
	SWAP(g->oldLableSize, g->labelSize);
	g->labelLen = 0;
}

void vbar_extend_reset(gadget_s* g){
	for( size_t i = 0; i < g->extendCount; ++i ){
		SWAP(g->extend[i].oldLabel, g->extend[i].label);
		SWAP(g->extend[i].oldLableSize, g->extend[i].labelSize);
		g->extend[i].labelLen = 0;
		g->extend[i].redraw = GADGET_NOREDRAW;
	}
}

__private void vbar_raise_refresh(vbar_s* vb, gadget_s* g){
	dbg_info("%s",g->instance);
	g->redraw = GADGET_NOREDRAW;	
	vbar_label_reset(g);

	gadget_s* old = vb->active;
   	vb->active = g;

	if( g->ellapse ){ g->ellapse(g);}
	
	if( g->event ){
		g->event(g, GADGET_EVENT_REFRESH, NULL);
		if( vb->extend == g ){
			vbar_extend_reset(vb->extend);
			g->event(g, GADGET_EVENT_EXTEND_REFRESH, NULL);
		}
	}
	
	vb->active = old;
}

/*
void _dump(phq_s *q){
    for(size_t i = 1; i < q->count; ++i){
		gadget_s* g = q->elements[i]->data;
        dbg_info("[%lu-%lu] priority=%lu data=%p %s", i, q->elements[i]->index, q->elements[i]->priority, q->elements[i]->data, g->instance); 
    }
}
*/

err_t vbar_deadline(__unused int type, void* vbar){
	vbar_s* vb = vbar;
	phqElement_s* raised;

	do{	
		iassert(phq_count(&vb->runner));
		raised = phq_pop(&vb->runner);
		iassert( raised );

		gadget_s* g = raised->data;
		iassert( g );
		dbg_info("deadline %s", g->instance);
		g->redraw = GADGET_NOREDRAW;	
		vbar_label_reset(g);

		vb->active = g;
		if( g->ellapse ){
			g->ellapse(g);
		}
	
		if( g->event ){
			g->event(g, GADGET_EVENT_REFRESH, NULL);
			if( vb->extend == g ){
				vbar_extend_reset(vb->extend);
				g->event(g, GADGET_EVENT_EXTEND_REFRESH, NULL);
			}
		}
	
		if( vb->active  ){
			gadget_tick(g);
			vb->active = NULL;
		}
		raised = phq_peek(&vb->runner);
	}while( raised->priority <= time_ms() );

	return 0;
}

void vbar_xorg_register_event(vbar_s* vb, gadget_s* g, int event, xcb_atom_t atom){
	xorgEventRegister_s* er = mem_new(xorgEventRegister_s);
	iassert(er);
	er->event = event;
	er->atom = atom;
	er->gadget = g;
	er->next = vb->xer;
	vb->xer = er;
}

void vbar_ipc_client_connect(socket_s* ipc){
	socket_init(ipc, NULL, NULL, NULL);
	if( socket_unix_open(ipc, VBAR_IPC, 0) ){
		socket_close(ipc);
		return;
	}
	if( socket_connect(ipc) ){
		socket_close(ipc);
		return;
	}
}

void vbar_ipc_send(socket_s* ipc, int type, char* instance, gadgetEventType_e event){
	if( !socket_isopen(ipc) ) return;
	switch( type ){
		case 1:
			socket_puts(ipc, VBAR_IPC_RAISE, 0);
		break;
		default:
		return;
	}

	socket_puts(ipc, instance, 0);
	socket_puts(ipc, " ", 0);

	switch( event ){
		case GADGET_EVENT_REFRESH:
			socket_puts(ipc, "refresh", 0);
		break;
		default:
		break;
	}
	socket_puts(ipc, "\n", 0);
}

__private char* vbar_ipc_instance_check(size_t* olen, vbar_s* vb, ipcproto_s* ipc, char* buf, size_t len){
	ipc->instance = buf;
	char* endclass = memchr(buf, ' ', len);
	if( !endclass ){
		dbg_warning("instance not have class");
		return NULL;
	}
	*endclass = 0;
	*olen = strlen(ipc->instance);
	if( chash_find((void**)&ipc->g, &vb->coyote, ipc->instance, *olen) ){
		dbg_warning("instance not exists");
		return NULL;
	}
	return endclass+1;
}

__private err_t vbar_ipc_parse(vbar_s* vb, ipcproto_s* ipc, char* buf, size_t len){
	iassert( buf != NULL);
	char* begin = buf;

	if( strncmp(begin, VBAR_IPC_RAISE, strlen(VBAR_IPC_RAISE)) ){
		dbg_warning("unknow message: '%s'", begin);
		return -1;
	}
	ipc->type = 1;
	buf += strlen(VBAR_IPC_RAISE);
	len -= strlen(VBAR_IPC_RAISE);

	size_t ilen;
	if( !(buf = vbar_ipc_instance_check(&ilen, vb, ipc, buf, len)) ){
		dbg_warning("wrong instance: '%s'", begin);
		return -1;
	}
	//len -= ilen;

	if( strncmp(buf, VBAR_IPC_EVENT_REFRESH, strlen(VBAR_IPC_EVENT_REFRESH)) ){
		dbg_warning("unknow event: '%s'", begin);
		return -1;
	}
	ipc->event = GADGET_EVENT_REFRESH;
	return 0;
}

__private void vbar_ipc_exec(ipcproto_s* ipc){
	if( ipc->type != 1 ) return;
	if( ipc->event != GADGET_EVENT_REFRESH ) return;
	if( !ipc->g ) return;
	if( ipc->g->event ){
		vbar_label_reset(ipc->g);
		ipc->g->event(ipc->g, ipc->event, NULL); 
	}
}

__private int vbar_ipc_read_request(socket_s* s){
	dbg_info("read request");
	vbar_s* vb = s->userdata;
	size_t len;
	char buf[VBAR_IPC_BUFFER_SIZE];

	while( socket_gets(&len, s, buf, VBAR_IPC_BUFFER_SIZE, '\n') > 0 ){
		ipcproto_s ipc;
		if( !vbar_ipc_parse(vb, &ipc, buf, len) ){
			vbar_ipc_exec(&ipc);
		}
	}
	return 0;
}

__private int vbar_ipc_close_client(socket_s* s){
	dbg_info("close client");
	vbar_s* vb = s->userdata;
	socket_close(s);
	deadpoll_unregister(&vb->events, s->fd);
	free(s);
	return 0;
}

__private int vbar_ipc_accept_client(socket_s* s){
	dbg_info("new client connection");
	vbar_s* vb = s->userdata;
	socket_s* client = mem_new(socket_s);
	iassert(client);
	socket_init(client, vbar_ipc_read_request, vbar_ipc_close_client, s->userdata);
	socket_accept(client, s);
	deadpoll_register(&vb->events, client->fd, (pollCbk_f)socket_parse_events, client, SOCKET_EPOLL_EVENTS);
	return 0;
}

__private int vbar_ipc_close(socket_s* s){
	vbar_s* vb = s->userdata;
	socket_close(s);
	deadpoll_unregister(&vb->events, s->fd);
	return 0;
}

__private void vbar_ipc_init(vbar_s* vb){
	socket_init( &vb->ipc, vbar_ipc_accept_client, vbar_ipc_close, vb);
	if( socket_unix_open(&vb->ipc, VBAR_IPC, 1) ){
		socket_close(&vb->ipc);
	}
	if( socket_listen(&vb->ipc) ){
		socket_close(&vb->ipc);
	}
	deadpoll_register(&vb->events, vb->ipc.fd, (pollCbk_f)socket_parse_events, &vb->ipc, SOCKET_EPOLL_EVENTS);
}

void vbar_begin(vbar_s* vb){
	deadpoll_init(&vb->events);
	vb->events.timeout = vbar_deadline;
	vb->events.timeoutarg = vb;
	
	phq_init(&vb->runner, 16, 8, phq_cmp_asc);
	
	chash_init(&vb->coyote, 64, 12, hash_fasthash, NULL);
	chash_init(&vb->list, 64, 12, hash_fasthash, NULL);
	chash_init(&vb->icons, 16, 12, hash_fasthash, NULL);
	
	vb->xer = NULL;
	
	vbar_ipc_init(vb);

	bar_begin(vb);
}

void vbar_register_gadget(vbar_s* vb){
	GADGET_AUTOGEN_PROTO_LOAD

	struct autoreg{
		char* name;
		int(*freg)(gadget_s*);
	};
	const struct autoreg greg[] = {
		GADGET_AUTOGEN_VECTOR_LOAD
		{NULL,NULL}
	};

	for( size_t i = 0; greg[i].name; ++i ){
		dbg_info("load gadget %s", greg[i].name);
		chash_add_unique(&vb->list, greg[i].name, strlen(greg[i].name), 0, greg[i].freg);
	}
}

//TODO release resources of gadget
//TODO release resources of icons	
void vbar_end(vbar_s* vb){
	bar_terminate(vb);
	tcc_delete(vb->tcc);
	free(vb->code);
	deadpoll_terminate(&vb->events);
	phq_free(&vb->runner);
	chash_free(&vb->coyote);
	chash_free(&vb->list);
	chash_free(&vb->icons);
}

gadget_s* vbar_gadget_load(vbar_s* vb, char const* class, char const* name){
	gadgetEvent_f load;
	if( chash_find((void*)&load, &vb->list, class, strlen(class)) ){
		dbg_warning("gadget %s::%s not exists", class, name);
		return NULL;
	}
	
	gadget_s* g = gadget_new(vb, class, name);
	if( load(g) ){
		dbg_warning("gadget %s::%s return error", class, name);
		gadget_free(g);
		return NULL;
	}
	
	if( vbar_gadget_insert(vb, g) ){
		dbg_warning("can't insert cadget");
		gadget_free(g);
		return NULL;
	}

	return g;
}

gadget_s* vbar_gadget_byposition(int* exline, int* icon, vbar_s* vb, unsigned x, unsigned y){
	if( y <= (unsigned)vb->bar.height ){
		*exline = -1;
		*icon = -1;
		gadget_s* g = vb->drawed;
		do{
			//dbg_info("%s %u > %u && %u < %u",g->instance, x, g->position.x, x, g->position.x + g->position.w);
			if( x > g->position.x && x < g->position.x + g->position.w && 
				y > g->position.y && y < g->position.y + g->position.h
			){
				return g;
			}
			g = g->next;
		}while( g != vb->drawed );
		return NULL;
	}
	
	gadget_s* g = vb->extend;
	if( !g ) return NULL;
	*exline = y / vb->bar.height;
	if( (unsigned)*exline  > g->extendCount ) return NULL;
	*icon = -1;
	if( g->extend[*exline].type == GADGET_LINE_SELECTOR ){
		unsigned w = vb->bar.height;
		unsigned allicon = w * g->extend[*exline].iconCount;
		unsigned barw = xorg_root_width(&vb->bar.x);
		unsigned beginx = barw - allicon;
		if( x > beginx ){
			*icon = (x - beginx) / w;
		}
	}
	return g;
}

__private char* lastTccError = NULL;

__private char* script_source_load(char const* source){
	FILE* fd = fopen(source, "r");
	char* code = NULL;
	if( !fd ) goto ONERROR;
	if( fseek(fd, 0, SEEK_END) < 0 ) goto ONERROR;
	long size;
	if( (size = ftell(fd)) < 0 ) goto ONERROR;
	rewind(fd);
	code = mem_many(char, size + 1);
	if( fread(code, 1, size, fd) != (unsigned long)size ) goto ONERROR;
	code[size] = 0;
	fclose(fd);
	return code;
ONERROR:
	dbg_error("config");
	dbg_errno();
	free(code);
	if( fd ) fclose(fd);
	lastTccError = "error on read config file";
	return NULL;
}

__private void my_tcc_error(__unused void *opaque, const char *msg){
	fprintf(stderr, "vbar config file error:\n%s\n",msg);
	size_t len = strlen(msg);
	if( lastTccError == NULL ){
		//char* testmsg = "test\nscroll\nerror\nmessage\n\nerror\ngenerate\nTCC\ncause\nconfig\n";
		//lastTccError = mem_many(char, strlen(testmsg)+1);
		//strcpy(lastTccError, testmsg);
		lastTccError = mem_many(char, strlen(VBAR_ERROR_BEGIN)+1);
		strcpy(lastTccError, VBAR_ERROR_BEGIN);
	}
	
	size_t errlen = strlen(lastTccError); 
	char* tmp = realloc(lastTccError, errlen+len+2);
	if( !tmp ){
		dbg_fail("realloc tcc error string");
	}
	lastTccError = tmp;
	strcpy(lastTccError+errlen, msg);
	lastTccError[errlen+len] = '\n';
	lastTccError[errlen+len+1] = 0;
}

err_t vbar_script_load(vbar_s* vb, char const* sourcefile){
	vb->tcc = tcc_new();
	tcc_set_output_type(vb->tcc, TCC_OUTPUT_MEMORY);
	tcc_set_options(vb->tcc, "-Wall");
	vb->code = script_source_load(sourcefile);
	if( !vb->code ){
		dbg_error("load script");
		return -1;
	}

	tcc_set_error_func(vb->tcc, NULL, my_tcc_error);
	//tcc_add_include_path(vb->tcc, "/home/vbextreme/Project/c/app/vbar/build"); 
	if( tcc_compile_string(vb->tcc, vb->code) == -1 ){
		dbg_error("script build");
		return -1;
	}
	return 0;
}



void vbar_change_ferr(const char* name){
	FILE* f = fopen(name,"w");
	if( !f ){
		dbg_error("on change stderror");
		dbg_errno();
		return;
	}
	fclose(stderr);
	stderr=f;
}

const char* vbar_script_error(void){
	return lastTccError;
}

__private vbar_s* _vbar_;
__private vbar_s* vbar_get(void){
	return _vbar_;
}

void vbar_register_symbol(vbar_s* vb){
	GADGET_AUTOGEN_PROTO_REGISTER
	typedef void(*cbkreg_f)(vbar_s*);
	const cbkreg_f grf[] = { GADGET_AUTOGEN_VECTOR_REGISTER NULL};

	_vbar_ = vb;
	config_add_symbol(vb, "vbar_get", vbar_get);
	config_add_symbol(vb, "vbar_icon_load", vbar_icon_load);
	config_add_symbol(vb, "vbar_raise_refresh", vbar_raise_refresh);
	config_add_symbol(vb, "gadget_new", vbar_gadget_load);
	config_add_symbol(vb, "gadget_interval", gadget_interval);
	config_add_symbol(vb, "gadget_change_interval", gadget_change_tick);
	config_add_symbol(vb, "gadget_start", gadget_start);
	config_add_symbol(vb, "gadget_stop", gadget_stop);
	config_add_symbol(vb, "gadget_event_register", gadget_event_register);
	config_add_symbol(vb, "gadget_background", gadget_background);
	config_add_symbol(vb, "gadget_foreground", gadget_foreground);
	config_add_symbol(vb, "gadget_border_color", gadget_border_color);
	config_add_symbol(vb, "gadget_border", gadget_border);
	config_add_symbol(vb, "gadget_align", gadget_align);
	config_add_symbol(vb, "gadget_icon", gadget_icon);
	config_add_symbol(vb, "gadget_hide", gadget_hide);
	config_add_symbol(vb, "gadget_status_get", gadget_status_get);
	config_add_symbol(vb, "gadget_status_set", gadget_status_set);
	config_add_symbol(vb, "gadget_text_lenght", gadget_text_lenght);
	config_add_symbol(vb, "gadget_text_reset", gadget_text_reset);
	config_add_symbol(vb, "gadget_text", gadget_text);
	config_add_symbol(vb, "gadget_tohuman", gadget_tohuman);
	config_add_symbol(vb, "gadget_redraw", gadget_redraw);
	config_add_symbol(vb, "gadget_extend_enable", gadget_extend_enable);
	config_add_symbol(vb, "gadget_extend_background", gadget_extend_background);
	config_add_symbol(vb, "gadget_extend_foreground", gadget_extend_foreground);
	config_add_symbol(vb, "gadget_extend_border_color", gadget_extend_border_color);
	config_add_symbol(vb, "gadget_extend_border", gadget_extend_border);
	config_add_symbol(vb, "gadget_extend_label", gadget_extend_label);
	config_add_symbol(vb, "gadget_extend_icons_enable", gadget_extend_icons_enable);
	config_add_symbol(vb, "gadget_extend_icons_set", gadget_extend_icons_set);
	config_add_symbol(vb, "gadget_extend_toggle", gadget_extend_toggle);
	config_add_symbol(vb, "gadget_extend_redraw", gadget_extend_redraw);

	bar_register_symbol(vb);
	for( size_t i = 0; grf[i]; ++i ){
		grf[i](vb);
	}
}

err_t vbar_script_run(vbar_s* vb){
	if( tcc_relocate(vb->tcc, TCC_RELOCATE_AUTO) == -1 ){
		dbg_error("script run");
		return -1;
	}

    void(*vbar_main)(void) = tcc_get_symbol(vb->tcc, "vbar_main");
	vbar_main();
	return 0;
}

void vbar_gadget_refresh_all(vbar_s* vb){
	dbg_info("refresh all gadget");
	gadget_s* g = vb->drawed;
	if( !g ) return;
	do{
		vbar_raise_refresh(vb, g);
		g = g->next;
	}while(g != vb->drawed);
}

void vbar_loop(vbar_s* vb){
	long timer = -1;
	
	while(1){
		phqElement_s* pk = phq_peek(&vb->runner);
		if( pk ){
			if( pk->priority <= time_ms() ){
				vbar_deadline(0, vb);
				continue;
			}
			
			timer = pk->priority - time_ms();
			dbg_info("wait %ld", timer);
		}
		while( DEADPOLL_EVENT == deadpoll_event(&vb->events, &timer) && timer > 0 );
		timer = -1;
	}
}


