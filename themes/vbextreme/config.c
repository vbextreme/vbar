#include <vbarScript.h>

/*********************************/
/*** you not need to edit this ***/
/*********************************/

//generic macro used in config.c
#define __Y__ 1
#define __N__ 0
#define TOPBAR 1
#define BOTTOMBAR 0
#define MODE_BACKGROUND 0
#define MODE_BORDER 1
#define MSTR(S) #S
#define MACRO_CAT(A, B) A ## B
#define DNAME(F,N) MACRO_CAT(F,N)
#define SNAME(N) MSTR(N)

//genric type
typedef void(*init_f)(void);
#define LOAD(TYPE,GNAME) DNAME(DNAME(TYPE,_init_),GNAME)


/**************/
/* config bar */
/**************/

//set monitor name, examples "HDMI1", NULL is primary
#define MONITOR (NULL)

//set fonts and size, first is primary and other is fallback, add your fonts before NULL
#define FONTS {\
	{"inconsolata", 12},\
	{"Symbola", 12,},\
	{"FontAwesome5FreeRegular", 12},\
	{"FontAwesome5FreeSolid", 12},\
	{"FontAwesome5Brands", 12},\
	{NULL,0} /*not remove this*/\
}

//color theme, gruvbox
#define BLACK    rgb(0x28, 0x28, 0x28)
#define RED      rgb(0xCC, 0x24, 0x1D)
#define GREEN    rgb(0x98, 0x97, 0x1A)
#define YELLOW   rgb(0xD7, 0x99, 0x21)
#define BLUE     rgb(0x45, 0x85, 0x88)
#define MAGENTA  rgb(0xB1, 0x62, 0x86)
#define CYAN     rgb(0x68, 0x9D, 0x6A)
#define GRAY     rgb(0xa8, 0x99, 0x84)
#define DGRAY    rgb(0x92, 0x83, 0x74)
#define LRED     rgb(0xFB, 0x49, 0x34)
#define LGREEN   rgb(0xB8, 0xBB, 0x26)
#define LYELLOW  rgb(0xFA, 0xBD, 0x2F)
#define LBLUE    rgb(0x83, 0xA5, 0x98)
#define LMAGENTA rgb(0xD3, 0x86, 0x9B)
#define LCYAN    rgb(0x8E, 0xC0, 0x7C)
#define WHITE    rgb(0xEB, 0xDB, 0xB2)

//background color
#define BACKGROUND (BLACK)
//foreground color
#define FOREGROUND (WHITE)
//warning color
#define WARNING (LYELLOW)
//urgent color
#define URGENT (LRED)
//good color
#define GOOD (LBLUE)

//set bar height, 0 is automatic set
#define HEIGHT 0
//set spacing top of text
#define SPACING_TOP 0
//set spacing bottom text
#define SPACING_BOTTOM 5

//set bar position TOPBAR or BOTTOMBAR
#define BARPOSITION TOPBAR

//set size bottom border
#define BORDER_SIZE 2

//object vbar
vbar_s* vbar;

/*****************/
/* config gadget */
/*******************************************************************/
/* you can copy and paste any gadget, rember to change GADGET_NAME */
/*******************************************************************/

/*************/
/* workspace */
/*************/

//create new workspace gadget named works
#define GADGET_NAME works
//set workspace active background colors
#define WORKSPACE_ACTIVE_BACKGROUND YELLOW 
//set workspace active foreground colors
#define WORKSPACE_ACTIVE_FOREGROUND FOREGROUND
//set workspace background colors
#define GADGET_BACKGROUND BACKGROUND
//set workspace foreground colors
#define GADGET_FOREGROUND FOREGROUND
//se border color on mouse in, only if BORDER_SIZE > 0
#define GADGET_HOVER_IN LYELLOW
//se border color on mouse out, only if BORDER_SIZE > 0
#define GADGET_HOVER_OUT YELLOW
//reorder name in workspace
#define WORKSPACE_ORDER __Y__
//align left
#define GADGET_ALIGNED (VBAR_ALIGNED_LEFT)
//enable graphical png icon, __N__ for disable
#define GADGET_ICON __Y__
#define GADGET_ICON_NAME "~/Immagini/perapp/workspace0.png"
//load gadget
#include "workspace.h"

