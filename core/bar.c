#include <vbar.h>
#include <ef/delay.h>
#include <ef/file.h>
#include <ef/memory.h>
#include <ef/image.h>
#include <ef/delay.h>
#include <ef/strong.h>

//TODO aggiungere click per uscire da evento mouse

extern long topSpacingOpt;

/*******************************************************************************************************************************************************************/
/*******************************************************************************************************************************************************************/
/******************************************************************* X EVENTS **************************************************************************************/
/*******************************************************************************************************************************************************************/
/*******************************************************************************************************************************************************************/

__private void bar_loop(__unused xorg_s* x, void* user){
	vbar_s* vb = user;
	while( vb->bar.callback.loop && !xorg_win_event(x, &vb->bar.callback, 0) ){
		dbg_info("event");
	}
	dbg_info("end");
}

__private void bar_mouse(xorgMouse_s* mouse){
	int line;
   	int icon;
	
	vbar_s* vb = mouse->user;

	gadget_s* g = vbar_gadget_byposition( &line, &icon, vb, mouse->relative.x, mouse->relative.y);

	if( g && g->event ){
		vbarMouse_s vm = { 
			.x = mouse->relative.x - g->position.x,
			.y = mouse->relative.y, //TODO add bar position y and subtract
			.button = mouse->button,
			.extend = line < 0 ? 0 : 1,
			.line = line,
			.icon = icon
		};

		gadgetEventType_e ev = GADGET_EVENT_MOUSE_RELEASE + mouse->event;

		if( g != vb->hover ){
			ev = GADGET_EVENT_MOUSE_ENTER;
			if( vb->hover ) g->event(vb->hover, GADGET_EVENT_MOUSE_LEAVE, &vm);
			vb->hover = g;
		}

		dbg_info("raise mouse event(%d) x:%u y:%u b:%u l:%u i:%u e:%d", ev, vm.x, vm.y, vm.button, vm.line, vm.icon, vm.extend);
		g->event(g, ev, &vm);
	}
	else if( vb->hover && vb->hover->event ){
		vbarMouse_s vm = { 
			.x = mouse->relative.x,
			.y = mouse->relative.y,
			.button = mouse->button,
			.extend = line < 0 ? 0 : 1,
			.line = line,
			.icon = icon
		};
		gadgetEventType_e ev = GADGET_EVENT_MOUSE_LEAVE;
		dbg_info("raise mouse event(%d) x:%u y:%u b:%u l:%u i:%u e:%d", ev, vm.x, vm.y, vm.button, vm.line, vm.icon, vm.extend);
		vb->hover->event(vb->hover, ev, &vm);
		vb->hover = NULL;
	}
}

/*
__private void bar_paint_clear(__unused xorg_s* x, void* user){
	dbg_info("");
	vbar_s* vb = user;
	g2dCoord_s dc = { .x = 0, .y = 0, .w = vb->bar.surface.img.w, .h = vb->bar.surface.img.h};
	g2d_clear(&vb->bar.surface.img, vb->bar.background, &dc);
}
*/

__private void bar_redraw(xorg_s* x, void* user, __unused g2dCoord_s* damaged){
	vbar_s* vb = user;
	if( vb->bar.callback.paint ){
		vb->bar.callback.paint(x, user);
		vb->bar.callback.paint = NULL;
	}
	xorg_win_surface_redraw(x, vb->bar.id, &vb->bar.surface);
}

__private void bar_atom(__unused xorg_s* x, void* user, int atom){
	vbar_s* vb = user;	
	for(xorgEventRegister_s* er = vb->xer; er; er = er->next){
		if( er->event == XCB_PROPERTY_NOTIFY && (int)er->atom == atom && er->gadget->event ){
			if( er->gadget->event ){
				er->gadget->redraw = GADGET_NOREDRAW;
				vbar_label_reset(er->gadget);
				er->gadget->event(er->gadget, GADGET_EVENT_REFRESH, &atom);
			}
		}
	}
	//if( atom == (int)x->atom[XORG_ATOM_NET_CURRENT_DESKTOP] ){
	//	dbg_error("D E S C K T O P  I S C H A N G E D..............................................###############################################");
	//}
}

