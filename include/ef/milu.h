#ifndef __EF_MILU_H__
#define __EF_MILU_H__

#include <ef/type.h>
#include <ef/memory.h>
#include <ef/strong.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

void funi_init(void);

typedef lua_State* milu_h;
typedef struct luaL_Reg miluReg_s;
typedef int(*milu_f)(milu_h);

#define milu_args_count(M) lua_gettop(M)

#define milu_arg_type(M,I) lua_type(M,I)

const char* milu_tostring(milu_h milu, int index);
int milu_tointeger(milu_h milu, int index);
double milu_tonumber(milu_h milu, int index);
#define milu_to(V,M,I) _Generic( (V),\
		char*:  milu_tostring,\
		int:    milu_tointeger,\
		double: milu_tonumber,\
		default: lua_touserdata\
		)(M,I)
#define milu_arg_get(V,M,I) (V) = milu_to(V,M,I)

void milu_dump_index(milu_h milu, int index);
int milu_stackdump(milu_h milu);

void milu_register_class(milu_h milu, char* className, const miluReg_s* meta, const miluReg_s* methods);

void milu_register_function(milu_h milu, const char* const name, milu_f fnc);

void milu_object_binding(milu_h milu, void* obj, const char* const class);

void milu_table_get_or_create(milu_h milu, const char* table);

void milu_table_register_function(milu_h milu, const char* table, const char* name, milu_f fnc);

#define P_MILU_TABLE_REGISTER_T(N,T) void milu_table_register_ ## N (milu_h milu, const char* table, const char* name, T val)
P_MILU_TABLE_REGISTER_T(int, int);
P_MILU_TABLE_REGISTER_T(uint,unsigned int);
P_MILU_TABLE_REGISTER_T(long, long);
P_MILU_TABLE_REGISTER_T(ulong, unsigned long);
P_MILU_TABLE_REGISTER_T(size_t, size_t);

void milu_table_register_number(milu_h milu, const char* table, const char* name, double val);
void milu_table_register_string(milu_h milu, const char* table, const char* name, char* val);
void milu_table_register_userdata(milu_h milu, const char* table, const char* name, void* val);
#define milu_table_register(M, T, N, V) _Generic((V),\
		char*: milu_table_register_string,\
		int: milu_table_register_int,\
		unsigned int: milu_table_register_uint,\
		long: milu_table_register_long,\
		unsigned long: milu_table_register_ulong,\
		double: milu_table_register_number,\
		default: milu_table_register_userdata\
	)(M,T,N,V)

#define milu_push(M,VAR) _Generic( (VAR),\
		char*: lua_pushstring,\
		int: lua_pushinteger,\
		long: lua_pushinteger,\
		unsigned int: lua_pushinteger,\
		unsigned long: lua_pushinteger,\
		double: lua_pushnumber,\
		default: lua_pushlightuserdata\
	)(M,VAR)

#define milu_register_variable(M,NAME,VAR) do{\
	milu_push(M,VAR);\
	lua_setglobal(M,NAME);\
}while(0)

milu_h milu_init();
#define milu_terminate(M) lua_close(M)

int milu_run(milu_h milu, const char* code, size_t len, const char* codename);
char* milu_build(size_t* size, const char* fin, char* fout);


#endif
