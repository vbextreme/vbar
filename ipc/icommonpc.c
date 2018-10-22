#include <vbar/ipc.h>
#include <vbar/string.h>
#include <vbar/delay.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <fcntl.h>

//signstop sigcont

#define SCROLL_BUFFER 256

typedef struct inCB{
	ipcCallBack_f cb;
	void* arg;
	int wd;
}inCB_s;

typedef struct evCB{
	ipcCallBack_f cb;
	void* arg;
}evCB_s;

__ef_private struct ipcModuleEvents{
	evCB_s cb[IPC_CALLBACK_MAX];
	inCB_s notify[IPC_CALLBACK_MAX];
	size_t evcount;
	size_t incount;
	int epfd;
	int infd;
}ime; 

#ifndef IPC_MAX_REGISTER 
	#define IPC_MAX_REGISTER 12
#endif
__ef_private char ipcreg[IPC_MAX_REGISTER][ATTRIBUTE_TEXT_MAX]; 

__ef_private void notify_cbk(__ef_unused void* arg){
	char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
	struct inotify_event* ev;

	int len;
	while( (len = read(ime.infd, buf, sizeof buf)) > 0 ){
		for(char* pbuf = buf; pbuf < buf + len; pbuf += sizeof(struct inotify_event) + ev->len){
			ev = (struct inotify_event*)pbuf;
			for( size_t i = 0; i < ime.incount; ++i ){
				if( ime.notify[i].wd == ev->wd ){
					dbg_info("wd %d", ev->wd);
					ime.notify[i].cb(ime.notify[i].arg);
					break;
				}
			}
		}
	}
}

int ipc_register_callback(int fd, ipcCallBack_f cbk, void* arg){
	dbg_info("[%lu] fd %d", ime.evcount, fd);
	struct epoll_event ev;
	ime.cb[ime.evcount].arg = arg;
	ime.cb[ime.evcount].cb = cbk;
	ev.data.ptr = &ime.cb[ime.evcount];
	ev.events = EPOLLIN | EPOLLET | EPOLLPRI;
	if( epoll_ctl(ime.epfd, EPOLL_CTL_ADD, fd, &ev) ){
		dbg_error("epoll_ctl on fd:%d", fd);
		dbg_errno();
		return -1;
	}

	++ime.evcount;
	return 0;
}

int ipc_register_inotify(char* fname, int flags, ipcCallBack_f cbk, void* arg){
	ime.notify[ime.incount].arg = arg;
	ime.notify[ime.incount].cb = cbk;
	if( (ime.notify[ime.incount].wd = inotify_add_watch(ime.infd, fname, flags)) == -1 ){
        dbg_error("inotify_add_watch on %s", fname);
        dbg_errno();
		return -1;
    }
	dbg_info("file(%d) '%s'", ime.notify[ime.incount].wd, fname);
	++ime.incount;
	return 0;
}

void ipc_init(bool_t clickevents){
	dbg_info("ipc init");
	ime.evcount = 0;
	ime.cb[0].arg = NULL;
	ime.cb[0].cb = NULL;
	ime.epfd = epoll_create1(0);
	if( ime.epfd == -1 ){
		dbg_errno();
		dbg_fail("epoll_create1");
	}
	ipc_register_callback(STDIN_FILENO, NULL, NULL);
	
	ime.incount = 0;
	ime.infd = inotify_init1(O_NONBLOCK);
	if( ime.infd == -1 ){
		dbg_error("inotify_init1");
		dbg_errno();
	}
	ipc_register_callback(ime.infd, notify_cbk, NULL);
	
	ipc_custom_init(clickevents);
}

void ipc_event_reset(event_s* ev){
	ev->name[0] = 0;
	ev->instance[0] = 0;
	ev->x = -1;
	ev->y = -1;
	ev->button = -1;
	ev->relative_x = -1;
	ev->relative_y = -1;
	ev->width = -1;
	ev->height = -1;
}

