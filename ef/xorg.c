#include <ef/xorg.h>
#include <ef/memory.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <xcb/randr.h>

#ifdef XCB_ERROR_ENABLE
	#define XCB_ERR_DEC xcb_generic_error_t* err
	#define XCB_ERR_VAR &err
	#define XCB_ERR_FREE free(err)
#else
	#define XCB_ERR_VAR NULL
	#define XCB_ERR_DEC do{}while(0)
	#define XCB_ERR_FREE do{}while(0)
#endif

err_t xorg_client_init(xorg_s* x){
	x->connection = xcb_connect(x->display,&x->screenDefault);
	if( x->connection == NULL ){
		dbg_error("on xcb connect");
		return -1;
	}
	if( xkb_x11_setup_xkb_extension(x->connection, XKB_X11_MIN_MAJOR_XKB_VERSION, XKB_X11_MIN_MINOR_XKB_VERSION, 0, NULL, NULL, NULL, NULL) != 1){
        xcb_disconnect(x->connection);
		dbg_fail("xkb enable extension");
		return -1;
	}
	if( (x->key.ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS)) == NULL ){
		xcb_disconnect(x->connection);
		dbg_fail("xkb create context");
		return -1;
	}
	if( (x->key.device = xkb_x11_get_core_keyboard_device_id(x->connection)) == -1 ){
		xcb_disconnect(x->connection);
		dbg_fail("xkb get device keyboard");
		return -1;
	}
	if( !(x->key.keymap = xkb_x11_keymap_new_from_device(x->key.ctx, x->connection, x->key.device, XKB_KEYMAP_COMPILE_NO_FLAGS)) ){
		xcb_disconnect(x->connection);
		dbg_fail("xkb get device keyboard");
		return -1;
	}
	
#ifdef XCB_ERROR_ENABLE
	x->err = NULL;
	if( xcb_errors_context_new(x->connection, &x->err) ){
		xcb_disconnect(x->connection);
		dbg_fail("xcb util errors");
	}
#endif

	xorg_atom_load(x);

	xorg_root_init(x, -1);
	xorg_randr_monitor_refresh(x);
	xorg_monitor_primary(x);

	x->clickms = XORG_MOUSE_CLICK_MS;
	x->dblclickms = XORG_MOUSE_DBLCLICL_MS;
	x->_mousetime = 0;
	x->_mousestate = 0;
	return 0;
}

void xorg_client_terminate(xorg_s* x){
	xkb_keymap_unref(x->key.keymap);
	xkb_context_unref(x->key.ctx);
#ifdef XCB_ERROR_ENABLE
	xcb_errors_context_free(x->err);
#endif
	if( x->connection ) xcb_disconnect(x->connection);
	x->connection = NULL;
	for(size_t i = 0; i < x->monitorCount; ++i){
		free(x->monitor[i].name);
	}
	free(x->monitor);
}

err_t xorg_root_init(xorg_s* x, int onscreen){
	if( onscreen < 0 ) onscreen = x->screenDefault;
	x->screen = xorg_screen_get(x, onscreen);
	if( x->screen == NULL ){
		dbg_error("fail get screen");
		return -1;
	}
	x->screenCurrent = onscreen;
	return 0;
}

void xorg_client_flush(xorg_s* x){
	xcb_flush(x->connection);
}

void xorg_client_sync(xorg_s* x){
	xcb_aux_sync(x->connection);
}

#ifdef XCB_ERROR_ENABLE
const char* xorg_error_major(xorg_s* x, xcb_generic_error_t* err){
	return xcb_errors_get_name_for_major_code(x->err, err->major_code);
}

const char* xorg_error_minor(xorg_s* x, xcb_generic_error_t* err){
	return xcb_errors_get_name_for_minor_code(x->err, err->major_code, err->minor_code);
}

const char* xorg_error_string(xorg_s* x, xcb_generic_error_t* err, const char** extensionname){
	return xcb_errors_get_name_for_error(x->err, err->error_code, extensionname);
}
#endif

xcb_screen_t* xorg_screen_get(xorg_s* x, int idScreen){
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(x->connection));
	for (; iter.rem && idScreen; --idScreen, xcb_screen_next(&iter));
	return iter.data;
}

void xorg_randr_monitor_refresh(xorg_s* x){
	xcb_randr_get_screen_resources_current_reply_t* rres = xcb_randr_get_screen_resources_current_reply( x->connection,
        xcb_randr_get_screen_resources_current(x->connection, xorg_root(x)),
        NULL
    );

    if( !rres ){
        dbg_error("Failed to get current randr screen resources\n");
        return;
    }

    int num = xcb_randr_get_screen_resources_current_outputs_length(rres);
    xcb_randr_output_t* outputs = xcb_randr_get_screen_resources_current_outputs(rres);

    if (num < 1) {
		dbg_error("No outputs");
        free(rres);
        return;
    }
	x->monitorCount = num;
	x->monitor = mem_zero_many(monitor_s, x->monitorCount);
	iassert(x->monitor);

    for( int i = 0; i < num; i++ ){
		xcb_randr_get_output_info_reply_t *routi = xcb_randr_get_output_info_reply(
            x->connection,
            xcb_randr_get_output_info(x->connection, outputs[i], XCB_CURRENT_TIME),
            NULL
        );

        if( !routi ){       
			free(routi);
			dbg_warning("output");
            continue;
        }
		
		x->monitor[i].connected = routi->connection == XCB_RANDR_CONNECTION_CONNECTED;

        int namelen;
        uint8_t *str;
        namelen = xcb_randr_get_output_info_name_length(routi);
        str = xcb_randr_get_output_info_name(routi);
		x->monitor[i].name = mem_many(char, namelen+1);
		iassert(x->monitor[i].name);
		memcpy(x->monitor[i].name, str, namelen);
		x->monitor[i].name[namelen] = 0;

		if( x->monitor[i].connected ){
			xcb_randr_get_crtc_info_reply_t *crtc = xcb_randr_get_crtc_info_reply(x->connection,
					xcb_randr_get_crtc_info(x->connection, routi->crtc, time(NULL)), NULL);
			x->monitor[i].size.x = crtc->x;
		   	x->monitor[i].size.y = crtc->y;
		   	x->monitor[i].size.w = crtc->width;
		   	x->monitor[i].size.h = crtc->height;
			free(crtc);

		}
        free(routi);
	}
    free(rres);
}

err_t xorg_monitor_byname(xorg_s* x, char const* name){
	for( size_t i = 0; i < x->monitorCount; ++i ){
		if( !strcmp(x->monitor[i].name, name) ){
			x->monitorCurrent = &x->monitor[i];
			return 0;
		}
	}
	return -1;
}

err_t xorg_monitor_bysize(xorg_s* x, g2dCoord_s* size){
	for( size_t i = 0; i < x->monitorCount; ++i ){
		if( x->monitor[i].size.x == size->x && x->monitor[i].size.y == size->y && x->monitor[i].size.w == size->w && x->monitor[i].size.h == size->h ){
			x->monitorCurrent = &x->monitor[i];
			return 0;
		}
	}
	return -1;
}

err_t xorg_monitor_primary(xorg_s* x){
	XCB_ERR_DEC;
	xcb_randr_get_monitors_reply_t* monitors = xcb_randr_get_monitors_reply(x->connection,
			xcb_randr_get_monitors(x->connection, xorg_root(x),0), XCB_ERR_VAR);
	dbg_info("");
	if( !monitors ){
#ifdef XCB_ERROR_ENABLE
		if( err ){
			dbg_error("monitor %d %s.%s::%s", err->error_code, xorg_error_major(x, err), xorg_error_minor(x, err), xorg_error_string(x, err, NULL));
			free(err);
		}
#endif
		dbg_error("monitor");
	   	return -1;
	}
	xcb_randr_monitor_info_iterator_t it = xcb_randr_get_monitors_monitors_iterator(monitors);
	dbg_info("nmonitors %u", monitors->nMonitors);

	for( size_t i = 0; i < monitors->nMonitors; ++i ){
		xcb_randr_monitor_info_t* monitor = it.data;
		if( monitor->primary ){
			g2dCoord_s pos = {.x = monitor->x, .y = monitor->y, .w = monitor->width, .h = monitor->height };
			if( xorg_monitor_bysize(x, &pos) ){
				free(monitors);
				dbg_error("internal priary research");
				return -1;
			}
			x->monitorPrimary = x->monitorCurrent;
			break;
		}
	 	xcb_randr_monitor_info_next(&it);
	}

	free(monitors);
	return 0;
}

