/***************************************************************************/
/* Descripcion:                                                            *
/*  Este header tiene todas las configuraciones del funcionamiento del     *
/*  programa. Es decir, aca se definen los puertos, que micro usar, macros *
/*  generales, configuracion de teclas, etc. Debe estar incluido en todos  *
/*  los .h y en los .c que no tengan su archivo .h.                        *
/***************************************************************************/

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <util/delay.h>

//------------ MACROS ------------//
#define set_bit(bit,reg) reg|=(1<<bit)
#define clr_bit(bit,reg) reg&=~(1<<bit)

#define rd_bit(bit,reg)  reg&(1<<bit)

//------------ MOTOR ------------//
#define wMOT_SIG		PORTD
#define dMOT_SIG		DDRD
#define dMOT_ENA		DDRB
#define wMOT_ENA		PORTB
#define mMOT_AR			0xF0
#define mMOT_DEC		0x0F

#define bAR_IN1			4 // PORTD
#define bAR_IN2			7 // PORTD
#define bAR_IN3			5 // PORTD
#define bAR_IN4			6 // PORTD

#define bAR_EN			1 // PORTB

#define bDEC_IN1		3 // PORTD
#define bDEC_IN2		2 // PORTD
#define bDEC_IN3		0 // PORTD
#define bDEC_IN4		1 // PORTD

#define bDEC_EN			0 // PORTB


//------------ SPI ------------//
#define P_dSPI    DDRB
#define P_wSPI    PORTB
#define MOSI      3
#define MISO      4
#define SCK       5
#define SS        2




