#ifndef __VBAR_WEB_H__
#define __VBAR_WEB_H__

#include <vbar/type.h>
#include <curl/curl.h>

#define WEB_BUFFER_PAGE 4096

typedef struct webBuffer{
	char* data;
	size_t len;
	size_t size;
}webBuffer_s;

typedef struct web{
	CURL* handle;
	webBuffer_s header;
	webBuffer_s body;
}web_s;

#define __ef_web_autofree __ef_cleanup(web_autoterminate)

int web_init(web_s* wb);
void web_terminate(web_s* wb);
void web_autoterminate(web_s* wb);
void web_url(web_s* wb, char* url);
void web_ssl(web_s* wb, int ssl);
void web_download_header(web_s* wb);
void web_download_body(web_s* wb);
int web_perform(web_s* wb);


#endif
