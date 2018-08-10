#include <vbar/memory.h>

void* ef_malloc_or_die(size_t sz){
	void* mem = malloc(sz);
	catch_null(mem){
		dbg_errno();
		dbg_fail("fail malloc");
	}
	return mem;
}

void ef_mem_matrix_free(void* mem, size_t y){
	void** b = (void**)mem;
    size_t i;
    for (i=0; i < y; ++i)
        free(b[i]);

    free(b);
}

void* ef_mem_matrix_new(size_t y, size_t sz){
    void **b;
    b = ef_mem_many(void*, y);
	catch_null(b) return NULL;
	
    for(size_t i = 0; i < y; ++i){
        b[i] = (void*)malloc(sz);
        catch_null( b[i] ){
            for (size_t j = 0; j < i; ++j)
                ef_mem_free(b[j]);
            ef_mem_free(b);
            return NULL;
        }
    }
    return b;
}

void ef_mem_free_auto(void* mem){
	void** mug = (void**)mem;
	free(*mug);
	*mug = NULL;
}
