#ifndef __EF_TYPE_H__
#define __EF_TYPE_H__

#define _GNU_SOURCE
#define _XOPEN_SPURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

typedef char byte_t;
typedef uint16_t word_t;
typedef uint32_t dword_t;
typedef uint64_t qword_t;
typedef dword_t flags_t;
typedef int err_t;
typedef enum { FALSE, TRUE } bool_t;

#define lenght_stack_vector(V) (sizeof(V)/sizeof(V[0]))

#ifndef _Y_ 
	#define _Y_ 1
#endif
#ifndef _N_
	#define _N_ 0
#endif

#define EMPTY_MACRO do{}while(0)

#define SWAP(A,B) ({ \
		__auto_type __tmp__ = A;\
		A = B;\
		B = __tmp__;\
	})

#define MTH_MAX(A,B) ((A>B)?A:B)
#define MTH_3MAX(A,B,C) MTH_MAX(MTH_MAX(A,B),C)
#define MTH_MIN(A,B) ((A<B)?A:B)
#define MTH_3MIN(A,B,C) MTH_MIN(MTH_MIN(A,B),C)

#define ROUND_UP(N,S) ((((N)+(S)-1)/(S))*(S))
#define FAST_MOD_POW_TWO(N,M) ((N) & ((M) - 1))
#define FAST_BIT_COUNT(B) __builtin_popcount(B)
#define MM_CHANNEL 2000
#define MM_ALPHA(COUNT) ((MM_CHANNEL)/((COUNT)+1))
#define MM_AHPLA(ALPHA) ((MM_CHANNEL/2)-ALPHA)
#define MM_NEXT(ALPHA,AHPLA,NEWVAL,OLDVAL) (((ALPHA)*(NEWVAL)+(AHPLA)*(OLDVAL))/(MM_CHANNEL/2))

#define __VA_COUNT__(...) __VA_COUNT_IMPL__(foo, ##__VA_ARGS__,9,8,7,6,5,4,3,2,1,0)
#define __VA_COUNT_IMPL__(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,N,...) N
#define __CONCAT__(A,B) A##B
#define __CONCAT_EXPAND__(A,B) __CONCAT__(A,B)
//#define myfnc(...) __CONCAT_EXPAND__(myfnc_n, __VA_COUNT(__VA_ARGS__))(##__VA_ARGS__)


#define forever() for(;;)
#define __private static
#define __unused __attribute__((unused))
#define __cleanup(FNC) __attribute__((__cleanup__(FNC)))
#define __printf(FRMT,VA) __attribute__((format (printf, FRMT, VA)))
#define __const __attribute__((const))
#define __packed __attribute__((packed))
#define __target(T) __attribute__((target(T)))
#define __target_clone(arg...) __attribute__((target_clones(## arg)))
#define __target_default __target("default")
#define __target_popcount __target("popcnt")
#define __target_vectorization __attribute__((target_clones("default","mmx","sse","sse2","sse3","ssse3","sse4.1","sse4.2","avx","avx2")))
#define __target_default_popcount __target_clone("default","popcnt")
#define __target_default_vectorization __target_clone("default","mmx","sse","sse2","sse3","ssse3","sse4.1","sse4.2","avx","avx2")
#define __target_all __target_clone("default","popcnt","mmx","sse","sse2","sse3","ssse3","sse4.1","sse4.2","avx","avx2")
#define __cpu_init() __builtin_cpu_init()
#define __cpu_supports_popcount() __builtin_cpu_supports ("popcnt")
#define __cpu_supports_vectorization() (__builtin_cpu_supports("mmx") || __builtin_cpu_supports("sse") || __builtin_cpu_supports("sse2") || __builtin_cpu_supports("sse3") || __builtin_cpu_supports("ssse3") || __builtin_cpu_supports("sse4.1") || __builtin_cpu_supports("sse4.2") || __builtin_cpu_supports("avx") || __builtin_cpu_supports("avx2"))




#define DO_PRAGMA(DOP) _Pragma(#DOP)
#define UNSAFE_BEGIN(FLAGS) DO_PRAGMA(GCC diagnostic push); DO_PRAGMA(GCC diagnostic ignored FLAGS)
#define UNSAFE_END DO_PRAGMA(GCC diagnostic pop)