const char* xorg_atom_name(xorg_s* x, xcb_atom_t atom){
    static char buf[1024];

    if (atom == 0)
        return NULL;

    xcb_generic_error_t *error = NULL;
    xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(x->connection, atom);
    xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(x->connection, cookie, &error);
    if(!reply || error){
		free(error);
		free(reply);
        return NULL;
	}
    
	int len = xcb_get_atom_name_name_length(reply);
    char* name = xcb_get_atom_name_name(reply);
	if( len > 1022 ){
		dbg_fail("buffer overflow");
	}
	sprintf(buf,"%.*s", len, name);
    return buf;
}

xcb_atom_t xorg_atom_id(xorg_s* x, const char* name){
	xcb_intern_atom_cookie_t cookie = xcb_intern_atom(x->connection, 1, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(x->connection, cookie, NULL);
	if( !reply ){
		return 0;
	}
	xcb_atom_t ret = reply->atom;
	free(reply);
	return ret;
}

xcb_atom_t xorg_atom_new_id(xorg_s* x, const char* name){
	xcb_intern_atom_cookie_t cookie = xcb_intern_atom(x->connection, 0, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(x->connection, cookie, NULL);
	if( !reply ){
		return 0;
	}
	xcb_atom_t ret = reply->atom;
	free(reply);
	return ret;
}

void xorg_atom_load(xorg_s* x){
	char* atomName[] = {
		[XORG_ATOM_NET_ACTIVE_WINDOW] = "_NET_ACTIVE_WINDOW",
		[XORG_ATOM_NET_NUMBER_OF_DESKTOPS] = "_NET_NUMBER_OF_DESKTOPS",
		[XORG_ATOM_NET_CURRENT_DESKTOP] = "_NET_CURRENT_DESKTOP",
		[XORG_ATOM_NET_DESKTOP_NAMES] = "_NET_DESKTOP_NAMES",
		[XORG_ATOM_NET_ACTIVE_DESKTOP] = "_NET_ACTIVE_DESKTOP",
		[XORG_ATOM_NET_WM_DESKTOP] = "_NET_WM_DESKTOP",
		[XORG_ATOM_NET_WM_WINDOW_TYPE] = "_NET_WM_WINDOW_TYPE",
		[XORG_ATOM_NET_WM_STATE] = "_NET_WM_STATE",
		[XORG_ATOM_NET_WM_VISIBLE_NAME] = "_NET_WM_VISIBLE_NAME",
		[XORG_ATOM_NET_WM_NAME] = "_NET_WM_NAME",
		[XORG_ATOM_NET_WM_STRUT] = "_NET_WM_STRUT",
		[XORG_ATOM_NET_WM_STRUT_PARTIAL] = "_NET_WM_STRUT_PARTIAL",
		[XORG_ATOM_NET_WM_PID] = "_NET_WM_PID",
		[XORG_ATOM_NET_WM_WINDOW_TYPE_DOCK] = "_NET_WM_WINDOW_TYPE_DOCK",
		[XORG_ATOM_XROOTPMAP_ID] = "_XROOTPMAP_ID",
		[XORG_ATOM_UTF8_STRING] = "UTF8_STRING"
	};

	xcb_intern_atom_cookie_t cookie[XORG_ATOM_COUNT];

	for( unsigned i = 0; i < XORG_ATOM_COUNT; ++i ){
		cookie[i] = xcb_intern_atom(x->connection, 1, strlen(atomName[i]), atomName[i]);
	}
	
	for( unsigned i = 0; i < XORG_ATOM_COUNT; ++i ){
	XCB_ERR_DEC;
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(x->connection, cookie[i], XCB_ERR_VAR);
		if( !reply ){
			dbg_error("xorg atom(%s) error", atomName[i]);
#ifdef XCB_ERROR_ENABLE
			if( err ){
				dbg_error("atom reply %d %s.%s::%s", err->error_code, xorg_error_major(x, err), xorg_error_minor(x, err), xorg_error_string(x, err, NULL));
				free(err);
			}
#else
			dbg_error("atom reply");
#endif
			x->atom[i] = 0;
		}
		else{
			x->atom[i] = reply->atom;
			free(reply);
		}
	}
}

int xorg_xcb_attribute(xorg_s* x, xcb_get_window_attributes_cookie_t cookie){
	xcb_generic_error_t* err = NULL;
	xcb_get_window_attributes_reply_t* reply = xcb_get_window_attributes_reply(x->connection, cookie, &err);
	int ret = -1;
	
	if( err ){
		dbg_error("xcb code: %d", err->error_code);
		goto ONEND;
	}
	if( !reply ){
		dbg_warning("xcb not reply");
		goto ONEND;
	}
	ret = reply->map_state;
ONEND:
	free(err);
	free(reply);
	return ret;
}

err_t xorg_xcb_geometry(xorg_s* x, xcb_get_geometry_cookie_t cookie, unsigned* X, unsigned* Y, unsigned* W, unsigned* H, unsigned* B){
	XCB_ERR_DEC;
	xcb_get_geometry_reply_t* reply = xcb_get_geometry_reply(x->connection, cookie, XCB_ERR_VAR);;
	err_t ret = -1;
	
#ifdef XCB_ERROR_ENABLE
	if( err ){
		dbg_error("xcb code: %d", err->error_code);
		goto ONEND;
	}
#endif
	if( !reply ){
		dbg_warning("xcb not reply");
		goto ONEND;
	}

	*X = reply->x;
	*Y = reply->y;
	*W = reply->width;
	*H = reply->height;
	*B = reply->border_width;
	ret = 0;
ONEND:
	XCB_ERR_FREE;
	free(reply);
	return ret;
}

inline __private err_t xorg_check_property_reply(xcb_get_property_reply_t* reply, unsigned type){
	if( !reply ){
		//dbg_warning("xcb not reply");
		return -1;
	}
	if( reply->type != type ){
		//dbg_warning("xcb type reply %u != %u", reply->type != type);
		return -1;
	}
	if( reply->format != 32 ){
		//dbg_warning("xcb format");
		return -1;
	}
	return 0;
}

int xorg_xcb_property_cardinal(xorg_s* x, xcb_get_property_cookie_t cookie){
	XCB_ERR_DEC;
	xcb_get_property_reply_t* reply = xcb_get_property_reply(x->connection, cookie, XCB_ERR_VAR);
	int ret = -1;

#ifdef XCB_ERROR_ENABLE	
	if( err ){
		dbg_error("xcb code: %d", err->error_code);
		goto ONEND;
	}
#endif
	if( xorg_check_property_reply(reply, XCB_ATOM_CARDINAL) ){
		goto ONEND;
	}
	if( xcb_get_property_value_length(reply) != sizeof(int) ){
		//dbg_warning("xcb length");
		goto ONEND;
	}
	int* val = (int*)xcb_get_property_value(reply);
	if( !val ){
		dbg_error("xcb have a big numbers of error fuck");
		goto ONEND;
	}
	ret = *val;
	
ONEND:
	XCB_ERR_FREE;
	free(reply);
	return ret;
}

xcb_get_property_cookie_t xorg_xcb_property_cookie_string(xorg_s* x, xcb_window_t win, xcb_atom_t atom){
	return xcb_get_property(x->connection, 0, win, atom, XCB_ATOM_STRING, 0, 4096L);
}

char* xorg_xcb_property_string(xorg_s* x, xcb_get_property_cookie_t cookie){
	XCB_ERR_DEC;
	xcb_get_property_reply_t* reply = xcb_get_property_reply(x->connection, cookie, XCB_ERR_VAR);
	char* dst = NULL;
#ifdef XCB_ERROR_ENABLE
	if( err ){
		dbg_error("xcb code: %d", err->error_code);
		goto ONEND;
	}
#endif
	if( !reply ){
		dbg_warning("xcb not reply");
		goto ONEND;
	}
	int len = xcb_get_property_value_length(reply);
	if( len <= 0 ){
		dbg_warning("xcb return 0 length");
		goto ONEND;
	}

	dst = mem_many(char, len+1);
	sprintf(dst,"%.*s", len, (char*)xcb_get_property_value(reply));
ONEND:
	XCB_ERR_FREE;
	free(reply);
	return dst;
}

char** xorg_xcb_property_string_list(xorg_s* x, xcb_get_property_cookie_t cookie){
	XCB_ERR_DEC;
	xcb_get_property_reply_t* reply = xcb_get_property_reply(x->connection, cookie, XCB_ERR_VAR);
	char** dst = NULL;
#ifdef XCB_ERROR_ENABLE
	if( err ){
		dbg_error("xcb code: %d", err->error_code);
		goto ONEND;
	}
#endif
	if( !reply ){
		dbg_warning("xcb not reply");
		goto ONEND;
	}
	int len = xcb_get_property_value_length(reply);
	if( len <= 0 ){
		dbg_warning("xcb return 0 length");
		//goto ONEND;
	}
	
	char* src = (char*)xcb_get_property_value(reply);
	//count matrix
	size_t count = 0;
	for( int i = 0; i < len; ++i){
		if( src[i] == 0 ) ++count;
	}
	dbg_info("count matrix %lu",count);

	dst = mem_many(char*, count+1);
	dst[count] = NULL;
	for( size_t i = 0; i < count; ++i){
		size_t ls = strlen(src);
		dst[i] = mem_many(char, ls+1);
		strcpy(dst[i], src);
		src+=ls+1;
	}
ONEND:
	XCB_ERR_FREE;
	free(reply);
	return dst;
}

err_t xorg_xcb_property_structure(void* out, xorg_s* x, xcb_get_property_cookie_t cookie, xcb_atom_t type, size_t size, size_t minsize){
	XCB_ERR_DEC;
	xcb_get_property_reply_t* reply = xcb_get_property_reply(x->connection, cookie, XCB_ERR_VAR);
	int ret = -1;
#ifdef XCB_ERROR_ENABLE
	if( err ){
		dbg_error("xcb (%d) %s.%s::%s", err->error_code, xorg_error_major(x, err), xorg_error_minor(x, err), xorg_error_string(x, err, NULL));
		goto ONEND;
	}
#endif
	if( xorg_check_property_reply(reply, type) ){
		goto ONEND;
	}
	
	size_t rsize = xcb_get_property_value_length(reply);
	if( rsize < minsize ){
		goto ONEND;
	}
	if( rsize > size ) rsize = size;
	void* val = xcb_get_property_value(reply);
	if( !val ){
		dbg_error("xcb have a big numbers of error fuck");
		goto ONEND;
	}
	memcpy(out, val, rsize);
	ret = 0;
	
ONEND:
	XCB_ERR_FREE;
	free(reply);
	return ret;
}

xcb_atom_t* xorg_xcb_property_atom_list(size_t* len, xorg_s* x, xcb_get_property_cookie_t cookie){
	XCB_ERR_DEC;
	xcb_get_property_reply_t* reply = xcb_get_property_reply(x->connection, cookie, XCB_ERR_VAR);
	xcb_atom_t* dst = NULL;
#ifdef XCB_ERROR_ENABLE
	if( err ){
		dbg_error("xcb code: %d", err->error_code);
		goto ONEND;
	}
#endif
	if( xorg_check_property_reply(reply, XCB_ATOM_ATOM) ){
		goto ONEND;
	}
	
	*len = xcb_get_property_value_length(reply) / sizeof(xcb_atom_t);
	if( *len ){
		dst = mem_many(xcb_atom_t, *len);
		iassert(dst);
		xcb_atom_t* at = (xcb_atom_t*)xcb_get_property_value(reply);
		for(unsigned z = 0; z < *len; ++z){
			dst[z] = at[z];
		}
	}

ONEND:
	XCB_ERR_FREE;
	free(reply);
	return dst;
}

xcb_pixmap_t xorg_xcb_property_pixmap(xorg_s* x, xcb_get_property_cookie_t cookie){
	XCB_ERR_DEC;
	xcb_get_property_reply_t* reply = xcb_get_property_reply(x->connection, cookie, XCB_ERR_VAR);
	xcb_pixmap_t ret = XCB_NONE;
#ifdef XCB_ERROR_ENABLE
	if( err ){
		dbg_error("xcb code: %d", err->error_code);
		goto ONEND;
	}
#endif
	if( xorg_check_property_reply(reply, XCB_ATOM_PIXMAP) ){
		dbg_error("check");
		goto ONEND;
	}
	if( xcb_get_property_value_length(reply) != sizeof(xcb_pixmap_t) ){
		dbg_warning("lenght pixmap");
		goto ONEND;
	}
	xcb_pixmap_t* val = (xcb_pixmap_t*)xcb_get_property_value(reply);
	if( !val ){
		dbg_error("xcb have a big numbers of error fuck");
		goto ONEND;
	}
	ret = *val;
	
ONEND:
	XCB_ERR_FREE;
	free(reply);
	return ret;
}

void xorg_send_creat(xorg_s* x, xcb_window_t parent, xcb_window_t win, int px, int py, int w, int h){
	xcb_create_notify_event_t* event = calloc(32,1);
	event->response_type = XCB_CREATE_NOTIFY;
	event->parent = parent;
	event->window = win;
	event->override_redirect = 0;
	event->x = px;
	event->y = py;
	event->width = w;
	event->height = h;
	xcb_send_event(x->connection, false, win, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (char*)event);
	xorg_client_flush(x);
	free(event);
}

void xorg_send_destroy(xorg_s* x, xcb_window_t win){
	xcb_destroy_notify_event_t* event = calloc(32,1);
	event->response_type = XCB_DESTROY_NOTIFY;
	event->window = win;
	xcb_send_event(x->connection, false, win, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (char*)event);
	xorg_client_flush(x);
	free(event);
}

void xorg_send_expose(xorg_s* x, xcb_window_t win, int px, int py, int w, int h){
	xcb_expose_event_t* event = calloc(32,1);
	event->response_type = XCB_EXPOSE;
	event->window = win;
	event->x = px;
	event->y = py;
	event->width = w;
	event->height = h;
	xcb_send_event(x->connection, false, win, XCB_EVENT_MASK_EXPOSURE, (char*)event);
	xorg_client_flush(x);
	free(event);
}

void xorg_send_key_press(xorg_s* x, xcb_window_t win, xcb_keycode_t keycode, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen){
	xcb_key_press_event_t* event = calloc(32,1);
	event->response_type = XCB_KEY_PRESS;
	event->root = event->event = event->child = win;
	event->detail = keycode;
	event->root_x = rx;
	event->root_y = ry;
	event->event_x = px;
	event->event_y = py;
	event->time = time;
	event->state = state;
	event->same_screen = samescreen;
	xcb_send_event(x->connection, false, win, XCB_EVENT_MASK_KEY_PRESS, (char*)event);
	xorg_client_flush(x);
	free(event);
}

void xorg_send_key_release(xorg_s* x, xcb_window_t win, xcb_keycode_t keycode, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen){
	xcb_key_release_event_t* event = calloc(32,1);
	event->response_type = XCB_KEY_RELEASE;
	event->root = event->event = event->child = win;
	event->detail = keycode;
	event->root_x = rx;
	event->root_y = ry;
	event->event_x = px;
	event->event_y = py;
	event->time = time;
	event->state = state;
	event->same_screen = samescreen;
	xcb_send_event(x->connection, false, win, XCB_EVENT_MASK_KEY_RELEASE, (char*)event);
	xorg_client_flush(x);
	free(event);
}

inline __private void xorg_send_mouse(xorg_s* x, xcb_window_t win, uint8_t type, int mask, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen){
	xcb_button_press_event_t* event = calloc(32,1);
	event->response_type = type;
	event->root = event->event = event->child = win;
	event->detail = button;
	event->root_x = rx;
	event->root_y = ry;
	event->event_x = px;
	event->event_y = py;
	event->time = time;
	event->state = state;
	event->same_screen = samescreen;
	xcb_send_event(x->connection, false, win, mask, (char*)event);
	xorg_client_flush(x);
	free(event);
}

void xorg_send_button_press(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen){
	xorg_send_mouse(x, win, XCB_BUTTON_PRESS, XCB_EVENT_MASK_BUTTON_PRESS, button, time, rx, ry, px, py, state, samescreen);
}

void xorg_send_button_release(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen){
	xorg_send_mouse(x, win, XCB_BUTTON_RELEASE, XCB_EVENT_MASK_BUTTON_RELEASE, button, time, rx, ry, px, py, state, samescreen);
}

void xorg_send_motion(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen){
	xorg_send_mouse(x, win, XCB_MOTION_NOTIFY, XCB_EVENT_MASK_POINTER_MOTION, button, time, rx, ry, px, py, state, samescreen);
}

void xorg_send_enter(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen){
	xorg_send_mouse(x, win, XCB_ENTER_NOTIFY, XCB_EVENT_MASK_POINTER_MOTION, button, time, rx, ry, px, py, state, samescreen);
}

void xorg_send_leave(xorg_s* x, xcb_window_t win, xcb_button_t button, xcb_timestamp_t time, int rx, int ry, int px, int py, int state, int samescreen){
	xorg_send_mouse(x, win, XCB_LEAVE_NOTIFY, XCB_EVENT_MASK_POINTER_MOTION, button, time, rx, ry, px, py, state, samescreen);
}

void xorg_send_focus_in(xorg_s* x, xcb_window_t win){
	xcb_focus_in_event_t* event = calloc(32,1);
	event->response_type = XCB_FOCUS_IN;
	event->event = win;
	xcb_send_event(x->connection, false, win, XCB_EVENT_MASK_ENTER_WINDOW, (char*)event);
	xorg_client_flush(x);
	free(event);
}

void xorg_send_focus_out(xorg_s* x, xcb_window_t win){
	xcb_focus_out_event_t* event = calloc(32,1);
	event->response_type = XCB_FOCUS_OUT;
	event->event = win;
	xcb_send_event(x->connection, false, win, XCB_EVENT_MASK_LEAVE_WINDOW, (char*)event);
	xorg_client_flush(x);
	free(event);
}

void xorg_send_map(xorg_s* x, xcb_window_t win){
	xcb_map_notify_event_t event = {0};
	event.response_type = XCB_MAP_NOTIFY;
	event.event = xorg_root(x);
	event.window = win;
	xcb_send_event(x->connection, false, win, XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (char*)&event);
	xorg_client_flush(x);
}

void xorg_send_unmap(xorg_s* x, xcb_window_t win){
	xcb_unmap_notify_event_t event = {0};
	event.response_type = XCB_UNMAP_NOTIFY;
	event.event = xorg_root(x);
	event.window = win;
	xcb_send_event(x->connection, false, win, XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (char*)&event);
	xorg_client_flush(x);
}

void xorg_send_configure(xorg_s* x, xcb_window_t win, int px, int py, int w, int h, int border){
	xcb_configure_notify_event_t event = {0};
	event.response_type = XCB_CONFIGURE_NOTIFY;
	event.event = event.above_sibling = event.window = win;
	event.override_redirect = 0;
	event.x = px;
	event.y = py;
	event.width = w;
	event.height = h;
	event.border_width = border;
	xcb_send_event(x->connection, false, win, XCB_EVENT_MASK_PROPERTY_CHANGE, (char*)&event);
	xorg_client_flush(x);
}

void xorg_send_property(xorg_s* x, xcb_window_t win, xcb_atom_t atom){
	xcb_property_notify_event_t event = {0};
	event.response_type = XCB_PROPERTY_NOTIFY;
	event.window = win;
	event.atom = atom;
	event.time = time(NULL);
	xcb_send_event(x->connection, false, win, XCB_EVENT_MASK_PROPERTY_CHANGE, (char*)&event);
	xorg_client_flush(x);
}

void xorg_send_client(xorg_s* x, xcb_window_t win, uint8_t type, xcb_atom_t atom, uint8_t* data, size_t len){
	xcb_client_message_event_t event = {0};
	event.response_type = XCB_CLIENT_MESSAGE;
	event.window = win;
	event.format = type;
	event.type = atom;
	memcpy(&event.data.data8[0], data, len);
	xcb_send_event(x->connection, false, win, XCB_EVENT_MASK_PROPERTY_CHANGE, (char*)&event);
	xorg_client_flush(x);
}

void xorg_send_client32(xorg_s* x, xcb_window_t win, xcb_window_t dest, xcb_atom_t atom, const uint32_t* data, size_t len){
	xcb_client_message_event_t event = {0};
	event.response_type = XCB_CLIENT_MESSAGE;
	event.window = win;
	event.format = 32;
	event.type = atom;
	memcpy(event.data.data32, data, len);
	xcb_send_event(x->connection, false, dest, XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (char*)&event);
	xorg_client_flush(x);
}

void xorg_send_active_window(xorg_s* x, xcb_window_t current, xcb_window_t activate){
	const uint32_t data[] = { 1, time(NULL), current};
	xorg_send_client32(x, activate, xorg_root(x), x->atom[XORG_ATOM_NET_ACTIVE_WINDOW], data, sizeof(data));
}

void xorg_send_current_desktop(xorg_s* x, uint32_t desktop){
	const uint32_t data[] = { desktop, time(NULL) };
	xorg_send_client32(x, 0, xorg_root(x), x->atom[XORG_ATOM_NET_CURRENT_DESKTOP], data, sizeof(data));
}

void xorg_send_set_desktop(xorg_s* x, xcb_window_t win, uint32_t desktop){
	const uint32_t data[] = { desktop, 1 };
	xorg_send_client32(x, 0, win, x->atom[XORG_ATOM_NET_WM_DESKTOP], data, sizeof(data));
}

void xorg_window_release(xorgWindow_s* win){
	free(win->name);
	free(win->title);
	free(win->class);
	free(win->state);
	free(win->type);
}


//WM_STATE
//WM_NORMAL_HINTS
struct xwReq{
	xcb_get_window_attributes_cookie_t attr;
	xcb_get_geometry_cookie_t geom;
	xcb_get_property_cookie_t desk;
	xcb_get_property_cookie_t type;
	xcb_get_property_cookie_t stat;
	xcb_get_property_cookie_t vnam;
	xcb_get_property_cookie_t name;
	xcb_get_property_cookie_t nnam;
	xcb_get_property_cookie_t clas;
	xcb_get_property_cookie_t hint;
	xcb_get_property_cookie_t nhin;
	xcb_get_property_cookie_t stru;
	xcb_get_property_cookie_t part;
	xcb_get_property_cookie_t wpid;
};

xorgWindow_s* xorg_query_tree(size_t* count, xorg_s* x, xcb_window_t idxcb){
	XCB_ERR_DEC;
	xcb_query_tree_cookie_t cookie = xcb_query_tree(x->connection, idxcb);
	
	xcb_query_tree_reply_t* tree = xcb_query_tree_reply(x->connection, cookie, XCB_ERR_VAR);

	if( !tree
#ifdef XCB_ERROR_ENABLE	
			|| err
#endif	
			){
		dbg_error("query tree (%d)", 
#ifdef XCB_ERROR_ENABLE
				err->error_code
#else
				-1
#endif
				);
		free(tree);
		XCB_ERR_FREE;
		return NULL;
	}

	xcb_window_t* windows = xcb_query_tree_children(tree);
	struct xwReq* wrq = mem_many(struct xwReq, tree->children_len);
	xorgWindow_s* win = mem_many(xorgWindow_s, tree->children_len);
	*count = tree->children_len;

	for (unsigned int i=0; i<tree->children_len; i++) {
		wrq[i].attr = xcb_get_window_attributes(x->connection, windows[i]);
		wrq[i].geom = xcb_get_geometry(x->connection, windows[i]);
		wrq[i].name = xcb_get_property(x->connection, 0, windows[i], XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0L, 2048L);
		wrq[i].clas = xcb_get_property(x->connection, 0, windows[i], XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 0L, 2048L);
		wrq[i].hint = xcb_get_property(x->connection, 0, windows[i], XCB_ATOM_WM_HINTS, XCB_ATOM_WM_HINTS, 0L, 9L);
		wrq[i].nhin = xcb_get_property(x->connection, 0, windows[i], XCB_ATOM_WM_NORMAL_HINTS, XCB_ATOM_WM_SIZE_HINTS, 0L, XCB_ICCCM_NUM_WM_SIZE_HINTS_ELEMENTS);
		wrq[i].desk = xcb_get_property(x->connection, 0, windows[i], x->atom[XORG_ATOM_NET_WM_DESKTOP], XCB_ATOM_CARDINAL, 0, sizeof(uint32_t));
		wrq[i].type = xcb_get_property(x->connection, 0, windows[i], x->atom[XORG_ATOM_NET_WM_WINDOW_TYPE], XCB_ATOM_ATOM, 0, 1024L);
		wrq[i].stat = xcb_get_property(x->connection, 0, windows[i], x->atom[XORG_ATOM_NET_WM_STATE], XCB_ATOM_ATOM, 0, 1024L);
		wrq[i].vnam = xcb_get_property(x->connection, 0, windows[i], x->atom[XORG_ATOM_NET_WM_VISIBLE_NAME], XCB_ATOM_STRING, 0, 2048L);
		wrq[i].nnam = xcb_get_property(x->connection, 0, windows[i], x->atom[XORG_ATOM_NET_WM_NAME], XCB_ATOM_STRING, 0, 2048L);
		wrq[i].stru = xcb_get_property(x->connection, 0, windows[i], x->atom[XORG_ATOM_NET_WM_STRUT], XCB_ATOM_CARDINAL, 0, 16L);
		wrq[i].part = xcb_get_property(x->connection, 0, windows[i], x->atom[XORG_ATOM_NET_WM_STRUT_PARTIAL], XCB_ATOM_CARDINAL, 0, 48L);
		wrq[i].wpid = xcb_get_property(x->connection, 0, windows[i], x->atom[XORG_ATOM_NET_WM_PID], XCB_ATOM_CARDINAL, 0, 4L);
	}

	for (unsigned int i=0; i<tree->children_len; i++) {
		//dbg_info("window[0x%X]", windows[i]);
		win[i].idxcb = windows[i];			
		
		win[i].netname = xorg_xcb_property_string(x, wrq[i].nnam);
		win[i].name = xorg_xcb_property_string(x, wrq[i].name);
		win[i].title = xorg_xcb_property_string(x, wrq[i].vnam);
		win[i].class = xorg_xcb_property_string(x, wrq[i].clas);

		int tmp = xorg_xcb_property_cardinal(x, wrq[i].desk);
		win[i].desktop = tmp == - 1 ? -666 : tmp;
		
		win[i].typeCount = 0;
		win[i].type = xorg_xcb_property_atom_list(&win[i].typeCount, x, wrq[i].type);

		win[i].stateCount = 0;
		win[i].state = xorg_xcb_property_atom_list(&win[i].stateCount, x, wrq[i].stat);
	
		win[i].visible = xorg_xcb_attribute(x, wrq[i].attr);

		win[i].x = 0;
		win[i].y = 0;
		win[i].w = 0;
		win[i].h = 0;
		win[i].border = 0;
		xorg_xcb_geometry(x, wrq[i].geom, &win[i].x, &win[i].y, &win[i].w, &win[i].h, &win[i].border);

		xorg_xcb_property_structure(&win[i].hints, x, wrq[i].hint, XCB_ATOM_WM_HINTS, sizeof(xcb_icccm_wm_hints_t), 9*4);
		win[i].hints.window_group = XCB_NONE;
	
		win[i].size.base_height = 0;
		win[i].size.base_width = 0;
		win[i].size.win_gravity = 0;
		xorg_xcb_property_structure(&win[i].size, x, wrq[i].nhin, XCB_ATOM_WM_NORMAL_HINTS, sizeof(xcb_size_hints_t), XCB_ICCCM_NUM_WM_SIZE_HINTS_ELEMENTS * 4);
		
		xorg_xcb_property_structure(&win[i].strut, x, wrq[i].stru, x->atom[XORG_ATOM_NET_WM_STRUT], sizeof(xorgWindowStrut_s), 4 * 4);
		
		xorg_xcb_property_structure(&win[i].partial, x, wrq[i].part, x->atom[XORG_ATOM_NET_WM_STRUT_PARTIAL], sizeof(xorgWindowStrutPartial_s), 12 * 4);

		win[i].pid = xorg_xcb_property_cardinal(x, wrq[i].wpid);
	}

	free(tree);
	free(wrq);
	return win;
}

xorgWindow_s* xorg_window_application(xorg_s* x,  size_t nworkspace, xcb_window_t id, xorgWindow_s* stack, size_t* appCount){
	size_t winCount = 0;
	xorgWindow_s* win = xorg_query_tree(&winCount, x, id);
	for(unsigned i = 0; i < winCount; ++i){
		if( win[i].desktop < nworkspace){
			if( stack == NULL ){
				stack = mem_many(xorgWindow_s,1);
			}
			else{
				xorgWindow_s* tmp = realloc(stack, sizeof(xorgWindow_s)*(*appCount+1));
				if( !tmp ) dbg_fail("realloc");
				stack = tmp;
			}
			stack[*appCount] = win[i];
			(*appCount)++;
		}
		else{
			xorg_window_release(&win[i]);
		}
		stack = xorg_window_application(x, nworkspace, win[i].idxcb, stack, appCount);
	}
	free(win);
	return stack;
}

unsigned xorg_workspace_count(xorg_s* x){
	unsigned ret = 0;
	XCB_ERR_DEC;
	xcb_get_property_cookie_t ws = xcb_get_property_unchecked(x->connection, 0, xorg_root(x), x->atom[XORG_ATOM_NET_NUMBER_OF_DESKTOPS], XCB_ATOM_CARDINAL, 0, sizeof(int));
	xcb_get_property_reply_t* prop = xcb_get_property_reply(x->connection, ws, XCB_ERR_VAR);
	if( prop ){
		unsigned* nd = (unsigned*)xcb_get_property_value(prop);
		ret = *nd;
	}
	else{
#ifdef XCB_ERROR_ENABLE
		if( err ){
			dbg_error("xcb (%d) %s.%s::%s", err->error_code, xorg_error_major(x, err), xorg_error_minor(x, err), xorg_error_string(x, err, NULL));
			free(err);
		}
		else
#endif
		{
			dbg_error("no reply");
		}
	}
	free(prop);
	return ret;
}

unsigned xorg_workspace_active(xorg_s* x){
	unsigned ret = 0;
	XCB_ERR_DEC;
	xcb_get_property_cookie_t ws = xcb_get_property(x->connection, 0, xorg_root(x), x->atom[XORG_ATOM_NET_CURRENT_DESKTOP], XCB_ATOM_CARDINAL, 0, sizeof(int));
	xcb_get_property_reply_t* prop = xcb_get_property_reply(x->connection, ws, XCB_ERR_VAR);
	if( prop ){
		unsigned* nd = (unsigned*)xcb_get_property_value(prop);
		ret = *nd;
	}
	else{
#ifdef XCB_ERROR_ENABLE
		if( err ){
			dbg_error("xcb (%d) %s.%s::%s", err->error_code, xorg_error_major(x, err), xorg_error_minor(x, err), xorg_error_string(x, err, NULL));
			free(err);
		}
		else
#endif
		{
			dbg_error("no reply");
		}
	}
	free(prop);
	return ret;
}

char** xorg_workspace_name_get(xorg_s* x){
	xcb_get_property_cookie_t cookie = xcb_get_property(x->connection, 0, xorg_root(x), x->atom[XORG_ATOM_NET_DESKTOP_NAMES], x->atom[XORG_ATOM_UTF8_STRING], 0, 2056L);
	//xcb_get_property_cookie_t cookie = xorg_xcb_property_cookie_string(x, xorg_root(x), x->atom[XORG_ATOM_NET_DESKTOP_NAMES]);
	char** name = xorg_xcb_property_string_list(x, cookie);
	//for(size_t i = 0; name[i]; ++i){
	//	dbg_info("workspace[%lu] name:%s",i, name[i]);
	//}
	return name;
}

uint8_t* xorg_ximage_get_composite(unsigned* outW, unsigned* outH, unsigned* outV, unsigned* outD, xorg_s* x, xcb_window_t id){
	XCB_ERR_DEC;
    xcb_composite_query_version_cookie_t comp_ver_cookie = xcb_composite_query_version(x->connection, 0, 2);
    xcb_composite_query_version_reply_t *comp_ver_reply = xcb_composite_query_version_reply(x->connection, comp_ver_cookie, XCB_ERR_VAR);
    if( comp_ver_reply ){
        if (comp_ver_reply->minor_version < 2) {
			dbg_error("query composite failure: server returned v%d.%d", comp_ver_reply->major_version, comp_ver_reply->minor_version);
            free(comp_ver_reply);
            return NULL;
        }
        free(comp_ver_reply);
    }
#ifdef XCB_ERROR_ENABLE
    else if( err ){
        dbg_error("xcb error: %d\n", err->error_code);
        free(err);
        return NULL;
    }
#endif
    xcb_composite_redirect_window(x->connection, id, XCB_COMPOSITE_REDIRECT_AUTOMATIC);
    int win_h, win_w;//, win_d;

    xcb_get_geometry_cookie_t gg_cookie = xcb_get_geometry(x->connection, id);
    xcb_get_geometry_reply_t *gg_reply = xcb_get_geometry_reply(x->connection, gg_cookie, XCB_ERR_VAR);
    if (gg_reply) {
        win_w = gg_reply->width;
        win_h = gg_reply->height;
        //win_d = gg_reply->depth;
        free(gg_reply);
    } 
	else{
#ifdef XCB_ERROR_ENABLE
        if( err ){
            dbg_error("get geometry: XCB error %d\n", err->error_code);
            free(err);
        }
#endif
        return NULL;
    }


    // create a pixmap
    xcb_pixmap_t win_pixmap = xcb_generate_id(x->connection);
    xcb_composite_name_window_pixmap(x->connection, id, win_pixmap);
	
    // get the image
    xcb_get_image_cookie_t gi_cookie = xcb_get_image(x->connection, XCB_IMAGE_FORMAT_Z_PIXMAP, win_pixmap, 0, 0, win_w, win_h, (uint32_t)(~0UL));
    xcb_get_image_reply_t *gi_reply = xcb_get_image_reply(x->connection, gi_cookie, XCB_ERR_VAR);
    uint8_t* data = NULL;
	if( gi_reply ){
        int data_len = xcb_get_image_data_length(gi_reply);
		iassert(data_len);
		size_t rsize = data_len;
		data = mem_many_aligned(uint8_t, &rsize, 16);
        *outV = gi_reply->visual;
        *outD = gi_reply->depth;
        *outW = win_w;
		*outH = win_h;
        uint8_t* raw = xcb_get_image_data(gi_reply);
		memcpy(data, raw, data_len);
        free(gi_reply);
    }
	else{
#ifdef XCB_ERROR_ENABLE
		if( err ){
			dbg_error("get image fail: %d %s.%s::%s", err->error_code, xorg_error_major(x, err), xorg_error_minor(x, err), xorg_error_string(x, err, NULL));
			free(err);
		}
		else
#endif
		{
			dbg_error("unknown data");
		}
	}
	
	xcb_free_pixmap(x->connection, win_pixmap);
    xcb_composite_unredirect_window(x->connection, id, XCB_COMPOSITE_REDIRECT_AUTOMATIC);
	
    return data;
}

xcb_pixmap_t xorg_root_pixmap_get(xorg_s* x){
	xcb_get_property_cookie_t cookie = xcb_get_property( x->connection, 0, xorg_root(x), x->atom[XORG_ATOM_XROOTPMAP_ID], XCB_ATOM_PIXMAP, 0, 4);
	return xorg_xcb_property_pixmap(x, cookie);
}

uint8_t* xorg_ximage_root_get(unsigned* outW, unsigned* outH, unsigned* outV, unsigned* outD, xorg_s* x){
	XCB_ERR_DEC;
	int win_h, win_w;

    xcb_get_geometry_cookie_t gg_cookie = xcb_get_geometry(x->connection, xorg_root(x));
    xcb_get_geometry_reply_t *gg_reply = xcb_get_geometry_reply(x->connection, gg_cookie, XCB_ERR_VAR);
    if (gg_reply) {
        win_w = gg_reply->width;
        win_h = gg_reply->height;
        free(gg_reply);
    } 
	else{
#if XCB_ERROR_ENABLE
        if( err ){
            dbg_error("get geometry: XCB error %d\n", err->error_code);
            free(err);
        }
#endif
        return NULL;
    }
	dbg_info("root %d*%d",win_w,win_h);
    // create a pixmap
    //xcb_pixmap_t rootpixmap = xorg_root_pixmap_get(x);
	
    // get the image
    xcb_get_image_cookie_t gi_cookie = xcb_get_image(x->connection, XCB_IMAGE_FORMAT_Z_PIXMAP, xorg_root(x), 0, 0, win_w, win_h, (uint32_t)(~0UL));
    xcb_get_image_reply_t *gi_reply = xcb_get_image_reply(x->connection, gi_cookie, XCB_ERR_VAR);
    uint8_t* data = NULL;
	if( gi_reply ){
        int data_len = xcb_get_image_data_length(gi_reply);
		iassert(data_len);
		size_t rsize = data_len;
		data = mem_many_aligned(uint8_t, &rsize, 16);
        *outV = gi_reply->visual;
        *outD = gi_reply->depth;
        *outW = win_w;
		*outH = win_h;
        uint8_t* raw = xcb_get_image_data(gi_reply);
		memcpy(data, raw, data_len);
        free(gi_reply);
    }
	else{
#ifdef XCB_ERROR_ENABLE
		if( err ){
			dbg_error("get image fail: %d %s.%s::%s", err->error_code, xorg_error_major(x, err), xorg_error_minor(x, err), xorg_error_string(x, err, NULL));
			free(err);
		}
		else
#endif
		{
			dbg_error("unknown data");
		}
	}
	
	//xcb_free_pixmap(x->connection, rootpixmap);
	
    return data;
}

err_t xorg_image_grab(g2dImage_s* dst, xorg_s* x, xcb_window_t id){
	unsigned w,h,v,d;
	uint8_t* data = xorg_ximage_get_composite(&w, &h, &v, &d, x, id);
	if( !data ) return -1;
	g2d_clone(dst, w, h, X_COLOR_MODE, data);
	return 0;
}

err_t xorg_root_image_grab(g2dImage_s* dst, xorg_s* x){
	unsigned w,h,v,d;
	uint8_t* data = xorg_ximage_root_get(&w, &h, &v, &d, x);
	if( !data ) return -1;
	g2d_clone(dst, w, h, X_COLOR_MODE, data);
	return 0;
}

void xorg_win_title(xorg_s* x, xcb_window_t id, char const* name){
	xcb_change_property(x->connection, XCB_PROP_MODE_REPLACE, id, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(name), name);
}

void xorg_win_class(xorg_s* x, xcb_window_t id, char const* name){
	xcb_change_property(x->connection, XCB_PROP_MODE_REPLACE, id, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, strlen(name), name);
}

void xorg_win_show(xorg_s* x, xcb_window_t id, int show){
	if( show ){
		xcb_map_window(x->connection, id);
		xorg_client_sync(x);
	}
	else{
		xcb_unmap_window(x->connection, id);
	}
}

void xorg_win_move(xorg_s* x, xcb_window_t id, unsigned X, unsigned y){
	uint32_t values[] = { X, y };
	xcb_configure_window(x->connection, id, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
}

void xorg_win_resize(xorg_s* x, xcb_window_t id, unsigned w, unsigned h){
	uint32_t values[] = { w, h };
	xcb_configure_window(x->connection, id, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
}

void xorg_win_coord(xorg_s* x, xcb_window_t id, g2dCoord_s* pos){
	uint32_t values[] = { pos->x, pos->y, pos->w, pos->h };
	xcb_configure_window(x->connection, id, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
}

void xorg_win_size(g2dCoord_s* out, unsigned* outBorder, xorg_s* x, xcb_window_t idxcb){
	xcb_get_geometry_cookie_t geom = xcb_get_geometry(x->connection, idxcb);
	xorg_xcb_geometry(x, geom, &out->x, &out->y, &out->w, &out->h, outBorder);
}


void xorg_win_surface_redraw(xorg_s* x, xcb_window_t id,  xorgSurface_s* surface){
	dbg_info("");
	xcb_image_put(x->connection, id, surface->gc, surface->ximage, 0, 0, 0);
	xorg_client_flush(x);
}

void xorg_win_dock(xorg_s* x, xcb_window_t id){
	xcb_change_property(
			x->connection, 
			XCB_PROP_MODE_REPLACE, id, 
			x->atom[XORG_ATOM_NET_WM_WINDOW_TYPE], 
			XCB_ATOM_ATOM, 32, 1, (unsigned char*)&x->atom[XORG_ATOM_NET_WM_WINDOW_TYPE_DOCK]
	);
}

void xorg_wm_reserve_dock_space_on_top(xorg_s* x, xcb_window_t id, unsigned X, unsigned w, unsigned h){
	xorgWindowStrutPartial_s partial = {0};
	partial.top = xorg_root_y(x) + h;
	partial.top_start_x = xorg_root_x(x) + X;
	partial.top_end_x = partial.top_start_x + w;
	dbg_info("reserve: top %u x %u ex: %u", partial.top, partial.top_start_x, partial.top_end_x);

	xcb_change_property(
			x->connection, 
			XCB_PROP_MODE_REPLACE, id, 
			x->atom[XORG_ATOM_NET_WM_STRUT_PARTIAL], 
			XCB_ATOM_CARDINAL, 32, 1, &partial
	);
}

void xorg_wm_reserve_dock_space_on_bottom(xorg_s* x, xcb_window_t id, unsigned X, unsigned w, unsigned h){
	xorgWindowStrutPartial_s partial = {0};
	partial.bottom = h;
	partial.bottom_start_x = xorg_root_x(x) + X;
	partial.bottom_end_x = partial.bottom_start_x + w;
	
	xcb_change_property(
			x->connection, 
			XCB_PROP_MODE_REPLACE, id, 
			x->atom[XORG_ATOM_NET_WM_STRUT_PARTIAL], 
			XCB_ATOM_CARDINAL, 32, 1, &partial
	);
}

void xorg_register_events(xorg_s* x, xcb_window_t window, unsigned int eventmask) {
	xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(x->connection, window, XCB_CW_EVENT_MASK, (uint32_t[]){eventmask});
#ifdef XCB_ERROR_ENABLE
	xcb_generic_error_t *err = xcb_request_check(x->connection, cookie);
	if (err != NULL) {
		dbg_error("xcb change windows attribute, event mask: %d", err->error_code);
		free(err);
	}
#else
	void *err = xcb_request_check(x->connection, cookie);
	if (err != NULL) {
		dbg_error("xcb change windows attributei");
		free(err);
	}
#endif
}

xcb_window_t xorg_win_new(xorgSurface_s* surface, xorg_s* x, xcb_window_t parent, g2dCoord_s* pos, unsigned border, g2dColor_t background){
	unsigned event = X_WIN_EVENT;
	
	xcb_window_t win = xcb_generate_id(x->connection);
	xcb_create_window(x->connection, XCB_COPY_FROM_PARENT, win, parent,
			xorg_root_x(x) + pos->x, xorg_root_y(x) + pos->y, pos->w, pos->h, border,
			//XCB_WINDOW_CLASS_INPUT_OUTPUT,
			XCB_WINDOW_CLASS_COPY_FROM_PARENT,
		   	xorg_root_visual(x), XCB_CW_EVENT_MASK, &event);
	
	if( surface ){
		g2d_init(&surface->img, pos->w, pos->h, X_COLOR_MODE);
		g2dCoord_s area = { .x = 0, .y = 0, .w = pos->w, .h = pos->h };
		g2d_clear(&surface->img, background, &area);
		surface->ximage = xcb_image_create(
				pos->w, pos->h,
				XCB_IMAGE_FORMAT_Z_PIXMAP, 32, 24, 32, 32, XCB_IMAGE_ORDER_LSB_FIRST, XCB_IMAGE_ORDER_LSB_FIRST,
				surface->img.pixel, surface->img.p * surface->img.h, surface->img.pixel);
		surface->gc = xcb_generate_id(x->connection);
		xcb_create_gc(x->connection, surface->gc, win, 0, 0);
	}
	return win;
}

void xorg_surface_resize(xorgSurface_s* surface, unsigned w, unsigned h){
	g2d_unload(&surface->img);
	g2d_init(&surface->img, w, h, X_COLOR_MODE);
	free(surface->ximage);
	surface->ximage = xcb_image_create(
		w, h,
		XCB_IMAGE_FORMAT_Z_PIXMAP, 32, 24, 32, 32, XCB_IMAGE_ORDER_LSB_FIRST, XCB_IMAGE_ORDER_LSB_FIRST,
		surface->img.pixel, surface->img.p * surface->img.h, surface->img.pixel
	);
}

void xorg_surface_resize_bitblt(xorgSurface_s* surface, unsigned w, unsigned h){
	g2dImage_s new = g2d_new(w, h, X_COLOR_MODE);
	g2dCoord_s blt = {
		.x = 0,
		.y = 0,
		.w = w > surface->img.w ? surface->img.w : w,
		.h = h > surface->img.h ? surface->img.h : h,
	};
	g2d_bitblt(&new, &blt, &surface->img, &blt);
	g2d_unload(&surface->img);
	surface->img = new;
	free(surface->ximage);
	surface->ximage = xcb_image_create(
		w, h,
		XCB_IMAGE_FORMAT_Z_PIXMAP, 32, 24, 32, 32, XCB_IMAGE_ORDER_LSB_FIRST, XCB_IMAGE_ORDER_LSB_FIRST,
		surface->img.pixel, surface->img.p * surface->img.h, surface->img.pixel
	);
}

void xorg_surface_destroy(xorg_s* x, xorgSurface_s* surface){
	xcb_free_gc(x->connection, surface->gc);
	free(surface->ximage);
	g2d_unload(&surface->img);
}

void xorg_win_destroy(xorg_s* x, xcb_window_t id){
	xcb_destroy_window(x->connection, id);
}

void xorg_win_focus(xorg_s* x, xcb_window_t id){
	xcb_set_input_focus(x->connection, XCB_INPUT_FOCUS_NONE, id, time(NULL));
}

err_t xorg_win_event(xorg_s* x, xorgCallbackEvent_s* callback, int async){

	xcb_generic_event_t* event;
	if( !async )
		event = xcb_wait_for_event(x->connection);
	else
		event = xcb_poll_for_event(x->connection);

	if( !event ) return -1;

	switch( event->response_type & ~0x80 ){
		case XCB_CREATE_NOTIFY: 
			if( callback->creat ){
				dbg_info("create");
				callback->creat(x, callback->user);
			}
		break;

		case XCB_DESTROY_NOTIFY:
			if( callback->destroy ){
				dbg_info("destroy");
				callback->destroy(x, callback->user);
			}
		return -1;
				
		case XCB_EXPOSE:
			if( callback->redraw ){
				xcb_expose_event_t* expose = (xcb_expose_event_t*)event;
				g2dCoord_s damaged = { .x = expose->x, .y = expose->y, .w = expose->width, .h = expose->height };
				dbg_info("redraw damaged : .x=%u .y=%u .w=%u .h=%u", damaged.x, damaged.y, damaged.w, damaged. h);
				callback->redraw(x, callback->user, &damaged);
			}
		break;
		
		case XCB_KEY_PRESS:
			if( callback->keyboard ){
				xcb_key_press_event_t* xk = (xcb_key_press_event_t*)event;
				xorgKeyboard_s key = {
					.x = x,
					.user = callback->user,
					.event = XORG_KEY_PRESS,
					.absolute.x = xk->root_x,
					.absolute.y = xk->root_y,
					.relative.x = xk->event_x,
					.relative.y = xk->event_y,
					.button = xk->state,
					.time = xk->time,
					.keycode = xk->detail
				};
				struct xkb_state* state;
				if( (state = xkb_x11_state_new_from_device(x->key.keymap, x->connection, x->key.device)) ){
					key.keysym = xkb_state_key_get_one_sym(state, key.keycode);
					int size = xkb_state_key_get_utf8(state, key.keycode, NULL, 0) + 1;
					if( size > 1 && size < XKB_UTF_MAX ){
						xkb_state_key_get_utf8(state, key.keycode, (char*)key.utf8, size);
					}
					else{
						key.utf8[0] = 0;
					}
					dbg_info("key press button: %u keycode: %lu keysym: %lu utf: %s", key.button, key.keycode, key.keysym, key.utf8);
					xkb_state_unref(state);
					callback->keyboard(&key);
				}
			}
		break;
		
		case XCB_KEY_RELEASE:
			if( callback->keyboard ){
				xcb_key_release_event_t* xk = (xcb_key_release_event_t*)event;
				xorgKeyboard_s key = {
					.x = x,
					.user = callback->user,
					.event = XORG_KEY_RELEASE,
					.absolute.x = xk->root_x,
					.absolute.y = xk->root_y,
					.relative.x = xk->event_x,
					.relative.y = xk->event_y,
					.button = xk->state,
					.time = xk->time,
					.keycode = xk->detail
				};
				struct xkb_state* state;
				if( (state = xkb_x11_state_new_from_device(x->key.keymap, x->connection, x->key.device)) ){
					key.keysym = xkb_state_key_get_one_sym(state, key.keycode);
					int size = xkb_state_key_get_utf8(state, key.keycode, NULL, 0) + 1;
					if( size > 1 && size < XKB_UTF_MAX ){
						xkb_state_key_get_utf8(state, key.keycode, (char*)key.utf8, size);
					}
					else{
						key.utf8[0] = 0;
					}
					dbg_info("key release button: %u keycode: %lu keysym: %lu utf: %s", key.button, key.keycode, key.keysym, key.utf8);
					xkb_state_unref(state);
					callback->keyboard(&key);
				}
			}
		break;
		
		case XCB_BUTTON_PRESS:
			if( callback->mouse ){
				xcb_button_press_event_t* button = (xcb_button_press_event_t*)event;
				xorgMouse_s mouse = {
					.x = x,
					.user = callback->user,
					.event = XORG_MOUSE_PRESS,
					.absolute.x = button->root_x,
					.absolute.y = button->root_y,
					.relative.x = button->event_x,
					.relative.y = button->event_y,
					.button = button->detail,
					.key = button->state,
					.time = button->time
				};
				if( mouse.button < 4 && mouse.time - x->_mousetime > x->dblclickms ){
					x->_mousetime = button->time;
					x->_mousestate = 1;
				}
				dbg_info("button %d A: .x %u .y %u R: .x %u .y %u .b %u .t %ld", 
						mouse.event, mouse.absolute.x, mouse.absolute.y, mouse.relative.x, mouse.relative.y, mouse.button, mouse.time);
				callback->mouse(&mouse);
			}
		break;

		case XCB_BUTTON_RELEASE:
			if( callback->mouse ){
				xcb_button_press_event_t* button = (xcb_button_press_event_t*)event;
				xorgMouse_s mouse = {
					.x = x,
					.user = callback->user,
					.event = XORG_MOUSE_RELEASE,
					.absolute.x = button->root_x,
					.absolute.y = button->root_y,
					.relative.x = button->event_x,
					.relative.y = button->event_y,
					.button = button->detail,
					.key = button->state,
					.time = button->time
				};
				if( mouse.button < 4 && x->_mousestate == 1 && mouse.time - x->_mousetime < x->clickms){
					mouse.event = XORG_MOUSE_CLICK;
					x->_mousestate = 2;
				}
				else if( mouse.button < 4 && x->_mousestate == 2 && mouse.time - x->_mousetime < x->dblclickms ){
					mouse.event = XORG_MOUSE_DBLCLICK;
				}
				dbg_info("button %d A: .x %u .y %u R: .x %u .y %u .b %u .t %ld", 
						mouse.event, mouse.absolute.x, mouse.absolute.y, mouse.relative.x, mouse.relative.y, mouse.button, mouse.time);
				if( callback->mouse ) callback->mouse(&mouse);
			}
		break;

		case XCB_MOTION_NOTIFY:
			if( callback->mouse ){
				xcb_motion_notify_event_t* button = (xcb_motion_notify_event_t*)event;
				xorgMouse_s mouse = {
					.x = x,
					.user = callback->user,
					.event = XORG_MOUSE_MOVE,
					.absolute.x = button->root_x,
					.absolute.y = button->root_y,
					.relative.x = button->event_x,
					.relative.y = button->event_y,
					.button = button->detail,
					.key = button->state,
					.time = button->time
				};
				dbg_info("button %d A: .x %u .y %u R: .x %u .y %u .b %u .t %ld", 
						mouse.event, mouse.absolute.x, mouse.absolute.y, mouse.relative.x, mouse.relative.y, mouse.button, mouse.time);
				callback->mouse(&mouse);
			}
		break;

		case XCB_ENTER_NOTIFY:
			if( callback->mouse ){
				xcb_motion_notify_event_t* button = (xcb_motion_notify_event_t*)event;
				xorgMouse_s mouse = {
					.x = x,
					.user = callback->user,
					.event = XORG_MOUSE_ENTER,
					.absolute.x = button->root_x,
					.absolute.y = button->root_y,
					.relative.x = button->event_x,
					.relative.y = button->event_y,
					.button = button->detail,
					.key = button->state,
					.time = button->time
				};
				dbg_info("button %d A: .x %u .y %u R: .x %u .y %u .b %u .t %ld", 
						mouse.event, mouse.absolute.x, mouse.absolute.y, mouse.relative.x, mouse.relative.y, mouse.button, mouse.time);
				callback->mouse(&mouse);
			}
		break;

		case XCB_LEAVE_NOTIFY:
			if( callback->mouse ){
				xcb_motion_notify_event_t* button = (xcb_motion_notify_event_t*)event;
				xorgMouse_s mouse = {
					.x = x,
					.user = callback->user,
					.event = XORG_MOUSE_LEAVE,
					.absolute.x = button->root_x,
					.absolute.y = button->root_y,
					.relative.x = button->event_x,
					.relative.y = button->event_y,
					.button = button->detail,
					.key = button->state,
					.time = button->time
				};
				dbg_info("button %d A: .x %u .y %u R: .x %u .y %u .b %u .t %ld", 
						mouse.event, mouse.absolute.x, mouse.absolute.y, mouse.relative.x, mouse.relative.y, mouse.button, mouse.time);
				callback->mouse(&mouse);
			}
		break;

		case XCB_FOCUS_IN:
			if( callback->focus ){
				dbg_info("focus in");
				callback->focus(x, callback->user, 1);
			}
		break;
		
		case XCB_FOCUS_OUT:
			if( callback->focus ){
				dbg_info("focus out");
				callback->focus(x, callback->user, 0);
			}
		break;

		case XCB_MAP_NOTIFY:
			if( callback->visibility ){
				dbg_info("map");
				callback->visibility(x, callback->user, 1);
			}
		break;

		case XCB_UNMAP_NOTIFY:
			if( callback->visibility ){
				dbg_info("unmap");
				callback->visibility(x, callback->user, 0);
			}
		break;

		case XCB_CONFIGURE_NOTIFY:
			if( callback->move ){
				xcb_configure_notify_event_t* conf = (xcb_configure_notify_event_t*)event;
				xorgMove_s move = { 
					.x = x,
					.user = callback->user,
					.border = conf->border_width,
					.coord.x = conf->x, 
					.coord.y = conf->y, 
					.coord.w = conf->width, 
					.coord.h = conf->height 
				};
				dbg_info("move .x %u .y %u .w %u .h %u .b %u", move.coord.x, move.coord.y, move.coord.w, move.coord.h, move.border);
				callback->move(&move);
			}
		break;

		case XCB_PROPERTY_NOTIFY:
			if( callback->atom ){
				xcb_property_notify_event_t* prop = (xcb_property_notify_event_t*)event;
				dbg_info("property atom %u name %s", prop->atom, xorg_atom_name(x, prop->atom));
				callback->atom(x, callback->user, prop->atom);
			}
		break;

		case XCB_CLIENT_MESSAGE:
			if( callback->message ){
				xcb_client_message_event_t* msg = (xcb_client_message_event_t*)event;
				dbg_info("message %u %s", msg->type, xorg_atom_name(x, msg->type));
				callback->message(x, callback->user, msg->format, msg->type, &msg->data.data8[0]);
			}
		break;

		default:
			dbg_info("unknown %u", event->response_type & ~0x80 );
		break;
	}
	return 0;
}


