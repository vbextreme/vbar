#include <vbar/os.h>
#include <vbar/memory.h>
#include <vbar/file.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <ucontext.h>
#include <execinfo.h>

bool_t ef_os_valid_addr_read(void* addr){
	__ef_fd_autoclose int fd;
	catch_posix( fd = open("/dev/zero",O_RDONLY) ){
		dbg_error("open");
		dbg_errno();
		return FALSE;
	}

	catch_posix( read(fd, addr, 1) ){
	   return FALSE;
	}	   
	
	return TRUE;
}

bool_t ef_os_valid_addr_write(void* addr){
	__ef_fd_autoclose int fd;
	catch_posix( fd = open("/dev/null",O_WRONLY) ){
		dbg_error("open");
		dbg_errno();
		return FALSE;
	}

	catch_posix( write(fd, addr, 1) ){
	   return FALSE;
	}	   
	
	return TRUE;
}

typedef struct prvsafectx{
	ucontext_t uc;
	killer_f kcbk;
	size_t memsize;
	void* mem;
	pkey_t key;
}prvsafectx_t;

__ef_private volatile prvsafectx_t* safe;
__ef_private sigaction_s oldsig;
__ef_private volatile bool_t safecalled;
__ef_private void* bkt[64];
__ef_private size_t btsize;
__ef_private char** bts;

__ef_private void sig_restore_context(__ef_unused int signum,__ef_unused siginfo_t *si, __ef_unused void *context){
	btsize = backtrace(bkt, 64);
	bts = backtrace_symbols(bkt, btsize);

	catch_posix( setcontext((struct ucontext_t*)&safe->uc) ){
		dbg_errno();
		dbg_fail("setcontext");
	}
}

void ef_os_segfault_report(killer_f fnc){
	size_t memsize = ROUND_UP(sizeof(struct prvsafectx), sizeof(void*));
	void* mem;
	catch_null( mem = ef_mem_heap_alloc(&memsize) ){
		dbg_errno();
		dbg_fail("ef_mem_heap_alloc");
	}
	safe = mem;
	safe->mem = mem;
	safe->memsize = memsize;
	safe->kcbk = fnc;

	catch_ef( ef_mem_protect((pkey_t*)&safe->key, (void*)safe, safe->memsize, EF_MEM_PROTECT_DISABLED) ){
		dbg_fail("ef_mem_protect");
	}
	
	ef_os_signal_set(&oldsig, SIGSEGV, sig_restore_context);

	catch_posix( getcontext((struct ucontext_t*)&safe->uc) ){
		dbg_errno();
		dbg_fail("getcontext");
	}

	if ( safecalled ){
		dbg_warning("catch crash state");
		ef_mem_protect_change(safe->key, EF_MEM_PROTECT_DISABLED, (void*)safe);
		if( safe->kcbk == NULL ){
			dbg_error("software stop working");
			for( size_t i = 0; i < btsize; ++i ){
				dbg_error("%s", bts[i]);
			}
			dbg_fail("end call stack");
		}
		else{
			safe->kcbk((void*)safe);
		}
		exit(1);
	}
	
	safecalled = TRUE;
	catch_ef( ef_mem_protect_change(safe->key, EF_MEM_PROTECT_WRITE, (void*)safe) ){
		dbg_fail("ef_mem_protect_change");
	}
}

void ef_os_segfault_release(void){
	ef_mem_protect_change(safe->key, EF_MEM_PROTECT_DISABLED, (void*)safe);
	ef_os_signal_restore(SIGSEGV, &oldsig);
	ef_mem_heap_close(safe->mem, safe->memsize);
}

