#include <vbar/web.h>
#include <vbar/memory.h>

__ef_private void web_buffer_need_len(webBuffer_s* buf, size_t size){
	if( buf->data == NULL ){
		buf->data = ef_mem_many(char, size);
		buf->len = 0;
		buf->size = size;
	}
	else if( buf->size - buf->len < size ){
		size += buf->size;
		size = ROUND_UP(size, WEB_BUFFER_PAGE);  
		char* nwb = realloc(buf->data, size);
		if( nwb == NULL ){
			dbg_errno();
			dbg_fail("realloc");
		}
		buf->data = nwb;
		buf->size = size;
	}
}


__ef_private size_t web_download_callback(void* ptr, size_t size, size_t nmemb, void* userp){
	webBuffer_s* buffer = userp;
	size *= nmemb;
	web_buffer_need_len(buffer, size);
	memcpy(&buffer->data[buffer->len], ptr, size);
	buffer->len += size;
	return size;
}

int web_init(web_s* wb){
	if ( !(wb->handle = curl_easy_init()) ){
		dbg_error("curl easy init");
		return -1;
	}
	curl_easy_setopt(wb->handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(wb->handle, CURLOPT_VERBOSE, 0L);
	
	wb->body.data = NULL;
	wb->body.len = 0;
	wb->body.size = 0;
	wb->header.data = NULL;
	wb->header.len = 0;
	wb->header.size = 0;
	return 0;
}

void web_terminate(web_s* wb){
	curl_easy_cleanup(wb->handle);
	free(wb->body.data);
	free(wb->header.data);
}

void web_autoterminate(web_s* wb){
	if( NULL == wb->handle ){
		dbg_warning("already closed");
		return;
	}
	web_terminate(wb);
}

void web_url(web_s* wb, char* url){
	curl_easy_setopt(wb->handle, CURLOPT_URL, url);
}

void web_ssl(web_s* wb, int ssl){

	switch( ssl ){
		case 0:	
			curl_easy_setopt(wb->handle, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(wb->handle, CURLOPT_SSL_VERIFYHOST, 0L);
		break;

		case 1:
			curl_easy_setopt(wb->handle, CURLOPT_USE_SSL, CURLUSESSL_ALL);
			curl_easy_setopt(wb->handle, CURLOPT_SSL_VERIFYPEER, 1L);
			curl_easy_setopt(wb->handle, CURLOPT_SSL_VERIFYHOST, 0L);
		break;

		case 2:
			curl_easy_setopt(wb->handle, CURLOPT_USE_SSL, CURLUSESSL_ALL);
			curl_easy_setopt(wb->handle, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(wb->handle, CURLOPT_SSL_VERIFYHOST, 2L);
		break;

		case 3: 
			curl_easy_setopt(wb->handle, CURLOPT_USE_SSL, CURLUSESSL_ALL);
			curl_easy_setopt(wb->handle, CURLOPT_SSL_VERIFYPEER, 1L);
			curl_easy_setopt(wb->handle, CURLOPT_SSL_VERIFYHOST, 2L);
		break;
	}
}

void web_download_header(web_s* wb){
	wb->header.len = 0;
	curl_easy_setopt(wb->handle, CURLOPT_WRITEFUNCTION, web_download_callback);
	curl_easy_setopt(wb->handle, CURLOPT_HEADERDATA, &wb->header);
}

void web_download_body(web_s* wb){
	wb->body.len = 0;
	curl_easy_setopt(wb->handle, CURLOPT_WRITEFUNCTION, web_download_callback);
    curl_easy_setopt(wb->handle, CURLOPT_WRITEDATA, &wb->body);
}

int web_perform(web_s* wb){
	CURLcode res;
	res = curl_easy_perform(wb->handle);
	
	if ( res != CURLE_OK ){
		errno = res;
		dbg_error("web perform request");
		dbg_error("curl %d descript: %s", res, curl_easy_strerror(res));
		return -1;
	}
	return 0;
}
