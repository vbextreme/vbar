#include <ef/memory.h>

__const size_t round_up_power_two(size_t n){
	if( n < 3 ) return 2;
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n |= n >> 32;
	++n;
	return n;
}

void* malloc_or_die(size_t sz){
	void* mem = malloc(sz);
	catch_null(mem){
		dbg_errno();
		dbg_fail("fail malloc");
	}
	return mem;
}

void mem_matrix_free(void* mem, size_t y){
	void** b = (void**)mem;
    size_t i;
    for (i=0; i < y; ++i)
        free(b[i]);

    free(b);
}

void* mem_matrix_new(size_t y, size_t sz){
    void **b;
    b = mem_many(void*, y);
	catch_null(b) return NULL;
	
    for(size_t i = 0; i < y; ++i){
        b[i] = (void*)malloc(sz);
        catch_null( b[i] ){
            for (size_t j = 0; j < i; ++j)
                mem_free(b[j]);
            mem_free(b);
            return NULL;
        }
    }
    return b;
}

void mem_free_auto(void* mem){
	void** mug = (void**)mem;
	free(*mug);
	*mug = NULL;
}

void* mem_many_aligned_raw(size_t* size, size_t aligneto){
	*size = ROUND_UP(*size, aligneto);
	return aligned_alloc(aligneto, *size);
}


