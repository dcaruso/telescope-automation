/***************************************************************************/
/* Descripcion:                                                            *
/*  Rutina de interrupción para la atención al teclado. La misma llama     *
/*  al programa que actua sobre el menu, una vez que reconoció la tecla    *
/*  pulsada                                                                *
/***************************************************************************/

#include "teclado.h"
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

void key_init (void)
{
 codKey=0;
 P_wKBRD = COLr; // pull-up en las filas
 P_dKBRD = FILo; // filas como entradas y columnas como salidas
 set_bit(ISC41,EICRB); // por flanco de bajada
 clr_bit(ISC40,EICRB); // por flanco de bajada
 clr_bit(INTKBRD,P_dIKBRD); // Pone como entrada la interrupcion
 set_bit(INTKBRD,P_wIKBRD); // Pull-up interno a la entrada de interrupcion
 set_bit(INTF4,EIFR);// borra el flag de interrupcion
 TCCR3B = (1<<CS30)|(1<<CS32)|(1<<WGM32); // Prescaler en 1024, modo CTC
 OCR3A = 0xB71B; // Carga 46875 => demora de 6 segundos @ 8MHz
 set_bit(OCIE3A,ETIMSK); // Habilita la interrupcion
 set_bit(INT4,EIMSK); // habilita la interrupcion ext 0
}

unsigned char deco_key (unsigned char key)
{ 
 switch (key)
 	{
 	 case BUT_0:
 	 	return 0;
 	 break;
 	 case BUT_1:
 	 	return 1;
 	 break;
 	 case BUT_2:
 	 	return 2;
 	 break;
 	 case BUT_3:
 	 	return 3;
 	 break;
 	 case BUT_4:
 	 	return 4;
 	 break;
 	 case BUT_5:
 	 	return 5;
 	 break;
 	 case BUT_6:
 	 	return 6;
 	 break;
 	 case BUT_7:
 	 	return 7;
 	 break;
 	 case BUT_8:
 	 	return 8;
 	 break;
 	 case BUT_9:
 	 	return 9;
 	 break;
 	 case BUT_ENTER:
 	 	return 10;
 	 break;
 	 case BUT_ESC:
 	 	return 11;
 	 break;
 	 default:
 	 	return 0xFF;
 	 break;
 	}
}

volatile unsigned char decodKey;
volatile unsigned char codKey;
ISR (INT4_vect)
{
 clr_bit(INT4,EIMSK); // deshabilita la interrupcion ext 0
 delay_us(1000);
 if ((P_rKBRD & COLr)!=COLr)
   { // Esta leyendo algo
    TCNT3 = 0;
    codKey = P_rKBRD & COLr; // Guarda el codigo de las columnas
    P_wKBRD = codKey | FILr; // Columnas con unos, solo fila identificada con cero
    P_dKBRD = (~codKey) & COLo;
	delay_1us;
    codKey |= (P_rKBRD & FILr);
    decodKey=deco_key(codKey);
    set_bit(LCD_BKL,LCD_wBKL); // Enciende el display, por cada tecla pulsada
    while ((P_rKBRD & FILr)!=FILr)
      {
       delay_us(2500); // espera a que suelta
      }
    P_wKBRD = COLr; // filas pull-up 0xF0
    P_dKBRD = FILo; // columnas como salidas 0x0F  
   }   
 set_bit(INTF4,EIFR);// borra el flag de interrupcion
 TCCR3B = (1<<CS30)|(1<<CS32)|(1<<WGM32); // Prende el timer nuevamente
 set_bit(INT4,EIMSK);
}

ISR (TIMER3_COMPA_vect)
{
 clr_bit(LCD_BKL,LCD_wBKL); // Apaga el el display despues de 4 seg
 TCCR3B = 0; // Apaga el prescaler, asi no entra mas a la interrupcion
}


