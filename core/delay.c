#include <vbar/delay.h>
#include <utime.h>
#include <time.h>
#include <sys/time.h>

/*** get time in milliseconds from starting os ***/
__ef_can_unused uint64_t time_ms(void)
{
	struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return ((uint64_t)t.tv_sec * 1000000ULL + t.tv_usec) / 1000;
}

/*** get time in microseconds from starting os ***/
__ef_can_unused uint64_t time_us(void)
{
	struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return ((uint64_t)t.tv_sec * 1000000ULL + t.tv_usec);
}

/*** get time in seconds from starting os ***/
double time_dbls(void)
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}

/*** sleep milliseconds ***/
void delay_ms(uint64_t ms)
{
	dbg_info("%lu ms", ms);
	struct timespec tv;
	tv.tv_sec = (time_t) ms / 1000;
	tv.tv_nsec = (long) ((ms - (tv.tv_sec * 1000)) * 1000000L);

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

/*** sleep microseconds ***/
void delay_us(uint64_t us)
{
	dbg_info("%lu us", us);
	struct timespec tv;
	tv.tv_sec = (time_t) us / 1000000;
	tv.tv_nsec = (long) ((us - (tv.tv_sec * 1000000)) * 1000L);

	while (1)
	{
		int rval = nanosleep (&tv, &tv);
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
	return;
}

/*** sleep seconds as double ***/
void delay_dbls(double s)
{
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
void delay_hard(uint64_t us)
{
	dbg_info("%lu us", us);
	uint32_t t = time_us();
	while( time_us() - t < us );
	dbg_info("end delay");
}


