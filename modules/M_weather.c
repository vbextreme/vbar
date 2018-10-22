#include <vbar.h>
#include <vbar/web.h>
#include <vbar/json.h>
#include <time.h>

#define OW_DAY 5
#define OW_HOURS 8
#define OW_HOURS_STEP 3
#define OW_CITY 128
#define OW_TEXT_MAX 32

typedef enum { 
	OW_TEMP, 
	OW_TEMP_MIN,
	OW_TEMP_MAX,
	OW_PRESSURE,
	OW_SEA_LEVEL,
	OW_HUMIDITY,
	OW_CLOUDS,
	OW_WIND_SPEED,
	OW_WIND_DEG,
	OW_RAIN,
	OW_COUNT
} ownumbertype_e;

typedef enum {
	OW_WEATHER_MAIN,
	OW_WEATHER_DESCRIPTION,
	OW_WEATHER_ICON,
	OW_DATETIME,
	OW_TEXT_COUNT
}owstring_e;

typedef struct owInfo{
	int isset;
	double data[OW_COUNT];
	char text[OW_TEXT_COUNT][OW_TEXT_MAX];
}owInfo_s;

typedef struct openweather{
	char city[OW_CITY];
	owInfo_s info[OW_DAY][OW_HOURS];
	unsigned curdd;
	unsigned curhh;
}openweather_s;

#define WEATHER_URL "https://api.openweathermap.org/data/2.5"
#define CITY_MAX 16
#define TOKEN_MAX 35

typedef struct weather{
	char city[CITY_MAX];
	char token[TOKEN_MAX];
	openweather_s ow;
}weather_s;

__ef_private void openweather_api_fiveday_url(char* dest, char* city, char* token){
	sprintf(dest, "%s/forecast?id=%s&APPID=%s", WEATHER_URL, city, token);
}

