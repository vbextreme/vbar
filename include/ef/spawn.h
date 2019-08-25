#ifndef __EF_SPAWN_H__
#define __EF_SPAWN_H__

#include <ef/type.h>

#ifndef SPAWN_SHELL
	#define SPAWN_SHELL_PATH "/bin/bash"
	#define SPAWN_SHELL "bash"
	#define SPAWN_ARGUMENT "-c"
#endif

void spawn_init(void);
pid_t spawn_shell(char* cmdline);
err_t spawn_shell_slurp(char** out, char** err, int* exitcode, char* cmdline);

#endif