/*********/
/* clock */
/*********/

//create new clock gadget named clk
#define GADGET_NAME clk
//set background colors
#define GADGET_BACKGROUND BACKGROUND
//set foreground colors
#define GADGET_FOREGROUND FOREGROUND
//se border color on mouse in, only if BORDER_SIZE > 0
#define GADGET_HOVER_IN GRAY
//se border color on mouse out, only if BORDER_SIZE > 0
#define GADGET_HOVER_OUT DGRAY
//align 
#define GADGET_ALIGNED (VBAR_ALIGNED_CENTER)
//time to refresh in ms
#define GADGET_INTERVAL 1000
//text to view
#define GADGET_TEXT " %02d.%02d.%02d ", gadget_clock_hour(g), gadget_clock_minutes(g), gadget_clock_seconds(g)
//enable graphical png icon, __N__ for disable
#define GADGET_ICON __Y__
#define GADGET_ICON_NAME "~/Immagini/perapp/clock1.png"
//se effect button on gadget
#define GADGET_CLICK_EFFECT __Y__
//set color on press
#define GADGET_BUTTON_DOWN GADGET_HOVER_IN
//set color on release
#define GADGET_BUTTON_UP BACKGROUND
//load gadget
#include "clock.h"

/******************/
/*** power menu ***/
/******************/

//create power menu gadget named poweroff
#define GADGET_NAME poweroff
//set background colors
#define GADGET_BACKGROUND BACKGROUND
//set foreground colors
#define GADGET_FOREGROUND FOREGROUND
//se border color on mouse in, only if BORDER_SIZE > 0
#define GADGET_HOVER_IN RED
//se border color on mouse out, only if BORDER_SIZE > 0
#define GADGET_HOVER_OUT LRED
//align 
#define GADGET_ALIGNED (VBAR_ALIGNED_RIGHT)
//time to refresh in ms
#define GADGET_INTERVAL 0
//text to view
#define GADGET_TEXT ""
//enable graphical png icon, __N__ for disable
#define GADGET_ICON __Y__
#define GADGET_ICON_NAME "~/Immagini/perapp/shutdown.png"
//click on gadget generate refresh event
#define GADGET_CLICK_SAME_REFRESH __Y__
//hide/show gadget
#define GADGET_HIDE 1
//run script only when click
#define SCRIPT_ONLY_ONCLICK __Y__
//pass arguments event to script
#define SCRIPT_ARG_EVENT __N__
//path of script
//#define SCRIPT_CMD "poweroff"
#define SCRIPT_CMD "poweroff"
#define SCRIPT_TXT_SLURPED __N__
//load gadget
#include "script.h"

//create power menu gadget named reboot
#define GADGET_NAME reboot
//set background colors
#define GADGET_BACKGROUND BACKGROUND
//set foreground colors
#define GADGET_FOREGROUND FOREGROUND
//se border color on mouse in, only if BORDER_SIZE > 0
#define GADGET_HOVER_IN RED
//se border color on mouse out, only if BORDER_SIZE > 0
#define GADGET_HOVER_OUT LRED
//align 
#define GADGET_ALIGNED (VBAR_ALIGNED_RIGHT)
//time to refresh in ms
#define GADGET_INTERVAL 0
//text to view
#define GADGET_TEXT ""
//enable graphical png icon, __N__ for disable
#define GADGET_ICON __Y__
#define GADGET_ICON_NAME "~/Immagini/perapp/reboot.png"
//click on gadget generate refresh event
#define GADGET_CLICK_SAME_REFRESH __Y__
//hide/show gadget
#define GADGET_HIDE 1
//run script only when click
#define SCRIPT_ONLY_ONCLICK __Y__
//pass arguments event to script
#define SCRIPT_ARG_EVENT __N__
//path of script
#define SCRIPT_CMD "reboot"
#define SCRIPT_TXT_SLURPED __N__
//load gadget
#include "script.h"

