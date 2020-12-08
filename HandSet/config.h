/***************************************************************************/
/* Descripcion:                                                            *
/*  Este header tiene todas las configuraciones del funcionamiento del     *
/*  programa. Es decir, aca se definen los puertos, que micro usar, macros *
/*  generales, etc. Debe estar incluido en todos los .h y en los .c que no *
/*  tengan .h.                                                             *
/***************************************************************************/

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#include <avr/pgmspace.h>

//------------ MACROS ------------//
#define set_bit(bit,reg) reg|=(1<<bit)
#define clr_bit(bit,reg) reg&=~(1<<bit)

//------------ LCD ------------//
#define LCD_wCTRL		PORTC/* BUS DE CONTROL */
#define LCD_dCTRL		DDRC
#define LCD_SIDE		PORTC
#define RST_LCD			2    /* RESET LCD */
#define DI_O			3    /* DATA / INSTRUCCION */
#define RW_O			5    /* READ/WRITE */
#define E_O				7    /* ENABLE */
#define C1				6    /* CHIP SELECT 1 */
#define C2				4    /* CHIP SELECT 2 */
#define LCD_wDATO		PORTA // Si no esta particionado, es un puerto completo
#define LCD_rDATO		PINA
#define LCD_dDATO		DDRA 
#define DER				C2
#define IZQ				C1
#define LCD_wBKL		PORTD // Backligth del LCD
#define LCD_dBKL		DDRD
#define LCD_BKL			7

//---------- TECLADO ----------//
#define P_wKBRD			PORTF
#define P_rKBRD			PINF
#define P_dKBRD			DDRF
#define P_wIKBRD		PORTE
#define P_dIKBRD		DDRE
#define INTKBRD			4

//---------- RTC ----------//
#define P_wRTC			PORTD
#define P_dRTC			DDRD
#define P_rRTC			PIND
#define INT1HZ			2
#define I2C_SDA			1
#define I2C_SCL			0

//------------ SPI ------------//
#define P_dSPI			DDRB
#define P_wSPI			PORTB
#define MOSI			2
#define SCK				1
#define MISO			3
#define SS_MD			0
#define SS_SD			4

//------------ BUZZER ------------//
#define P_dBUZ			DDRE
#define P_wBUZ			PORTE
#define BUZZER			6

