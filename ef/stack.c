#include <ef/stack.h>
#include <ef/memory.h>

void stack_init(stack_s* stk, size_t sizetype, size_t max){
	stk->stk = malloc(sizetype * max);
	iassert(stk->stk);
	stk->st = sizetype;
	stk->max = stk->block = max;
}

err_t stack_realloc(stack_s* stk){
	void* mem = realloc(stk->stk, stk->st * (stk->max+stk->block));
	if( !mem ) return -1;
	stk->stk = mem;
	stk->max += stk->block;
	return 0;
}


