#ifndef __VBAR_H__
#define __VBAR_H__

//TODO
//[0] vbarScript ninja not auto regenerate if edit baseScript, is not a bug?
//[0] :( input not works with bar
//[1] check bar moved is xorg_win_resize?
//[3] protect draw text offscreen
//[3] on config gadget_load can return NULL, is better return error gadget
//[4] gadget_unload
//[6] gadget separator
//[6] gadget script
//[6] gadget error
//[8] extend selector

#include <ef/type.h>
#include <ef/xorg.h>
#include <ef/image.h>
#include <ef/ft.h>
#include <ef/chash.h>
#include <ef/phq.h>
#include <ef/deadpoll.h>
#include <ef/delay.h>
#include <ef/socket.h>
#include <ef/memory.h>
#include <libtcc.h>

#define GADGET_TEXT_RESIZE 32
#define BAR_TB_SPACING_FACTOR 3

#define VBAR_BORDER_LEFT   0x01
#define VBAR_BORDER_TOP    0x02
#define VBAR_BORDER_RIGHT  0x04
#define VBAR_BORDER_BOTTOM 0x08

#define VBAR_CONFIG "~/.config/vbar/config.c"
#define VBAR_IPC "/tmp/vbar.ipc"

#define VBAR_EXTEND_INPUT_SIZE 32
#define VBAR_ERROR_DISPLAY_LOC 5
#define VBAR_ERROR_BK_R 20
#define VBAR_ERROR_BK_G 20
#define VBAR_ERROR_BK_B 20
#define VBAR_ERROR_FG_R 250
#define VBAR_ERROR_FG_G 250
#define VBAR_ERROR_FG_B 250
#define VBAR_ERROR_NAME "__vbar_error__"
#define VBAR_ERROR_BEGIN "vbar config error:\n"
#define VBAR_ERROR_OFFSET_X 10
#define VBAR_ERROR_OFFSET_Y 10
#define VBAR_ERROR_SCROLL_Y 3
#define VBAR_SIMPLE_FONT "mono"
#define VBAR_SIMPLE_FONT_SIZE 12
#define VBAR_SIMPLE_HEIGHT 50

#define VBAR_INTERNAL_MESSAGE_MAX 1024

typedef struct vbarMouse{
	unsigned x, y, button, extend, line, icon;
}vbarMouse_s;

typedef enum { 
	GADGET_EVENT_REFRESH, 
	GADGET_EVENT_EXTEND_OPEN,
	GADGET_EVENT_EXTEND_CLOSE,
	GADGET_EVENT_EXTEND_REFRESH,
	GADGET_EVENT_MOUSE_RELEASE, 
	GADGET_EVENT_MOUSE_PRESS, 
	GADGET_EVENT_MOUSE_MOVE, 
	GADGET_EVENT_MOUSE_ENTER, 
	GADGET_EVENT_MOUSE_LEAVE, 
	GADGET_EVENT_MOUSE_CLICK, 
	GADGET_EVENT_MOUSE_DBLCLICK
}gadgetEventType_e;

typedef enum { VBAR_ALIGNED_LEFT, VBAR_ALIGNED_CENTER, VBAR_ALIGNED_RIGHT } vbarAligned_e;
typedef enum { GADGET_NOREDRAW, GADGET_REDRAW_PARTIAL, GADGET_REDRAW, GADGET_FULL_REDRAW } gadgetRedraw_e;

typedef struct scrollImg{
	g2dImage_s img;
	g2dCoord_s scroll;
}scrollImg_s;

typedef struct bar{
	xorg_s x;
	xcb_window_t id;
	xorgSurface_s surface;
	xorgCallbackEvent_s callback;

	g2dColor_t background;
	g2dColor_t foreground;
	int height;
	unsigned extendH;
	int topSpacing;
	int bottomSpacing;
	int topBar;
	int borderSize;

	ftlib_h lib;
	ftFonts_s fonts;
	unsigned fontHeight;

	scrollImg_s errw;
}bar_s;

typedef struct vbar vbar_s;
typedef struct gadget gadget_s;

typedef int(*gadgetEvent_f)(gadget_s* g);
typedef void(*gadgetScriptEvent_f)(gadget_s* g, gadgetEventType_e ev, void* arg);

