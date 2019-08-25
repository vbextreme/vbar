#ifndef __EF_DELAY_H__
#define __EF_DELAY_H__

#include <ef/type.h>
#include <sys/sysinfo.h>

uint64_t time_ms(void);
uint64_t time_us(void);
double time_dbls(void);
void delay_ms(uint64_t ms);
void delay_us(uint64_t us);
void delay_dbls(double s);
void delay_hard(uint64_t us);

#endif