int ipc_wait(event_s* ev, long timeend){
	struct epoll_event evs[IPC_CALLBACK_MAX];

	while(1){
		int ms = timeend - time_ms();
		if( ms <= 0 ){
			return IPC_TIMEOUT;
		}

		int nev;
		switch( (nev=epoll_wait(ime.epfd, evs, ime.evcount, ms)) ){
			case -1:
				dbg_error("epoll_wait");
				dbg_errno();
			break;

			case 0:
			return IPC_TIMEOUT;

			default:
				for( int i = 0; i < nev; ++i ){
					if( evs[i].data.ptr == &ime.cb[0] ){
						if( !ipc_onstdin(ev) ){
							return ( (long)time_ms() >= timeend ) ? IPC_TIMEOUT | IPC_EVENT : IPC_EVENT;
						}
					}
					else{
						dbg_info("callback %p", evs[i].data.ptr);
						evCB_s* icb = evs[i].data.ptr;
						icb->cb(icb->arg);
					}
				}
			break;
		}
	}

	return -1;
}

void ipc_reg_store(size_t nr, char* val, size_t lenV){
	if( nr >= IPC_MAX_REGISTER ){
		dbg_warning("not have register %lu", nr);
		return;
	}
	str_nncpy_src(ipcreg[nr], ATTRIBUTE_TEXT_MAX, val, lenV);
	dbg_info("store %%%lu, %s", nr, ipcreg[nr]);
}

void ipc_reg_swap(size_t nra, size_t nrb){
	if( nra >= IPC_MAX_REGISTER || nrb > IPC_MAX_REGISTER ){
		dbg_warning("not have register a %lu b %lu", nra, nrb);
	}
	/*restrict*/
	if( nra == nrb ){
		return;
	}
	
	char tmp[ATTRIBUTE_TEXT_MAX];
	strcpy(tmp, &ipcreg[nra][0]);
	UNSAFE_BEGIN("-Wrestrict");
		strcpy(ipcreg[nra], ipcreg[nrb]);
	UNSAFE_END;
	strcpy(ipcreg[nrb], tmp);
}

__ef_can_null char* ipc_reg_load(size_t nr){
	if( nr >= IPC_MAX_REGISTER ){
		dbg_warning("not have register %lu", nr);
		return NULL;
	}
	dbg_info("ret %%%lu, %s", nr, ipcreg[nr]);
	return ipcreg[nr];
}

void ipc_set_attribute_byname(attribute_s* att, char* name, char* value){
	dbg_info("%s = %s", name, value);

	if( !strcmp(name, "text.long") ){
		if( strlen(value) > ATTRIBUTE_TEXT_MAX ){
			dbg_warning("value to long");
			return;
		}
		strcpy(att->longunformat, value);
	}
	else if( !strcmp(name, "text.short") ){
		if( strlen(value) > ATTRIBUTE_TEXT_MAX ){
			dbg_warning("value to long");
			return;
		}
		strcpy(att->shortunformat, value);
	}
	else if( !strcmp(name, "color") ){
		att->useshort = strtoul(value, NULL, 10);
	}
	else if( !strcmp(name, "border") ){
		att->border = strtoul(value, NULL, 10);
	}
	else if( !strcmp(name, "background") ){
		att->background = strtoul(value, NULL, 10);
	}
	else if( !strcmp(name, "min_width") ){
		att->min_width = strtoul(value, NULL, 10);
	}
	else if( !strcmp(name, "name") ){
		if( strlen(value) > ATTRIBUTE_TEXT_MAX ){
			dbg_warning("value to long");
			return;
		}
		strcpy(att->name, value);
	}
	else if( !strcmp(name, "separator") ){
		att->separator = strtoul(value, NULL, 10);
	}
	else if( !strcmp(name, "separator_block_width") ){
		att->separator_block_width = strtoul(value, NULL, 10);
	}
	else if( !strcmp(name, "markup") ){
		att->markup = strtoul(value, NULL, 10);
	}
	else if( !strcmp(name, "blink") ){
		att->blink = strtoul(value, NULL, 10);
	}
	else if( !strcmp(name, "refresh") ){
		att->reftime = strtoul(value, NULL, 10);
	}
	else if( !strcmp(name, "iconsel") ){
		size_t n = strtoul(value, NULL, 10);
		if( n >= att->iconcount ){
			dbg_warning("iconsel out of index");
			return;
		}
		att->icoindex = n;
	}
	else if( !strcmp(name, "hide") ){
		att->hide = strtoul(value, NULL, 10);
	}
}

