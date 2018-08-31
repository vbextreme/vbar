#include <vbar/ipc.h>
#include <vbar/string.h>
#include <vbar/delay.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <fcntl.h>

//signstop sigcont

typedef struct inCB{
	ipcCallBack_f cb;
	void* arg;
	int wd;
}inCB_s;

typedef struct evCB{
	ipcCallBack_f cb;
	void* arg;
}evCB_s;

struct ipcModuleEvents{
	evCB_s cb[IPC_CALLBACK_MAX];
	inCB_s notify[IPC_CALLBACK_MAX];
	size_t evcount;
	size_t incount;
	int epfd;
	int infd;
}ime;

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