__private void bar_error_mouse(xorgMouse_s* mouse){
	vbar_s* vb = mouse->user;
	if( mouse->event == XORG_MOUSE_RELEASE ){
		if( vb->bar.errw.img.h <= vb->bar.surface.img.h ) return;
		if( mouse->button == 5 ){ //down
			//dbg_info("DOWN---------------------------------------------------------------");
			vb->bar.errw.scroll.y += vb->bar.errw.scroll.h;
			if( vb->bar.errw.scroll.y + vb->bar.surface.img.h > vb->bar.errw.img.h ){
				vb->bar.errw.scroll.y = vb->bar.errw.img.h - vb->bar.surface.img.h;
			}
			g2dCoord_s pd = { .x = 0, .y = 0, .w = vb->bar.surface.img.w, vb->bar.surface.img.h};
			g2dCoord_s ps = { .x = 0, .y = vb->bar.errw.scroll.y, .w = vb->bar.surface.img.w, vb->bar.surface.img.h};
			g2d_bitblt(&vb->bar.surface.img, &pd, &vb->bar.errw.img, &ps);
			xorg_win_surface_redraw(mouse->x, vb->bar.id, &vb->bar.surface);
		}
		else if( mouse->button == 4 ){ //up
			//dbg_info("UP------------------------------------------------------------------");
			if( vb->bar.errw.scroll.y <  vb->bar.errw.scroll.h ){
				vb->bar.errw.scroll.y = 0;
			}
			else{
				vb->bar.errw.scroll.y -= vb->bar.errw.scroll.h;
			}
	
			g2dCoord_s pd = { .x = 0, .y = 0, .w = vb->bar.surface.img.w, vb->bar.surface.img.h};
			g2dCoord_s ps = { .x = 0, .y = vb->bar.errw.scroll.y, .w = vb->bar.surface.img.w, vb->bar.surface.img.h};
			g2d_bitblt(&vb->bar.surface.img, &pd, &vb->bar.errw.img, &ps);
			xorg_win_surface_redraw(mouse->x, vb->bar.id, &vb->bar.surface);
		}
	}
	else if( mouse->event == XORG_MOUSE_CLICK && mouse->button == 1 ){
		exit(1);
	}
}

/*****************************************************************************************************************************************************************/
/*****************************************************************************************************************************************************************/
/******************************************************************* SCRIPT **************************************************************************************/
/*****************************************************************************************************************************************************************/
/*****************************************************************************************************************************************************************/

__private void bar_monitor_set(vbar_s* vb, char const* monitor){	
	char const* mn = monitor ? monitor : vb->monitorName;
	
	if( mn && xorg_monitor_byname(&vb->bar.x, mn) ){
		dbg_error("monitor %s", mn);
		exit(1);
	}
}

__private void bar_fonts_set(vbar_s* vb, char const* name, size_t size){
	dbg_info("set fonts %s", name);
	ftFont_s* font;
	if( !(font=ft_fonts_load(&vb->bar.fonts, name)) ){
		dbg_error("on load fonts");
		exit(1);
	}
	ft_font_size(font, 0, size);
	if( font->height > vb->bar.fontHeight ) vb->bar.fontHeight = font->height;
}	

__private void bar_colors_set(vbar_s* vb, unsigned background, unsigned foreground){
	vb->bar.background = background;
	vb->bar.foreground = foreground;
}

__private void bar_topbar_set(vbar_s* vb, unsigned ontop){
	vb->bar.topBar = ontop;
}

__private void bar_height_set(vbar_s* vb, unsigned height, unsigned spacingTop, unsigned spacingBottom){
	if( spacingTop == 0 ){
		spacingTop = (vb->bar.fontHeight / BAR_TB_SPACING_FACTOR)/2;
	}
	if( spacingBottom == 0 ){
		spacingTop = (vb->bar.fontHeight / BAR_TB_SPACING_FACTOR)/2;
	}
	vb->bar.topSpacing = spacingTop + topSpacingOpt;
	vb->bar.bottomSpacing = spacingBottom;
	vb->bar.height = ( height < vb->bar.fontHeight ) ? 
			vb->bar.fontHeight + vb->bar.topSpacing + vb->bar.bottomSpacing 
		: 
			(unsigned)height;
}

