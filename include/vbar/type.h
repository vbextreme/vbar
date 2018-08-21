#ifndef __EF_TYPE_H__
#define __EF_TYPE_H__

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>

#ifndef _Y_ 
	#define _Y_ 1
#endif
#ifndef _N_
	#define _N_ 0
#endif

#define EMPTY_MACRO do{}while(0)
#define ROUND_UP(N,S) ((((N)+(S)-1)/(S))*(S))

#define __VA_COUNT__(...) __VA_COUNT_IMPL__(__VA_ARGS__, 7,6,5,4,3,2,1)
#define __VA_COUNT_IMPL__(_1,_2,_3,_4,_5,_6,_7,N,...) N

#define forever() for(;;)
#define __ef_private static
#define __ef_unused __attribute__((unused))
#define __ef_cleanup(FNC) __attribute__((__cleanup__(FNC)))
#define __ef_printf(FRMT,VA) __attribute__((format (printf, FRMT, VA)))

typedef uint8_t byte_t;
typedef uint16_t word_t;
typedef uint32_t dword_t;
typedef uint64_t qword_t;
typedef dword_t flags_t;
typedef int err_t;

typedef enum { FALSE, TRUE } bool_t;

#define DO_PRAGMA(DOP) _Pragma(#DOP)
#define UNSAFE_BEGIN(FLAGS) DO_PRAGMA(GCC diagnostic push); DO_PRAGMA(GCC diagnostic ignored FLAGS)
#define UNSAFE_END DO_PRAGMA(GCC diagnostic pop)

#define DBG_OUTPUT stderr

#ifdef EF_DEBUG_COLOR
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

#ifndef EF_DBG_INFO
	#define DBG_INFO    "info"
#endif
#ifndef EF_DBG_WARNING
	#define DBG_WARNING "warning"
#endif
#ifndef EF_DBG_ERROR
	#define DBG_ERROR   "error"
#endif
#ifndef EF_DBG_FAIL
	#define DBG_FAIL    "fail"
#endif
#ifndef EF_DBG_ERRNO
	#define DBG_ERRNO   "errno"
#endif

#ifndef EF_DBG_LVL_FAIL
	#define DBG_LVL_FAIL    1
#endif
#ifndef EF_DBG_LVL_ERROR
	#define DBG_LVL_ERROR   2
#endif
#ifndef EF_DBG_LVL_WARNING
	#define DBG_LVL_WARNING 3
#endif
#ifndef EF_DBG_LVL_INFO
	#define DBG_LVL_INFO    4
#endif

#if EF_DEBUG_ENABLE >= 1
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

#if EF_DEBUG_ENABLE > DBG_LVL_FAIL
	#define dbg_error(FORMAT, arg...) dbg(DBG_ERROR, DBG_COLOR_ERROR, FORMAT, ## arg)
#else
	#define dbg_error(FORMAT, arg...) EMPTY_MACRO
#endif

#if EF_DEBUG_ENABLE > DBG_LVL_ERROR
	#define dbg_warning(FORMAT, arg...) dbg(DBG_WARNING, DBG_COLOR_WARNING, FORMAT, ## arg)
#else
	#define dbg_warning(FORMAT, arg...) EMPTY_MACRO
#endif

#if EF_DEBUG_ENABLE > DBG_LVL_WARNING
	#define dbg_info(FORMAT, arg...) dbg(DBG_INFO, DBG_COLOR_INFO, FORMAT, ## arg)
#else
	#define dbg_info(FORMAT, arg...) EMPTY_MACRO
#endif
	
#if EF_ASSERT_ENABLE == _Y_
	#define iassert(C) do{ if ( !(C) ){fprintf(stderr,"assertion fail %s[%u]: %s\n", __FILE__, __LINE__, #C); exit(0);}}while(0)
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
