#include <ef/socket.h>
//#include <ef/list.h>
#include <ef/memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>

void socket_init(socket_s* s, socketEvent_f onread, socketEvent_f onclose, void* userdata){
	iassert(s);
	s->fd = -1;
	s->name = NULL;
	s->ungetch = mem_many(char, SOCKET_UNGET_SIZE);
	s->ungsize = SOCKET_UNGET_SIZE;
	s->credentials.gid = 0;
	s->credentials.uid = 0;
	s->credentials.pid = 0;
	s->unglen = 0;
	s->onread = onread;
	s->onclose = onclose;
	s->userdata = userdata;
}

void socket_close(socket_s* s){
	iassert(s);

	if( s->fd != -1 ){
		dbg_info("close fd %d", s->fd);
		close(s->fd);
		s->fd = -1;
	}
		
	free( s->ungetch );
}	
	
err_t socket_unix_open(socket_s* s, char* name, int nonblock){
	iassert(s);
	iassert(name);

	int type = SOCK_STREAM;
	
	if( nonblock ){
		type |= SOCK_NONBLOCK;
	}
	
	if( (s->fd = socket(AF_UNIX, type, 0)) == -1 ){
		dbg_error("socket");
		dbg_errno();
		return -1;
	}
	
	memset(&s->addr, 0, sizeof(s->addr));
	s->addr.sun_family = AF_UNIX;
	
	if( *name == '\0' ){
		dbg_info("global name '%s'", s->name + 1);
		*s->addr.sun_path = '\0';
		strncpy(s->addr.sun_path+1, name + 1, sizeof(s->addr.sun_path)-2);
	}
	else{
		dbg_info("file name '%s'", name);
		strncpy(s->addr.sun_path, name, sizeof(s->addr.sun_path)-1);
	}
		
	return 0;
}

const char* socket_unix_name_get(socket_s* s){
	return *s->addr.sun_path == 0 ? s->addr.sun_path + 1 : s->addr.sun_path;
}

const char* socket_unix_name_raw_get(socket_s* s){
	return s->addr.sun_path;
}

err_t socket_listen(socket_s* s){
	if( *s->addr.sun_path ) unlink(s->addr.sun_path);

	if( bind(s->fd, (struct sockaddr*)&s->addr, sizeof(s->addr)) == -1 ){
		dbg_error("can't bind file");
		dbg_errno();
		return -1;
	}
	
	if( listen(s->fd, SOCKET_SIMULTANEOUS_CONNECTION_MAX) == -1 ){
		dbg_error("can't listen on server");
		dbg_errno();
		return -1;
	}
	
	return 0;
}

err_t socket_connect(socket_s* s){
	if( connect(s->fd, (struct sockaddr*)&s->addr, sizeof s->addr) == -1 ){
		dbg_error("can't connect to server");
		dbg_errno();
		return -1;
	}
	return 0;
}

__private err_t socket_get_credentials(socket_s* s){
	socklen_t len = sizeof(struct ucred);

	if( getsockopt(s->fd, SOL_SOCKET, SO_PEERCRED, &s->credentials, &len) == -1) {
		dbg_error("can't get credentials");
		dbg_errno();
		return -1;
	}

	dbg_info("socket credentials uid:%d gid:%d pid:%d", s->credentials.uid, s->credentials.gid, s->credentials.pid);
	return 0;
}

err_t socket_accept(socket_s* out, socket_s* server){
	int fd;
	if( (fd = accept(server->fd, NULL, NULL)) == -1 ){
		dbg_error("can't accept connection");
		dbg_errno();
		return -1;
	}
    
    out->fd = fd;
    socket_get_credentials(out);
	return 0;
}

err_t socket_parse_events(int event, socket_s* s){
	dbg_info("event:: EPOLLIN:%d EPOLLHUP:%d EPOLLRDHUP:%d", !!(event&EPOLLIN), !!(event&EPOLLHUP), !!(event&EPOLLRDHUP));

	if( event & EPOLLHUP || event & EPOLLRDHUP ){
		dbg_info("close event");
		if( s->onclose) return s->onclose(s);
	}
	else if( event & EPOLLIN ) {
		dbg_info("read event");
		if( s->onread ) return s->onread(s);
	}
	else{
		dbg_warning("unknow event %d",event);
	}
	return 0;
}

int socket_isopen(socket_s* s){
	return s->fd == -1 ? 0 : 1;
}

int socket_status(socket_s* s){
	int ecode;
	unsigned int ecodesize = sizeof(int);
	getsockopt(s->fd, SOL_SOCKET, SO_ERROR, &ecode, &ecodesize);
	return ecode;
}

err_t socket_write(socket_s* s, const void* buf, size_t len){
	dbg_info("write %ld",len);
	const char* cur = buf;

	while( len > 0 ){
		ssize_t ret = write(s->fd, cur, len);
		if( ret < 0 ){
			if (errno == EINTR || errno == EAGAIN) continue;
			dbg_error("write");
			dbg_errno();
			return -1;
		}
		else if( ret == 0 ){
			if( len ){
				dbg_error("write return 0 but not send all data");
			}
			break;
		}
		len -= ret;
		cur += ret;
	}
	return 0;
}

