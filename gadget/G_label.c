#include <vbar.h>

__private void label_set(gadget_s* g, char* txt){
	g->data = txt;
}

__private const char* label_get(gadget_s* g){
	return g->data;
}

int gadget_label_load(gadget_s* g){
	dbg_info("load label");
	g->data = NULL;
	g->ellapse = NULL;
	g->free = NULL;

	return 0;
}

void gadget_label_register(vbar_s* vb){
	dbg_info("register label");
	config_add_symbol(vb, "gadget_label_set", label_set);
	config_add_symbol(vb, "gadget_label_get", label_get);
}

