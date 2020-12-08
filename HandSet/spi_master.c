/***************************************************************************/
/* Descripcion:                                                            *
/*  Rutinas de control del protocolo SPI, Master Side                      *
/***************************************************************************/

#include "spi_master.h"
#include "lcd.h"
#include "teclado.h"
#include "menu.h"

volatile spi_data sdata;

/* SPI Inicialización (MASTER) */
void spi_init(void)
{
 P_wSPI =  (1<<MISO)|(1<<SS_MD)|(1<<SS_SD); // pullup MISO
 P_dSPI =(1<<MOSI)|(1<<SCK)|(1<<SS_MD)|(1<<SS_SD);
 SPCR =(1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);	// Fosc/128
}


spi_data spi_read(char spi_dtx)
{ // Pasa como parámetro el comando, en spi_dtx
 unsigned char spi_state=SPI_RXCMD;
 unsigned char spi_flag= SPI_OK;
 spi_data d_rx;
 unsigned char error_count=0;
 unsigned char tecla;

 d_rx.cmd=spi_dtx;
 unsigned char spi_last = d_rx.cmd&SPI_DSIZE;

 while (spi_state!=SPI_IDLE)  // Espera a que se termine la transferencia anterior
   {
    delay_ms(1);
    clr_bit(SS_MD,P_wSPI); // Habilita el dialogo con el esclavo
    SPDR=spi_dtx;           // Carga el dato a enviar
    while(!(SPSR & (1<<SPIF)));	// Espero hasta que se complete la Tx.
    set_bit(SS_MD,P_wSPI);

    switch (spi_state)
       {
        case SPI_RXCMD: // Recibio ack basura -> descarta
           spi_dtx=SPDR; // Lectura obligada para escribir
           spi_dtx = d_rx.cmd; // Retransmite el comando
           spi_state++;
        break;
        case SPI_RXEND0:
           spi_flag=SPDR;
           spi_dtx=spi_flag;
           spi_state++;
        break;
        case SPI_RXEND1:
           spi_flag=SPDR;
           if (spi_flag!=SPI_OK) // Si no llego bien, retransmite
             {
              error_count=error_count+1;
              if (error_count==SPI_CERR)
                {
                 lcd_clr();
                 lcd_string_P(PSTR("Error RX\n"));
                 lcd_string_P(PSTR("Verifique el cable al esclavo\n Luego ENTER"));
                 do
                   {
                    READ_KEY(tecla,spi_flag);
                   }
                 while (tecla!=BUT_ENTER);
                }
              spi_state=SPI_RXCMD;
              spi_dtx = d_rx.cmd;
              spi_flag=SPI_OK;
             }
           else
              spi_state=SPI_IDLE; // Termino la lectura
        break;
        default: //(SPI_RXD[0-5])
           d_rx.buf[(spi_state-2)]=SPDR; // Prepara el siguiente dato
           spi_dtx = d_rx.buf[(spi_state-2)]; // Retransmite lo recibido
           if (spi_state==(spi_last+2))
              spi_state=SPI_RXEND0;
           else
              spi_state++;
        break;
       }
   }
 return d_rx;
}


// Los datos deben estar cargados en spitx_bf
void spi_send(unsigned char size, unsigned char cmd, ...)
{
 int i;
 spi_data d_tx;
// int16_t val;
 va_list vl;
 va_start(vl,size);
 for (i=0;i<size;i++)
	 d_tx.buf[i]=va_arg(vl, int16_t);
 va_end(vl);
 unsigned char spi_state=SPI_TXCMD;
 unsigned char spi_flag= SPI_OK;
 unsigned char spi_last = cmd&SPI_DSIZE;
 unsigned char error_count=0;
 unsigned char tecla;
 char spi_dtx=cmd;

 while (spi_state!=SPI_IDLE)
   {
    delay_ms(1);
    clr_bit(SS_MD,P_wSPI); // Habilita el dialogo con el esclavo
    SPDR=spi_dtx;           // Carga el dato a enviar
    while(!(SPSR & (1<<SPIF)));	// Espero hasta que se complete la Tx.
    set_bit(SS_MD,P_wSPI);

    switch (spi_state)
       {
        case SPI_TXCMD: // Recibio ack basura -> descarta
           spi_dtx = SPDR; // Lectura obligada para escribir
           spi_dtx = d_tx.buf[(spi_state-1)]; // Prepara el siguiente dato
           spi_state++;
        break;
        case SPI_TXEND:
           if ((SPDR==d_tx.buf[spi_last])&&(spi_flag==SPI_OK)) // El ACK recibido es igual al ultimo dato mandado?
              spi_state=SPI_IDLE;
           else
             { // Debe retransmitir el paquete
              error_count=error_count+1;
              if (error_count==SPI_CERR)
                {
                 lcd_clr();
                 lcd_string_P(PSTR("\nError TX\n"));
                 lcd_string_P(PSTR("Verifique el cable al esclavo\n Luego ENTER"));
                 do
                   {
                    READ_KEY(tecla,spi_flag);
                   }
                 while (tecla!=BUT_ENTER);
                }
              spi_state=SPI_TXCMD;
              spi_dtx = d_tx.cmd;
              spi_flag=SPI_OK;
             }
        break;
        default: //(SPI_TXD[0-5])
           if (spi_state==SPI_TXD0) // Recibio ACK Comando
             {
              if (SPDR!=cmd)
                {
                 spi_flag=SPI_ERROR;
                }
             }
           else
             { // Debe verificar los datos
              if (SPDR!=d_tx.buf[(spi_state-3)]) // Revisa el anterior si llego bien
                {
                 spi_flag=SPI_ERROR;
                }
             }
           if (spi_state==(spi_last+2))
             {
              spi_dtx=spi_flag; // Fin de comando
              spi_state=SPI_TXEND;
             }
           else
             {
              spi_dtx = d_tx.buf[(spi_state-1)]; // Prepara el siguiente dato
              spi_state++;
             }
        break;
       }
   }
}