__private unsigned inutility_rgb(int r, int g, int b){
	return g2d_color_gen(X_COLOR_MODE, 0, r, g, b);
}

__private void bar_border_width_set(vbar_s* vb, unsigned size){
	vb->bar.borderSize = size;
}

/*****************************************************************************************************************************************************************/
/*****************************************************************************************************************************************************************/
/********************************************************************* BAR ***************************************************************************************/
/*****************************************************************************************************************************************************************/
/*****************************************************************************************************************************************************************/

__private void bar_extend_resize_up(bar_s* bar, unsigned extendH){
	dbg_info("h:%u bh:%u rsh:%u", extendH, bar->height, extendH + bar->height);
	xorg_surface_resize_bitblt(&bar->surface, xorg_root_width(&bar->x), extendH + bar->height);
	g2dCoord_s area = { .x = 0, .y = bar->height, .w = xorg_root_width(&bar->x), extendH };
	g2d_clear(&bar->surface.img, bar->background, &area);

	if( bar->topBar ){
		xorg_wm_reserve_dock_space_on_top(&bar->x, bar->id, 0, xorg_root(&bar->x), extendH + bar->height);	
	}
	else{
		xorg_wm_reserve_dock_space_on_bottom(&bar->x, bar->id, 0, xorg_root(&bar->x), extendH + bar->height);
	}

	xorg_win_resize(&bar->x, bar->id, xorg_root(&bar->x), extendH + bar->height);
	//exit(1);
}

__private void bar_extend_resize_down(bar_s* bar, unsigned extendH){
	dbg_info("h:%u", extendH);
	xorg_surface_resize_bitblt(&bar->surface, xorg_root_width(&bar->x), extendH + bar->height);
	if( extendH > 0 ){
		g2dCoord_s area = { .x = 0, .y = bar->height, .w = xorg_root_width(&bar->x), extendH };
		g2d_clear(&bar->surface.img, bar->background, &area);
	}

	xorg_win_resize(&bar->x, bar->id, xorg_root(&bar->x), extendH + bar->height);

	if( bar->topBar ){
		xorg_wm_reserve_dock_space_on_top(&bar->x, bar->id, 0, xorg_root(&bar->x), extendH + bar->height);	
	}
	else{
		xorg_wm_reserve_dock_space_on_bottom(&bar->x, bar->id, 0, xorg_root(&bar->x), extendH + bar->height);
	}
}

__private void bar_extend_resize(bar_s* bar, unsigned extendH){
	if( extendH > bar->extendH ){
		bar_extend_resize_up(bar, extendH);
	}
	else if( extendH < bar->extendH ){
		bar_extend_resize_down(bar, extendH);
	}
	bar->extendH = extendH;
}

void bar_extend_close(bar_s* b){
	bar_extend_resize(b, 0);
}

__private err_t bar_xorg_event(__unused int type, void* vbar){
	dbg_info("");
	vbar_s* vb = vbar;
	while( !xorg_win_event(&vb->bar.x, &vb->bar.callback, 1) );	
	return 0;
}

err_t bar_begin(vbar_s* vb){
	dbg_info("vbar_begin");
	if( xorg_client_init(&vb->bar.x) ){
		dbg_error("x init");
		return -1;
	}
	ft_init(&vb->bar.lib);
	ft_fonts_init(vb->bar.lib, &vb->bar.fonts);

	vb->bar.callback.redraw = bar_redraw;
	vb->bar.callback.mouse = bar_mouse;
	vb->bar.callback.loop = bar_loop;
	vb->bar.callback.atom = bar_atom;
	vb->bar.callback.user = vb;

	deadpoll_register(&vb->events, xorg_fd(&vb->bar.x), bar_xorg_event, vb, 0);
	return 0;
}

