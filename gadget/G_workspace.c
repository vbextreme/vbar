#include <vbar.h>

__private size_t TYPE = 0;

__private int workspace_count(gadget_s* g){
	if( g->type != TYPE ) return 0;
	return xorg_workspace_count(&g->vbar->bar.x);
}

__private int workspace_active(gadget_s* g){
	if( g->type != TYPE ) return 0;
	return xorg_workspace_active(&g->vbar->bar.x);
}

__private char** workspace_names(gadget_s* g){
	if( g->type != TYPE ) return NULL;
	return xorg_workspace_name_get(&g->vbar->bar.x);
}

__private void workspace_names_free(char** names){
	for( size_t i = 0; names[i]; ++i){
		free(names[i]);
	}
	free(names);
}

__private void workspace_activate(gadget_s* g, unsigned idvd){
	if( g->type != TYPE ) return;
	xorg_send_current_desktop(&g->vbar->bar.x, idvd);
}

__private void workspace_enable_events(gadget_s* g){
  	if( g->type != TYPE ) return;
	vbar_xorg_register_event(g->vbar, g, XCB_PROPERTY_NOTIFY, g->vbar->bar.x.atom[XORG_ATOM_NET_CURRENT_DESKTOP]);
	vbar_xorg_register_event(g->vbar, g, XCB_PROPERTY_NOTIFY, g->vbar->bar.x.atom[XORG_ATOM_NET_DESKTOP_NAMES]);
}

int gadget_workspace_load(gadget_s* g){
	g->data = NULL;
	g->ellapse = NULL;
	g->free = NULL;
	return 0;
}

void gadget_workspace_register(vbar_s* vb){
	dbg_info("register workspace");
	TYPE = gadget_type_get(vb, "workspace");
	config_add_symbol(vb, "gadget_workspace_enable_events", workspace_enable_events);
	config_add_symbol(vb, "gadget_workspace_count", workspace_count);
	config_add_symbol(vb, "gadget_workspace_active", workspace_active);
	config_add_symbol(vb, "gadget_workspace_activate", workspace_activate);
	config_add_symbol(vb, "gadget_workspace_names", workspace_names);
	config_add_symbol(vb, "gadget_workspace_names_free", workspace_names_free);
}

