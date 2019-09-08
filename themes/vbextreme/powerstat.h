#include <vbarScript.h>

//global gadget
static gadget_s* DNAME(bat_, GADGET_NAME);

#define STATUS_BLINK_NORMAL   0x0
#define STATUS_BLINK_ON       0x1
#define STATUS_BLINK_OFF      0x2
#define STATUS_BLINK_GET(V)   ((V) & 0x3)
#define STATUS_BLINK_SET(V,M) (((V) & (~0x3)) | (M))
#define STATUS_EHORI_OFF      0x00
#define STATUS_EHORI_ON       0x01
#define STATUS_EHORI_GET(V)   (((V)>>2) & 0x1)
#define STATUS_EHORI_SET(V,M) (((V) & (~(0x1<<2))) | ((M)<<2))


static void DNAME(powerstat_events_,GADGET_NAME)(gadget_s* g, gadgetEventType_e event, void* arg){
	static char* iconName[] = {
		SNAME(GADGET_NAME) "battery-0",
		SNAME(GADGET_NAME) "battery-1",
		SNAME(GADGET_NAME) "battery-2",
		SNAME(GADGET_NAME) "battery-3",
		SNAME(GADGET_NAME) "battery-4",
		SNAME(GADGET_NAME) "battery-5",
		SNAME(GADGET_NAME) "battery-charging-0",
		SNAME(GADGET_NAME) "battery-charging-1",
		SNAME(GADGET_NAME) "battery-charging-2",
		SNAME(GADGET_NAME) "battery-charging-3",
		SNAME(GADGET_NAME) "battery-charging-4",
		SNAME(GADGET_NAME) "battery-charging-5",
		SNAME(GADGET_NAME) "battery-missing"
	};
	
	switch( event ){
	
		case GADGET_EVENT_REFRESH:{
			int capacity = gadget_powerstat_capacity_get(g);
			int hours = gadget_powerstat_timeleft_hours_get(g);
			int minutes = gadget_powerstat_timeleft_minutes_get(g);
			double voltageNow = gadget_powerstat_voltage_now_get(g, 1000000);
			double voltageMin = gadget_powerstat_voltage_min_get(g, 1000000);
			double energyFull = gadget_powerstat_energy_full_get(g, 1000000);
			double energyNow = gadget_powerstat_energy_now_get(g, 1000000);
			const char* batteryStatus = gadget_powerstat_status_get(g);
			switch( STATUS_EHORI_GET(gadget_status_get(g)) ){
				default: case 0:
					gadget_text(g, GADGET_TEXT);
				break;
				case 1:
					gadget_text(g, GADGET_TEXT_EHORI);
				break;
			}
			#if GADGET_ICON != 0
				unsigned idcon = 12;		
				if( batteryStatus ){
					if( !strcmp(batteryStatus, "Full") ) idcon = 5;
					else if( !strcmp(batteryStatus, "Charging") ) idcon = 6 + ( capacity / 20 );
					else if( !strcmp(batteryStatus, "Discharging") ) idcon = 0 + ( capacity / 20 );
				}
				if( idcon > 12 ) idcon = 12;
				gadget_icon(g, iconName[idcon]);
			#endif
			#if GADGET_BLINK > 0
				int status = gadget_status_get(g);
				if( capacity < GADGET_BLINKON ){
					switch( STATUS_BLINK_GET(status) ){
						case STATUS_BLINK_NORMAL:
							gadget_change_interval(g, GADGET_BLINK);
						case STATUS_BLINK_ON:
							#if GADGET_BLINK_MODE == 0
								gadget_background(g, GADGET_BLINK_BACKGROUND);
								gadget_foreground(g, GADGET_BLINK_FOREGROUND);
							#else
								gadget_border_color(g, GADGET_BLINK_BACKGROUND);
							#endif
							gadget_status_set(g,STATUS_BLINK_SET(status, STATUS_BLINK_OFF));
						break;
						case STATUS_BLINK_OFF:
							#if GADGET_BLINK_MODE == 0
								gadget_background(g, GADGET_BACKGROUND);
								gadget_foreground(g, GADGET_FOREGROUND);
							#else
								gadget_border_color(g, GADGET_HOVER_OUT);
							#endif
							gadget_status_set(g,STATUS_BLINK_SET(status, STATUS_BLINK_ON));
						break;
					}
				}else if( STATUS_BLINK_GET(status) != STATUS_BLINK_NORMAL ){
					gadget_change_interval(g, GADGET_INTERVAL);
					gadget_status_set(g, STATUS_BLINK_SET(status, STATUS_BLINK_NORMAL));
				}
			#elif GADGET_LOW_LEVEL
				if( capacity < GADGET_BLINKON ){
					gadget_background(g, GADGET_BLINK_BACKGROUND);
					gadget_foreground(g, GADGET_BLINK_FOREGROUND);
				}
				else{
					gadget_background(g, GADGET_BACKGROUND);
					gadget_foreground(g, GADGET_FOREGROUND);
				}
			#endif

			gadget_redraw(g);
		}
		break;

		#if GADGET_EXPAND_VERT > 0
		case GADGET_EVENT_EXTEND_REFRESH:
		#if GADGET_EXTEND_LINE_COUNT > 0
			gadget_extend_label(g, 0, GADGET_TEXT_EXTEND_0 );
		#endif
		#if GADGET_EXTEND_LINE_COUNT > 1
			gadget_extend_label(g, 1, GADGET_TEXT_EXTEND_1 );
		#endif
		#if GADGET_EXTEND_LINE_COUNT > 2
			gadget_extend_label(g, 2, GADGET_TEXT_EXTEND_2 );
		#endif
		#if GADGET_EXTEND_LINE_COUNT > 3
			gadget_extend_label(g, 3, GADGET_TEXT_EXTEND_3 );
		#endif
		#if GADGET_EXTEND_LINE_COUNT > 4
			gadget_extend_label(g, 4, GADGET_TEXT_EXTEND_4 );
		#endif
		#if GADGET_EXTEND_LINE_COUNT > 5
			gadget_extend_label(g, 5, GADGET_TEXT_EXTEND_5 );
		#endif
		#if GADGET_EXTEND_LINE_COUNT > 6
			gadget_extend_label(g, 6, GADGET_TEXT_EXTEND_6 );
		#endif
		#if GADGET_EXTEND_LINE_COUNT > 7
			gadget_extend_label(g, 7, GADGET_TEXT_EXTEND_7 );
		#endif
		#if GADGET_EXTEND_LINE_COUNT > 8
			gadget_extend_label(g, 8, GADGET_TEXT_EXTEND_8 );
		#endif
		#if GADGET_EXTEND_LINE_COUNT > 9
			gadget_extend_label(g, 9, GADGET_TEXT_EXTEND_9 );
		#endif
			gadget_extend_redraw(g);
		break;
	
		case GADGET_EVENT_EXTEND_OPEN:
			gadget_change_interval(g, GADGET_EXPAND_VERT);
			gadget_extend_redraw(g);
		break;

		case GADGET_EVENT_EXTEND_CLOSE:
			gadget_change_interval(g, GADGET_INTERVAL);
			gadget_redraw(g);
		break;
		#endif
		case GADGET_EVENT_MOUSE_ENTER:
		#if BORDER_SIZE > 0
			gadget_border_color(g, GADGET_HOVER_IN);
			gadget_redraw(g);
		#endif
		break;

		case GADGET_EVENT_MOUSE_LEAVE:
		#if BORDER_SIZE > 0
			gadget_border_color(g, GADGET_HOVER_OUT);
			gadget_redraw(g);
		#endif
		break;

		case GADGET_EVENT_MOUSE_CLICK:
		#if GADGET_EXPAND_HORI > 0 || GADGET_EXPAND_VERT > 0
		{
			vbarMouse_s* mouse = arg;
			#if GADGET_EXPAND_HORI > 0
			if( mouse->button == 1 ){
				int status = gadget_status_get(g);
				if( STATUS_EHORI_GET(status) == STATUS_EHORI_OFF )
					gadget_status_set(g, STATUS_EHORI_SET(status, STATUS_EHORI_ON));
				else
					gadget_status_set(g, STATUS_EHORI_SET(status, STATUS_EHORI_OFF));
				gadget_text_reset(g);
				DNAME(powerstat_events_,GADGET_NAME)(g, GADGET_EVENT_REFRESH, NULL);
			}
			#endif
			#if GADGET_EXPAND_VERT > 0
			if( mouse->button == 3 ){
				gadget_extend_toggle(g);
			}	
			#endif
		}
		#endif
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		case GADGET_EVENT_MOUSE_RELEASE:
		#if GADGET_CLICK_EFFECT > 0
			gadget_background(g, GADGET_BUTTON_UP);
			gadget_redraw(g);
		#endif
		break;
	}
}

