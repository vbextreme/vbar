#ifndef __EF_STACK_H__
#define __EF_STACK_H__

#include <ef/type.h>

typedef struct stack{
	void* stk;
	size_t count;
	size_t st;
	size_t max;
	size_t block;
}stack_s;

void stack_init(stack_s* stk, size_t sizetype, size_t max);
err_t stack_realloc(stack_s* stk);

#define stack_destroy(STK) do{ free((STK)->stk); (STK)->stk=NULL; }while(0)

#define stack_push(STK,TYPE,V) do{\
	if((STK)->count >= (STK)->max){\
		if( stack_realloc((STK)) ){\
			dbg_fail("stack reallocation");\
		}\
	}\
	TYPE* __els__ = (STK)->stk;\
	__els__[(STK)->count++] = V;\
}while(0)

#define stack_pop(STK, TYPE) ({\
	if( (STK)->count == 0 ){\
		dbg_fail("underflow stack");\
	}\
	TYPE* __els__ = (STK)->stk;\
	__els__[--(STK)->count];\
})

#define stack_peek(STK, TYPE) ({\
	if( (STK)->count == 0 ){\
		dbg_fail("underflow stack");\
	}\
	TYPE* __els__ = (STK)->stk;\
	__els__[(STK)->count-1];\
})

#define stack_empty(STK) ((STK)->count == 0)

#endif
