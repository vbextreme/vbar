#ifndef __EF_XORG_H__
#define __EF_XORG_H__

#include <ef/type.h>
#include <ef/image.h>
#include <ef/utf8.h>
#include <ef/stack.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <xcb/composite.h>
#include <xcb/xcb_errors.h>
#include <xkbcommon/xkbcommon.h>

#define XKB_UTF_MAX 32

#define X_COLOR_MODE G2D_MODE_ARGB

#define X_WIN_EVENT XCB_EVENT_MASK_EXPOSURE |\
	XCB_EVENT_MASK_KEY_PRESS |\
	XCB_EVENT_MASK_KEY_RELEASE |\
	XCB_EVENT_MASK_BUTTON_PRESS |\
	XCB_EVENT_MASK_BUTTON_RELEASE |\
	XCB_EVENT_MASK_POINTER_MOTION |\
	XCB_EVENT_MASK_ENTER_WINDOW |\
	XCB_EVENT_MASK_LEAVE_WINDOW |\
	XCB_EVENT_MASK_VISIBILITY_CHANGE |\
	XCB_EVENT_MASK_STRUCTURE_NOTIFY |\
	XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |\
	XCB_EVENT_MASK_PROPERTY_CHANGE

#define XORG_MOUSE_CLICK_MS 200
#define XORG_MOUSE_DBLCLICL_MS 350

typedef enum {
	XORG_ATOM_NET_ACTIVE_WINDOW,
	XORG_ATOM_NET_NUMBER_OF_DESKTOPS,
	XORG_ATOM_NET_CURRENT_DESKTOP,
	XORG_ATOM_NET_DESKTOP_NAMES,
	XORG_ATOM_NET_ACTIVE_DESKTOP,
	XORG_ATOM_NET_WM_DESKTOP, 
	XORG_ATOM_NET_WM_WINDOW_TYPE, 
	XORG_ATOM_NET_WM_STATE, 
	XORG_ATOM_NET_WM_VISIBLE_NAME, 
	XORG_ATOM_NET_WM_NAME,
	XORG_ATOM_NET_WM_STRUT,
	XORG_ATOM_NET_WM_STRUT_PARTIAL,
	XORG_ATOM_NET_WM_PID,
	XORG_ATOM_NET_WM_WINDOW_TYPE_DOCK,
	XORG_ATOM_XROOTPMAP_ID,
	XORG_ATOM_UTF8_STRING,
	XORG_ATOM_COUNT
}xorgAtom_e;

typedef struct xkb{
	struct xkb_context* ctx;
	struct xkb_keymap* keymap;
	int device;
}xkb_s;

typedef struct xorgSurface{
	xcb_gcontext_t gc;
	xcb_image_t* ximage;
	g2dImage_s img;
}xorgSurface_s;

typedef struct monitor{
	char* name;
	bool_t connected;
	g2dCoord_s size;
}monitor_s;

typedef struct xorg{
	xcb_connection_t* connection;
	xcb_screen_t* screen;
	int screenDefault;
	int screenCurrent;
	monitor_s* monitor;
	size_t monitorCount;
	monitor_s* monitorCurrent;
	monitor_s* monitorPrimary;
	char* display;
	xkb_s key;
	xcb_errors_context_t* err;
	xorgAtom_e atom[XORG_ATOM_COUNT];
	long clickms;
	long dblclickms;
	long _mousetime;
	unsigned _mousestate;
}xorg_s;

#define XORG_WINDOW_HINTS_FLAGS_URGENCY(XWPTR)  ((XWPTR)->hints.flags & XCB_ICCCM_WM_HINT_X_URGENCY)
#define XORG_WINDOW_VISIBLE_UNMAP  XCB_MAP_STATE_UNMAPPED
#define XORG_WINDOW_VISIBLE_UNVIEW XCB_MAP_STATE_UNVIEWABLE
#define XORG_WINDOW_VISIBLE_MAP    XCB_MAP_STATE_VIEWABLE

struct xorgWindowStrut{
	unsigned left, right, top, bottom;
} __packed;
typedef struct xorgWindowStrut xorgWindowStrut_s;

struct xorgWindowStrutPartial{
	unsigned left, right, top, bottom;
	unsigned left_start_y, left_end_y;
	unsigned right_start_y, right_end_y;
	unsigned top_start_x, top_end_x;
	unsigned bottom_start_x,bottom_end_x;
} __packed;
typedef struct xorgWindowStrutPartial xorgWindowStrutPartial_s;

typedef struct xorgWindow{
	xcb_window_t idxcb;
	char* class;
	char* name;
	char* netname;
	char* title;
	xcb_icccm_wm_hints_t hints;
	xcb_size_hints_t size;
	unsigned x, y, w, h, border, visible;
	unsigned desktop;
	xcb_atom_t* type;
	size_t typeCount;
	xcb_atom_t* state;
	size_t stateCount;
	xorgWindowStrut_s strut;
	xorgWindowStrutPartial_s partial;
	pid_t pid;
}xorgWindow_s;

