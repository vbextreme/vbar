
//global vector gadget
static gadget_s** DNAME(ws_, GADGET_NAME);

//workspace count;
static size_t DNAME(ws_count_, GADGET_NAME);;

#ifndef WORKSPACE_GADGET_H
#define WORKSPACE_GADGET_H
struct workspaceorder{
	char* name;
	int id;
};

int workspace_cmp(const void* a, const void* b){
	struct workspaceorder* wa = (struct workspaceorder*)a;
	struct workspaceorder* wb = (struct workspaceorder*)b;
	if( !wa->name || ! wb->name ) return 0;
	char* en = NULL;
	long na = strtol(wa->name, &en, 10);
	if( na == 0 && en == wa->name ){
		return strcmp(wa->name, wb->name);
	}
	long nb = strtol(wa->name, &en, 10);
	if( nb == 0 && en == wb->name ){
		return strcmp(wa->name, wb->name);
	}
	return na - nb;
}
#endif

static void DNAME(workspace_set_, GADGET_NAME)(int redraw){
	gadget_s** g = DNAME(ws_, GADGET_NAME);
	size_t count = gadget_workspace_count(g[0]);
	size_t active = gadget_workspace_active(g[0]);

	if( count == 0 || active > count){
		gadget_text(g[0], "workspace error");
		return;
	}

	if( count > 1 && count > DNAME(ws_count_, GADGET_NAME) ){
		DNAME(ws_, GADGET_NAME) = realloc(DNAME(ws_, GADGET_NAME), sizeof(gadget_s*) * count );
		g = DNAME(ws_, GADGET_NAME);
		for(size_t i = DNAME(ws_count_, GADGET_NAME); i < count; ++i){
			char name[12];
			sprintf(name, "%lu", i);
			g[i] = gadget_new(vbar, "workspace", name);
			#if BORDER_SIZE > 0
				gadget_border(g[i], VBAR_BORDER_BOTTOM);
			#endif
		}
		DNAME(ws_count_, GADGET_NAME) = count;
	}
	char** names = gadget_workspace_names(g[0]);
	struct workspaceorder* win = (struct workspaceorder*)malloc(sizeof(struct workspaceorder) * count);
	for( size_t i = 0; i < count; ++i){
		win[i].name = names[i];
		win[i].id = i;
	}
	#if WORKSPACE_ORDER > 0
		qsort(win, count, sizeof(struct workspaceorder), workspace_cmp);
	#endif	
	
	for( size_t i = 0; i < count; ++i){
		gadget_hide(g[i], 0);
		gadget_text_reset(g[i]);
		gadget_text(g[i], " %s ", win[i].name);
		gadget_status_set(g[i], win[i].id);
		gadget_background(g[i], win[i].id==(int)active ? WORKSPACE_ACTIVE_BACKGROUND : GADGET_BACKGROUND);
		gadget_foreground(g[i], win[i].id==(int)active ? WORKSPACE_ACTIVE_FOREGROUND : GADGET_FOREGROUND);
		#if GADGET_ICON != 0
			gadget_icon(g[i], NULL);
		#endif
		#if BORDER_SIZE > 0
			gadget_border_color(g[i], GADGET_HOVER_OUT);
		#endif
		if(redraw) gadget_redraw(g[i]);
	}
	for( size_t i = count; i < DNAME(ws_count_, GADGET_NAME) ; ++i){
		gadget_hide(g[i], 1);
		if(redraw) gadget_redraw(g[i]);
		#if GADGET_ICON != 0
			gadget_icon(g[i], NULL);
		#endif
	}
	#if GADGET_ICON != 0
		gadget_icon(g[0], SNAME(GADGET_NAME));
		if(redraw) gadget_redraw(g[0]);
	#endif

	gadget_workspace_names_free(names);
	free(win);
}

static void DNAME(workspace_events_,GADGET_NAME)(gadget_s* g, gadgetEventType_e event, void* arg){
	switch( event ){
		case GADGET_EVENT_REFRESH:
			if( gadget_status_get(g) != 0 ) break;
			DNAME(workspace_set_, GADGET_NAME)(1);
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

		case GADGET_EVENT_MOUSE_CLICK:
			gadget_workspace_activate(g, gadget_status_get(g));
		break;

	}
}

static void DNAME(workspace_init_, GADGET_NAME)(){
	DNAME(ws_count_, GADGET_NAME) = 1;
	DNAME(ws_, GADGET_NAME) = (gadget_s**)malloc(sizeof(gadget_s*));
	DNAME(ws_, GADGET_NAME)[0] = gadget_new(vbar, "workspace", "0");
	DNAME(workspace_set_, GADGET_NAME)(0);
	gadget_s** g = DNAME(ws_, GADGET_NAME);
	for( size_t i = 0; i < DNAME(ws_count_, GADGET_NAME); ++i){
		gadget_event_register(g[i], DNAME(workspace_events_, GADGET_NAME));	
		gadget_align(g[i], GADGET_ALIGNED);
		#if BORDER_SIZE > 0
			gadget_border_color(g[i], GADGET_HOVER_OUT);
			gadget_border(g[i], VBAR_BORDER_BOTTOM);
		#endif
		gadget_interval(g[i], 0);
		gadget_start(g[i]);
	}
	gadget_workspace_enable_events(g[0]);
	#if GADGET_ICON != 0
		vbar_icon_load(vbar, GADGET_ICON_NAME, SNAME(GADGET_NAME));
		gadget_icon(g[0], SNAME(GADGET_NAME));
	#endif
}

#undef GADGET_NAME
#undef WORKSPACE_ACTIVE_BACKGROUND
#undef WORKSPACE_ACTIVE_FOREGROUND
#undef GADGET_BACKGROUND
#undef GADGET_FOREGROUND
#undef GADGET_HOVER_IN
#undef GADGET_HOVER_OUT 
#undef WORKSPACE_ORDER
#undef GADGET_ALIGNED
#undef GADGET_ICON
#undef GADGET_ICON_NAME
