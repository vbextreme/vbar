#include <ef/milu.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <time.h>

const char* milu_tostring(milu_h milu, int index){
	return lua_tostring(milu,index);
}

int milu_tointeger(milu_h milu, int index){
	return lua_tointeger(milu,index);
}

double milu_tonumber(milu_h milu, int index){
	return lua_tonumber(milu,index);
}

void milu_dump_index(milu_h milu, int index){
	int t = lua_type(milu, index);
		
	switch (t){
		case LUA_TNIL:
			dbg_info("dump.nil");
		break;

        case LUA_TSTRING:
            dbg_info("dump.string: \"%s\"", lua_tostring(milu, index));
		break;

		case LUA_TBOOLEAN:
			dbg_info("dump.boolean: %s", lua_toboolean(milu, index) ? "true" : "false");
		break;

		case LUA_TNUMBER:
			dbg_info("dump.number: %g", lua_tonumber(milu, index));
		break;	
			
		case LUA_TTABLE:
			lua_pushvalue(milu, index);
		    lua_pushnil(milu);
		    while (lua_next(milu, -2)) {
				lua_pushvalue(milu, -2);
		        __unused const char *key = lua_tostring(milu, -1);
				dbg_info("dump.table.%s", key);
				milu_dump_index(milu, -2);
				lua_pop(milu, 2);
			}
			lua_pop(milu,1);
		break;

		default:
			dbg_info("unknow type: %s", lua_typename(milu, t));
        break;
    }
}

int milu_stackdump(milu_h milu){
	int nargs = milu_args_count(milu);
	dbg_info("dump: %d args", nargs);

	for( int i = 1; i <= nargs; i++){
		milu_dump_index(milu, i);	
    }
	return 0;
}

void milu_register_class(milu_h milu, char* className, const miluReg_s* meta, const miluReg_s* methods){
    int libid, metaid;

    /* newclass = {} */
    lua_createtable(milu, 0, 0);
    libid = lua_gettop(milu);

    /* metatable = {} */
    luaL_newmetatable(milu, className);
    metaid = lua_gettop(milu);
    luaL_setfuncs(milu, meta, 0);

    /* metatable.__index = _methods */
    //luaL_newlib(milu, methods);
	
	lua_newtable(milu);
	luaL_setfuncs(milu, methods,0);
	lua_setfield(milu, metaid, "__index");

    /* metatable.__metatable = _meta */
    //luaL_newlib(milu, meta);
    lua_newtable(milu);
	luaL_setfuncs(milu, meta,0);
	lua_setfield(milu, metaid, "__metatable");

    /* class.__metatable = metatable */
    lua_setmetatable(milu, libid);

    /* _G["Foo"] = newclass */
    lua_setglobal(milu, className);
}

void milu_register_function(milu_h milu, const char* const name, milu_f fnc){
    lua_pushcfunction(milu, fnc);
    lua_setglobal(milu, name);
}

void milu_object_binding(milu_h milu, void* obj, const char* const class){
	lua_pushlightuserdata(milu, obj);
    luaL_getmetatable(milu, class);
    lua_setmetatable(milu, -2);
}

void milu_table_get_or_create(milu_h milu, const char* table){
	lua_getglobal(milu, table);
    if (!lua_istable(milu, -1)) {
        lua_createtable(milu, 0, 1);
        lua_setglobal(milu, table);
        lua_pop(milu, 1);
        lua_getglobal(milu, table);
    }
}

void milu_table_register_function(milu_h milu, const char* table, const char* name, milu_f fnc){
    milu_table_get_or_create(milu, table);
    lua_pushstring(milu, name);
    lua_pushcfunction(milu, fnc);
    lua_settable(milu, -3);
    lua_pop(milu, 1);
}

#define MILU_TABLE_REGISTER_T(N,T) void milu_table_register_ ## N (milu_h milu, const char* table, const char* name, T val){\
    milu_table_get_or_create(milu, table);\
    lua_pushstring(milu, name);\
    lua_pushinteger(milu, val);\
    lua_settable(milu, -3);\
    lua_pop(milu, 1);\
}

MILU_TABLE_REGISTER_T(int, int)
MILU_TABLE_REGISTER_T(uint,unsigned int)
MILU_TABLE_REGISTER_T(long, long)
MILU_TABLE_REGISTER_T(ulong, unsigned long)
MILU_TABLE_REGISTER_T(size_t, size_t)

