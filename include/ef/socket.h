#ifndef __EF_SOCKET_H__
#define __EF_SOCKET_H__

#include <ef/type.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <linux/wireless.h>

#define SOCKET_EPOLL_EVENTS (EPOLLIN | EPOLLRDHUP | EPOLLHUP)
#define SOCKET_SIMULTANEOUS_CONNECTION_MAX 30
#define SOCKET_EVENT_MAX 256
#define SOCKET_UNGET_SIZE 64
#define SOCKET_WIFI_ESSID_SIZE IW_ESSID_MAX_SIZE

typedef struct gsocket socket_s;

typedef int(*socketEvent_f)(socket_s* s);

typedef struct gsocket {
	char* name;
	void* userdata;
	char* ungetch;
	size_t ungsize;
	size_t unglen;
	socketEvent_f onread;
	socketEvent_f onclose;
	int fd;
	struct ucred credentials;
	struct sockaddr_un addr;
}socket_s;

void socket_init(socket_s* s, socketEvent_f onread, socketEvent_f onclose, void* userdata);
void socket_close(socket_s* s);
err_t socket_unix_open(socket_s* s, char* name, int nonblock);
const char* socket_unix_name_get(socket_s* s);
const char* socket_unix_name_raw_get(socket_s* s);
err_t socket_listen(socket_s* s);
err_t socket_connect(socket_s* s);
err_t socket_accept(socket_s* out, socket_s* server);
err_t socket_parse_events(int event, socket_s* s);
int socket_isopen(socket_s* s);
int socket_status(socket_s* s);
err_t socket_write(socket_s* s, const void* buf, size_t len);
#define socket_printf(SCK, FORMAT, arg...) dprintf(SCK->fd, FORMAT, ## arg)
err_t socket_puts(socket_s* s, const char* str, int zero);
err_t socket_unget(socket_s* s, void* data, size_t size);
size_t socket_available(socket_s* s);
err_t socket_read(size_t* out, socket_s* s, void* buf, size_t len);
int socket_gets(size_t* out, socket_s* s, char* dst, size_t size, int terminator);
err_t socket_nget(socket_s* s, void* dst, size_t size);
void socket_flush(socket_s* s);
err_t socket_wifi_info(char* device, char* essid, int* dbm, int* bitrate);

#endif