#ifdef OMP_ENABLE
	#define __parallef DO_PRAGMA(omp parallel for)
	#define __parallefc(Z) DO_PRAGMA(omp parallel for collapse Z)
#else
	#define __parallef
	#define __parallefc(Z) 
#endif

#define DBG_OUTPUT stderr

#ifdef DEBUG_COLOR
	#define DBG_COLOR_INFO    "\033[36m"
	#define DBG_COLOR_WARNING "\033[93m"
	#define DBG_COLOR_ERROR   "\033[31m"
	#define DBG_COLOR_FAIL    "\033[91m"
	#define DBG_COLOR_RESET   "\033[m"
#else
	#define DBG_COLOR_INFO    ""
	#define DBG_COLOR_WARNING ""
	#define DBG_COLOR_ERROR   ""
	#define DBG_COLOR_FAIL    ""
	#define DBG_COLOR_RESET   ""
#endif

#ifndef DBG_INFO
	#define DBG_INFO    "info"
#endif
#ifndef DBG_WARNING
	#define DBG_WARNING "warning"
#endif
#ifndef DBG_ERROR
	#define DBG_ERROR   "error"
#endif
#ifndef DBG_FAIL
	#define DBG_FAIL    "fail"
#endif
#ifndef DBG_ERRNO
	#define DBG_ERRNO   "errno"
#endif

#ifndef DBG_LVL_FAIL
	#define DBG_LVL_FAIL    1
#endif
#ifndef DBG_LVL_ERROR
	#define DBG_LVL_ERROR   2
#endif
#ifndef DBG_LVL_WARNING
	#define DBG_LVL_WARNING 3
#endif
#ifndef DBG_LVL_INFO
	#define DBG_LVL_INFO    4
#endif

#if DEBUG_ENABLE >= 1
	#define dbg(TYPE, COLOR, FORMAT, arg...) do{\
										fprintf(DBG_OUTPUT, "%s[%u]:{%d} %s(): %s%s" DBG_COLOR_RESET "::" FORMAT "\n",\
										__FILE__,\
									   	__LINE__,\
									   	0,\
										__FUNCTION__,\
										COLOR, TYPE,\
										## arg); \
										fflush(DBG_OUTPUT);\
									}while(0)

	#define dbg_fail(FORMAT, arg...) do{ \
										dbg(DBG_FAIL, DBG_COLOR_FAIL, FORMAT, ## arg);\
										exit(1);\
									 }while(0)

	#define dbg_errno() dbg(DBG_ERRNO, DBG_COLOR_ERROR, " %d descript: %s", errno, strerror(errno)) 
#else
	#define dbg(TYPE, FORMAT, arg...) EMPTY_MACRO
	#define dbg_fail(FORMAT, arg...) do{exit(1);}while(0)
	#define dbg_errno() EMPTY_MACRO
#endif

#if DEBUG_ENABLE > DBG_LVL_FAIL
	#define dbg_error(FORMAT, arg...) dbg(DBG_ERROR, DBG_COLOR_ERROR, FORMAT, ## arg)
#else
	#define dbg_error(FORMAT, arg...) EMPTY_MACRO
#endif

#if DEBUG_ENABLE > DBG_LVL_ERROR
	#define dbg_warning(FORMAT, arg...) dbg(DBG_WARNING, DBG_COLOR_WARNING, FORMAT, ## arg)
#else
	#define dbg_warning(FORMAT, arg...) EMPTY_MACRO
#endif

#if DEBUG_ENABLE > DBG_LVL_WARNING
	#define dbg_info(FORMAT, arg...) dbg(DBG_INFO, DBG_COLOR_INFO, FORMAT, ## arg)
#else
	#define dbg_info(FORMAT, arg...) EMPTY_MACRO
#endif
	
#if ASSERT_ENABLE == _Y_
	#define iassert(C) do{ if ( !(C) ){fprintf(stderr,"assertion fail %s[%u]: %s(%s)\n", __FILE__, __LINE__, __FUNCTION__, #C); exit(0);}}while(0)
#else
	#define iassert(C) EMPTY_MACRO
#endif

#define try_syscall(EXP) do{\
		errno = 0;\
		EXP;\
	while(errno == EAGAIN) 

#define catch_posix(EXP) while( (EXP) == -1 )
#define catch_null(EXP) while( (EXP) == NULL )
#define catch_ef(EXP) while( (EXP) )

#endif 
