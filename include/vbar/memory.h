#ifndef __EF_MEMORY_H__
#define __EF_MEMORY_H__

#include <vbar/type.h>
#include <malloc.h>
#include <sys/mman.h>

#define __ef_mem_autofree __ef_cleanup(ef_mem_free_auto)

typedef int pkey_t;

#define EF_MEM_PROTECT_DISABLED 0
#define EF_MEM_PROTECT_WRITE PKEY_DISABLE_WRITE
#define EF_MEM_PROTECT_RW PKEY_DISABLE_ACCESS

/*** memory.c ***/
#define ef_mem_new(TYPE) (TYPE*)malloc(sizeof(TYPE))
#define ef_mem_many(TYPE,COUNT) (TYPE*)malloc(sizeof(TYPE)*(COUNT))
#define ef_mem_free(OBJ) free(OBJ)
#define ef_mem_free_safe(OBJ) do{free(OBJ); OBJ=NULL;}while(0)
void ef_mem_free_auto(void* mem);
void* ef_malloc_or_die(size_t sz);
#define ef_mem_new_or_die(TYPE) (TYPE*)ef_malloc_or_die(sizeof(TYPE))
#define ef_mem_many_or_die(TYPE) (TYPE*)ef_malloc_or_die(sizeof(TYPE)*(COUNT))
void* ef_mem_matrix_new(size_t y, size_t sz);
void ef_mem_matrix_free(void* b, size_t y); 
#define ef_mem_clear(T,M) do{memset(M,0,sizeof(T));}while(0)

/*** shared.c ***/
void* ef_mem_shared_create(char* name, int privilege, size_t size);
void* ef_mem_shared_open(char* name);
void* ef_mem_shared_create_or_map(char* name, int priv, size_t size);
size_t ef_mem_shared_size(char* name);
void ef_mem_shared_close(void* mem, size_t size);
err_t ef_mem_shared_delete(char* name);
void* ef_mem_alloc(void** mem, size_t size);

/*** heap.c ***/
void ef_mem_heap_close(void* mem, size_t size);
void* ef_mem_heap_alloc(size_t* size);

/*** protect.c ***/
pkey_t ef_mem_pkey_new(unsigned int mode);
err_t ef_mem_protect(pkey_t* key, void* addr, size_t size, unsigned int mode);
err_t ef_mem_protect_change(pkey_t key, unsigned int mode, void* addr);

#endif
