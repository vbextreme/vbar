#ifndef __VBAR_IPC_H__
#define __VBAR_IPC_H__

#include <vbar/type.h>
#include <sys/sysinfo.h>

#define ATTRIBUTE_ICONS_SIZE 8
#define ATTRIBUTE_FORMAT_MAX 6
#define ATTRIBUTE_SPAWN_MAX 1024
#define ATTRIBUTE_TEXT_MAX 126

typedef enum {ALIGN_CENTER, ALIGN_RIGHT, ALIGN_LEFT} align_e;

typedef struct attribute{
	char longformat[ATTRIBUTE_TEXT_MAX];
	char shortformat[ATTRIBUTE_TEXT_MAX];
	char longunformat[ATTRIBUTE_TEXT_MAX];
	char shortunformat[ATTRIBUTE_TEXT_MAX];
	
	int color;
	int border;
	int background;
	int min_width;

	align_e align;
	
	char name[ATTRIBUTE_TEXT_MAX];
	char instance[ATTRIBUTE_TEXT_MAX];

	int urgent;
	int seaparator;
	int separator_block_width;
	int markup;

	long blinktime;
	int blink;
	int blinkstatus;
	
	long reftime;
	long tick;
	
	char** icons;
	size_t iconcount;
	size_t icoindex;
	
	char** format;
	size_t formatcount;

	char onevent[ATTRIBUTE_SPAWN_MAX];
}attribute_s;


#define IPC_TIMEOUT 0x01
#define IPC_EVENT   0x02

typedef struct event{
	char name[ATTRIBUTE_TEXT_MAX];
	char instance[ATTRIBUTE_TEXT_MAX];
	int x;
	int y;
	int button;
	int relative_x;
	int relative_y;
	int width;
	int height;
}event_s;

void ipc_init(bool_t clickevents);
void ipc_write_element(attribute_s* el, bool_t next);
void ipc_event_reset(event_s* ev);
int ipc_wait(event_s* ev, long timeend);
void ipc_begin_elements();
void ipc_end_elements();


#endif