void milu_table_register_number(milu_h milu, const char* table, const char* name, double val){
    milu_table_get_or_create(milu, table);
    lua_pushstring(milu, name);
    lua_pushnumber(milu, val);
    lua_settable(milu, -3);
    lua_pop(milu, 1);
}

void milu_table_register_string(milu_h milu, const char* table, const char* name, char* val){
    milu_table_get_or_create(milu, table);
    lua_pushstring(milu, name);
    lua_pushstring(milu, val);
    lua_settable(milu, -3);
    lua_pop(milu, 1);
}

void milu_table_register_userdata(milu_h milu, const char* table, const char* name, void* val){
    milu_table_get_or_create(milu, table);
    lua_pushstring(milu, name);
    lua_pushlightuserdata(milu, val);
    lua_settable(milu, -3);
    lua_pop(milu, 1);
}

milu_h milu_init(){
	milu_h milu = luaL_newstate();
	if( milu == NULL ){
		return NULL;
	}
	luaL_openlibs(milu);
	return milu;
}

int milu_run(milu_h milu, const char* code, size_t len, const char* codename){
	if( len == 0 ){
		len = strlen(code);
	}

	if( luaL_loadbuffer(milu, code, len, codename) ){
		dbg_error("milu: %s", lua_tostring(milu, -1));
		return -1;
	}

	if (lua_pcall(milu, 0, LUA_MULTRET, 0)) {
		dbg_error("milu: %s", lua_tostring(milu, -1));
		return -1;
    }
	return 0;
}

#include <spawn.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

extern char** environ;

void spawn_init(void){
	struct sigaction arg = {
		.sa_handler=SIG_IGN,
		.sa_flags=SA_NOCLDWAIT
	};
	sigaction(SIGCHLD, &arg, NULL);
}

pid_t spawn_shell(char* cmdline){
	pid_t child;
	char* argv[] = { "bash", "-c", cmdline, NULL };
	dbg_info("%s %s %s", "/bin/bash", "-c", cmdline);
	posix_spawn_file_actions_t fact;
	if( posix_spawn_file_actions_init(&fact) ){
		dbg_error("file action init");
		return -1;
	}
	if( posix_spawn_file_actions_addclose(&fact, 0) ){
		dbg_error("close stdin");
		return -1;
	}
	if( posix_spawn_file_actions_addclose(&fact, 1) ){
		dbg_error("close stdout");
		return -1;
	}
	errno = posix_spawn(&child, "/bin/bash", &fact, NULL, argv, environ);
	if( errno ){
		dbg_error("posix_spawn %s", cmdline);
		dbg_errno();
		return -1;
	}
	return child;
}

int spawn_wait(pid_t spawn){
	int status;
	dbg_info("");
	if( waitpid(spawn, &status, 0) == -1 ){
		dbg_error("waitpid");
		dbg_errno();
		return -1;
	}
	return 0;
}

size_t fsize(FILE* fd){
	long old =  ftell(fd);
	fseek(fd, 0, SEEK_END);
	long sz = ftell(fd);
	dbg_info("ftell %ld",sz);
	fseek(fd, old, SEEK_SET);
	return (size_t)sz;
}

int fexists(const char* name){
	FILE* fd = fopen(name, "r");
	if( NULL == fd ){
		return 0;
	}
	fclose(fd);
	return 1;
}

int fdtcmp(char const* a, char const* b){
	struct stat sa;
	struct stat sb;
	if( stat(a, &sa) ) return -1;
	if( stat(b, &sb) ) return 1;
	
	long s = sa.st_mtim.tv_sec - sb.st_mtim.tv_sec;
	long n = sa.st_mtim.tv_nsec - sb.st_mtim.tv_nsec;
	if( s )	return s;
	return n;
}

__private const char* const TVAL = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-._";


#define FUNI_RETRY 1000
void funi_init(void){
	time_t t;
	srand(time(&t));
}

__private unsigned int funi_rval(void){
	size_t len = strlen(TVAL);
	return rand()%len;
}

__private int funi(char* template){
	char* begin = strchr(template, '@');
	if( !begin ){
		dbg_error("no char @ on template");	
		return -1;
	}
	
	char* ch = begin;
	while( *ch++ == '@' ) ++ch;
	size_t nt = (ch-1) - begin;

	for( size_t i = 0; i < FUNI_RETRY; ++i ){
		ch = begin;
		for( size_t j = 0; j < nt; ++j ){ 
			*ch++ = TVAL[funi_rval()];
		}	
		dbg_info("name %s", template);
		if( !fexists(template) ){
			return 0;
		}
	}
	return -1;
}

