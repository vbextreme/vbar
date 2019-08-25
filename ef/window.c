#include <ef/window.h>
#include <ef/memory.h>
#include <ef/strong.h>
#include <xkbcommon/xkbcommon-x11.h>

void win_paint(__unused xorg_s* x, gadget_s* gadget){
	g2dCoord_s area = { .x = 0, .y = 0, .w = gadget->pos.w, .h = gadget->pos.h };
	g2d_clear(&gadget->surface, gadget->background, &area);
}

void win_move(gadgetMove_s* move){
	dbg_info("");
	move->gadget->pos = move->coord;
	move->gadget->border = move->border;
	if( move->event & GADGET_MOVE_RESIZE ){
		gadget_surface_update_size(move->gadget);
		if( move->gadget->paint ){
			move->gadget->paint(move->x, move->gadget);
			if( move->gadget->redraw ) move->gadget->redraw(move->x, move->gadget, NULL);
		}
	}
}

void win_redraw(xorg_s* x, gadget_s* win, __unused g2dCoord_s* damaged){
	if( win->paint ) win->paint(x, win);
	gadget_redraw(x, win);
}

void win_terminate(xorg_s* x, gadget_s* win){
	gadget_destroy(x, win);
}

void win_loop(xorg_s* x, gadget_s* win){
	win->eventLoop = 1;
	while( win->eventLoop && !gadget_event(x, win,1) );
}

void win_title(xorg_s* x, gadget_s* win, char* name){
	free(win->name);
	win->name = str_dup(name, 0);
	xcb_change_property(x->connection, XCB_PROP_MODE_REPLACE, win->idxcb, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(name), name);
}

void win_class(xorg_s* x, gadget_s* win, char* name){
	free(win->class);
	win->class = str_dup(name, 0);
	xcb_change_property(x->connection, XCB_PROP_MODE_REPLACE, win->idxcb, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, strlen(name), name);
}

void win_show(xorg_s* x, gadget_s* win, int show){
	if( show ){
		if( win->visible ) return;
		xcb_map_window(x->connection, win->idxcb);
		xorg_client_sync(x);
		win->redraw(x, win, NULL);
	}
	else{
		if( !win->visible ) return;
		xcb_unmap_window(x->connection, win->idxcb);
	}
	win->visible = show;
}

void win_new(xorg_s* x, gadget_s* parent, gadget_s* win, char* name, char* class, g2dCoord_s* pos, unsigned border, g2dColor_t background){
	gadget_init(win, name, pos, border, background);
	win->redraw = win_redraw;
	win->paint = win_paint;
	win->terminate = win_terminate;
	win->loop = win_loop;
	win->move = win_move;
	win->show = win_show;
	gadget_surface_create(x, parent, win);
	if( name ) win_title(x, win, name);
	if( class ) win_class(x, win, class);
}
	













