#include <vbar/spawn.h>
#include <spawn.h>
#include <signal.h>

extern char** environ;

void spawn_init(void){
	struct sigaction arg = {
		.sa_handler=SIG_IGN,
		.sa_flags=SA_NOCLDWAIT
	};
	sigaction(SIGCHLD, &arg, NULL);
}

int spawn_shell(char* cmdline){
	pid_t child;
	char* argv[] = { SPAWN_SHELL, SPAWN_ARGUMENT, cmdline, NULL };
	dbg_info("%s %s %s", SPAWN_SHELL_PATH, SPAWN_ARGUMENT, cmdline);
	posix_spawn_file_actions_t fact;
	if( posix_spawn_file_actions_init(&fact) ){
		dbg_error("file action init");
		return -1;
	}
	if( posix_spawn_file_actions_addclose(&fact, 0) ){
		dbg_error("close stdin");
		return -1;
	}
	if( posix_spawn_file_actions_addclose(&fact, 1) ){
		dbg_error("close stdout");
		return -1;
	}
	errno = posix_spawn(&child, SPAWN_SHELL_PATH, &fact, NULL, argv, environ);
	if( errno ){
		dbg_error("posix_spawn %s", cmdline);
		dbg_errno();
		return -1;
	}
	return 0;
}
