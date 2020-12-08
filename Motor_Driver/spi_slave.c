/***************************************************************************/
/* Descripcion:                                                            *
/*  Programa control SPI del esclavo. Descrive los procesos para recibir   *
/*  y enviar comandos. La funcion importante es spi_poll, ya que es la que *
/*  consulta si llego un nuevo comando.                                    *
/***************************************************************************/

#include "spi_slave.h"
#include "motor.h"

volatile unsigned char spi_state=SPI_IDLE;
volatile spi_data spi_dint; // Datos necesarios en la interrupcion
volatile char spi_flag= SPI_OK;

/*------------------------------------------------*/
/* SPI Inicialización (SLAVE)                     */
/*------------------------------------------------*/
void spi_init(void)
{
 P_dSPI=P_dSPI|(1<<MISO);
 P_wSPI=P_wSPI|(1<<MOSI)|(1<<SCK)|(1<<SS);
 SPCR = (1<<SPE)|(1<<SPIE); //0x40; 				//Habilito la comunicación SPI (seteo del bit SPE) y las interrupciones
}

/*------------------------------------------------*/
/* SPI Interrupcion (lectura y escritura)         */
/*------------------------------------------------*/
ISR (SPI_STC_vect)
{
 unsigned char aux=SPDR; // Lectura innecesaria, para habilitar la escritura
 if (spi_state==SPI_IDLE)
	{
	 spi_dint.cmd=aux; // Toma el comando que viene como primer dato
	 spi_state++; // Incrementa, porque ya no esta en IDLE => (RXCMD o TXCMD)
	}
 unsigned char spi_last = spi_dint.cmd&SPI_DSIZE;
 if ((spi_dint.cmd&SPI_SRBIT)==0)
	{ // Es cero => Lectura al esclavo
	 switch (spi_state)
		{
		 case SPI_RXCMD:
			switch (spi_dint.cmd)
				{ // Carga el buffer con los datos del esclavo
				 case RMOTST:
					 if (stepsAR!=0)
						spi_dint.buf[0]=1;
					 else
						spi_dint.buf[0]=0;
					 if (stepsDEC!=0)
						spi_dint.buf[1]=1;
					 else
						spi_dint.buf[1]=0;
				 break;
				 case RPOS:
					 spi_dint.buf[0]=tpos.ra.h;
					 spi_dint.buf[1]=tpos.ra.m;
					 spi_dint.buf[2]=tpos.ra.s;
					 spi_dint.buf[3]=tpos.dec.d;
					 spi_dint.buf[4]=tpos.dec.m;
					 spi_dint.buf[5]=tpos.dec.s;
				 break;
				 default:
					 spi_flag=SPI_ERROR; // Si no conoce el comando manda error
				 break;
				}
			 SPDR=spi_dint.buf[0]; // Carga el dato 1, para tx al maestro
			 spi_state++;
		 break;
		 case SPI_RXEND0:
			 if (spi_dint.buf[spi_last]!=aux)
				 spi_flag=SPI_ERROR;
			 SPDR=spi_flag; // Carga el fin de comando 2
			 spi_state++;
		 break;
		 case SPI_RXEND1:
			 spi_state=SPI_IDLE; // Si hubo error, el master retransmite, el esclavo vuelve a IDLE
			 spi_flag=SPI_OK;
		 break;
		 default: // RXD[0-5]
			 if (spi_state==SPI_RXD0)
				{
				 if (aux!=spi_dint.cmd) // Si no recibio nuevamente el comando hay error
					 spi_flag=SPI_ERROR;
				}
			 else
				{// Debe verificar los datos
				 if (aux!=spi_dint.buf[(spi_state-3)])
					 spi_flag=SPI_ERROR;
				}
			 if (spi_state==(spi_last+2))
				{
				 SPDR=spi_flag; // Fin de comando
				 spi_state=SPI_RXEND0;
				}
			 else
				{
				 SPDR=spi_dint.buf[(spi_state-1)]; // Manda los datos
				 spi_state++;
				}
		 break;
		}
	}
 else
	{// Transmision al esclavo
	 switch (spi_state)
		{
		 case SPI_TXCMD:
			 spi_state++; // Retransmite lo almacenado, ACK CMD
		 break;
		 case SPI_TXEND:
			 if (SPDR==SPI_OK) // el maestro debera transmitir
				{
				 spi_state=SPI_IDLE;
				 spi_flag=SPI_OK;
				}
		 break;
		 default: //(SPI_TXD[0-5])
			 spi_dint.buf[(spi_state-2)]=SPDR; // carga los datos que llegan, en SPDR queda el dato para el ACK
			 if (spi_state==(spi_last+2))
				 spi_state=SPI_TXEND;
			 else
				 spi_state++;
		 break;
		}
	}
}

