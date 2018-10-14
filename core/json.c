#include <vbar.h>
#include <vbar/json.h>
#include <vbar/memory.h>
#include <vbar/string.h>

__ef_private inline jsonType_e json_type(char** json){
	*json = str_skip_hn(*json);
	//dbg_info("type for %c", **json);
	switch( **json ){
		case '{': return JSON_OBJECT;
		case '[': return JSON_ARRAY;
		case '"': return JSON_STRING;
		case '-': case '0' ... '9': return JSON_NUMBER;
		case 'n': return strncmp(*json, "null", 4) ? JSON_ERROR : JSON_NULL;
		case 't': return strncmp(*json, "true", 4) ? JSON_ERROR : JSON_BOOLEAN;
		case 'f': return strncmp(*json, "false", 5) ? JSON_ERROR : JSON_BOOLEAN;
	}
	return JSON_ERROR;
}

__ef_private int json_element_count(size_t* out, char* json){
	*out = 0;
	if( *json == 0 || (*json != '{' && *json != '[') ){
		dbg_error("json not begin {[");
		return -1;
	}
	char ch = *str_skip_hn(json+1);
	if( ch == '}' || ch == ']' ){
		return 0;
	}
	
	*out = 1;
	int balance = 0;
	do{
		if( *json == '{' || *json == '[' ) ++balance;
		else if( *json == '}' || *json == ']' ) --balance;
		else if( balance == 1 && *json == ',' ) ++(*out);
		++json;
	}while( *json && balance );
	//dbg_info("have %lu element", *out);
	if( balance ){
		dbg_error("not balance %d", balance);
	}
	return balance ? -1 : 0;
}

__ef_private char* json_pairs_name(substr_s* name, char* json){
	json = str_skip_hn(json);
	
	if( *json != '"' ){
		dbg_error("format json aspect \"");
		return NULL;
	}
	++json;
	name->begin = json;
	name->end = strchr(json, '"');
	if( !name->end ){
		dbg_error("not end quote");
		return NULL;
	}
	
	json = str_skip_hn(name->end+1);
	if( *json != ':' ){
		dbg_error("format json aspect :");
		return NULL;
	}

	//dbg_info("pair name %.*s", (int)substr_len(name), name->begin);
	return json+1;
}

__ef_private int json_value_decode(jsonValue_s* value, char** json){
	switch( value->type ){
		case JSON_NULL:
			//dbg_info("decode null");
			return 0;
		break;

		case JSON_NUMBER:{
			char* ends;
			value->number = strtod(*json, &ends);
			*json = ends;
			//dbg_info("decode number %lf", value->number);
		}
		break;

		case JSON_STRING:
			++(*json);
			value->string.begin = *json;
			value->string.end = strchr(*json, '"');
			if( value->string.end == NULL ){
				dbg_error("quote not terminated");
				return -1;
			}
			*json = value->string.end + 1;
			//dbg_info("decode string %.*s", (int)substr_len(&value->string), value->string.begin);
		break;

		case JSON_BOOLEAN:
			value->boolean = **json == 't' ? 1 : 0;
			*json += value->boolean ? 4 : 5;
			//dbg_info("decode boolean %d", value->boolean);
		break;

		case JSON_OBJECT:
			//dbg_info("decode object");
			if( json_element_count(&value->count, *json) ){
				dbg_error("fail to count object");
				value->type = JSON_ERROR;
				return -1;
			}

			value->object = ef_mem_many(jsonPairs_s, value->count);
			for( size_t i = 0; i < value->count; ++i )
				value->object[i].value.type = JSON_ERROR;
			
			++(*json);
			for( size_t i = 0; i < value->count; ++i ){
				*json = json_pairs_name(&value->object[i].name, *json);
				if( *json == NULL ){
					dbg_error("wrong json object name");
					value->object[i].value.type = JSON_ERROR;
					return -1;
				}
				value->object[i].value.type = json_type(json);
				if( json_value_decode(&value->object[i].value, json) ){
					dbg_error("wrong json object");
					value->object[i].value.type = JSON_ERROR;
					return -1;
				}
				*json = str_skip_hn(*json);
				if( **json == ',' ) ++(*json);
			}
			*json = strchr(*json, '}');
			if( **json != '}' ){
				dbg_error("fail to close object");
				return -1;
			}
			++(*json);
		break;

		case JSON_ARRAY:
			//dbg_info("decode array");
			if( json_element_count(&value->count, *json) ){
				dbg_error("fail to count array");
				value->type = JSON_ERROR;
				return -1;
			}

			value->array = ef_mem_many(jsonValue_s, value->count);
			for( size_t i = 0; i < value->count; ++i )
				value->array[i].type = JSON_ERROR;
			
			++(*json);
			for( size_t i = 0; i < value->count; ++i ){
				value->array[i].type = json_type(json);
				if( json_value_decode(&value->array[i], json) ){
					dbg_error("wrong json element");
					value->array[i].type = JSON_ERROR;
					return -1;
				}
				*json = str_skip_hn(*json);
				if( **json == ',' ) ++(*json);
			}
			*json = strchr(*json, ']');
			if( **json != ']' ){
				dbg_error("fail to close array");
				return -1;
			}
			++(*json);
		break;

		default: case JSON_ERROR:
			dbg_error("decode error");
			return -1;
		break;
	}

	return 0;
}

