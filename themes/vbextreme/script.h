#include <vbarScript.h>

//global gadget
static gadget_s* DNAME(scr_, GADGET_NAME);

static void DNAME(script_events_,GADGET_NAME)(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
		#if GADGET_CLICK_SAME_REFRESH > 0
		case GADGET_EVENT_MOUSE_CLICK:
			gadget_text_reset(g);
			#if SCRIPT_ONLY_ONCLICK > 0
				#if SCRIPT_ARG_EVENT > 0
					gadget_script_shell_event(g, SCRIPT_CMD, event, arg);
				#else
					gadget_script_shell_event(g, SCRIPT_CMD, -1, arg);
				#endif
				gadget_text(g, "%s", GADGET_TEXT);
				#if SCRIPT_TXT_SLURPED > 0
					const char* txt = gadget_script_txt(g);
					if( txt ) gadget_text(g, "%s", txt);
				#endif
				gadget_redraw(g);
			#endif
		break;
		#endif

		case GADGET_EVENT_REFRESH:
			#if SCRIPT_ONLY_ONCLICK == 0
				#if SCRIPT_ARG_EVENT > 0
					gadget_script_shell_event(g, SCRIPT_CMD, event, arg);
				#else
					gadget_script_shell_event(g, SCRIPT_CMD, -1, arg);
				#endif
			#endif
			gadget_text(g, "%s", GADGET_TEXT);
			#if SCRIPT_ONLY_ONCLICK == 0
				#if SCRIPT_TXT_SLURPED > 0
					const char* txt = gadget_script_txt(g);
					if( txt ) gadget_text(g, "%s", txt);
				#endif
			#endif
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
	}	
}

static void DNAME(script_init_, GADGET_NAME)(){
	DNAME(scr_, GADGET_NAME) = gadget_new(vbar, "script", SNAME(GADGET_NAME));
	gadget_s* g = DNAME(scr_, GADGET_NAME);
	gadget_event_register(g, DNAME(script_events_, GADGET_NAME));	
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
#undef SCRIPT_PATH
#undef SCRIPT_CMD
#undef SCRIPT_ARG_EVENT
#undef SCRIPT_TXT_SLURPED
#undef SCRIPT_ONLY_ONCLICK
