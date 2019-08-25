#include <ef/deadpoll.h>
#include <ef/delay.h>
#include <ef/memory.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <fcntl.h>

int deadpoll_unregister(deadpoll_s* dp, int fd){
	pollEvent_s** ev;
	for(ev = &dp->events; (*ev); ev = &(*ev)->next){
		if( (*ev)->fd == fd ){
			pollEvent_s* rm = (*ev);
			if( epoll_ctl(dp->pollfd, EPOLL_CTL_DEL, rm->fd, NULL) ){
				dbg_error("epoll_ctl on fd:%d", fd);
				dbg_errno();
				return -1;
			}
			(*ev) = rm->next;
			free(rm);
			return 0;
		}
	}
	return -1;
}

int deadpoll_register(deadpoll_s* dp, int fd, pollCbk_f cbk, void* arg, int onevents){
	dbg_info("register fd %d", fd);
	if( onevents == 0 ) onevents = EPOLLIN | EPOLLET | EPOLLPRI;

	pollEvent_s* dpe = mem_new(pollEvent_s);
	iassert( dpe );

	struct epoll_event ev = {
		.data.ptr = dpe,
		.events = onevents,
	};
	if( epoll_ctl(dp->pollfd, EPOLL_CTL_ADD, fd, &ev) ){
		dbg_error("epoll_ctl on fd:%d", fd);
		dbg_errno();
		free( dpe );
		return -1;
	}

	dpe->fd = fd;
	dpe->arg = arg;
	dpe->callback = cbk;
	dpe->event = onevents;
	dpe->next = dp->events;
	dp->events = dpe;
	return 0;
}

err_t deadpoll_init(deadpoll_s* dp){
	dp->pollfd = epoll_create1(0);
	if( dp->pollfd < 0 ){
		dbg_fail("epoll_create1");
		dbg_errno();
		return -1;
	}
	dp->events = NULL;
	return 0;
}

err_t deadpoll_terminate(deadpoll_s* dp){
	pollEvent_s* next;
	while( dp->events ){
		next = dp->events->next;
		epoll_ctl(dp->pollfd, EPOLL_CTL_DEL, dp->events->fd, NULL);
		free( dp->events );
		dp->events = next;
	}
	close( dp->pollfd );
	return 0;
}

int deadpoll_event(deadpoll_s* dp, long* timems){
	int eventCount;
	struct epoll_event epollEvent[DEADPOLL_POLLING_EVENTS];
	memset(epollEvent, 0, sizeof(struct epoll_event) * DEADPOLL_POLLING_EVENTS);
	long timer = time_ms();

	dbg_info("poll ms %ld", *timems);
	switch( (eventCount=epoll_wait(dp->pollfd, epollEvent, DEADPOLL_POLLING_EVENTS, *timems)) ){
		case -1:
			dbg_error("epoll_wait");
			dbg_errno();
		return DEADPOLL_ERROR;

		case 0:
			dbg_info("timeout");
			*timems = 0;
			if( dp->timeout ){
				if( dp->timeout(0, dp->timeoutarg) ){
					return DEADPOLL_ERROR;
				}
				return DEADPOLL_TIMEOUT;
			}
		return DEADPOLL_TIMEOUT;

		default:
			dbg_info("event");
			for( size_t i = 0; i < (size_t)eventCount; ++i ){
				pollEvent_s* ev = epollEvent[i].data.ptr;
				if( ev->fd == 0 ) {
					dbg_warning("no fd on %ld",i); 
					continue;
				}
				dbg_info("callback %ld fd %d", i, ev->fd);
				
				if( ev->callback(epollEvent[i].events, ev->arg) ){
					return DEADPOLL_ERROR;
				}
			}
			*timems -= time_ms() - timer;
		return DEADPOLL_EVENT;
	}

	return DEADPOLL_ERROR;
}


int deadpoll_loop(deadpoll_s* dp, long timems){
	long timestart = timems;

	while(1){
		switch( deadpoll_event(dp, &timems) ){
			case DEADPOLL_ERROR: return DEADPOLL_ERROR;
			case DEADPOLL_TIMEOUT: break;
			case DEADPOLL_EVENT:
				if( timems && dp->timeout){
					timems -= timestart;
					while( timems < 0 ){ timems += timestart; }
				}
				else{
					timems = timestart;
				}
			break;
		}
	}
	return DEADPOLL_ERROR;	
}


