#ifndef __EF_DEADPOLL_H__
#define __EF_DEADPOLL_H__

#include <ef/type.h>
#include <sys/sysinfo.h>
#include <sys/epoll.h>

#define DEADPOLL_ERROR -1
#define DEADPOLL_TIMEOUT 1
#define DEADPOLL_EVENT 0

#define DEADPOLL_POLLING_EVENTS 32

typedef err_t(*pollCbk_f)(int, void*);

typedef struct pollEvent{
	struct pollEvent* next;
	int fd;
	int event;
	pollCbk_f callback;
	void* arg;
}pollEvent_s;

typedef struct deadpoll{
	pollEvent_s* events;
	int pollfd;
	pollCbk_f timeout;
	void* timeoutarg;
}deadpoll_s;


int deadpoll_unregister(deadpoll_s* dp, int fd);
int deadpoll_register(deadpoll_s* dp, int fd, pollCbk_f cbk, void* arg, int onevents);
err_t deadpoll_init(deadpoll_s* dp);
err_t deadpoll_terminate(deadpoll_s* dp);
int deadpoll_event(deadpoll_s* dp, long* timems);
int deadpoll_loop(deadpoll_s* dp, long timems);




#endif
