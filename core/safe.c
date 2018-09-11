#include <vbar/os.h>
#include <vbar/memory.h>
#include <vbar/file.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <ucontext.h>

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

__ef_private void* safemem;
__ef_private size_t safememsize;
__ef_private pkey_t safekey;
__ef_private sigaction_s oldsig;
__ef_private volatile bool_t safecalled;
__ef_private volatile ucontext_t* safeuc;
__ef_private volatile killer_f kcbk;

__ef_private void sig_restore_context(__ef_unused int signum,__ef_unused siginfo_t *si, __ef_unused void *context){
	catch_posix( setcontext((ucontext_t*)&safeuc) ){
		dbg_errno();
		dbg_fail("setcontext");
	}
}

void ef_os_segfault_report(killer_f fnc){
	kcbk = fnc;
	safememsize = ROUND_UP(sizeof(ucontext_t), sizeof(void*));
	catch_null( safemem = ef_mem_heap_alloc(&safememsize) ){
		dbg_errno();
		dbg_fail("ef_mem_heap_alloc");
	}
	safeuc = safemem;
	
	catch_ef( ef_mem_protect(&safekey, (void*)safeuc, safememsize, EF_MEM_PROTECT_DISABLED) ){
		dbg_fail("ef_mem_protect");
	}
	
	ef_os_signal_set(&oldsig, SIGSEGV, sig_restore_context);

	catch_posix( getcontext((ucontext_t*)&safeuc) ){
		dbg_errno();
		dbg_fail("getcontext");
	}

	if ( safecalled ){
		dbg_warning("catch crash state");
		ef_mem_protect_change(safekey, EF_MEM_PROTECT_DISABLED, (void*)safeuc);
		if( kcbk == NULL ){
			dbg_fail("software stop working");
		}
		else{
			kcbk((void*)&safeuc);
		}
		exit(1);
	}
	
	safecalled = TRUE;
	catch_ef( ef_mem_protect_change(safekey, EF_MEM_PROTECT_WRITE, (void*)safeuc) ){
		dbg_fail("ef_mem_protect_change");
	}
}

void ef_os_segfault_release(void){
	ef_mem_protect_change(safekey, EF_MEM_PROTECT_DISABLED, (void*)safeuc);
	ef_os_signal_restore(SIGSEGV, &oldsig);
	ef_mem_heap_close(safemem, safememsize);
}

