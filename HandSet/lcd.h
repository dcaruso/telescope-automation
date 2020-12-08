/***************************************************************************/
/* Descripcion:                                                            *
/*  Tiene los prototipos de las funciones que estan en lcd.c, junto con    *
/*  las constantes usadas para los comandos del lcd y los metodos para     *
/*  particionar el bus de datos.                                           *
/***************************************************************************/

#include <stdio.h>
#include <avr/io.h>
#include "config.h"

//// Definicion de los metodos de escritura y lectura del bus de datos
#define wDATO(data) LCD_wDATO=data
#define rDATO       LCD_rDATO
#define sBUS(data)  LCD_dDATO=data

/* REGISTROS DEL LCD */
#define Y_INI   0xB8    /* COMANDO POSICION Y=0 FILAS*/
#define X_INI   0x40    /* CAMANDO POSICIION X=0 COLUMNAS*/
// Nota: en el manual estan al reves los nombres de X e Y.
#define ST_LN   0xC0    /* CAMANDO LINEA INICIAL=0  */
#define D_ON    0x3F    /* CAMANDO DISPLAY ON */
#define D_OFF   0x3E    /* COMANDO DISPLAY OFF */
#define BSY     0x80

#define LCD_CLEAR		0	// No invierte el dibujo
#define LCD_INV	1	// Invierte el dibujo

extern volatile unsigned char CursorX, CursorY;
extern volatile unsigned char lcd_state;

// Prototipos de funciones
void lcd_clr();
void lcd_init();
void lcd_setcursor (unsigned char, unsigned char);
void lcd_wrdat (unsigned char, unsigned char);
void lcd_wrcom (unsigned char);
//void lcd_busy (void);
void lcd_putchar (char);
void lcd_string (char *);
void lcd_string_P (PGM_P);
void lcd_selfont (char);
void lcd_bmp(PGM_P);
unsigned char lcd_rdat(void);

