#ifndef __EF_WINDOW_H__
#define __EF_WINDOW_H__

#include <ef/type.h>
#include <ef/gadget.h>

void win_paint(__unused xorg_s* x, gadget_s* gadget);
void win_move(gadgetMove_s* move);
void win_redraw(xorg_s* x, gadget_s* win, __unused g2dCoord_s* damaged);
void win_terminate(xorg_s* x, gadget_s* win);
void win_loop(xorg_s* x, gadget_s* win);
void win_title(xorg_s* x, gadget_s* win, char* name);
void win_new(xorg_s* x, gadget_s* parent, gadget_s* win, char* name, char* class, g2dCoord_s* pos, unsigned border, g2dColor_t background);
void win_show(xorg_s* x, gadget_s* win, int show);



#endif 
