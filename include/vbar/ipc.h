#ifndef __VBAR_IPC_H__
#define __VBAR_IPC_H__

#include <vbar/type.h>
#include <sys/sysinfo.h>

#define I3BAR_TEXT_MAX 126
#define I3BAR_COLOR_MAX 8
#define I3BAR_NUM_MAX 12
#define I3BAR_AL_MAX 7

typedef enum {I3_ALIGN_CENTER, I3_ALIGN_RIGHT, I3_ALIGN_LEFT} i3align_e;

typedef struct i3element{
	char full_text[I3BAR_TEXT_MAX];
	char short_text[I3BAR_TEXT_MAX];
	int color;
	int background;
	int border;
	int min_width;
	i3align_e align;
	char name[I3BAR_TEXT_MAX];
	char instance[I3BAR_TEXT_MAX];
	int urgent;
	int seaparator;
	int separator_block_width;
	int markup;
}i3element_s;

typedef struct i3events{
	int wip;
	char name[I3BAR_TEXT_MAX];
	char instance[I3BAR_TEXT_MAX];
	int x;
	int y;
	int button;
	int relative_x;
	int relative_y;
	int width;
	int height;
}i3event_s;

#define I3BAR_TIMEOUT 0x01
#define I3BAR_EVENT   0x02

void i3bar_init(bool_t clickevents);
void i3bar_write_element(i3element_s* el, bool_t next);
void i3bar_event_reset(i3event_s* ev);
int i3bar_wait(i3event_s* ev, long timeend);
void i3bar_begin_elements();
void i3bar_end_elements();

#endif
