#ifndef __EF_OS_H__
#define __EF_OS_H__

#include <vbar/type.h>
#include <signal.h>

typedef void(*signal_f)(int, siginfo_t*, void*);
typedef struct sigaction sigaction_s;
typedef void(*killer_f)(void*);

/*** signal.c ***/
err_t ef_os_signal_get_status(sigset_t* mask);
err_t ef_os_signal_set(sigaction_s* old, int num, signal_f fnc);
err_t ef_os_signal_restore(int num, sigaction_s* sa);
void ef_os_signal_wait(void);

/*** safe.c ***/
bool_t ef_os_valid_addr_read(void* addr);
bool_t ef_os_valid_addr_write(void* addr);
void ef_os_segfault_report(killer_f fnc);
void ef_os_segfault_release(void);

#endif