__ef_private int openweather_api_fiveday(openweather_s* ow, char* city, char* token){
	char url[1024];
	openweather_api_fiveday_url(url, city, token);
	dbg_info("url %s", url);

	for( int d = 0; d < OW_DAY; ++d )
		for( int h = 0; h < OW_HOURS; ++h)
			ow->info[d][h].isset = 0;

	__ef_web_autofree web_s rest;
	if( web_init(&rest) ){
		return -1;
	}
	
	web_ssl(&rest, 3);
	web_url(&rest, url);
	web_download_body(&rest);
	dbg_info("perform");
	if( web_perform(&rest) ){
		return -1;
	}
	
	dbg_info("body   size %lu", rest.body.len);		
	
	json_s json;
	dbg_info("decode");
	if( json_decode(&json, rest.body.data) ) goto ONERROR;

	jsonPairs_s* pairs = json_object_find(json.value, "list");
	if( pairs == NULL ){
		dbg_error("no list");
		goto ONERROR;
	}

	if( pairs->value.type != JSON_ARRAY ){
		dbg_error("not have array of days");
		goto ONERROR;
	}
	
	if( pairs->value.count < 1 ){
		dbg_error("no have five days");
		goto ONERROR;
	}

	/*get begin date*/
	int dd = 0;
	jsonPairs_s* dt = json_object_find(&pairs->value.array[0], "dt_txt");
	if( dt == NULL ){
		dbg_error("day one not have datetime");
		goto ONERROR;
	}
	char* tohh = strchr(dt->value.string.begin, ' ');
	if( !tohh || tohh >= (dt->value.string.end-3) ){
		dbg_error("datetime %.*s not respect format", (int)substr_len(&dt->value.string), dt->value.string.begin);	
		goto ONERROR;
	}
	++tohh;
	if( *tohh < '0' || *tohh > '2' ){
		dbg_error("datetime %.*s not respect format", (int)substr_len(&dt->value.string), dt->value.string.begin);	
		goto ONERROR;
	}
	int hh = ((*tohh - '0') * 10 + *(tohh+1) - '0') / 3;
	dbg_info("begin at %d", hh);
	ow->curdd = dd;
	ow->curhh = hh;

	/*scan day && hours*/
	static char* owmain[] = { "temp", "temp_min", "temp_max", "pressure", "sea_level", "humidity", NULL };
	static char* owwind[] = { "speed", "deg", NULL };
	static char* owweather[] = { "main", "description", "icon", NULL };

	for( size_t i = 0; i < pairs->value.count && dd < OW_DAY; ++i ){
		jsonValue_s* obj = &pairs->value.array[i];
		if( obj->type != JSON_OBJECT ) goto ONERROR;
		jsonPairs_s* ele;
		jsonPairs_s* prop;

		if( (ele = json_object_find(obj, "main")) == NULL ) goto ONERROR;
		for( size_t j = 0; owmain[j]; ++j ){
			if( (prop = json_object_find(&ele->value, owmain[j])) == NULL ) goto ONERROR;
			//dbg_info("%s::%lf", owmain[j], prop->value.number);
			ow->info[dd][hh].data[j] = prop->value.number;
		}

		if( (ele = json_object_find(obj, "wind")) == NULL ) goto ONERROR;
		for( size_t j = OW_WIND_SPEED; owwind[j]; ++j ){
			if( (prop = json_object_find(&ele->value, owwind[j])) == NULL ) goto ONERROR;
			//dbg_info("%s::%lf", owwind[j], prop->value.number);
			ow->info[dd][hh].data[j] = prop->value.number;
		}

		if( (ele = json_object_find(obj, "clouds")) == NULL ) goto ONERROR;
		if( (prop = json_object_find(&ele->value, "all")) == NULL ) goto ONERROR;
		//dbg_info("clouds:%lf", prop->value.number);
		ow->info[dd][hh].data[OW_CLOUDS] = prop->value.number;

		if( (ele = json_object_find(obj, "rain")) == NULL ){
			//dbg_info("no rain set");
			ow->info[dd][hh].data[OW_RAIN] = 0.0;
		}
		else{
			if( (prop = json_object_find(&ele->value, "3h")) == NULL ){
				//dbg_info("rain: unset no rain");
				ow->info[dd][hh].data[OW_RAIN] = 0.0;
			}
			else{
				//dbg_info("rain:%lf", prop->value.number);
				ow->info[dd][hh].data[OW_RAIN] = prop->value.number;
			}
		}

		if( (ele = json_object_find(obj, "weather")) == NULL ) goto ONERROR;
		if( ele->value.type != JSON_ARRAY ) goto ONERROR;
		if( ele->value.array[0].type != JSON_OBJECT ) goto ONERROR;
		for( size_t j = 0; owweather[j]; ++j ){
			if( (prop = json_object_find(&ele->value.array[0], owweather[j])) == NULL ) goto ONERROR;
			//dbg_info("weather.%s:%.*s", owweather[j], (int)substr_len(&prop->value.string), prop->value.string.begin);
			str_nncpy_src(ow->info[dd][hh].text[j], OW_TEXT_MAX, prop->value.string.begin, substr_len(&prop->value.string));
		}

		if( (ele = json_object_find(obj, "dt_txt")) == NULL ) goto ONERROR;
		//dbg_info("datetime:%.*s", (int)substr_len(&ele->value.string), ele->value.string.begin);
		str_nncpy_src(ow->info[dd][hh].text[OW_DATETIME], OW_TEXT_MAX, ele->value.string.begin, substr_len(&ele->value.string));

		ow->info[dd][hh].isset = 1;

		++hh;
		dd += (hh >> 3) & 1;
		hh &= 7;
		//dbg_info("next dd %d hh %d", dd, hh);
	}

	if( (pairs = json_object_find(json.value, "city")) == NULL ) goto ONERROR;
	if( (pairs = json_object_find(&pairs->value, "name")) == NULL ) goto ONERROR;
	//dbg_info("city:%.*s", (int)substr_len(&pairs->value.string), pairs->value.string.begin);
	str_nncpy_src(ow->city, OW_CITY, pairs->value.string.begin, substr_len(&pairs->value.string));

	json_free(&json);
	return 0;

ONERROR:
	dbg_error("json format error");
	json_free(&json);
	return -1;
}

