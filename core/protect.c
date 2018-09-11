#include <vbar/memory.h>
#include <sys/mman.h>

pkey_t ef_mem_pkey_new(unsigned int mode){
	return pkey_alloc(0 ,mode);
}

err_t ef_mem_protect(pkey_t* key, void* addr, size_t size, unsigned int mode){
	int prot = PROT_READ | PROT_WRITE;
#ifdef EF_MEM_PKEY_DISABLE
	*key = -1;
	switch( mode ){
		case EF_MEM_PROTECT_WRITE: prot = PROT_READ; break;
		case EF_MEM_PROTECT_RW: prot = PROT_NONE; break;
	}	
#else
	catch_posix( *key = pkey_alloc(0, mode) ){
		dbg_error("pkey_alloc");
		dbg_errno();
		return errno;
	}
#endif	
	catch_posix( pkey_mprotect(addr, size, prot, *key) ){
		dbg_error("pkey_mprotect");
		dbg_errno();
		return errno;
	}

#ifdef EF_MEM_PKEY_DISABLE
	*key = size;
#endif

	return 0;
}

err_t ef_mem_protect_change(pkey_t key, unsigned int mode,
#ifdef EF_MEM_PKEY_DISABLE
 void* addr){
	switch( mode ){
		case EF_MEM_PROTECT_DISABLED: mode = PROT_READ | PROT_WRITE; break;
		case EF_MEM_PROTECT_WRITE: mode = PROT_READ; break;
		case EF_MEM_PROTECT_RW: mode = PROT_NONE; break;
	}
	catch_posix( pkey_mprotect(addr, key, mode, -1) ){
		dbg_error("pkey_mprotect");
		dbg_errno();
		return errno;
	}
	return 0;
}
#else
 __ef_unused void* addr){
	return pkey_set(key ,mode);
 }
#endif