void ipc_set_attribute_byreg(attribute_s* att, char* name, size_t reg){
	dbg_info("%s = %%%lu", name, reg);

	char* val = ipc_reg_load(reg);
	if( val == NULL ) return;
	ipc_set_attribute_byname(att, name, val);
}

void ipc_reg_store_attribute_byname(attribute_s* att, char* name, size_t reg){
	dbg_info("%%%lu = %s", reg, name);

	char* value = ipc_reg_load(reg);
	if( value == NULL ){
		dbg_warning("not have register %lu", reg);
		return;
	}

	if( !strcmp(name, "text.long") ){
		strcpy(value,att->longunformat);
	}
	else if( !strcmp(name, "text.short") ){
		strcpy(value,att->shortunformat);
	}
	else if( !strcmp(name, "color") ){
		sprintf(value,"%d",	att->useshort);
	}
	else if( !strcmp(name, "border") ){
		sprintf(value,"%d",	att->border);
	}
	else if( !strcmp(name, "background") ){
		sprintf(value,"%d",	att->background);
	}
	else if( !strcmp(name, "min_width") ){
		sprintf(value,"%d",	att->min_width);
	}
	else if( !strcmp(name, "name") ){
		strcpy(value, att->name);
	}
	else if( !strcmp(name, "separator") ){
		sprintf(value,"%d",	att->separator);
	}
	else if( !strcmp(name, "separator_block_width") ){
		sprintf(value,"%d",	att->separator_block_width);
	}
	else if( !strcmp(name, "markup") ){
		sprintf(value,"%d",	att->markup);
	}
	else if( !strcmp(name, "blink") ){
		sprintf(value,"%d",	att->blink);
	}
	else if( !strcmp(name, "refresh") ){
		sprintf(value,"%ld", att->reftime);
	}
	else if( !strcmp(name, "iconsel") ){
		sprintf(value,"%lu",	att->icoindex);
	}
	else if( !strcmp(name, "hide") ){
		sprintf(value,"%d",	att->hide);
	}
}

void ipc_toggle_attribute_byname(attribute_s* att, char* name){
	dbg_info("!%s", name);

	if( !strcmp(name, "separator") ){
		att->separator = !att->separator;
	}
	else if( !strcmp(name, "blink") ){
		att->blink = att->blink;
	}
	else if( !strcmp(name, "hide") ){
		att->hide = !att->hide;
	}
	else if( !strcmp(name, "text.short.enable") ){
		att->useshort = !att->useshort;
	}
}

void ipc_store_blink_mode(attribute_s* att){
	switch( att->blink ){
		default: case BLINK_URGENT: case BLINK_DISABLE: break;

		case BLINK_BACKGROUND:
			att->blinkold = att->background;
		break;

		case BLINK_FOREGROUND:
			att->blinkold = att->color;
		break;
	}
}

void ipc_set_blink_mode(attribute_s* att){
	switch( att->blink ){
		default: case BLINK_DISABLE: break;

		case BLINK_URGENT:
			att->urgent = att->blinktoggle;
		break;
		case BLINK_BACKGROUND:
			if( att->blinktoggle ){
				att->background = att->blinkcolor;
			}
			else{
				att->background = att->blinkold;
			}
		break;
		case BLINK_FOREGROUND:
			if( att->blinktoggle ){
				att->color = att->blinkcolor;
			}
			else{
				att->color = att->blinkold;
			}
		break;
	}
}

void ipc_set_scroll(attribute_s* att){
	char* org = att->longformat;
	char* str = att->scrollformat;
	char* cur = att->scrollch;
	int nch = att->scrollsize;

	while( nch > 0 && (*str++ = *cur++) ){
		--nch;
	}
	
	if( nch > 0 ){
		--str;
		//dbg_info("rewind for %d", nch);
		char* en = att->scrollch;
		cur = org;
		while( nch-->0 && cur < en && (*str++ = *cur++) );
		//dbg_info("remain %d", nch);
	}

	//dbg_info("remain %d", nch);
	
	if( nch > 0 ){
		--str;
		while( nch > 0 ){
			*str++ = ' ';
			--nch;
		}
	}

	*str = 0;

	//dbg_info("ss %d org '%s' scr '%s'", att->useshort, org, att->scrollformat);
}
