#include <ef/spawn.h>
#include <ef/file.h>
#include <spawn.h>
#include <signal.h>
#include <sys/wait.h>

extern char** environ;

void spawn_init(void){
	struct sigaction arg = {
		.sa_handler=SIG_IGN,
		.sa_flags=SA_NOCLDWAIT
	};
	sigaction(SIGCHLD, &arg, NULL);
}

pid_t spawn_shell(char* cmdline){
	pid_t child;
	char* argv[] = { SPAWN_SHELL, SPAWN_ARGUMENT, cmdline, NULL };
	dbg_info("%s %s %s", SPAWN_SHELL_PATH, SPAWN_ARGUMENT, cmdline);
	posix_spawn_file_actions_t fact; //?free????
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
	return child;
}

err_t spawn_shell_slurp(char** out, char** err, int* exitcode, char* cmdline){
	pid_t child;
	int opipe[2];
	int epipe[2];
	char* argv[] = { SPAWN_SHELL, SPAWN_ARGUMENT, cmdline, NULL };
	posix_spawn_file_actions_t fact;
	
	if( pipe(opipe) ){
		dbg_error("opening pipe");
		dbg_errno();
	}
	if( pipe(epipe) ){
		close(opipe[0]);
		close(opipe[1]);
		dbg_error("opening pipe");
		dbg_errno();
	}

	posix_spawn_file_actions_init(&fact);
	posix_spawn_file_actions_addclose(&fact, opipe[0]);
	posix_spawn_file_actions_addclose(&fact, epipe[0]);
	posix_spawn_file_actions_adddup2(&fact, opipe[1], 1);
	posix_spawn_file_actions_adddup2(&fact, epipe[1], 2);
	posix_spawn_file_actions_addclose(&fact, opipe[1]);
	posix_spawn_file_actions_addclose(&fact, epipe[1]);

	errno = posix_spawn(&child, SPAWN_SHELL_PATH, &fact, NULL, argv, environ);
	if( errno ){
		dbg_error("posix_spawn %s", cmdline);
		dbg_errno();
		return -1;
	}
	
	close(opipe[1]);
	close(epipe[1]);
	
	size_t unusedlen;
	*out = file_fd_slurp(&unusedlen, opipe[0]);
	*err = file_fd_slurp(&unusedlen, epipe[0]);
	if( waitpid(child, exitcode, 0) < 0 ){
		dbg_error("waitpid");
		dbg_errno();
		*exitcode = -1;
	}
	else{
		*exitcode = WIFEXITED(*exitcode) ? WEXITSTATUS(*exitcode) : -1;
	}

	close(opipe[0]);
	close(epipe[0]);
	posix_spawn_file_actions_destroy(&fact);
	return 0;
}