void bar_register_symbol(vbar_s* vb){
	config_add_symbol(vb, "rgb", inutility_rgb);
	config_add_symbol(vb, "vbar_monitor_set", bar_monitor_set);
	config_add_symbol(vb, "vbar_fonts_set", bar_fonts_set);
	config_add_symbol(vb, "vbar_colors_set", bar_colors_set);
	config_add_symbol(vb, "vbar_height_set", bar_height_set);
	config_add_symbol(vb, "vbar_topbar_set", bar_topbar_set);
	config_add_symbol(vb, "vbar_border_width_set", bar_border_width_set);
}

err_t bar_start(vbar_s* vb){
	dbg_info("create bar");
	g2dCoord_s pos = {.x = 0, .y = 0, .w = xorg_root_width(&vb->bar.x), .h = vb->bar.height };
	if( !vb->bar.topBar ){
		pos.y = xorg_root_height(&vb->bar.x) - pos.h;
	}
	vb->bar.id = xorg_win_new(&vb->bar.surface, &vb->bar.x, xorg_root(&vb->bar.x), &pos, 0, vb->bar.background);
	
	xorg_win_dock(&vb->bar.x, vb->bar.id);
	if( vb->bar.topBar ){
		dbg_info("bar on top");
		xorg_wm_reserve_dock_space_on_top(&vb->bar.x, vb->bar.id, 0, xorg_root_width(&vb->bar.x), vb->bar.height);
	}
	else{
		dbg_info("bar on bottom");
		xorg_wm_reserve_dock_space_on_bottom(&vb->bar.x, vb->bar.id, 0, xorg_root_width(&vb->bar.x), vb->bar.height);
	}
	
	g2dCoord_s dc = { .x = 0, .y = 0, .w = vb->bar.surface.img.w, .h = vb->bar.surface.img.h};
	g2d_clear(&vb->bar.surface.img, vb->bar.background, &dc);

	//register root events for _NET
	xorg_register_events(&vb->bar.x, xorg_root(&vb->bar.x), XCB_EVENT_MASK_PROPERTY_CHANGE);
	return 0;
}

void bar_terminate(vbar_s* vb){
	xorg_client_terminate(&vb->bar.x);
	ft_fonts_free(&vb->bar.fonts);
	ft_terminate(vb->bar.lib);
}

void bar_show(vbar_s* vb){
	xorg_win_show(&vb->bar.x, vb->bar.id,1);
}

void bar_simple_setting(vbar_s* vb){
	bar_monitor_set(vb, NULL);
	bar_fonts_set(vb, VBAR_SIMPLE_FONT, VBAR_SIMPLE_FONT_SIZE);
	bar_height_set(vb, VBAR_SIMPLE_HEIGHT, 0, 0);
}

err_t bar_error(vbar_s* vb, utf8_t* errd){
	dbg_info("show vbar error");

	unsigned w = xorg_root_width(&vb->bar.x);
	unsigned h = ft_autowrap_height(&vb->bar.fonts, errd, w - VBAR_ERROR_OFFSET_X) + ft_line_height(&vb->bar.fonts);
	dbg_info("text autowrap height %u", h);
	vb->bar.errw.img = g2d_new(w, h, X_COLOR_MODE);
	g2dColor_t bke = g2d_color_make(&vb->bar.errw.img, 255, VBAR_ERROR_BK_R, VBAR_ERROR_BK_G, VBAR_ERROR_BK_B);
	g2dColor_t fge = g2d_color_make(&vb->bar.errw.img, 255, VBAR_ERROR_FG_R, VBAR_ERROR_FG_G, VBAR_ERROR_FG_B);
	g2dCoord_s pos = { .x = 0, .y = 0, .w = w, .h = h };
	g2d_clear(&vb->bar.errw.img, bke, &pos);
	g2dCoord_s pen = { .x = VBAR_ERROR_OFFSET_X, .y = VBAR_ERROR_OFFSET_Y, .w = 0, .h = 0 };
	g2d_string_autowrap(&vb->bar.errw.img, &pen, &vb->bar.fonts, errd, fge, VBAR_ERROR_OFFSET_X);
	vb->bar.errw.scroll.x = 0;
	vb->bar.errw.scroll.y = 0;
	vb->bar.errw.scroll.w = 0;
	vb->bar.errw.scroll.h = ft_line_height(&vb->bar.fonts) / VBAR_ERROR_SCROLL_Y;

	vb->bar.callback.mouse = bar_error_mouse;
	bar_extend_resize(&vb->bar, vb->bar.fontHeight * VBAR_ERROR_DISPLAY_LOC);

	pos.h = vb->bar.surface.img.h;
	g2d_clear(&vb->bar.surface.img, bke, &pos);

	if( pos.h > vb->bar.errw.img.h ) pos.h = vb->bar.errw.img.h;
	g2d_bitblt(&vb->bar.surface.img, &pos, &vb->bar.errw.img, &pos);

	return 0;
}