typedef enum { GADGET_LINE_SEPARATOR, GADGET_LINE_LABEL, GADGET_LINE_SELECTOR} gLine_e;
typedef struct gLine{
	gLine_e type;

	utf8_t* oldLabel;
	size_t oldLableSize;
	utf8_t* label;
	size_t labelLen;
	size_t labelSize;

	char** oldIconName;
	char** iconName;
	size_t iconCount;

	g2dColor_t background;
	g2dColor_t foreground;

	g2dColor_t borderColor;
	unsigned border;

	gadgetRedraw_e redraw;
}gLine_s;

typedef struct gadget{
	char* class;
	size_t lenClass;
	char* name;
	size_t lenName;
	char* instance;

	int hiden;

	utf8_t* oldLabel;
	size_t oldLableSize;
	utf8_t* label;
	size_t labelLen;
	size_t labelSize;

	g2dCoord_s position;
	vbarAligned_e align;

	char const* oldIcon;
	char const* iconName;
	g2dColor_t background;
	g2dColor_t foreground;
	gadgetRedraw_e redraw;

	g2dColor_t borderColor;
	unsigned border;

	long eventTimems;
	int status;

	gLine_s* extend;
	size_t extendCount;
	size_t extendHeight;

	phqElement_s selfE;
	vbar_s* vbar;
	
	void* data;
	gadgetEvent_f load;
	gadgetEvent_f ellapse;
	gadgetEvent_f free;
	gadgetScriptEvent_f event;

	struct gadget* next;
	struct gadget* prev;
}gadget_s;

typedef struct icon{
	char const* name;
	char const* path;
	g2dImage_s img;
}icon_s;

typedef struct xorgEventRegister{
	int event;
	xcb_atom_t atom;
	gadget_s* gadget;
	struct xorgEventRegister* next;
}xorgEventRegister_s;

typedef struct vbar{
	bar_s bar;
	deadpoll_s events;
	phq_s runner;
	chash_s coyote;
	chash_s list;
	chash_s icons;
	TCCState* tcc;
	char* code;
	gadget_s* drawed;
	gadget_s* active;
	gadget_s* extend;
	gadget_s* hover;
	xorgEventRegister_s* xer;
	socket_s ipc;
}vbar_s;

/*
 * IPC
 * raise class:name events
*/

typedef struct ipcproto{
	int type;
	char* instance;
	gadgetEventType_e event;
	gadget_s* g;
}ipcproto_s;

#define config_add_symbol(GS, TOK, ARG) tcc_add_symbol((GS)->tcc, TOK, ARG)
err_t vbar_script_load(vbar_s* vb, char const* sourcefile);
void vbar_change_ferr(const char* name);
const char* vbar_script_error(void);
err_t vbar_script_run(vbar_s* vb);
void vbar_register_symbol(vbar_s* vb);
void vbar_ipc_client_connect(socket_s* ipc);
void vbar_ipc_send(socket_s* ipc, int type, char* instance, gadgetEventType_e event);
void vbar_begin(vbar_s* vb);
//err_t vbar_internal_message_send(vbar_s* vb, gadget_s* g, int type, icbk_f cbk, char* data, size_t size);
void vbar_xorg_register_event(vbar_s* vb, gadget_s* g, int event, xcb_atom_t atom);
void vbar_register_gadget(vbar_s* vb);
void vbar_end(vbar_s* vb);
gadget_s* vbar_gadget_load(vbar_s* vb, char const* class, char const* name);
void vbar_gadget_refresh_all(vbar_s* vb);
void vbar_label_reset(gadget_s* g);
void vbar_extend_reset(gadget_s* g);
void vbar_loop(vbar_s* vb);
gadget_s* vbar_gadget_byposition(int* exline, int* icon, vbar_s* vb, unsigned x, unsigned y);
err_t bar_begin(vbar_s* bar);
void bar_simple_setting(vbar_s* vb);
void bar_register_symbol(vbar_s* vb);
err_t bar_start(vbar_s* vb);
void bar_terminate(vbar_s* vb);
void bar_show(vbar_s* vb);
err_t bar_error(vbar_s* vb, utf8_t* errd);
void bar_draw(vbar_s* vb);
void bar_draw_extend(gadget_s* g);
void bar_gadget_draw(vbar_s* vb, gadget_s* g, utf8_t* oldLabel);
void bar_extend_close(bar_s* b);

#endif
