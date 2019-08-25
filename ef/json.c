#include <ef/json.h>

__private char* jsonErrStr[JSON_ERR_COUNT] = {
	[JSON_ERR_STRING_END]      = "string not terminated with \"",
	[JSON_ERR_STRING_LEN]      = "string length",
	[JSON_ERR_NUMBER]          = "number",
	[JSON_ERR_CONSTANT]        = "not valid constant true/false/null",
	[JSON_ERR_OBJECT_NAME]     = "object not have name",
	[JSON_ERR_OBJECT_COLON]    = "name not separated with colon",
	[JSON_ERR_OBJECT_VALUE]    = "object not have value",
	[JSON_ERR_OBJECT_END]      = "object not terminated with }",
	[JSON_ERR_ARRAY_END]       = "array not terminated with ]",
	[JSON_ERR_UNASPECTED_CHAR] = "unaspected char",
	[JSON_ERR_USER]            = "user error"
};

void json_error(json_s* json){
	if( json->err == 0 ){
		fputs("json ok\n", stderr);
		return;
	}
	if( json->err < 0 || json->err >= JSON_ERR_COUNT ){
		fprintf(stderr, "json error(%d):unknow descript error, on line %lu\n", json->err, json->cline);
	}
	else{
		fprintf(stderr, "json error(%d):%s on line %lu\n", json->err, jsonErrStr[json->err], json->cline);
	}
	char const* ch = json->beginLine;
	char const* bl = json->beginLine;
	size_t max = 80; 
	if( json->current - json->beginLine > 80 ){
		bl = json->current - 40;
		ch = bl;
	}
	while( *ch && *ch != '\n' && max-->0 ) fputc(*ch++, stderr);
	fputc('\n',stderr);	
	size_t off = json->current - bl;
	while( off-->0 ) fputc(' ', stderr);
	fputs("^\n",stderr);
}

inline __private void json_token_next(json_s* json){
	while(1){
		while( *json->current && (*json->current == ' ' || *json->current == '\t') ) ++json->current;
		if( *json->current == '\n' ){
			++json->current;
			json->beginLine = json->current;
			++json->cline;
			continue;
		}
		break;
	}
}

__private err_t json_string_size(char const* data, size_t* len){
	char const* begin = data;
	while( *data && *data != '"' ){
		if( *data == '\\' ){
			data++;
		}
		data++;
	}
	if( *data != '"' ) return JSON_ERR_STRING_END;
	*len = data - begin;
	if( *len == 0 ) return JSON_ERR_STRING_LEN;
	return JSON_OK;
}

inline __private int json_number_isdigt(char ch){
	return (ch >= '0' && ch <= '9') ? 1 : 0;
}	

__private err_t json_number_size(char const* data, size_t* len, int* isdouble){
	char const* begin = data;
	*isdouble = 0;
	if( *data == '-' ) ++data;
	while( json_number_isdigt(*data) ) ++data;
	if( *data == '.' ){
		*isdouble = 1;
	   	++data;
		if( !json_number_isdigt(*data) ) goto ONERR;
		while( json_number_isdigt(*data) ) ++data;
	}

	if( *data == 'e' || *data == 'E' ){
		++data;
		if( *data == '+' || *data == '-' ) ++data;
		if( !json_number_isdigt(*data) ) goto ONERR;
	}

	*len = data - begin;
	if( *len == 0 ) goto ONERR;
	return JSON_OK;
ONERR:
	return JSON_ERR_NUMBER;
}

__private int json_lexer_value(json_s* json);