static void DNAME(powerstat_init_, GADGET_NAME)(){
	DNAME(bat_, GADGET_NAME) = gadget_new(vbar, "powerstat", SNAME(GADGET_NAME));
	gadget_s* g = DNAME(bat_, GADGET_NAME);
	gadget_event_register(g, DNAME(powerstat_events_, GADGET_NAME));	
	gadget_align(g, GADGET_ALIGNED);
	gadget_background(g, GADGET_BACKGROUND);
	gadget_foreground(g, GADGET_FOREGROUND);
	#if BORDER_SIZE > 0
		gadget_border_color(g, GADGET_HOVER_OUT);
		gadget_border(g, VBAR_BORDER_BOTTOM);
	#endif	
	#if GADGET_ICON != 0
		vbar_icon_load(vbar, POWERSTAT_ICON_BATTERY0, SNAME(GADGET_NAME) "battery-0");
		vbar_icon_load(vbar, POWERSTAT_ICON_BATTERY1, SNAME(GADGET_NAME) "battery-1");
		vbar_icon_load(vbar, POWERSTAT_ICON_BATTERY2, SNAME(GADGET_NAME) "battery-2");
		vbar_icon_load(vbar, POWERSTAT_ICON_BATTERY3, SNAME(GADGET_NAME) "battery-3");
		vbar_icon_load(vbar, POWERSTAT_ICON_BATTERY4, SNAME(GADGET_NAME) "battery-4");
		vbar_icon_load(vbar, POWERSTAT_ICON_BATTERY5, SNAME(GADGET_NAME) "battery-5");
		vbar_icon_load(vbar, POWERSTAT_ICON_CHARGING0, SNAME(GADGET_NAME) "battery-charging-0");
		vbar_icon_load(vbar, POWERSTAT_ICON_CHARGING1, SNAME(GADGET_NAME) "battery-charging-1");
		vbar_icon_load(vbar, POWERSTAT_ICON_CHARGING2, SNAME(GADGET_NAME) "battery-charging-2");
		vbar_icon_load(vbar, POWERSTAT_ICON_CHARGING3, SNAME(GADGET_NAME) "battery-charging-3");
		vbar_icon_load(vbar, POWERSTAT_ICON_CHARGING4, SNAME(GADGET_NAME) "battery-charging-4");
		vbar_icon_load(vbar, POWERSTAT_ICON_CHARGING5, SNAME(GADGET_NAME) "battery-charging-5");
		vbar_icon_load(vbar, POWERSTAT_ICON_MISSING, SNAME(GADGET_NAME) "battery-missing");
	#endif
	#if GADGET_EXPAND_VERT > 0 
		gadget_extend_enable(g, GADGET_EXTEND_LINE_COUNT);
	#endif
	gadget_powerstat_device_set(g, POWERSTAT_DEVICE);
	gadget_status_set(g,0);
	gadget_interval(g, GADGET_INTERVAL);
	gadget_start(g);
	gadget_hide(g, GADGET_HIDE);
}