__ef_private int weather_mod_refresh(__ef_unused module_s* mod){
	weather_s* we = mod->data;	
	if( openweather_api_fiveday(&we->ow, we->city, we->token) ){
		mod->att.reftime = 60000;
	}
	else{
		mod->att.reftime = 3600000UL * 3;
	}
	
	mod->att.icoindex = strtoul( we->ow.info[we->ow.curdd][we->ow.curhh].text[OW_WEATHER_ICON], NULL, 10);
	
	switch( mod->att.icoindex ){
		default: case 0: mod->att.icoindex = 0; break;
		case 1: case 2: case 3: --mod->att.icoindex; break;
		case 9: case 10: mod->att.icoindex = 3; break;
		case 11: mod->att.icoindex = 4; break;
		case 13: mod->att.icoindex = 5; break;
	} 

	return 0;
}

__ef_private int weather_mod_env(module_s* mod, int id, char* dest){
	weather_s* we = mod->data;
	unsigned dd;
	unsigned hh;

	*dest = 0;
	if( id < 1000 ){
		dd = we->ow.curdd;
		hh = we->ow.curhh;
	}
	else if( id > 99 && id < 714 ){
		dd = we->ow.curdd;
		hh = id / 100;
		id -= hh * 100;
	}
	else if( id > 999 && id < 4714 ){
		dd = id / 1000;
		id -= dd * 1000;
		hh = id / 100;
		id -= hh * 100;
	}
	else{
		dbg_error("index to large");
		return -1;
	}
	
	if( id < 0 ){
		dbg_error("index to short");
		return -1;
	}

	if( dd >= OW_DAY || hh >= OW_HOURS || !we->ow.info[dd][hh].isset ){
		dbg_warning("dd %u hh %u isn't set",dd,hh);
		return -1;
	}

	if( id < 3 ){
		sprintf(dest, modules_format_get(mod, id, "lf"), we->ow.info[dd][hh].data[id] - 273.15);
	}
	else if( id < 10 ){
		sprintf(dest, modules_format_get(mod, id, "lf"), we->ow.info[dd][hh].data[id]);
	}
	else if ( id < 14 ){
		strcpy(dest, we->ow.info[dd][hh].text[id-10]);
	}
	else{
		dbg_error("invalid id");
		return -1;
	}
	//dbg_info("ENV{%d} %s", id, dest);
	return 0;
}

__ef_private int weather_mod_free(module_s* mod){
	free(mod->data);
	return 0;
}

int weather_mod_load(module_s* mod, char* path){
	weather_s* we = ef_mem_new(weather_s);
	we->city[0] = 0;
	we->token[0] = 0;

	mod->data = we;
	mod->refresh = weather_mod_refresh;
	mod->getenv = weather_mod_env;
	mod->free = weather_mod_free;

	strcpy(mod->att.longunformat, "we $0");
	strcpy(mod->att.shortunformat, "$0");
	strcpy(mod->att.name, "generic");
	strcpy(mod->att.instance, "weather");
	modules_icons_init(mod, 6);
	modules_icons_set(mod, 0, "🌣");
	modules_icons_set(mod, 1, "⛅");
	modules_icons_set(mod, 2, "☁️");
	modules_icons_set(mod, 3, "⛆");
	modules_icons_set(mod, 4, "🌩");
	modules_icons_set(mod, 5, "❄");

	modules_format_init(mod, 14);
	for( size_t i = 0; i < 10; ++i){
		modules_format_set(mod, i, "5.2");
	}
	for( size_t i = 10; i < 14; ++i){
		modules_format_set(mod, i, "");
	}
	
	config_s conf;
	config_init(&conf, 256);
	modules_default_config(mod, &conf);
	config_add(&conf, "city", CNF_S, we->city, CITY_MAX, 0, NULL);
	config_add(&conf, "token", CNF_S, we->token, TOKEN_MAX, 0, NULL);
	config_load(&conf, path);
	config_destroy(&conf);

	weather_mod_refresh(mod);

	return 0;
}


