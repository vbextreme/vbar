#include <ef/delay.h>
#include <utime.h>
#include <time.h>
#include <sys/time.h>

/*** get time in milliseconds from starting os ***/
uint64_t time_ms(void){
	struct timespec ts; 
	clock_gettime(CLOCK_REALTIME, &ts); 
    return ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

/*** get time in microseconds from starting os ***/
uint64_t time_us(void){
	struct timespec ts; 
	clock_gettime(CLOCK_REALTIME, &ts); 
    return ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
}

/*** get time in seconds from starting os ***/
double time_dbls(void){
	struct timespec ts;  
	clock_gettime(CLOCK_REALTIME, &ts); 
    return ts.tv_sec + ts.tv_nsec*1e-9;
}

/*** sleep milliseconds ***/
void delay_ms(uint64_t ms){
	struct timespec tv;
	tv.tv_sec = (time_t) ms / 1000;
	tv.tv_nsec = (long) ((ms - (tv.tv_sec * 1000)) * 1000000L);

	while (1){
		int rval = nanosleep(&tv, &tv);
		if (rval == 0){
			return;
		}
		else if (errno == EINTR){
			dbg_warning("is correct recall nanosleep with same delay?");
			continue;
		}
		else{
			dbg_error("nanosleep fail");
			dbg_errno();
			return;
		}
	}
}

/*** sleep microseconds ***/
void delay_us(uint64_t us){
	struct timespec tv;
	tv.tv_sec = (time_t) us / 1000000;
	tv.tv_nsec = (long) ((us - (tv.tv_sec * 1000000)) * 1000L);

	while (1){
		int rval = nanosleep (&tv, &tv);
		if (rval == 0){
			return;
		}
		else if (errno == EINTR){
			dbg_warning("is correct recall nanosleep with same delay?");
			continue;
		}
		else{
			dbg_error("nanosleep fail");
			dbg_errno();
			return;
		}
	}
	return;
}

/*** sleep seconds as double ***/
void delay_dbls(double s){
	dbg_info("%lf s", s);
	struct timespec tv;
	tv.tv_sec = (time_t) s;
	tv.tv_nsec = (long) ((s - tv.tv_sec) * 1e+9);

	while (1)
	{
		int rval = nanosleep(&tv, &tv);
		if (rval == 0){
			dbg_info("end delay");
			return;
		}
		else if (errno == EINTR){
			dbg_warning("is correct recall nanosleep with same delay?");
			continue;
		}
		else{
			dbg_error("nanosleep fail");
			dbg_errno();
			return;
		}
	}
}

/*** wait microsecons without sleep ***/
/*** this function can be use 100% of cpu if the time is long ***/
void delay_hard(uint64_t us){
	dbg_info("%lu us", us);
	uint32_t t = time_us();
	while( time_us() - t < us );
	dbg_info("end delay");
}