//create power menu
#define GADGET_NAME povermenu
//set background colors
#define GADGET_BACKGROUND BACKGROUND
//set foreground colors
#define GADGET_FOREGROUND FOREGROUND
//se border color on mouse in, only if BORDER_SIZE > 0
#define GADGET_HOVER_IN RED
//se border color on mouse out, only if BORDER_SIZE > 0
#define GADGET_HOVER_OUT LRED
//align 
#define GADGET_ALIGNED (VBAR_ALIGNED_RIGHT)
//time to refresh in ms
#define GADGET_INTERVAL 0
//text to view
#define GADGET_TEXT ""
//enable graphical png icon, __N__ for disable
#define GADGET_ICON __Y__
#define GADGET_ICON_NAME "~/Immagini/perapp/povermenu.png"
//hide/show gadget
#define GADGET_HIDE 0
//enable/disable label custom action
#define LABEL_ACTION_ENABLE __Y__
//declare action
#define LABEL_ACTION \
	if( gadget_status_get(lbl_povermenu) ){\
		gadget_hide(scr_poweroff, 0);\
		gadget_hide(scr_reboot, 0);\
		vbar_icon_load(vbar, "~/Immagini/perapp/back.png", "poverback");\
		gadget_icon(lbl_povermenu, "poverback");\
		gadget_redraw(g);\
		gadget_status_set(g,0);\
	}\
	else{\
		gadget_hide(scr_poweroff, 1);\
		gadget_hide(scr_reboot, 1);\
		gadget_icon(lbl_povermenu, "povermenu");\
		gadget_redraw(g);\
		gadget_status_set(g,-1);\
	}
//load gadget
#include "label.h"

/************/
/*** alsa ***/
/************/

//create new alsa gadget
#define GADGET_NAME alsa
//set background colors
#define GADGET_BACKGROUND BACKGROUND
//set foreground colors
#define GADGET_FOREGROUND FOREGROUND
//se border color on mouse in, only if BORDER_SIZE > 0
#define GADGET_HOVER_IN LGREEN
//se border color on mouse out, only if BORDER_SIZE > 0
#define GADGET_HOVER_OUT GREEN
//align 
#define GADGET_ALIGNED (VBAR_ALIGNED_RIGHT)
//time to refresh in ms, alsa not need this
#define GADGET_INTERVAL 0
//text to view
#define GADGET_TEXT "%3d%%", (unsigned)((100.0*volume)/volumeMax)
//enable expand bar horizontal, alsa not have many value to show
#define GADGET_EXPAND_HORI __N__
//text to view when bar is expand
#define GADGET_TEXT_EHORI 
//enable expand bar vertical, value is new time to refresh gadget in expand mode, alsa not have many value to show
#define GADGET_EXPAND_VERT __N__
//numer of line to view in extend mode, max 10 line
#define GADGET_EXTEND_LINE_COUNT 0
//unused
#define GADGET_TEXT_EXTEND_0 
#define GADGET_TEXT_EXTEND_1 
#define GADGET_TEXT_EXTEND_2 
#define GADGET_TEXT_EXTEND_3
#define GADGET_TEXT_EXTEND_4
#define GADGET_TEXT_EXTEND_5
#define GADGET_TEXT_EXTEND_6
#define GADGET_TEXT_EXTEND_7
#define GADGET_TEXT_EXTEND_8
#define GADGET_TEXT_EXTEND_9
//set time blink if value < blinkon, __N__ to disable, if blink enable low level is disabled
#define GADGET_BLINK 300
//set low level mode, gadget change color but not blink if value < blinkon, if blink is enabled low level not works
#define GADGET_LOW_LEVEL __N__
//mode blink MODE_BACKGROUND MODE_BORDER
#define GADGET_BLINK_MODE MODE_BACKGROUND
//color of blinking
#define GADGET_BLINK_BACKGROUND URGENT
#define GADGET_BLINK_FOREGROUND FOREGROUND
//blink if volume < blinkon
#define GADGET_BLINKON 1
//enable graphical png icon, __N__ for disable
#define GADGET_ICON __Y__
#define ALSA_ICON_BATTERY0 "~/Immagini/perapp/alsa-0.png"
#define ALSA_ICON_BATTERY1 "~/Immagini/perapp/alsa-1.png"
#define ALSA_ICON_BATTERY2 "~/Immagini/perapp/alsa-2.png"
#define ALSA_ICON_BATTERY3 "~/Immagini/perapp/alsa-3.png"
//hide/show gadget
#define GADGET_HIDE 0
//load gadget
#include "alsa.h"


