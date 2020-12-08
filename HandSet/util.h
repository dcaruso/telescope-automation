// Header de util.c
#include "config.h"
#include <math.h>

#define STEP_PCN	5

#define TCN_VA		1
#define TCN_NVA		0

#define END_TDELAY	(TIFR&(1<<OCF1A))
#define END_KTDELAY	(TIFR&(1<<OCF0))
#define delay_1us	asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n") // Delay 0,125useg @ 8MHz=F_CPU
#define delay_5us	delay_1us;delay_1us;delay_1us;delay_1us;delay_1us;
#define delay_10us	delay_5us;delay_5us;

#define MAXDINT 9

void delay_ms (int);
void delay_ms_ck (int);
void rst_delay_ms(void);
void delay_us(int);
void int2str(long);
void float2str(float);
void strcpy_Pe(char *, PGM_P);
unsigned char strncmp_Pe(char *, PGM_P, unsigned char);
unsigned char strncmp_e(char *, char *, unsigned char);
int strtolong(const char* str);
unsigned char strchr_e(char*, char);
void sound_init (void);
void sound_error (void);
void sound_ok (void);
