#include <ef/memory.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


void* mem_heap_alloc(size_t* size) {	
	*size = ROUND_UP(*size, getpagesize());
	void* heap = mmap(NULL, *size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if( heap == (void*)-1 ){
		dbg_error("mmap");
		dbg_errno();
		return NULL;
	}
	return heap;
}

void mem_heap_close(void* mem, size_t size){
	if( mem == NULL ){
		dbg_warning("shmem already closed");
		return;
	}

	catch_posix( munmap(mem, size) ){
		dbg_error("munmap");
		dbg_errno();
	}
}
