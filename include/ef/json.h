#ifndef __EF_JSON_H__
#define __EF_JSON_H__

#include <ef/type.h>
#include <ef/stack.h>

typedef enum {
	JSON_OK, 
	JSON_ERR_STRING_END, 
	JSON_ERR_STRING_LEN, 
	JSON_ERR_NUMBER,
	JSON_ERR_CONSTANT,
	JSON_ERR_OBJECT_NAME,
	JSON_ERR_OBJECT_COLON,
	JSON_ERR_OBJECT_VALUE,
	JSON_ERR_OBJECT_END,
	JSON_ERR_ARRAY_END,
	JSON_ERR_UNASPECTED_CHAR,
	JSON_ERR_USER,
	JSON_ERR_COUNT
}jsonError_e;

typedef err_t(*jsonEvent_f)(void* ctx);
typedef err_t(*jsonEventName_f)(void* ctx, char const* name, size_t len);

typedef struct json{
	jsonEvent_f objectNew;
	jsonEvent_f objectNext;
	jsonEventName_f objectProperties;
	jsonEvent_f objectEnd;
	jsonEvent_f arrayNew;
	jsonEvent_f arrayNext;
	jsonEvent_f arrayEnd;
	jsonEvent_f valueNull;
	jsonEvent_f valueTrue;
	jsonEvent_f valueFalse;
	jsonEventName_f valueInteger;
	jsonEventName_f valueFloat;
	jsonEventName_f valueString;
	void* usrctx;
	size_t cline;
	char const* text;
	char const* beginLine;
	char const* current;
	jsonError_e err;
}json_s;

err_t json_lexer(json_s* json, char const* data);
void json_error(json_s* json);

#endif

