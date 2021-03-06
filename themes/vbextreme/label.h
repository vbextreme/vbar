#include <vbarScript.h>

//global gadget
static gadget_s* DNAME(lbl_, GADGET_NAME);

static void DNAME(label_events_,GADGET_NAME)(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
	
		case GADGET_EVENT_REFRESH:
			gadget_text(g, "%s", gadget_label_get(g));
			gadget_redraw(g);
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
			gadget_background(g, GADGET_BUTTON_DOWN);
			gadget_redraw(g);
		break;
		#endif
		
		#if GADGET_CLICK_EFFECT > 0 || LABEL_ACTION_ENABLE > 0
		case GADGET_EVENT_MOUSE_CLICK:
		#endif
		#if LABEL_ACTION_ENABLE > 0
			LABEL_ACTION
		#endif
		#if GADGET_CLICK_EFFECT > 0
		case GADGET_EVENT_MOUSE_DBLCLICK:
		case GADGET_EVENT_MOUSE_RELEASE:
			gadget_background(g, GADGET_BUTTON_UP);
			gadget_redraw(g);
		#endif
		#if GADGET_CLICK_EFFECT || LABEL_ACTION_ENABLE
		break;
		#endif
	}	
}

static void DNAME(label_init_, GADGET_NAME)(){
	DNAME(lbl_, GADGET_NAME) = gadget_new(vbar, "label", SNAME(GADGET_NAME));
	gadget_s* g = DNAME(lbl_, GADGET_NAME);
	gadget_event_register(g, DNAME(label_events_, GADGET_NAME));	
	gadget_align(g, GADGET_ALIGNED);
	gadget_background(g, GADGET_BACKGROUND);
	gadget_foreground(g, GADGET_FOREGROUND);
	#if BORDER_SIZE > 0
		gadget_border_color(g, GADGET_HOVER_OUT);
		gadget_border(g, VBAR_BORDER_BOTTOM);
	#endif	
	#if GADGET_ICON != 0
		vbar_icon_load(vbar, GADGET_ICON_NAME, SNAME(GADGET_NAME));
		gadget_icon(g, SNAME(GADGET_NAME));
	#endif
	gadget_label_set(g, GADGET_TEXT);
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
#undef LABEL_ACTION_ENABLE
#undef LABEL_ACTION