/******************/
/*** power stat ***/
/******************/

//create new powerstat gadget
#define GADGET_NAME battery
//set background colors
#define GADGET_BACKGROUND BACKGROUND
//set foreground colors
#define GADGET_FOREGROUND FOREGROUND
//se border color on mouse in, only if BORDER_SIZE > 0
#define GADGET_HOVER_IN LGREEN
//se border color on mouse out, only if BORDER_SIZE > 0
#define GADGET_HOVER_OUT GREEN
//align 
#define GADGET_ALIGNED (VBAR_ALIGNED_RIGHT)
//time to refresh in ms
#define GADGET_INTERVAL 1000
//text to view
#define GADGET_TEXT " %3d%% ", capacity
//enable expand bar horizontal
#define GADGET_EXPAND_HORI 1
//text to view when bar is expand
#define GADGET_TEXT_EHORI " %3d%% %4d:%02d", capacity, hours, minutes
//enable expand bar vertical, value is new time to refresh gadget in expand mode
#define GADGET_EXPAND_VERT 1000 
//numer of line to view in extend mode, max 10 line
#define GADGET_EXTEND_LINE_COUNT 3
//text to view for each line
#define GADGET_TEXT_EXTEND_0 "voltage::  now: %6.2fV   min: %6.2fV", gadget_powerstat_voltage_now_get(g, 1000000), gadget_powerstat_voltage_min_get(g, 1000000)
#define GADGET_TEXT_EXTEND_1 " energy:: full: %6.2fW/h now: %6.2fW/h", gadget_powerstat_energy_full_get(g, 1000000), gadget_powerstat_energy_now_get(g, 1000000)
#define GADGET_TEXT_EXTEND_2 " status:: '%s'", gadget_powerstat_status_get(g)
//unused line
#define GADGET_TEXT_EXTEND_3
#define GADGET_TEXT_EXTEND_4
#define GADGET_TEXT_EXTEND_5
#define GADGET_TEXT_EXTEND_6
#define GADGET_TEXT_EXTEND_7
#define GADGET_TEXT_EXTEND_8
#define GADGET_TEXT_EXTEND_9
//set time blink if value < blinkon, __N__ to disable, if blink enable low level is disabled
#define GADGET_BLINK 300
//set low level mode, gadget change color but not blink if value < blinkon, if blink is enabled low level not works
#define GADGET_LOW_LEVEL __N__
//mode blink MODE_BACKGROUND MODE_BORDER
#define GADGET_BLINK_MODE MODE_BACKGROUND
//color of blinking
#define GADGET_BLINK_BACKGROUND URGENT
#define GADGET_BLINK_FOREGROUND FOREGROUND
//blink if capacity < blinkon
#define GADGET_BLINKON 20
//enable graphical png icon, __N__ for disable
#define GADGET_ICON __Y__
#define POWERSTAT_ICON_BATTERY0 "~/Immagini/perapp/battery-0.png"
#define POWERSTAT_ICON_BATTERY1 "~/Immagini/perapp/battery-1.png"
#define POWERSTAT_ICON_BATTERY2 "~/Immagini/perapp/battery-2.png"
#define POWERSTAT_ICON_BATTERY3 "~/Immagini/perapp/battery-3.png"
#define POWERSTAT_ICON_BATTERY4 "~/Immagini/perapp/battery-4.png"
#define POWERSTAT_ICON_BATTERY5 "~/Immagini/perapp/battery-5.png"
#define POWERSTAT_ICON_CHARGING0 "~/Immagini/perapp/battery-charging-0.png"
#define POWERSTAT_ICON_CHARGING1 "~/Immagini/perapp/battery-charging-1.png"
#define POWERSTAT_ICON_CHARGING2 "~/Immagini/perapp/battery-charging-2.png"
#define POWERSTAT_ICON_CHARGING3 "~/Immagini/perapp/battery-charging-3.png"
#define POWERSTAT_ICON_CHARGING4 "~/Immagini/perapp/battery-charging-4.png"
#define POWERSTAT_ICON_CHARGING5 "~/Immagini/perapp/battery-charging-5.png"
#define POWERSTAT_ICON_MISSING "~/Immagini/perapp/battery-missing.png"
//hide/show gadget
#define GADGET_HIDE 0
//device of battey
#define POWERSTAT_DEVICE "BAT0"
//load gadget
#include "powerstat.h"