__private int milu_luac(const char* in, const char* out){
	char* cmd = NULL;
	asprintf(&cmd, "luac -o %s %s", out, in);
	dbg_info("%s",cmd);
	pid_t plc = spawn_shell(cmd);
	if( plc == -1 ){
		free(cmd);
		unlink(out);
		return -1;
	}
	spawn_wait(plc);
	free(cmd);
	return 0;
}

char* milu_build(size_t* size, const char* fin, char* fout){
	if( !fexists(fin) ){
		dbg_error("file %s not exists", fin);
		return NULL;
	}

	char tmp[]="/tmp/milu.@@@@@@@@@@@@";

	if( fout == NULL ){
		dbg_info("tmpname");
		if( funi(tmp) ){
			dbg_error("tmp file unavailable");
			return NULL;
		}
		fout = tmp;
		milu_luac(fin, fout);
	}
	else if( !fexists(fout) ){
		milu_luac(fin, fout);
	}
	else if( fdtcmp(fin, fout) ){
		unlink(fout);
		milu_luac(fin, fout);
	}
	
	FILE* fd = fopen(fout, "r");
	if( fd == NULL ){
		dbg_error("fopen");
		dbg_errno();
		return NULL;
	}
	*size = fsize(fd);
	if( *size == 0 ){
		dbg_error("file size");
		fclose(fd);
		if( fout == tmp ) unlink(fout);
		return NULL;
	}
	dbg_info("read %lu",*size);

	char* build = mem_many(char, *size);
	if( fread(build, 1, *size, fd) != *size ){
		dbg_error("fread");
		dbg_errno();
		free(build);
		if( fout == tmp ) unlink(fout);
		return NULL;
	}
	fclose(fd);

	if( fout == tmp ) unlink(fout);
	return build;
}

int foo_gc();
int foo_index();
int foo_newindex();
int foo_dosomething();
int foo_new();

struct foo {
    int color;
};

__private const miluReg_s module_meta[] = {
    {"__gc", foo_gc},
    {"__index", foo_index},
    {"__newindex", foo_newindex},
    { NULL, NULL }
};

__private const miluReg_s module_methods[] = {
    {"new", foo_new},
    {"dosomething", foo_dosomething},
	{"color", foo_dosomething},
    { NULL, NULL }
};

int foo_gc(lua_State* lua) {
    dbg_warning("gc");
	milu_stackdump(lua);
    return 0;
}

int foo_newindex(lua_State* lua) {
    dbg_warning("__newindex");
	milu_stackdump(lua);
    return 0;
}

int foo_index(lua_State* lua) {
    dbg_warning("__index");
	milu_stackdump(lua);
	foo_new(lua);
	return 0;
}

int foo_dosomething(lua_State* lua) {
    dbg_warning("dosomething");
		
	struct foo* foo;
	milu_arg_get(foo, lua, 1);
	dbg_info("foo.color = %d", foo->color);
	foo->color++;
	milu_stackdump(lua);
    return 0;
}

static struct foo test;

int foo_new(lua_State* lua) {
    dbg_info("new");
	milu_stackdump(lua);

	//lua_newuserdata(lua,sizeof(struct foo);
	//lua_pushlightuserdata(lua, &test);
    //luaL_getmetatable(lua, "Module");
    //lua_setmetatable(lua, -2);
	milu_object_binding(lua, &test, "Module");

    return 1;
}

static const char* code = ""
		"foo = Module:new('cpu')\n"
		"foo:dosomething()\n"
		"foo.color = 1\n"
		"print(foo:color())\n"
		"print(testing)\n"
		"testing=1\n"
		"print(prova.t)\n"
		"mydump(prova)\n"
		"print(Module.www)\n"
		"";
	

int main(){
	milu_h milu = milu_init();
	if( NULL == milu ){
		dbg_error("wtf");
		return 1;
	}
	
	milu_register_class(milu, "Module", module_meta, module_methods);

	int test = 7;
	milu_register_variable(milu, "testing", test);	
	milu_table_register(milu, "prova","t",test);
	milu_register_function(milu, "mydump",milu_stackdump); 

	milu_table_register(milu, "Module", "www", test);
	
	size_t szc;
	dbg_info("build code");
	char* fcode = milu_build(&szc, "lulu.lu", NULL);
	if( fcode ){
		dbg_info("run code");
		milu_run(milu, fcode, szc, "lulu.lu");

	//milu_run(milu, code, 0, "test");
		milu_terminate(milu);
		free(fcode);
	}
	else{
		dbg_error("on load milu code");
	}
	return 0;
}
