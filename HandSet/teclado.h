// Header del teclado.c
#include "config.h"
#include "util.h"

// Mapa del teclado
#define BUT_UP		0xB7
#define BUT_LT		0xD7
#define BUT_RT		0x77
#define BUT_DW		0xE7

#define BUT_1		0x7E
#define BUT_2		0x7D
#define BUT_3		0x7B

#define BUT_4		0xBE
#define BUT_5		0xBD
#define BUT_6		0xBB

#define BUT_7		0xDE
#define BUT_8		0xDD
#define BUT_9		0xDB

#define BUT_ESC		0xEE
#define BUT_0		0xED
#define BUT_ENTER	0xEB


#define FILr		0xF0  // pull-up en las filas
#define COLr		0x0F  // 
#define COLo		0x0F  // Pone las columnas como salida
#define FILo		0xF0  // Pone las filas como salida

#define READ_KEY(key,dkey)	{while(codKey==0);key=codKey;dkey=decodKey;codKey=0;}

// variable externas
extern volatile unsigned char codKey;
extern volatile unsigned char decodKey;


unsigned char deco_key (unsigned char);
void key_init(void);

// Teclado de celular
static const char __attribute__ ((progmem)) keyCell[10][5] = {{'0',' ',0,0,0},
															  {'1',0,0,0,0},
															  {'2','A','B','C',0},
															  {'3','D','E','F',0},
															  {'4','G','H','I',0},
															  {'5','J','K','L',0},
															  {'6','M','N','O',0},
															  {'7','P','Q','R','S'},
															  {'8','T','U','V',0},
															  {'9','W','X','Y','Z'}};


// Prototipos de funciones

// Funciones de teclas por posicion
//                                  ^
//1     2     3     E1     |  x     |     x     x
//                         |
//4     5     6     E2     |  <-    x     ->    x
//                         |
//7     8     9     E3     |  x     |     x     x
//                         |        
//ESC   0     Ent   E4     |  x     x     x     x