/***************/
/*** network ***/
/***************/

//create new wireless gadget
#define GADGET_NAME wifi
//set background colors
#define GADGET_BACKGROUND BACKGROUND
//set foreground colors
#define GADGET_FOREGROUND FOREGROUND
//se border color on mouse in, only if BORDER_SIZE > 0
#define GADGET_HOVER_IN LBLUE
//se border color on mouse out, only if BORDER_SIZE > 0
#define GADGET_HOVER_OUT BLUE
//align 
#define GADGET_ALIGNED (VBAR_ALIGNED_RIGHT)
//time to refresh in ms
#define GADGET_INTERVAL 1000
//text to view
#define GADGET_TEXT " %s ^%6.2f%s v%6.2f%s", essid, speedDownload, unitDownload, speedUpload, unitUpload
//enable graphical png icon, __N__ for disable
#define GADGET_ICON __Y__
#define NETWORK_ICON_DISCONNECT "~/Immagini/perapp/wifi-no-route.png"
#define NETWORK_ICON_POWER0 "~/Immagini/perapp/wifi0.png"
#define NETWORK_ICON_POWER1 "~/Immagini/perapp/wifi1.png"
#define NETWORK_ICON_POWER2 "~/Immagini/perapp/wifi2.png"
#define NETWORK_ICON_POWER3 "~/Immagini/perapp/wifi3.png"
#define NETWORK_ICON_POWER4 "~/Immagini/perapp/wifi4.png"
//click on gadget generate refresh event
#define GADGET_CLICK_SAME_REFRESH __N__
//hide/show gadget
#define GADGET_HIDE 0
//set device for network
#define NETWORK_DEVICE "wlp3s0"
//set unit base, 1000, 1026
#define NETWORK_UNIT 1024
//load gadget
#include "network.h"

/*****************************/
/*** order gadget and init ***/
/*****************************/

//after define gadget you need to load for display on bar.
//the order of LOAD is same where is positioning on the bar

init_f initFnc[] = {
	LOAD(workspace, works),
	LOAD(clock, clk),
	LOAD(label, povermenu),
	LOAD(script, poweroff),
	LOAD(script, reboot),
	LOAD(powerstat, battery),
	LOAD(network, wifi),
	NULL //not remove this
};

/*********************************/
/*** you not need to edit this ***/
/*********************************/

void vbar_main(void){
	//get vbar object
	vbar = vbar_get();
	
	//set monitor
	vbar_monitor_set(vbar, MONITOR);
	
	//set fonts
	struct fontssize{ const char* name; size_t size; } fontl[] = FONTS;
	for( size_t i = 0; fontl[i].name; ++i ){
		vbar_fonts_set(vbar, fontl[i].name, fontl[i].size);
	}
	
	//set background and foreground
	vbar_colors_set(vbar, BACKGROUND, FOREGROUND);
	
	//set size
	vbar_height_set(vbar, HEIGHT, SPACING_TOP, SPACING_BOTTOM);
	
	//set position
	vbar_topbar_set(vbar, BARPOSITION);

	//set border size
	#if defined(BORDER_SIZE) > 0
		vbar_border_width_set(vbar, BORDER_SIZE);
	#endif

	for( size_t i = 0; initFnc[i]; ++i ){
		initFnc[i]();
	}
	
}

