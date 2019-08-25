#ifndef __EF_PROC_H__
#define __EF_PROC_H__

#include <ef/type.h>

typedef enum { CPU_USER, CPU_NICE, CPU_SYSTEM, CPU_IDLE, CPU_IOWAIT, CPU_IRQ, CPU_SOFTIRQ, CPU_STEAL, CPU_GUEST, CPU_GUEST_NICE, CPU_TIME_COUNT } cputime_e;

int cpu_core_count(void);
err_t cpu_tick_get(size_t* tick, int ncores);
size_t cpu_time_tick(size_t* tick);
double cpu_load_average(size_t* tickS, size_t* tickE, unsigned core, int ncores);

typedef struct memInfo{
	size_t total;
	size_t free;
	size_t available;
	size_t buffers;
	size_t cached;
	size_t totalswap;
	size_t freeswap;
	size_t shared;
	size_t SReclaimable;
	size_t SUnreclaim;
	size_t used;
	size_t toblink;
}memInfo_s;

void meminfo_read(memInfo_s* mem);

typedef enum { ND_BYTES, ND_PACKETS, ND_ERRS, ND_DROP, ND_FIFO, ND_FRAME, ND_COMPRESSED, ND_MULTICAST, ND_COUNT } netDev_e;

typedef struct netDev {
	size_t receive[ND_COUNT];
	size_t transmit[ND_COUNT];
}netDev_s;

err_t net_device(netDev_s* net, char* device);

#endif
