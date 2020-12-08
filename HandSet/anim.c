/***************************************************************************/
/* Descripcion:                                                            *
/*  Contiene rutinas para realizar animaciones en el menu                  *
/***************************************************************************/

#include "anim.h"
#include "lcd.h"
#include "images.h"

volatile s_anim animx;
volatile unsigned char pres2;
void start_anim (unsigned char xpos, unsigned char ypos, unsigned time, unsigned char name)
{
// F_CPU=8MHz, min=20mseg max=5100mseg
 OCR2 = (time/20);
 TCNT2 = 0;
 TCCR2 = (1<<CS20)|(1<<CS22)| (1<<WGM21); // Activa el prescaler y modo de trabajo CTC
 set_bit(OCIE2,TIMSK); // Habilita la interrupcion
 animx.name=name;
 animx.state=0;
 animx.xpos=xpos;
 animx.ypos=ypos;
 pres2=0;
}

void stop_anim (void)
{
 clr_bit(OCIE2,TIMSK);
 TCCR2 = 0;
 set_bit(OCF2,TIMSK);
}

ISR(TIMER2_COMP_vect)
{
 pres2++;
 if (pres2==156)
	{
	 unsigned char xpos_aux=CursorX,ypos_aux=CursorY;
	 lcd_setcursor(animx.xpos,animx.ypos);
	 animx.state++;
	 switch (animx.name)
		{
		 case ANIM_LOADING1:
			if (animx.state==4)
				animx.state=0;
			switch (animx.state)
				{
				 case 0:
					lcd_putchar('-');
				 break;
				 case 1:
					lcd_putchar(92);
				 break;
				 case 2:
					lcd_putchar('|');
				 break;
				 case 3:
					lcd_putchar('/');
				 break;
				}
		 break;
		 case ANIM_LOADING2:
			if (animx.state==3)
				animx.state=0;
			switch (animx.state)
				{
				 case 0:
					lcd_string_P(PSTR(".  "));
				 break;
				 case 1:
					lcd_string_P(PSTR(" . "));
				 break;
				 case 2:
					lcd_string_P(PSTR("  ."));
				 break;
				}
		 break;
		 case ANIM_LOADING3:
			if (animx.state==3)
				animx.state=0;
			switch (animx.state)
				{
				 case 0:
					lcd_wrdat(0x00,0);
					lcd_wrdat(0x00,0);
					lcd_wrdat(0x60,0);
					lcd_wrdat(0x60,0);
					lcd_wrdat(0x00,0);
					lcd_wrdat(0x00,0);
					lcd_string_P(PSTR("  "));
				 break;
				 case 1:
					lcd_putchar(' ');
					lcd_wrdat(0x00,0);
					lcd_wrdat(0x70,0);
					lcd_wrdat(0x70,0);
					lcd_wrdat(0x70,0);
					lcd_wrdat(0x00,0);
					lcd_wrdat(0x00,0);
					lcd_putchar(' ');
				 break;
				 case 2:
					lcd_string_P(PSTR("  "));
					lcd_wrdat(0x00,0);
					lcd_wrdat(0x30,0);
					lcd_wrdat(0x78,0);
					lcd_wrdat(0x78,0);
					lcd_wrdat(0x30,0);
					lcd_wrdat(0x00,0);
				 break;
				}
		 break;
		 case ANIM_UTN:
			switch (animx.state)
				{
				 case 0:
					lcd_bmp(&putn0[0]);
				 break;
				 case 1:
					lcd_bmp(&putn1[0]);
				 break;
				 case 2:
					lcd_bmp(&putn2[0]);
				 break;
				 case 3:
					lcd_bmp(&putn3[0]);
				 break;
				 case 4:
					lcd_bmp(&putn4[0]);
				 break;
				 case 5:
					lcd_bmp(&putn5[0]);
				 break;
				 case 6:
					lcd_bmp(&putn6[0]);
				 break;
				 case 7:
					lcd_bmp(&putn7[0]);
				 break;
				 case 8:
					animx.xpos=0;
					animx.ypos=0;
					lcd_bmp(&putn8[0]);
				 break;
				 case 9:
					 TCCR2 = 0;
					 clr_bit(OCIE2,TIMSK);
					 lcd_setcursor(0,0);
					 lcd_bmp(&p00000_Caratula[0]); //Logo de la UTN y Presentacion
				 break;
				}
		 break;
		}
	 lcd_setcursor(xpos_aux,ypos_aux);
	 pres2=0;
	}
}
