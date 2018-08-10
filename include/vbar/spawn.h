#ifndef __VBAR_SPAWN_H__
#define __VBAR_SPAWN_H__

#include <vbar/type.h>

#ifndef SPAWN_SHELL
	#define SPAWN_SHELL_PATH "/bin/bash"
	#define SPAWN_SHELL "bash"
	#define SPAWN_ARGUMENT "-c"
#endif

void spawn_init(void);
int spawn_shell(char* cmdline);

#endif