__private void _gadget_draw_border(vbar_s* vb, gadget_s* g, g2dCoord_s* rect, unsigned isz){
	if( g->border & VBAR_BORDER_LEFT ){
		g2dCoord_s bc ={
			.x = rect->x - isz,
			.w = vb->bar.borderSize,
			.y = 0,
			.h = vb->bar.height
		};
		g2d_rect_fill(&vb->bar.surface.img, &bc, g->borderColor);
	}
	if( g->border & VBAR_BORDER_TOP ){
		g2dCoord_s bc ={
			.x = rect->x - isz,
			.w = rect->w + isz,
			.y = 0,
			.h = vb->bar.borderSize
		};
		g2d_rect_fill(&vb->bar.surface.img, &bc, g->borderColor);
	}
	if( g->border & VBAR_BORDER_RIGHT ){
		g2dCoord_s bc = {
			.x = rect->x + (rect->w - vb->bar.borderSize),
			.w = vb->bar.borderSize,
			.y = 0,
			.h = vb->bar.height
		};
		g2d_rect_fill(&vb->bar.surface.img, &bc, g->borderColor);
	}
	if( g->border & VBAR_BORDER_BOTTOM ){
		g2dCoord_s bc ={
			.x = rect->x - isz,
			.w = rect->w + isz,
			.y = vb->bar.height - vb->bar.borderSize,
			.h = vb->bar.borderSize
		};
		g2d_rect_fill(&vb->bar.surface.img, &bc, g->borderColor);
	}
}

__private unsigned _gadget_draw(vbar_s* vb, gadget_s* g, icon_s* icon, g2dCoord_s* rect){
	unsigned isz = 0;
	if( rect->x + rect->w > vb->bar.surface.img.w ) return 0;
	if( icon ){
		g2dCoord_s dc = {
			.x = rect->x,
			.y = vb->bar.topSpacing,
			.w = icon->img.w,
			.h = icon->img.h
		};
		g2dCoord_s sc = {
			.x = 0,
			.y = 0,
			.w = icon->img.w,
			.h = icon->img.h
		};
		g2d_bitblt(&vb->bar.surface.img, &dc, &icon->img, &sc);
		rect->x += sc.w;
		rect->w -= sc.w;
		isz = sc.w;
	}
	g->position.x = rect->x;
	g->position.y = rect->y;
	g->position.w = rect->w;
	g->position.h = rect->h;

	dbg_info("gadget:: %s text: '%s' height: %u", g->instance, g->label, rect->h);

	g2d_clear(&vb->bar.surface.img, g->background, rect);
	if( g->border ) _gadget_draw_border(vb, g, rect, isz);
	unsigned begin = rect->x;
	g2d_string(&vb->bar.surface.img, rect, &vb->bar.fonts, g->label, g->foreground, rect->x);
	dbg_info("gadget:: %s text: '%s' height: %u", g->instance, g->label, rect->h);

	return begin;
}

__private void _extend_separator(vbar_s* vb, gLine_s* gl, unsigned y, unsigned w, unsigned h){
	if( gl->redraw == GADGET_NOREDRAW ) return;

	g2dCoord_s area = {.x = 0, .y = y, .w = w, .h = h };

	if( gl->redraw == GADGET_REDRAW ){
		g2d_clear(&vb->bar.surface.img, gl->background, &area);
	}
	
	switch( gl->border ){
		case 0: return;
		case 0x01: 
			area.h = vb->bar.borderSize;
		break;
		case 0x02: 
			area.h = vb->bar.borderSize;
			area.y = h / 2 - area.h / 2; 
		break;
		case 0x03: 
			area.h = vb->bar.borderSize;
			area.y = h - area.h;
		break;
	}

	g2d_clear(&vb->bar.surface.img, gl->borderColor, &area);
}