typedef enum {XORG_MOUSE_RELEASE, XORG_MOUSE_PRESS, XORG_MOUSE_MOVE, XORG_MOUSE_ENTER, XORG_MOUSE_LEAVE, XORG_MOUSE_CLICK, XORG_MOUSE_DBLCLICK} xorgMouse_e;
typedef enum {XORG_KEY_RELEASE, XORG_KEY_PRESS} xorgKey_e;

typedef struct xorgMouse{
	xorg_s* x;
	void* user;
	xorgMouse_e event;
	g2dPoint_s absolute;
	g2dPoint_s relative;
	unsigned button;
	unsigned key;
	long time;
}xorgMouse_s;

typedef struct xorgKeyboard{
	xorg_s* x;
	void* user;
	xorgKey_e event;
	g2dPoint_s absolute;
	g2dPoint_s relative;
	unsigned button;
	unsigned long keycode;
	unsigned long keysym;
	uint8_t utf8[XKB_UTF_MAX];
	long time;
}xorgKeyboard_s;

typedef struct xorgMove{
	xorg_s* x;
	void* user;
	g2dCoord_s coord;
	unsigned border;
}xorgMove_s;

typedef void(*xorgGeneric_f)(xorg_s*, void* user);
typedef void(*xorgCoord_f)(xorg_s*, void* user, g2dCoord_s*);
typedef void(*xorgValue_f)(xorg_s* x, void* user, int value);
typedef void(*xorgMessage_f)(xorg_s* x, void* user, int format, int atom, uint8_t* data);
typedef void(*xorgMouse_f)(xorgMouse_s*);
typedef void(*xorgKeyboard_f)(xorgKeyboard_s*);
typedef void(*xorgMove_f)(xorgMove_s*);

typedef struct xorgCallbackEvent{
	xorgGeneric_f creat;
	xorgGeneric_f terminate;
	xorgCoord_f redraw;
	xorgGeneric_f paint;
	xorgGeneric_f destroy;
	xorgGeneric_f loop;
	xorgMouse_f mouse;
	xorgKeyboard_f keyboard;
	xorgValue_f focus;
	xorgValue_f visibility;
	xorgValue_f show;
	xorgMove_f move;
	xorgValue_f atom;
	xorgMessage_f message;
	void* user;
}xorgCallbackEvent_s;

#define xorg_root(XORG) ((XORG)->screen->root)
#define xorg_root_x(XORG) ((XORG)->monitorCurrent->size.x)
#define xorg_root_y(XORG) ((XORG)->monitorCurrent->size.y)
#define xorg_root_width(XORG) ((XORG)->monitorCurrent->size.w)
#define xorg_root_height(XORG) ((XORG)->monitorCurrent->size.h)
#define xorg_root_visual(XORG) ((XORG)->screen->root_visual)
#define xorg_fd(XORG) xcb_get_file_descriptor((XORG)->connection)

