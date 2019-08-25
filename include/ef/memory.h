#ifndef __EF_MEMORY_H__
#define __EF_MEMORY_H__

#include <ef/type.h>
#include <malloc.h>
#include <sys/mman.h>

#define __mem_autofree __cleanup(mem_free_auto)

typedef int pkey_t;

#define MEM_PROTECT_DISABLED 0
#ifdef MEM_UNDECLARE_PKEY
	#define MEM_PROTECT_WRITE 1
	#define MEM_PROTECT_RW 2
#else
	#define MEM_PROTECT_WRITE PKEY_DISABLE_WRITE
	#define MEM_PROTECT_RW PKEY_DISABLE_ACCESS
#endif

/*** memory.c ***/
#define mem_new(TYPE) (TYPE*)malloc(sizeof(TYPE))
#define mem_many(TYPE,COUNT) (TYPE*)malloc(sizeof(TYPE)*(COUNT))
void* mem_many_aligned_raw(size_t* size, size_t alignedto);
#define mem_many_aligned(TYPE, PTRCOUNT, ALIGNED) ({\
		size_t n = *(PTRCOUNT)*sizeof(TYPE);\
		(TYPE*)mem_many_aligned_raw(&n,ALIGNED);\
	})

#define mem_zero_many(TYPE,COUNT) (TYPE*)calloc(COUNT,sizeof(TYPE))
#define mem_free(OBJ) free(OBJ)
#define mem_free_safe(OBJ) do{free(OBJ); OBJ=NULL;}while(0)
void mem_free_auto(void* mem);
void* malloc_or_die(size_t sz);
#define mem_new_or_die(TYPE) (TYPE*)malloc_or_die(sizeof(TYPE))
#define mem_many_or_die(TYPE) (TYPE*)malloc_or_die(sizeof(TYPE)*(COUNT))
void* mem_matrix_new(size_t y, size_t sz);
void mem_matrix_free(void* b, size_t y); 
#define mem_clear(T,M) do{memset(M,0,sizeof(T));}while(0)
__const size_t round_up_power_two(size_t n);

/*** shared.c ***/
void* mem_shared_create(char* name, int privilege, size_t size);
void* mem_shared_open(char* name);
void* mem_shared_create_or_map(char* name, int priv, size_t size);
size_t mem_shared_size(char* name);
void mem_shared_close(void* mem, size_t size);
err_t mem_shared_delete(char* name);
void* mem_alloc(void** mem, size_t size);

/*** heap.c ***/
void mem_heap_close(void* mem, size_t size);
void* mem_heap_alloc(size_t* size);

/*** protect.c ***/
pkey_t mem_pkey_new(unsigned int mode);
err_t mem_protect(pkey_t* key, void* addr, size_t size, unsigned int mode);
err_t mem_protect_change(pkey_t key, unsigned int mode, void* addr);

#endif
