#include <vbar/config.h>
#include <vbar/string.h>
#include <pwd.h>
#include <sys/types.h>

/*
 * #comment
 * nome = value | str | int | uint | double
 * nome = "value value" str
 * nome[n] = value
*/

#define CONF_BUFF_LINE_SIZE 512
#define CONF_NAME_MAX 128
#define CONF_VALUE_MAX 128
#define CONF_COMMENT '#'
#define CONF_END_NAME " =[\t"
#define CONF_VECTOR_START '['
#define CONF_VECTOR_END ']'
#define CONF_ASSIGN '='
#define CONF_QUOTE '\''
#define CONF_DOUBLE_QUOTE '"'
#define CONF_VALUE_END " \t\n#"
#define CONF_STR_QUOTE "\'"
#define CONF_STR_DOUBLE_QUOTE "\""

#define HASH_F(H,CH) ((CH) + 31 * (H))
#define HASH_N(H,SZ) ((H) % (SZ))

err_t ef_path_home(char* path){
        char *hd;
        if( (hd = secure_getenv("HOME")) == NULL ){
                struct passwd* spwd = getpwuid(getuid());
                catch_null( spwd ){
                        dbg_error("no home available");
                        dbg_errno();
                        *path = 0;
                        return errno;
                }
                strcpy(path, spwd->pw_dir);
        }
        else{
                strcpy(path, hd);
        }
        return 0;
}

__ef_private int path_resolve(char* path, char* res){
	if( *path == '~' ){
		if( ef_path_home(res) ){
			return -1;
		}
		strcpy(&res[strlen(res)], &path[1]);
		return 0;
	}
	strcpy(res, path);
	return 0;
}

__ef_private size_t kr_hash(char*s, size_t size){
	size_t hash;
	for( hash = 0; *s; ++s)
		hash = *s + 31 * hash;
	return hash % size;
}

__ef_private size_t kr_nhash(char*s, size_t len, size_t size){
	size_t hash = 0;
	for( size_t i = 0; i < len; ++i)
		hash = s[i] + 31 * hash;
	return hash % size;
}

void config_init(config_s* cf, size_t maxhash){
	cf->count = maxhash;
	cf->elems = ef_mem_many(configElement_s*, maxhash);
	for( size_t i = 0; i < maxhash; ++i )
		cf->elems[i] = NULL;
}

void config_destroy(config_s* cf){
	for( size_t i = 0; i < cf->count; ++i ){
		configElement_s* next;
		while( cf->elems[i] ){
			next = cf->elems[i]->next;
			free(cf->elems[i]);
			cf->elems[i] = next;
		}	
	}
	free(cf->elems);
}

void config_add(config_s* cf, char* name, config_e type, void* ptr, size_t maxlen, size_t isvector){
	size_t hash = kr_hash(name, cf->count);
	configElement_s* ce = ef_mem_new(configElement_s);
	ce->name = name;
	ce->type = type;
	ce->isvector = isvector;
	ce->ptr = ptr;
	ce->maxlen = maxlen;
	ce->next = cf->elems[hash];
	cf->elems[hash] = ce;
}

__ef_private configElement_s* config_search(config_s* cf, char* name, size_t len, size_t hash){
	configElement_s* ce = cf->elems[hash];
	for(; ce; ce = ce->next ){
		if( 0 == str_len_cmp(ce->name, strlen(ce->name), name, len) ){
			return ce;
		}
	}
	return NULL;
}

__ef_private int config_value_base(char* value, size_t len){
	if( *value++ != '0' || len-- == 0 ) return 10;
	if( *value++ != 'x' || len == 0 ) return 8;
	return 16;
}

