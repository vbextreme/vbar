#include <ef/memory.h>
#include <sys/mman.h>

#ifdef MEM_UNDECLARE_PKEY
pkey_t mem_pkey_new(__ef_unused unsigned int mode){
	return 0;
}
#else
pkey_t mem_pkey_new(unsigned int mode){
	return pkey_alloc(0 ,mode);
}
#endif

err_t mem_protect(pkey_t* key, void* addr, size_t size, unsigned int mode){
	int prot = PROT_READ | PROT_WRITE;
#ifdef MEM_PKEY_DISABLE
	*key = -1;
	switch( mode ){
		case MEM_PROTECT_WRITE: prot = PROT_READ; break;
		case MEM_PROTECT_RW: prot = PROT_NONE; break;
	}	
#else
	#ifndef MEM_UNDECLARE_PKEY 
	catch_posix( *key = pkey_alloc(0, mode) ){
		dbg_error("pkey_alloc");
		dbg_errno();
		return errno;
	}
	#endif
#endif

#ifdef MEM_UNDECLARE_PKEY
	catch_posix( mprotect(addr, size, prot) ){
		dbg_error("mprotect");
		dbg_errno();
		return errno;
	}
#else
	catch_posix( pkey_mprotect(addr, size, prot, *key) ){
		dbg_error("pkey_mprotect");
		dbg_errno();
		return errno;
	}
#endif

#ifdef MEM_PKEY_DISABLE
	*key = size;
#endif

	return 0;
}

err_t mem_protect_change(pkey_t key, unsigned int mode,
#ifdef EF_MEM_PKEY_DISABLE
 void* addr){
	switch( mode ){
		case MEM_PROTECT_DISABLED: mode = PROT_READ | PROT_WRITE; break;
		case MEM_PROTECT_WRITE: mode = PROT_READ; break;
		case MEM_PROTECT_RW: mode = PROT_NONE; break;
	}
	#ifdef MEM_UNDECLARE_PKEY
	catch_posix( mprotect(addr, key, mode) ){
		dbg_error("pkey_mprotect");
		dbg_errno();
		return errno;
	}
	#else
	catch_posix( pkey_mprotect(addr, key, mode, -1) ){
		dbg_error("pkey_mprotect");
		dbg_errno();
		return errno;
	}
	#endif
	return 0;
}
#else
 __unused void* addr){
	return pkey_set(key ,mode);
 }
#endif
