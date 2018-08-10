#ifndef __VBAR_H__
#define __VBAR_H__

#include <vbar/type.h>
#include <vbar/memory.h>
#include <vbar/string.h>
#include <vbar/delay.h>
#include <vbar/config.h>
#include <vbar/ipc.h>
#include <vbar/modules.h>
#include <sys/sysinfo.h>

int cpu_mod_load(module_s* mod, char* path);
int mem_mod_load(module_s* mod, char* path);

#endif