__ef_private void config_assign(config_s* cf, char* name, size_t lenName, char* value, size_t lenValue, size_t index){
	size_t hash = kr_nhash(name, lenName, cf->count);
	
	configElement_s* ce = config_search(cf, name, lenName, hash);
	if( ce == NULL ){
		dbg_warning("config '%.*s' is not set", (int)lenName, name);
		return;
	}

	if( index > ce->isvector ){
		dbg_warning("config '%s[%lu]' buffer overflow with index %lu", ce->name, ce->isvector, index);
		return;
	}
	
	char* enparse = value;
	int base;
	errno  = 0;

	switch( ce->type ){
		case CNF_LF:
			((double*)ce->ptr)[index] = strtod(value, &enparse);
		break;

		case CNF_F:
			((float*)ce->ptr)[index] = strtof(value, &enparse);
		break;

		case CNF_LU:
			base = config_value_base(value, lenValue);
			((unsigned long*)ce->ptr)[index] = strtoul(value, &enparse, base);
		break;

		case CNF_U:
			base = config_value_base(value, lenValue);
			((unsigned*)ce->ptr)[index] = strtoul(value, &enparse, base);
		break;
		
		case CNF_LD:
			base = config_value_base(value, lenValue);
			((long*)ce->ptr)[index] = strtol(value, &enparse, base);
		break;

		case CNF_D:
			base = config_value_base(value, lenValue);
			((int*)ce->ptr)[index] = strtol(value, &enparse, base);
		break;
		
		case CNF_C:
			((char*)ce->ptr)[index] = *value;
		break;

		case CNF_S:
			if( ce->isvector )
				enparse = str_nncpy_src(((char**)ce->ptr)[index], ce->maxlen, value, lenValue - 1);
			else
				enparse = str_nncpy_src(ce->ptr, ce->maxlen, value, lenValue - 1);
		break;

		default:
			dbg_error("type configElems not exists");
		break;
	}

	if( enparse < value + lenValue || errno != 0 ){
		dbg_warning("incorrect(%d) parsing value on %.*s[%lu] = '%.*s'", errno,(int)lenName, name, index, (int)lenValue, value);
	}
}

void config_load(config_s* cf, char* fconf){
	char rp[PATH_MAX];
	if( path_resolve(fconf, rp) ){
		dbg_errno();
		dbg_fail("realpath %s", fconf);
		return;
	}
	dbg_info("real path %s", rp);

	FILE* fd = fopen(rp, "r");
	if( fd == NULL ){
		dbg_warning("no config file %s", rp);
		dbg_errno();
		return;
	}
	
	char line[CONF_BUFF_LINE_SIZE];
	while( fgets(line, CONF_BUFF_LINE_SIZE, fd) ){
		/*dbg_info("parse %.*s", (int)strlen(line)-1, line);*/
		char* parse = str_skip_h(line);
		if( 0 == *parse || *parse == CONF_COMMENT || *parse == '\n' ) continue;
		
		char* name = parse;
		parse = strpbrk(parse, CONF_END_NAME);
		if( parse == NULL ){
			dbg_warning("malformed name on line '%s'", line);
			continue;
		}
		size_t lenName = parse - name;

		size_t index = 0;
		parse = str_skip_h(parse);
		if( *parse == CONF_VECTOR_START ){
			parse = str_skip_h(parse + 1);
			index = strtoul(parse, &parse, 10);
			parse = strchr(parse, CONF_VECTOR_END);
			if( parse == NULL || *parse == 0 ){
				dbg_warning("malformed index on line '%s'", line);
				continue;
			}
			++parse;
			parse = str_skip_h(parse);
		}
	
		if( *parse != CONF_ASSIGN ){
			dbg_warning("not assigne value on line '%s'", line);
			continue;
		}
		++parse;

		parse = str_skip_h(parse);
		char* enval;
		switch( *parse ){
			case CONF_QUOTE:
				++parse;
				enval = CONF_STR_QUOTE;
			break;
			
			case CONF_DOUBLE_QUOTE:
				++parse;
				enval = CONF_STR_DOUBLE_QUOTE;
			break;

			default:
				enval = CONF_VALUE_END;
			break;
		}
		char* value = parse;
		parse = strpbrk(parse, enval);
		if( NULL == parse ){
			dbg_warning("quote not closed on line '%s'", line);
			continue;
		}
		size_t lenValue = (parse - value);
		//dbg_info("%.*s[%lu] = %.*s", (int)lenName, name, index, (int)lenValue, value);
		config_assign(cf,name, lenName, value, lenValue, index);
	}
	dbg_info("end parse");
}

