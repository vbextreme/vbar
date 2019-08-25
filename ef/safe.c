#include <ef/os.h>
#include <ef/memory.h>
#include <ef/file.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <sys/types.h>
#include <fcntl.h> 
#include <ucontext.h>
#include <execinfo.h>
#include <signal.h>

bool_t os_valid_addr_read(void* addr){
	__fd_autoclose int fd;
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

bool_t os_valid_addr_write(void* addr){
	__fd_autoclose int fd;
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

typedef struct _sig_ucontext {
 unsigned long     uc_flags;
 struct ucontext   *uc_link;
 stack_t           uc_stack;
 struct sigcontext uc_mcontext;
 sigset_t          uc_sigmask;
} sig_ucontext_t;

__private volatile prvsafectx_t* safe;
__private sigaction_s oldsig;
__private volatile bool_t safecalled;
__private void* bkt[64];
__private size_t btsize;
__private char** bts;

__private void sig_restore_context(__unused int signum,__unused siginfo_t *si, void *context){
	sig_ucontext_t* uc = context;
	void* ca = NULL;
#if defined(__i386__)
	ca = (void *) uc->uc_mcontext.eip;
#elif defined(__x86_64__)
	ca = (void *) uc->uc_mcontext.rip;
#endif

	btsize = backtrace(bkt, 64);
	if( ca ) bkt[1] = ca;
	bts = backtrace_symbols(bkt, btsize);

	catch_posix( setcontext((struct ucontext_t*)&safe->uc) ){
		dbg_errno();
		dbg_fail("setcontext");
	}
}

void os_segfault_report(killer_f fnc){
	size_t memsize = ROUND_UP(sizeof(struct prvsafectx), sizeof(void*));
	void* mem;
	catch_null( mem = mem_heap_alloc(&memsize) ){
		dbg_errno();
		dbg_fail("ef_mem_heap_alloc");
	}
	safe = mem;
	safe->mem = mem;
	safe->memsize = memsize;
	safe->kcbk = fnc;

	catch_ef( mem_protect((pkey_t*)&safe->key, (void*)safe, safe->memsize, MEM_PROTECT_DISABLED) ){
		dbg_fail("ef_mem_protect");
	}
	
	os_signal_set(&oldsig, SIGSEGV, sig_restore_context);

	catch_posix( getcontext((struct ucontext_t*)&safe->uc) ){
		dbg_errno();
		dbg_fail("getcontext");
	}

	if ( safecalled ){
		dbg_warning("catch crash state");
		mem_protect_change(safe->key, MEM_PROTECT_DISABLED, (void*)safe);
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
	catch_ef( mem_protect_change(safe->key, MEM_PROTECT_WRITE, (void*)safe) ){
		dbg_fail("ef_mem_protect_change");
	}
}

void os_segfault_release(void){
	mem_protect_change(safe->key, MEM_PROTECT_DISABLED, (void*)safe);
	os_signal_restore(SIGSEGV, &oldsig);
	mem_heap_close(safe->mem, safe->memsize);
}

