#ifndef __VBAR_JSON_H__
#define __VBAR_JSON_H__

#include <vbar/type.h>

typedef enum { JSON_ERROR = -1, JSON_NULL, JSON_STRING, JSON_NUMBER, JSON_OBJECT, JSON_ARRAY, JSON_BOOLEAN } jsonType_e;

typedef struct jsonPairs jsonPairs;

typedef struct jsonValue{
	jsonType_e type;
	size_t count;
	union {
		substr_s string;
		double number;
		int boolean;
		struct jsonPairs* object;
		struct jsonValue* array;
	};
}jsonValue_s;

typedef struct jsonPairs{
	substr_s name;
	jsonValue_s value;
}jsonPairs_s;

typedef struct json{
	jsonValue_s* value;
}json_s;

void json_free(json_s* json);
int json_decode(json_s* json, char* str);
jsonPairs_s* json_object_find(jsonValue_s* objects, char* name);
void json_print(json_s* json);

#endif
