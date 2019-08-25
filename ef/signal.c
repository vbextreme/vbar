#include <ef/os.h>

err_t os_signal_get_status(sigset_t* mask){
	catch_posix( sigprocmask(0, NULL, mask) ){
		dbg_error("sigprocmask");
		dbg_errno();
		return errno;
	}
	return 0;
}

err_t os_signal_set(sigaction_s* old, int num, signal_f fnc){
	sigaction_s sa = {
		.sa_sigaction = fnc,
		.sa_flags = SA_RESTART | SA_SIGINFO,
	};
	sigemptyset(&sa.sa_mask);
	
	catch_posix( sigaction(num, &sa, old) ){
		dbg_error("sigaction");
		dbg_errno();
		return errno;
	}

	return 0;
}

err_t os_signal_restore(int num, sigaction_s* sa){
	catch_posix( sigaction(num, sa, NULL) ){
		dbg_error("sigaction");
		dbg_errno();
		return errno;
	}
	return 0;
}

void signal_wait(void){
	pause();
}