/*


void error_event(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
		case GADGET_EVENT_REFRESH:
			gadget_text(g, " %s ", gadget_label_get(g));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;
	
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_CLICK:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void cpu_event(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
		case GADGET_EVENT_REFRESH:
			gadget_text(g, " 💻 %6.2lf%% ", gadget_cpu_load_average(g, 0));
			//gadget_text(g, "%6.2lf%%", gadget_cpu_load_average(g, 0));

			gadget_redraw(g);
		break;

		case GADGET_EVENT_EXTEND_REFRESH:
			gadget_extend_label(g, 1, " 💻[1] %6.2lf%%", gadget_cpu_load_average(g, 1) );
			gadget_extend_label(g, 1, " 💻[2] %6.2lf%%", gadget_cpu_load_average(g, 2) );
			gadget_extend_label(g, 1, " 💻[3] %6.2lf%%", gadget_cpu_load_average(g, 3) );
			gadget_extend_label(g, 1, " 💻[4] %6.2lf%%", gadget_cpu_load_average(g, 4) );
			gadget_extend_label(g, 2, " 💻[1] %1.2lfGhz", gadget_cpufreq_fq_get(0, 0) / 1000000.0 );
			gadget_extend_label(g, 2, " 💻[2] %1.2lfGhz", gadget_cpufreq_fq_get(0, 1) / 1000000.0 );
			gadget_extend_label(g, 2, " 💻[3] %1.2lfGhz", gadget_cpufreq_fq_get(0, 2) / 1000000.0 );
			gadget_extend_label(g, 2, " 💻[4] %1.2lfGhz", gadget_cpufreq_fq_get(0, 3) / 1000000.0 );
			for(int i = 0; i < 4; ++i){
				char* tmp = gadget_cpufreq_current_governor_get(i);
				gadget_extend_label(g, 3, " 💻[%1d] %s", i, tmp );
				gadget_cpufreq_current_governor_free(tmp);
			}
			gadget_extend_redraw(g);
		break;

		case GADGET_EVENT_EXTEND_OPEN:
			gadget_extend_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_CLICK:
			gadget_extend_toggle(g);
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void cpufreq_event(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
		case GADGET_EVENT_REFRESH:
		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_MOUSE_CLICK:
		case GADGET_EVENT_MOUSE_ENTER:
		case GADGET_EVENT_MOUSE_LEAVE:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void mem_event(gadget_s* g, gadgetEventType_e event, void* arg){
	#define UNIT MIB
	switch( event ){
		case GADGET_EVENT_REFRESH:
			gadget_text(g, " 🐏 %.2lf GiB ", (double)gadget_memory_used_get(g)/UNIT);
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_MOUSE_CLICK:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
			system("notify-send -a \"hello\" world");
		break;
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void alsa_event(gadget_s* g, gadgetEventType_e event, void* arg){
	#define ASTEPV 15
	switch( event ){
		case GADGET_EVENT_REFRESH:{
			long max = gadget_alsa_volume_max_get(g);
			long volume = gadget_alsa_volume_get(g);
			gadget_text(g, " alsa ➖ %3ld%% ➕ ", (100*volume)/max);
			gadget_redraw(g);
		}
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_CLICK:{
			vbarMouse_s* event = arg;
			//printf("X:%u Y%u\n", event->x, event->y);
			long volume = gadget_alsa_volume_get(g);
			long max = gadget_alsa_volume_max_get(g);
			long step = max / ASTEPV;
			//printf("volume:%ld ", volume);
			if( event->x > 75 && event->x < 90 ){
				//printf("decrease %ld", volume - step);
				gadget_alsa_volume_set(g, event->button == 1 ? volume - step : 0);
			}
			else if( event->x > 125 && event->x < 135 ){
				//printf("increase %ld", volume + step);
				gadget_alsa_volume_set(g, event->button == 1 ? volume + step : max);
			}
			//printf("\n");
		}
		break;

		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void pulse_event(gadget_s* g, gadgetEventType_e event, void* arg){
	#define PSTEPV 15
	switch( event ){
		case GADGET_EVENT_REFRESH:{
			int volume = gadget_pulseaudio_volume_get(g);
			gadget_text(g, " pulse ➖ %3ld%% ➕ ", volume);
			gadget_redraw(g);
		}
		break;

		case GADGET_EVENT_MOUSE_ENTER:
			gadget_border_color(g, rgb(30,30,220));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
			gadget_border_color(g, rgb(220,30,30));
			gadget_redraw(g);
		break;

		case GADGET_EVENT_MOUSE_CLICK:{
			vbarMouse_s* event = arg;
			size_t mst = gadget_text_lenght(g, " pulse ");
			size_t men = gadget_text_lenght(g, "➖") + mst;
			size_t pst = gadget_text_lenght(g, " 000% ") + men;
			size_t pen = gadget_text_lenght(g, "➕") + pst;
			//printf("X:%u Y%u\n", event->x, event->y);
			if( event->x > mst && event->x < men ){
				//printf("delta %d", -PSTEPV);
				gadget_pulseaudio_volume_delta(g, -PSTEPV);
			}
			else if( event->x > pst && event->x < pen ){
				//printf("delta %d", PSTEPV);
				gadget_pulseaudio_volume_delta(g, PSTEPV);
			}
			//printf("\n");
		}
		break;

		case GADGET_EVENT_EXTEND_REFRESH:
		case GADGET_EVENT_EXTEND_OPEN:
		case GADGET_EVENT_EXTEND_CLOSE:
		case GADGET_EVENT_MOUSE_RELEASE:
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_MOVE:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		break;
	}
}

void vbar_main(void){

	alsa = gadget_new(vbar, "alsa", "all");
	gadget_event_register(alsa, alsa_event);	
	gadget_align(alsa, VBAR_ALIGNED_RIGHT);
	gadget_border_color(alsa, rgb(220,30,30));
	gadget_border(alsa, VBAR_BORDER_BOTTOM);
	gadget_alsa_connect(alsa);
	gadget_interval(alsa, 0);
	gadget_start(alsa);

	pulse = gadget_new(vbar, "pulseaudio", "all");
	gadget_event_register(pulse, pulse_event);	
	gadget_align(pulse, VBAR_ALIGNED_RIGHT);
	gadget_border_color(pulse, rgb(220,30,30));
	gadget_border(pulse, VBAR_BORDER_BOTTOM);
	gadget_pulseaudio_connect(pulse);
	gadget_interval(pulse, 0);
	gadget_start(pulse);

	mem = gadget_new(vbar, "memory", "all");
	gadget_event_register(mem, mem_event);	
	gadget_align(mem, VBAR_ALIGNED_RIGHT);
	gadget_border_color(mem, rgb(220,30,30));
	gadget_border(mem, VBAR_BORDER_BOTTOM);
	gadget_interval(mem, 2000);
	gadget_start(mem);

	cpu = gadget_new(vbar, "cpu", "all");
	gadget_event_register(cpu, cpu_event);	
	//gadget_align(cpu, VBAR_ALIGNED_LEFT);
	//gadget_align(cpu, VBAR_ALIGNED_CENTER);
	gadget_align(cpu, VBAR_ALIGNED_RIGHT);
	gadget_border_color(cpu, rgb(220,30,30));
	gadget_border(cpu, VBAR_BORDER_BOTTOM);
	//gadget_background(cpu, rgb(50,150,150));
	gadget_interval(cpu, 900);
	gadget_extend_enable(cpu, 4);
	gadget_extend_background(cpu, 0, rgb(10,10,55));
	gadget_start(cpu);

	cpufreq = gadget_new(vbar, "cpufreq", "all");
	gadget_event_register(cpufreq, cpufreq_event);	
	gadget_interval(cpufreq, 0);
	gadget_start(cpufreq);

	if( !(test = gadget_new(vbar, "return null", "all")) ){
		error = gadget_new(vbar, "label", "all");
		gadget_event_register(error, error_event);	
		gadget_align(error, VBAR_ALIGNED_LEFT);
		gadget_border_color(error, rgb(220,30,30));
		gadget_border(error, VBAR_BORDER_BOTTOM);
		gadget_background(error, rgb(100,10,10));
		gadget_label_set(error, "error: on create gadget");
		gadget_interval(error, 1000);
		gadget_start(error);
	}

	//gadget_colors_set(clock, BACKGROUND, FOREGROUND");
	//gadget_interval_set(clock, 60000);
	//gadget_events_set(clock, clock_event);
}
*/
