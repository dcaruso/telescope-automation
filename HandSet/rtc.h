// Header de rtc.c
#include "config.h"

#define		HEX2BCD(data)	(((data/10)<<4)|(data%10))
#define		BCD2HEX(data)	((10*(data>>4))+(data&0x0F))


unsigned char rtc_read(void);
unsigned char rtc_writeTime(unsigned char,unsigned char,unsigned char);
unsigned char rtc_writeDate(unsigned char,unsigned char,unsigned char);
unsigned char rtc_write(void);
unsigned char rtc_out1hz(void);
void rtc_init(void);
unsigned char rtc_wrbkp(void);
unsigned char rtc_rdbkp(void);

//void rtcfalse_init (void);
//void rtcfalse_print(void);
//void rtc_print (void);

// Memoria del RTC

//00 - Segundos
//01 - Minutos
//02 - Horas
//03 - Dia de la semana
//04 - Dia
//05 - Mes
//06 - Año
//07 - Control
//---- RAM
//08 - Huso Horario
//09 - Latitud (Grados)
//10 - Latitud (Minutos)
//11 - Latitud (Hemisferio)
//12 - Longitud (Grados)
//13 - Longitud (Minutos)
//14 - Longitud (Este/Oeste)
//15 - Altura sobre el mar (hmar)
//16 - Parking flag