__ef_private void json_value_free(jsonValue_s* value){
	if( value->type == JSON_OBJECT ){
		for(size_t i = 0; i < value->count; ++i){
			if( value->object[i].value.type == JSON_OBJECT || value->object[i].value.type == JSON_ARRAY ){
				json_value_free(&value->object[i].value);
			}
		}
		free(value->object);
		value->object = NULL;
	}
	else if( value->type == JSON_ARRAY ){
		for(size_t i = 0; i < value->count; ++i){
			if( value->array[i].type == JSON_OBJECT || value->array[i].type == JSON_ARRAY ){
				json_value_free(&value->array[i]);
			}
		}
		free(value->array);
		value->array = NULL;
	}
}

void json_free(json_s* json){
	json_value_free(json->value);
	free(json->value);
}

int json_decode(json_s* json, char* str){
	jsonValue_s* current = json->value = ef_mem_new(jsonValue_s);

	current->type = json_type(&str);
	if( json_value_decode(json->value, &str) ){
		dbg_error("json format error");
		return -1;
	}

	return 0;
}

jsonPairs_s* json_object_find(jsonValue_s* objects, char* name){
	if( objects->type != JSON_OBJECT ){
		dbg_warning("jsonValue isn't object");
		return NULL;
	}

	for( size_t i = 0; i < objects->count; ++i ){
		if( !substr_cmp_str(&objects->object[i].name, name) ){
			return &objects->object[i];
		}
	}
	dbg_warning("not find %s", name);
	return NULL;
}

__ef_private void print_tab(int tab){
	while( tab --> 0 ){
		putchar('\t');
	}
}

__ef_private void json_print_value(jsonValue_s* value, int tab){
	switch( value->type ){
		case JSON_ERROR:
			puts("error");
		break;

		case JSON_NULL:
			puts("null");
		break;

		case JSON_BOOLEAN:
			printf("%s\n", value->boolean ? "true" : "false");
		break;

		case JSON_NUMBER:
			printf("%lf\n", value->number ); 
		break;

		case JSON_STRING:
			printf("\"%.*s\"\n", (int)substr_len(&value->string), value->string.begin);
		break;

		case JSON_OBJECT:
			//print_tab(tab);
			puts("{");
			for(size_t i = 0; i < value->count; ++i){
				print_tab(tab);
				printf("\"%.*s\"", (int)substr_len(&value->object[i].name), value->object[i].name.begin);
				putchar(':');
				json_print_value(&value->object[i].value, tab+1);
			}
			print_tab(tab);
			puts("}");
		break;

		case JSON_ARRAY:
			//print_tab(tab);
			puts("[");
			for(size_t i = 0; i < value->count; ++i){
				print_tab(tab);
				json_print_value(&value->array[i], tab+1);
			}
			print_tab(tab);
			puts("]");
		break;

		default:
			puts("unknow");
		break;
	}
}

void json_print(json_s* json){
	json_print_value(json->value, 0);	
}