__private void _extend_label(vbar_s* vb, gLine_s* gl, unsigned y, unsigned w, unsigned h){
	g2dCoord_s area = {.x = 0, .y = y, .w = w, .h = h };

	dbg_info("label position %u %u %u*%u", area.x, area.y, area.w, area.h);
	if( 1 ){//gl->redraw == GADGET_REDRAW ){
		dbg_info("redraw \"%s\"", gl->label);
		g2d_clear(&vb->bar.surface.img, gl->background, &area);
		//area.h=vb->bar.fontHeight;
		g2d_string(&vb->bar.surface.img, &area, &vb->bar.fonts, gl->label, gl->foreground, area.x);
	}
	else if( gl->redraw == GADGET_REDRAW_PARTIAL ){
		dbg_info("partial");
		//area.h=vb->bar.fontHeight;
		g2d_string_replace(&vb->bar.surface.img, &area, &vb->bar.fonts, gl->label, gl->oldLabel, gl->foreground, gl->background, area.x);	
	}
}

__private void _extend_selector(vbar_s* vb, gLine_s* gl, unsigned y, unsigned w, unsigned h){
	_extend_label(vb, gl, y, w, h);

	g2dCoord_s area = {.x = w - h, .y = y, .w = h, .h = h };

	for( size_t i = 0; i < gl->iconCount; ++i ){
		if( gl->redraw == GADGET_NOREDRAW ) continue;
		if( gl->redraw == GADGET_REDRAW || (gl->iconName[i] && gl->iconName[i] != gl->oldIconName[i])){
			icon_s* icon = NULL;
			chash_find((void*)&icon, &vb->icons, gl->iconName[i], strlen(gl->iconName[i]));
			iassert(icon != NULL);
			g2dCoord_s dc = {
				.x = area.x,
				.y = vb->bar.topSpacing,
				.w = icon->img.w,
				.h = icon->img.h
			};
			g2dCoord_s sc = {
				.x = 0,
				.y = 0,
				.w = icon->img.w,
				.h = icon->img.h
			};
			g2d_bitblt(&vb->bar.surface.img, &dc, &icon->img, &sc);
			area.x -= h;
		}
	}
}

void bar_draw_extend(gadget_s* g){
	bar_s* bar = &g->vbar->bar;

	bar_extend_resize(bar, g->extendHeight);
	unsigned y = bar->height;
	unsigned const w = xorg_root_width(&bar->x);
	unsigned const h = bar->height;
	
	dbg_info("extend %lu", g->extendCount);

	for( size_t i = 0; i < g->extendCount; ++i, y += h ){
		switch( g->extend[i].type ){
			case GADGET_LINE_SEPARATOR:
				dbg_info("line %lu is separator", i);
				_extend_separator(g->vbar, &g->extend[i], y, w, h);
			break;

			case GADGET_LINE_LABEL:
				dbg_info("line %lu is label", i);
				_extend_label(g->vbar, &g->extend[i], y, w, h);
			break;

			case GADGET_LINE_SELECTOR:
				dbg_info("line %lu is selector", i);
				_extend_selector(g->vbar, &g->extend[i], y, w, h);
			break;
		}
	}

	xorg_win_surface_redraw(&bar->x, bar->id, &bar->surface);
}