#undef GADGET_NAME
#undef GADGET_BACKGROUND
#undef GADGET_FOREGROUND
#undef GADGET_HOVER_IN
#undef GADGET_HOVER_OUT 
#undef GADGET_ALIGNED
#undef GADGET_INTERVAL
#undef GADGET_TEXT
#undef GADGET_EXPAND_HORI
#undef GADGET_TEXT_EHORI
#undef GADGET_EXPAND_VERT
#undef GADGET_EXTEND_LINE_COUNT
#undef GADGET_TEXT_EXTEND_0
#undef GADGET_TEXT_EXTEND_1
#undef GADGET_TEXT_EXTEND_2
#undef GADGET_TEXT_EXTEND_3
#undef GADGET_TEXT_EXTEND_4
#undef GADGET_TEXT_EXTEND_5
#undef GADGET_TEXT_EXTEND_6
#undef GADGET_TEXT_EXTEND_7
#undef GADGET_TEXT_EXTEND_8
#undef GADGET_TEXT_EXTEND_9
#undef GADGET_ICON
#undef GADGET_ICON_NAME
#undef GADGET_CLICK_SAME_REFRESH
#undef GADGET_HIDE
#undef GADGET_BLINK
#undef GADGET_BLINK_MODE
#undef GADGET_BLINK_BACKGROUND
#undef GADGET_BLINK_FOREGROUND
#undef GADGET_LOW_LEVEL
#undef GADGET_BLINKON
#undef POWERSTAT_DEVICE
#undef POWERSTAT_ICON_BATTERY0
#undef POWERSTAT_ICON_BATTERY1
#undef POWERSTAT_ICON_BATTERY2
#undef POWERSTAT_ICON_BATTERY3
#undef POWERSTAT_ICON_BATTERY4
#undef POWERSTAT_ICON_BATTERY5
#undef POWERSTAT_ICON_CHARGING0
#undef POWERSTAT_ICON_CHARGING1
#undef POWERSTAT_ICON_CHARGING2
#undef POWERSTAT_ICON_CHARGING3
#undef POWERSTAT_ICON_CHARGING4
#undef POWERSTAT_ICON_CHARGING5
#undef POWERSTAT_ICON_MISSING
#undef STATUS_BLINK_NORMAL
#undef STATUS_BLINK_ON
#undef STATUS_BLINK_OFF
#undef STATUS_BLINK_GET
#undef STATUS_BLINK_SET
#undef STATUS_EHORI_OFF
#undef STATUS_EHORI_ON
#undef STATUS_EHORI_GET
#undef STATUS_EHORI_SET


