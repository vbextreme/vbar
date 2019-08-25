#ifndef __VBAR_SCRIPT_H__
#define __VBAR_SCRIPT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define VBAR_BORDER_LEFT   0x01
#define VBAR_BORDER_TOP    0x02
#define VBAR_BORDER_RIGHT  0x04
#define VBAR_BORDER_BOTTOM 0x08

#define KIB (1024UL)
#define MIB (KIB*KIB)
#define GIB (MIB*KIB)

typedef int err_t;
typedef enum { FALSE, TRUE } bool_t;
typedef enum { VBAR_ALIGNED_LEFT, VBAR_ALIGNED_CENTER, VBAR_ALIGNED_RIGHT } vbarAligned_e;
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

typedef struct vbar vbar_s;
typedef struct gadget gadget_s;

typedef struct vbarMouse{
	unsigned x, y, button, extend, line, icon;
}vbarMouse_s;


typedef void(*gadgetScriptEvent_f)(gadget_s* g, gadgetEventType_e ev, void* arg);

vbar_s* vbar_get(void);

void vbar_monitor_set(vbar_s* vb, char const* monitor);
void vbar_fonts_set(vbar_s* vb, char const* name, size_t size);
void vbar_colors_set(vbar_s* vb, unsigned background, unsigned foreground);
void vbar_topbar_set(vbar_s* vb, unsigned ontop);
void vbar_height_set(vbar_s* vb, unsigned height, unsigned spacingTop, unsigned spacingBottom);
void vbar_border_width_set(vbar_s* vb, unsigned size);
void vbar_icon_load(vbar_s* vb, char const* path, char const* name, unsigned background);
unsigned rgb(int r, int g, int b);

gadget_s* gadget_new(vbar_s* vb, char const* class, char const* name);
void gadget_interval(gadget_s* g, long newtime);
void gadget_change_interval(gadget_s* g, long newtime);
void gadget_start(gadget_s* g);
void gadget_stop(gadget_s* g);
void gadget_event_register(gadget_s* g, gadgetScriptEvent_f script);
void gadget_background(gadget_s* g, unsigned color);
void gadget_foreground(gadget_s* g, unsigned color);
void gadget_border_color(gadget_s* g, unsigned color);
void gadget_border(gadget_s* g, unsigned border);
void gadget_align(gadget_s* g, vbarAligned_e align);
void gadget_icon(gadget_s* g, char const* name);
void gadget_hide(gadget_s* g, int hide);
int gadget_status_get(gadget_s* g);
void gadget_status_set(gadget_s* g, int status);
size_t gadget_text_lenght(gadget_s* g, char* txt);
void gadget_text_reset(gadget_s* g);
void gadget_text(gadget_s* g, char const* format,  ...);
const char* gadget_tohuman(double* out, double value, int initial, double div);
void gadget_redraw(gadget_s* g);

void gadget_extend_enable(gadget_s* g, size_t count);
void gadget_extend_background(gadget_s* g, size_t index, unsigned color);
void gadget_extend_foreground(gadget_s* g, size_t index, unsigned color);
void gadget_extend_border_color(gadget_s* g, size_t index, unsigned color);
void gadget_extend_border(gadget_s* g, size_t index, unsigned border);
void gadget_extend_label(gadget_s* g, size_t index, char const* format,  ...);
void gadget_extend_icons_enable(gadget_s* g, size_t index, size_t count);
void gadget_extend_icons_set(gadget_s* g, size_t index, size_t id, char* name);
void gadget_extend_toggle(gadget_s* g);
void gadget_extend_redraw(gadget_s* g);