err_t xorg_client_init(xorg_s* x);
void xorg_client_terminate(xorg_s* x);
err_t xorg_root_init(xorg_s* x, int onscreen);
void xorg_client_flush(xorg_s* x);
void xorg_client_sync(xorg_s* x);
const char* xorg_error_major(xorg_s* x, xcb_generic_error_t* err);
const char* xorg_error_minor(xorg_s* x, xcb_generic_error_t* err);
const char* xorg_error_string(xorg_s* x, xcb_generic_error_t* err, const char** extensionname);
xcb_screen_t* xorg_screen_get(xorg_s* x, int idScreen);
void xorg_randr_monitor_refresh(xorg_s* x);
err_t xorg_monitor_byname(xorg_s* x, char const* name);
err_t xorg_monitor_bysize(xorg_s* x, g2dCoord_s* size);
err_t xorg_monitor_primary(xorg_s* x);
const char* xorg_atom_name(xorg_s* x, xcb_atom_t atom);
xcb_atom_t xorg_atom_id(xorg_s* x, const char* name);
xcb_atom_t xorg_atom_new_id(xorg_s* x, const char* name);
void xorg_atom_load(xorg_s* x);
int xorg_xcb_property_cardinal(xorg_s* x, xcb_get_property_cookie_t cookie);
xcb_get_property_cookie_t xorg_xcb_property_cookie_string(xorg_s* x, xcb_window_t win, xcb_atom_t atom);
char* xorg_xcb_property_string(xorg_s* x, xcb_get_property_cookie_t cookie);
err_t xorg_xcb_property_structure(void* out, xorg_s* x, xcb_get_property_cookie_t cookie, xcb_atom_t type, size_t size, size_t minsize);
xcb_pixmap_t xorg_xcb_property_pixmap(xorg_s* x, xcb_get_property_cookie_t cookie);
int xorg_xcb_attribute(xorg_s* x, xcb_get_window_attributes_cookie_t cookie);
err_t xorg_xcb_geometry(xorg_s* x, xcb_get_geometry_cookie_t cookie, unsigned* X, unsigned* Y, unsigned* W, unsigned* H, unsigned* B);
void xorg_send_creat(xorg_s* x, xcb_window_t parent, xcb_window_t win, int px, int py, int w, int h);
void xorg_send_destroy(xorg_s* x, xcb_window_t win);
void xorg_send_expose(xorg_s* x, xcb_window_t win, int px, int py, int w, int h);
void xorg_send_key_press(xorg_s* x, xcb_window_t win, xcb_keycode_t keycode, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);
void xorg_send_key_release(xorg_s* x, xcb_window_t win, xcb_keycode_t keycode, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);
void xorg_send_button_press(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);
void xorg_send_button_release(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);
void xorg_send_motion(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);
void xorg_send_enter(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);
void xorg_send_leave(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen);
void xorg_send_focus_in(xorg_s* x, xcb_window_t win);
void xorg_send_focus_out(xorg_s* x, xcb_window_t win);
void xorg_send_map(xorg_s* x, xcb_window_t win);
void xorg_send_unmap(xorg_s* x, xcb_window_t win);
void xorg_send_configure(xorg_s* x, xcb_window_t win, int px, int py, int w, int h, int border);
void xorg_send_property(xorg_s* x, xcb_window_t win, xcb_atom_t atom);
void xorg_send_client(xorg_s* x, xcb_window_t win, uint8_t type, xcb_atom_t atom, uint8_t* data, size_t len);
void xorg_send_client32(xorg_s* x, xcb_window_t win, xcb_window_t dest, xcb_atom_t atom, const uint32_t* data, size_t len);
void xorg_send_active_window(xorg_s* x, xcb_window_t current, xcb_window_t activate);
void xorg_send_current_desktop(xorg_s* x, uint32_t desktop);
void xorg_send_set_desktop(xorg_s* x, xcb_window_t win, uint32_t desktop);
void xorg_window_release(xorgWindow_s* win);
xorgWindow_s* xorg_query_tree(size_t* count, xorg_s* x, xcb_window_t root);
xorgWindow_s* xorg_window_application(xorg_s* x,  size_t nworkspace, xcb_window_t id, xorgWindow_s* stack, size_t* appCount);
unsigned xorg_workspace_count(xorg_s* x);
unsigned xorg_workspace_active(xorg_s* x);
char** xorg_workspace_name_get(xorg_s* x);
uint8_t* xorg_ximage_get_composite(unsigned* outW, unsigned* outH, unsigned* outV, unsigned* outD, xorg_s* x, xcb_window_t id);
xcb_pixmap_t xorg_root_pixmap_get(xorg_s* x);
uint8_t* xorg_ximage_root_get(unsigned* outW, unsigned* outH, unsigned* outV, unsigned* outD, xorg_s* x);
err_t xorg_image_grab(g2dImage_s* dst, xorg_s* x, xcb_window_t id);
err_t xorg_root_image_grab(g2dImage_s* dst, xorg_s* x);
void xorg_win_title(xorg_s* x, xcb_window_t id, char const* name);
void xorg_win_class(xorg_s* x, xcb_window_t id, char const* name);
void xorg_win_show(xorg_s* x, xcb_window_t id, int show);
void xorg_win_move(xorg_s* x, xcb_window_t id, unsigned X, unsigned y);
void xorg_win_resize(xorg_s* x, xcb_window_t id, unsigned w, unsigned h);
void xorg_win_coord(xorg_s* x, xcb_window_t id, g2dCoord_s* pos);
void xorg_win_size(g2dCoord_s* out, unsigned* outBorder, xorg_s* x, xcb_window_t idxcb);
void xorg_win_surface_redraw(xorg_s* x, xcb_window_t id,  xorgSurface_s* surface);
void xorg_win_dock(xorg_s* x, xcb_window_t id);
void xorg_wm_reserve_dock_space_on_top(xorg_s* x, xcb_window_t id, unsigned X, unsigned w, unsigned h);
void xorg_wm_reserve_dock_space_on_bottom(xorg_s* x, xcb_window_t id, unsigned X, unsigned w, unsigned h);
void xorg_register_events(xorg_s* x, xcb_window_t window, unsigned int eventmask);
xcb_window_t xorg_win_new(xorgSurface_s* surface, xorg_s* x, xcb_window_t parent, g2dCoord_s* pos, unsigned border, g2dColor_t background);
void xorg_surface_resize(xorgSurface_s* surface, unsigned w, unsigned h);
void xorg_surface_resize_bitblt(xorgSurface_s* surface, unsigned w, unsigned h);
void xorg_surface_destroy(xorg_s* x, xorgSurface_s* surface);
void xorg_win_destroy(xorg_s* x, xcb_window_t id);
void xorg_win_focus(xorg_s* x, xcb_window_t id);
err_t xorg_win_event(xorg_s* x, xorgCallbackEvent_s* callback, int async);



#endif 
