#include <vbarScript.h>

//global gadget
static gadget_s* DNAME(net_, GADGET_NAME);

static void DNAME(network_events_,GADGET_NAME)(gadget_s* g, gadgetEventType_e event, void* arg){
	static char* iconName[] = {
		SNAME(GADGET_NAME) "power0",
		SNAME(GADGET_NAME) "power1",
		SNAME(GADGET_NAME) "power2",
		SNAME(GADGET_NAME) "power3",
		SNAME(GADGET_NAME) "power4",
		SNAME(GADGET_NAME) "disconnect"
	};
	
	switch( event ){
	
		case GADGET_EVENT_REFRESH:{
			double speedDownload;
			double speedUpload;
			const char* unitDownload = gadget_tohuman(&speedDownload, gadget_network_receive_speed(g), 0, NETWORK_UNIT);
			const char* unitUpload = gadget_tohuman(&speedUpload, gadget_network_transmit_speed(g), 0, NETWORK_UNIT);
			const char* essid = gadget_network_essid_get(g);
			int decibel = gadget_network_dbm_get(g);
			if( !unitDownload ) unitDownload = "--";
			if( !unitUpload ) unitUpload = "--";
			if( !essid ) essid = "";
			gadget_text(g, GADGET_TEXT);
			#if GADGET_ICON != 0
				unsigned idcon = 5;
				if( *essid ){
					idcon = (decibel / 25) + 4;
					if( idcon > 4 ) idcon = 4;
					if( idcon < 0 ) idcon = 0;
				}
				gadget_icon(g, iconName[idcon]);
			#endif
			gadget_redraw(g);
		}
		break;

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

		#if GADGET_CLICK_EFFECT > 0
		case GADGET_EVENT_MOUSE_PRESS:
		case GADGET_EVENT_MOUSE_CLICK:
		case GADGET_EVENT_MOUSE_DBLCLICK:
		case GADGET_EVENT_MOUSE_RELEASE:
			gadget_background(g, GADGET_BUTTON_UP);
			gadget_redraw(g);
		break;
		#endif
	}	
}

static void DNAME(network_init_, GADGET_NAME)(){
	DNAME(net_, GADGET_NAME) = gadget_new(vbar, "network", SNAME(GADGET_NAME));
	gadget_s* g = DNAME(net_, GADGET_NAME);
	gadget_event_register(g, DNAME(network_events_, GADGET_NAME));	
	gadget_align(g, GADGET_ALIGNED);
	gadget_background(g, GADGET_BACKGROUND);
	gadget_foreground(g, GADGET_FOREGROUND);
	#if BORDER_SIZE > 0
		gadget_border_color(g, GADGET_HOVER_OUT);
		gadget_border(g, VBAR_BORDER_BOTTOM);
	#endif	
	#if GADGET_ICON != 0
		vbar_icon_load(vbar, NETWORK_ICON_DISCONNECT, SNAME(GADGET_NAME) "disconnect");
		vbar_icon_load(vbar, NETWORK_ICON_POWER0, SNAME(GADGET_NAME) "power0");
		vbar_icon_load(vbar, NETWORK_ICON_POWER1, SNAME(GADGET_NAME) "power1");
		vbar_icon_load(vbar, NETWORK_ICON_POWER2, SNAME(GADGET_NAME) "power2");
		vbar_icon_load(vbar, NETWORK_ICON_POWER3, SNAME(GADGET_NAME) "power3");
		vbar_icon_load(vbar, NETWORK_ICON_POWER4, SNAME(GADGET_NAME) "power4");
		gadget_icon(g, SNAME(GADGET_NAME)"disconnect");
	#endif
	gadget_network_device_set(g, NETWORK_DEVICE);
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
#undef GADGET_ICON
#undef GADGET_ICON_NAME
#undef GADGET_CLICK_SAME_REFRESH
#undef GADGET_HIDE
#undef NETWORK_DEVICE
#undef NETWORK_UNIT
#undef NETWORK_ICON_DISCONNECT
#undef NETWORK_ICON_POWER0
#undef NETWORK_ICON_POWER1
#undef NETWORK_ICON_POWER2
#undef NETWORK_ICON_POWER3
#undef NETWORK_ICON_POWER4