err_t socket_puts(socket_s* s, const char* str, int zero){
	size_t len = strlen(str) + zero;
	return socket_write(s, str, len);
}

size_t socket_available(socket_s* s){
	int count;
	if( ioctl(s->fd, FIONREAD, &count) ){
		dbg_error("ioctl");
		dbg_errno();
		return s->unglen;
	}
	if( count < 0 ) return s->unglen;
	return count + s->unglen;
}

err_t socket_unget(socket_s* s, void* data, size_t size){
	dbg_info("ungetch %lu", size);
	if( size > s->ungsize - s->unglen ){
		size_t nsz = s->ungsize + ROUND_UP(size, SOCKET_UNGET_SIZE);
		void* nmem = realloc(s->ungetch, nsz);
		if( NULL == nmem ){
			dbg_error("realloc");
			dbg_errno();
			return -1;
		}
		s->ungetch = nmem;
		s->ungsize = nsz;
	}
	memcpy(&s->ungetch[s->unglen], data, size);
	s->unglen += size;
	return 0;
}

err_t socket_read(size_t* out, socket_s* s, void* buf, size_t len){
	if( s->unglen > 0 ){
		size_t nc = s->unglen > len ? len : s->unglen;
		memcpy(buf, s->ungetch, nc);
		s->unglen -= nc;
		if( s->unglen ){
			memmove(s->ungetch, &s->ungetch[nc], s->unglen); 
		}
		dbg_info("readed ungetch %ld", nc);
		if( out ) *out = nc;
		return 0;
		if( len - nc == 0 ){
			return 0;
		}
	}
	
	ssize_t ret = read(s->fd, buf, len);
	if( ret == -1 ) {
		dbg_error("read");
		dbg_errno();
		return -1;
	}
	if( out ) *out = (size_t)ret;
	dbg_info("readed %ld", ret);
	return 0;
}

int socket_gets(size_t* out, socket_s* s, char* dst, size_t size, int terminator){
	char* cur = dst;
	size_t rem = size;
	size_t total = 0;
	
	while( rem ){
		size_t nr = 0;
		char* endptr;
		if( !socket_available(s) ) return 0;
		if( socket_read(&nr, s, cur, rem) ){
			return -1;
		}
		if( nr == 0 ) return 0;
		total += nr;
		if( (endptr = memchr(cur, terminator, nr)) ){
			*out = (endptr-dst)+1;
			dbg_info("real len %lu raw len:%lu", *out, total);
			if( total > *out ){
				dbg_info("read many char, ungetch");
				size_t nug = total - *out;
				socket_unget(s, endptr+1, nug);
			}
			return 1;
		}
		rem -= nr;
		cur += nr;
	}
	return 0;
}

err_t socket_nget(socket_s* s, void* dst, size_t size){
	char* cur = dst;

	while(size){
		size_t nr = 0;
		//dbg_info("start");
		if( socket_read(&nr, s, cur, size) < 0 ){
			return -1;
		}
		if( nr == 0 && size){
			return -1;
		}
		size -= nr;
		//dbg_info("read %lu remain %lu", nr, size);
		cur += nr;
	}
	return 0;
}

void socket_flush(socket_s* s){
	fsync(s->fd);
}

err_t socket_wifi_info(char* device, char* essid, int* dbm, int* bitrate){
	int fd;
	struct iwreq rqsk;
	struct iw_statistics iwstat;

	if( (fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
		dbg_error("socket");
		dbg_errno();
		return -1;
	}
	
	//memset(&rqsk, 0, sizeof(struct iwreq));
	strcpy(rqsk.ifr_name, device);
	
	rqsk.u.essid.length = IW_ESSID_MAX_SIZE;
	rqsk.u.essid.pointer = essid;
	essid[0] = 0;
	if( ioctl(fd, SIOCGIWESSID, &rqsk) == -1 ){
		dbg_warning("ioctl essid");
		dbg_errno();
		essid[0] = 0;
	}
	else{
		essid[rqsk.u.essid.length] = 0;
	}

	//memset(&rqsk, 0, sizeof(struct iwreq));
	rqsk.u.essid.length = sizeof(struct iw_statistics);
	rqsk.u.essid.pointer = &iwstat;
	if( ioctl(fd, SIOCGIWSTATS, &rqsk) == -1 ){
		dbg_warning("ioctl stats");
		dbg_errno();
		*dbm = 0;
	}
	else if( ((struct iw_statistics *)rqsk.u.data.pointer)->qual.updated & IW_QUAL_DBM){
        //signal is measured in dBm and is valid for us to use
        *dbm = ((struct iw_statistics *)rqsk.u.data.pointer)->qual.level - 256;
	}
	else{
		*dbm = 0;
	}

	if( ioctl(fd, SIOCGIWRATE, &rqsk) == -1 ){
		dbg_warning("ioctl bitrate");
		dbg_errno();
		*bitrate = 0;
	}
	else{
		memcpy(&bitrate, &rqsk.u.bitrate.value, sizeof(int));
	}

	close(fd);
	return 0;
} 