__private int json_lexer_object(json_s* json){
	if( json->objectNew && (json->err=json->objectNew(json->usrctx)) ) return -1;
	do{
		++json->current;
		json_token_next(json);
		if( *json->current == '}' ) break;
		if( json->objectNext && (json->err=json->objectNext(json->usrctx)) ) return -1;

		if( *json->current != '"' ){
			json->err = JSON_ERR_OBJECT_NAME;
			return -1;
		}
		++json->current;
		size_t len;
		if( (json->err=json_string_size(json->current, &len)) ) return -1;
		if( json->objectProperties && (json->err=json->objectProperties(json->usrctx, json->current, len)) ) return -1;
		json->current += len + 1;
		json_token_next(json);
		if( *json->current != ':' ){
			json->err = JSON_ERR_OBJECT_COLON;
			return -1;
		}
		++json->current;
		if( json_lexer_value(json) ) return -1;
		json_token_next(json);
	}while(*json->current == ',');
	if( *json->current != '}' ){
		json->err = JSON_ERR_OBJECT_END;
		return -1;
	}
	++json->current;
	if( json->objectEnd && (json->err=json->objectEnd(json->usrctx)) ) return -1;
	return 0;
}

__private int json_lexer_array(json_s* json){
	if( json->arrayNew && (json->err=json->arrayNew(json->usrctx)) ) return -1;
	do{
		++json->current;
		json_token_next(json);
		if( *json->current == ']' ) break;
		if( json->arrayNext && (json->err=json->arrayNext(json->usrctx)) ) return -1;
		if( json_lexer_value(json) ) return -1;
		json_token_next(json);
	}while(*json->current == ',');
	if( *json->current != ']' ){
		json->err = JSON_ERR_ARRAY_END;
		return -1;
	}
	++json->current;
	if( json->arrayEnd && (json->err=json->arrayEnd(json->usrctx)) ) return -1;
	return 0;
}

__private int json_lexer_value(json_s* json){
	switch( *json->current ){
		case 0: return 1;
		
		case '{': return json_lexer_object(json);
		
		case '[': return json_lexer_array(json);

		case '"':{
			size_t len;
			++json->current;
			if( (json->err=json_string_size(json->current, &len)) ) return -1;
			if( json->valueString && (json->err=json->valueString(json->usrctx, json->current, len)) ) return -1;
			json->current += len + 1;
		}
		break;

		case '-': case '0' ... '9':{
			size_t len;
			int isdouble;
			if( (json->err=json_number_size(json->current, &len, &isdouble)) ) return -1;
			if( isdouble ){
				if( json->valueFloat && (json->err=json->valueFloat(json->usrctx, json->current, len)) ) return -1;
			}
			else{
				if( json->valueInteger && (json->err=json->valueInteger(json->usrctx, json->current, len)) ) return -1;
			}
			json->current += len;
		}
		break;

		case 't':
			if( strncmp(json->current, "true", 4) ){
				json->err = JSON_ERR_CONSTANT;
			   	return -1;
			}
			if( json->valueTrue && (json->err=json->valueTrue(json->usrctx)) ) return -1;
			json->current += 4;
		break;

		case 'f':
			if( strncmp(json->current, "false", 5) ){
				json->err = JSON_ERR_CONSTANT;
			   	return -1;
			}
			if( json->valueFalse && (json->err=json->valueFalse(json->usrctx)) ) return -1;
			json->current += 5;
		break;

		case 'n':
			if( strncmp(json->current, "null", 4) ){
				json->err = JSON_ERR_CONSTANT;
			   	return -1;
			}
			if( json->valueNull && (json->err=json->valueNull(json->usrctx)) ) return -1;
			json->current += 4;
		break;

		default: json->err = JSON_ERR_UNASPECTED_CHAR; return -1;
	}

	return 0;
}

err_t json_lexer(json_s* json, char const* data){
	json->cline = 0;
	json->err = JSON_OK;
	json->beginLine = json->text = data;
	json->current = data-1;
	do{
		++json->current;
		json_token_next(json);
		int ret = json_lexer_value(json);
		switch( ret ){	
			case -1: return -1;
			case  1: return  0;
		} 
		json_token_next(json);
	}while(*json->current == ',');
	if( *json->current ) json->err = JSON_ERR_UNASPECTED_CHAR;
	return *json->current;
}

