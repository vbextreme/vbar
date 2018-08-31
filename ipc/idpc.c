#include <vbar/ipc.h>
#include <vbar/string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DWM_MAX_TITLE
	#define DWM_MAX_TITLE 256
#endif

__ef_private Display *dpy;
__ef_private int screen;
__ef_private Window root;

__ef_private char title[DWM_MAX_TITLE];
__ef_private char* ttw;

__ef_private void ipc_write(char* name){
	dbg_info("setroot:'%s'",name);
	XStoreName(dpy, root, name);
	XFlush(dpy);
}

int ipc_onstdin(__ef_unused event_s* ev){
	dbg_warning("not implementated");
	return -1;
}

void ipc_custom_init(__ef_unused bool_t clickevents){
	dbg_info("ipc dwm init");
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		dbg_fail("unable to open display %s", XDisplayName(NULL));
	}
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
}

void ipc_begin_elements(){
	title[0] = 0;
	ttw = title;
}

void ipc_end_elements(){
	ipc_write(title);
}

void ipc_write_element(attribute_s* el, bool_t next){
	if( el->blinkstatus && el->urgent ){
		size_t len = strlen(el->longformat);
		if( DWM_MAX_TITLE - (ttw - title) < (int)len ){
			len = DWM_MAX_TITLE - (ttw-title);
		}
		while( len-->0 ){
			*ttw++ = ' ';
		}
	}
	else{
		ttw = str_encpy(ttw, DWM_MAX_TITLE - (ttw - title), el->longformat);
	}	
	
	if( el->separator && next ){
		*ttw++ = '|';
		*ttw = 0;
	}
}

