/***************************************************************************/
/* Descripcion:                                                            *
/*  Contiene las funciones útiles y variadas para distintas aplicaciones   *
/***************************************************************************/

#include "util.h"
#include "lcd.h"

// Delay en Mili-segundos con chequeo
// F_CPU=8MHz => Max_delay=8,38seg, Min_delay=0,128mseg
void delay_ms_ck (int delay)
{
 delay=delay * 7 + (delay/16)*13;
 OCR1AH = (delay>>8);
 OCR1AL = (delay&0x00FF);
 TCNT1H = 0;
 TCNT1L = 0;
 TCCR1B = 0x0D; // Activa el prescaler y modo de trabajo CTC
}

// Delay en Mili-segundos
void delay_ms (int delay)
{
 delay_ms_ck(delay);
 while (!END_TDELAY);
 set_bit(OCF1A,TIFR);
 TCCR1B=0;
}

// Restart delay
void rst_delay_ms(void)
{
 TCNT1H=0;
 TCNT1L=0;
 set_bit(OCF1A,TIFR);
}

// Delay en Mili-segundos, SOLO PARA EL TECLADO
void delay_us (int delay)
{// F_CPU=8MHz, Max 8192mseg, Min 32useg
 OCR0 = delay>>5; // Equiv = (delay/1e6)*(F_CPU/256)
 TCNT0 = 0;
 TCCR0 = 0x0E; // Activa el prescaler y modo de trabajo CTC
 while (!END_KTDELAY);
 TIFR = ((TIFR | (1<<OCF0))&(~(1<<OCF1A))); // Intenta no apagar el flag del timer 1
 TCCR0=0;
}


void int2str(long c)
{// Convierte numeros [-32768;32767]
 unsigned char v[MAXDINT];
 short int i;
 if (c<0)
	{
	 c=-c;
	 lcd_putchar('-');
	}
 for (i=0;i<MAXDINT;i++)
	{
	 v[i] = c%10; // Guarda el resto que es el numero
	 c=c/10;
	 if (c==0)
		break;
	}
 for (;i>=0;i--)
	{
	 lcd_putchar(((v[i])+'0'));
	}
}

void float2str(float x)
{
 long a=floor(x);
 int2str(a); // Imprime la parte entera
 lcd_putchar('.');
 long b = floor(100*(x - a));
 int2str(b); // Toma 5 decimales
}

void strcpy_Pe (char *dest, PGM_P org)
{
 unsigned i=0;
 do
	{
	 dest[i]=pgm_read_byte(&org[i]);
	 i++;
	}
 while (((pgm_read_byte(&org[i]))!=0)||((pgm_read_byte(&org[i]))!='\0'));
 dest[i]='\0';
}

unsigned char strncmp_Pe(char *s1, PGM_P s2, unsigned char n)
{
 unsigned char i=0;
 while (i<n)
	{
	 if (s1[i]!=pgm_read_byte(s2++))
		return 1;
	 i++;
	}
 return 0;
}

unsigned char strncmp_e(char *s1, char *s2, unsigned char n)
{
 unsigned char i=0;
 while (i<n)
	{
	 if (s1[i]!=s2[i])
		return 1;
	 i++;
	}
 return 0;
}

unsigned char strchr_e(char *s, char c)
{
 unsigned char n=0;
 while (1)
	{
	 if (s[n]==c)
		return (n+1);// Da el siguiente
	 n++;
	}
}

int strtolong(const char* str)
{
 int l = 0;
 while(*str >= '0' && *str <= '9')
	 l = l * 10 + (*str++ - '0');
 return l;
}

// Control del BUZZER
void sound_init (void)
{
 set_bit(BUZZER,P_dBUZ);
 clr_bit(BUZZER,P_wBUZ);
}

void sound_error (void)
{
 set_bit(BUZZER,P_wBUZ);
 delay_ms(150);
 clr_bit(BUZZER,P_wBUZ);
 delay_ms(70);
 set_bit(BUZZER,P_wBUZ);
 delay_ms(150);
 clr_bit(BUZZER,P_wBUZ);
}

void sound_ok (void)
{
 set_bit(BUZZER,P_wBUZ);
 delay_ms(300);
 clr_bit(BUZZER,P_wBUZ);
}
