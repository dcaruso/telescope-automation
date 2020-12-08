/***************************************************************************/
/* Descripcion:                                                            *
/*  Contiene las rutinas para controlar la comunicación con la PC,         *
/*  aplicando el protocolo del telescopio Meade Lx200                      *
/***************************************************************************/

#include "uart.h"
#include "util.h"
#include "menu.h"
#include "font.h"

void uart_init (void)
{
 /* set baud rate */
 UBRR0H = UBRRVAL >> 8;
 UBRR0L = UBRRVAL & 0xff;
/* set frame format: 8 bit, no parity, 1 bit */
 UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    /* enable serial receiver and transmitter */
 UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
 com_state=COM_IDLE;
 int i=0;
 for (;i<MAX_COM;i++)
	pc[i].cmd=NO_CMDPC;
 respondPC=AUTO;
}

void uart_TxByte(char c)
{
 while ( !( UCSR0A & (1<<UDRE0)) );   //Esperar transmision completa
 UDR0 = c;
}

void int2uart (int c)
{
 unsigned char v[MAXDINT];
 short int i;
 if (c<10)
	uart_TxByte('0');
 for (i=0;i<MAXDINT;i++)
	{
	 v[i] = c%10; // Guarda el resto que es el numero
	 c=c/10;
	 if (c==0)
		break;
	}
 for (;i>=0;i--)
	{
	 uart_TxByte(((v[i])+'0'));
	}
}

void uart_send(unsigned char size, char *sep,...) 
{
 int i;
 va_list vl;
 va_start(vl,size);
 for (i=0;i<size;i++)
	{
	 int2uart(va_arg(vl, int16_t));
	 uart_TxByte(*sep);
	 sep++;
	}
 va_end(vl);
}

volatile unsigned char com_state=COM_IDLE;
volatile uart_data pc[MAX_COM];
volatile unsigned char index_pcdata;
volatile unsigned char ind_pcread=MAX_COM-1, ind_pcwrite=MAX_COM-1;
volatile unsigned char respondPC;
ISR(USART0_RX_vect)
{
 char c;
 c = UDR0; //Read the value out of the UART buffer
 if (c=='#') // Descarta el primer '#'
	{
	 com_state=COM_IDLE;
	 if ((respondPC==AUTO)&&(ind_pcread!=ind_pcwrite))
		{
		 ind_pcread++;
		 if (ind_pcread==MAX_COM)
			ind_pcread=0;
		 switch (pc[ind_pcread].cmd)
			{
			 case GET_RA:
				 uart_send(3,"::#",tpos.ra.h,tpos.ra.m,tpos.ra.s);
			 break;
			 case GET_DEC:
				 if ((tpos.dec.d<0)||(tpos.dec.m<0)||(tpos.dec.s<0))
					 uart_TxByte('-');
				 else
					 uart_TxByte('+');
				 uart_send(3,&sym_decPC[0],abs(tpos.dec.d),abs(tpos.dec.m),abs(tpos.dec.s));
			 break;
			}
		 pc[ind_pcread].cmd=NO_CMDPC;
		}
	}
 else
	{
	 switch (com_state)
		{
		 case COM_IDLE: // Comienza el comando con ':'
			 if (c!=':')
				com_state=COM_ERR;
			 else
				{
				 ind_pcwrite++; // Cuando valida un nuevo comando, lo guarda en la siguiente posicion
				 if (ind_pcwrite==MAX_COM)
					 ind_pcwrite=0;
				 index_pcdata=0;
				 pc[ind_pcwrite].data[0]=0;
				}
		 break;
		 case COM_PAR0:
			pc[ind_pcwrite].cmd=c; // Primera letra que define al comando
		 break;
		 case COM_PAR1:
			pc[ind_pcwrite].cmd+=c;// Segunda letra que define al comando
		 break;
		 default: // Resto de los datos
			 if ((c<='9') &&(c>='0')) // Si vienen numeros los convierte
				 pc[ind_pcwrite].data[index_pcdata]=pc[ind_pcwrite].data[index_pcdata]*10+ (c-'0');
			 if ((c==':')|| (c==223)|| (c=='*'))
				{
				 index_pcdata++;
				 pc[ind_pcwrite].data[index_pcdata]=0;
				}
			 if (c=='+')
				pc[ind_pcwrite].sign=1;
			 if (c=='-')
				pc[ind_pcwrite].sign=-1;
		 break;	
		}
	 com_state++;
	}
}