void bar_draw(vbar_s* vb){
	dbg_info("redraw all gadget");
	gadget_s* g = vb->drawed;
	if( !g ) return;

	unsigned left = 0;
	unsigned right = xorg_root_width(&vb->bar.x);
	unsigned center = right / 2;
	g2dCoord_s rect = { .x = 0, .y = vb->bar.topSpacing, .w = right, .h = vb->bar.height - vb->bar.topSpacing};
	unsigned centerLenght = 0;
	
	g2d_clear(&vb->bar.surface.img, vb->bar.background, &rect);
	rect.h -= vb->bar.bottomSpacing;
	do{
		if( !g->hiden && g->label ){
			rect.w = ft_line_lenght(&vb->bar.fonts, g->label);
			icon_s* icon = NULL;
			if( g->iconName ){
				chash_find((void*)&icon, &vb->icons, g->iconName, strlen(g->iconName));
				iassert(icon != NULL);
				rect.w += icon->img.w;
			}
			if( rect.w == 0 && icon == NULL ){
				g = g->next;
				continue;
			}

			switch( g->align ){
				default: case VBAR_ALIGNED_LEFT:
					rect.x = left;
					_gadget_draw(vb, g, icon, &rect);
					left = rect.x;
				break;

				case VBAR_ALIGNED_CENTER:
					centerLenght += rect.w;
				break;

				case VBAR_ALIGNED_RIGHT:
					right -= rect.w;
					rect.x = right;
					_gadget_draw(vb, g, icon, &rect);	
				break;
			}
		}
		g = g->next;
	}while(g != vb->drawed);

	if( centerLenght ){
		center -= centerLenght / 2;
		g = vb->drawed;
		do{
			if( !g->hiden && g->label ){
				if( g->align == VBAR_ALIGNED_CENTER ){
					rect.w = ft_line_lenght(&vb->bar.fonts, g->label);
					icon_s* icon = NULL;
					if( g->iconName ){
						chash_find((void*)&icon, &vb->icons, g->iconName, strlen(g->iconName));
						iassert(icon != NULL);
						rect.w += icon->img.w;
					}
					rect.x = center;
					//g->position.x = 
					_gadget_draw(vb, g, icon, &rect);	
					//g->position.y = rect.y;
					//g->position.w = rect.w;
					//g->position.h = rect.h;
					center = rect.x;
				}
			}
			g = g->next;
		}while(g != vb->drawed);
	}

	xorg_win_surface_redraw(&vb->bar.x, vb->bar.id, &vb->bar.surface);
}

void bar_gadget_draw(vbar_s* vb, gadget_s* g, utf8_t* oldLabel){
	dbg_info("text:\"%s\"", g->label);
	iassert(g);
//bar_draw(vb);return;
	if( g->redraw == GADGET_FULL_REDRAW ){
		dbg_warning("gadget full redraw");
		bar_draw(vb);
		return;
	}
	
	if( g->hiden ) return;

	if( g->position.w == 0 ){
		dbg_warning("no position redraw all");
		bar_draw(vb);
		return;
	}
	
	g2dCoord_s rect = {
		.x = g->position.x, 
		.y = g->position.y, 
		.w = g->position.w,
		.h = g->position.h
	};

   	if( (g->iconName != g->oldIcon) ){
		dbg_info("different icon");
		g->oldIcon = g->iconName;
		bar_draw(vb);
		return;
	}

	if( !oldLabel ){
		/*
		icon_s* icon = NULL;
		if( g->iconName ){
			chash_find((void*)&icon, &vb->icons, g->iconName, strlen(g->iconName));
			iassert(icon != NULL);
			//rect.w += icon->img.w;
		}
		*/
		if( ft_line_lenght(&vb->bar.fonts, g->oldLabel) != ft_line_lenght(&vb->bar.fonts, g->label) ){
			bar_draw(vb);
		}
		else{
			_gadget_draw(vb, g, NULL, &rect);
		}
		//dbg_info("clear %u %u %d*%d", rect.x, rect.y, rect.w, rect.h);
		//g2d_clear(&vb->bar.surface.img, g->background, &rect);
		//if( g->label && *g->label ) g2d_string(&vb->bar.surface.img, &rect, &vb->bar.fonts, g->label, g->foreground, rect.x);
	}
	else{
		if( ft_line_lenght(&vb->bar.fonts, g->oldLabel) != ft_line_lenght(&vb->bar.fonts, g->label) ){
			bar_draw(vb);
		}
		dbg_info("redraw from %u/%u %u", rect.x, xorg_root_width(&vb->bar.x), rect.y);
		if( g->label && *g->label ) g2d_string_replace(&vb->bar.surface.img, &rect, &vb->bar.fonts, g->label, oldLabel, g->foreground, g->background, rect.x);	
	}
	
	xorg_win_surface_redraw(&vb->bar.x, vb->bar.id, &vb->bar.surface);
}

