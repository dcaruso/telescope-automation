#include "config.h"
#include "util.h"

#define SPI_CERR	3
#define SPI_SIZE	6
#define SPI_SRBIT	0x80
#define SPI_DSIZE	0x07
// SPI States
#define SPI_IDLE     0
					   //    Slave			Master
#define SPI_TXCMD    1 // Tx Comando (Inicio Tx)
   #define SPI_TXD0  2 // Envio Dato 1 - Recibe ACK Comando
   #define SPI_TXD1  3 // Envio Dato 2 - Recibe ACK Dato 1
   #define SPI_TXD2  4 // Envio Dato 3 - Recibe ACK Dato 2
   #define SPI_TXD3  5 // Envio Dato 4 - Recibe ACK Dato 3
   #define SPI_TXD4  6 // Envio Dato 5 - Recibe ACK Dato 4
   #define SPI_TXD5  7 // Envio Dato 6 - Recibe ACK Dato 5
#define SPI_TXEND    8 // Envio Comando Fin - Recibe ACK Dato 6

#define SPI_RXCMD    1    // Rx Comando (Inicio Rx)
   #define SPI_RXD0  2 // Repite Comando - Recibe Dato 1
   #define SPI_RXD1  3 // Repite Dato 1 - Recibe Dato 2
   #define SPI_RXD2  4 // Repite Dato 2 - Recibe Dato 3
   #define SPI_RXD3  5 // Repite Dato 3 - Recibe Dato 4
   #define SPI_RXD4  6 // Repite Dato 4 - Recibe Dato 5
   #define SPI_RXD5  7 // Repite Dato 5 - Recibe Dato 6
#define SPI_RXEND0   8 // Repite Dato 6 - Fin de Comando 1 (Valida hasta Dato 5)
#define SPI_RXEND1   9 // Fin de Comando 1 - Fin de Comando 1 (Valida Dato 6)

#define SPI_ERROR    0xFF // Cuando se recibe otra cosa que no se esperaba
#define SPI_OK       0xEE  // Cuando esta todo OK!


typedef struct  // Tipo de variable para comunicacion SPI
{
 unsigned char cmd;     // Comando
// unsigned char buf[SPI_SIZE]; // Buffer de datos
 char buf[SPI_SIZE]; // Buffer de datos
}spi_data;

extern volatile spi_data sdata;

// Estructura del Comando
//    d7    d6    d5    d4    d3    d2    d1    d0
//   S/R    A2    A1    A0    C0    S2    S1    S0

// S/R = Set / Read
// A[2-0] = Direccion del comando
// C[0] = Parametros variantes del comando
// S[2-0] = Tamaño de la trama de datos del comando - 1

/********************************/
/* Comando Movimiento del motor */
/********************************/
// Uso: Antes de poner en estacion el equipo, permite mover el motor de a pasos
#define SMOVMOT      0x91
#define DIRNEG       0x01
#define DEC          0x01
#define AR           0x00
#define DIRPOS       0x00
#define MOTRUN       0x02
#define MOTSTOP      0x00
//      Comando    1 0 0 1  0 0 1 0 (D= Direccion [1=-, 0=+], R= Run/Stop)
//        Respuesta Esclavo => ACK (Basura)
//      MOV AR     => [1 Byte] (0000 00RD)
//        Respuesta Esclavo => ACK (al comando)
//      MOV DEC     => [1 Byte] (0000 00RD)
//        Respuesta Esclavo => ACK (al AR)
//      Fin del comando (FF=OK, EE=Ret) (Esto no seria de mucha utilidad, porque puede retransmitir)
//        Respuesta Esclavo => ACK (al DEC)

/****************************/
/* Setear Posición (AR/DEC) */
/****************************/
// Uso: En el final de la puesta en estacion. Establece la posicion de origen.
#define SPOSORG      0xB5
#define SPOSTEL      0xBD
//      Comando              1 0 1 1  T 1 0 1 (T=0: Origen; T=1: No es Origen)
//        Respuesta Esclavo => ACK (Basura)
//      Coordenada AR h => [1 Byte]
//        Respuesta Esclavo => ACK (al comando)
//      Coordenada AR ' => [1 Byte]
//        Respuesta Esclavo => ACK (a AR h)
//      Coordenada AR " => [1 Byte]
//        Respuesta Esclavo => ACK (a AR ')
//      Coordenada DEC º => [1 Byte]
//        Respuesta Esclavo => ACK (al AR ")
//      Coordenada DEC ' => [1 Byte]
//        Respuesta Esclavo => ACK (al DEC º)
//      Coordenada DEC " => [1 Byte]
//        Respuesta Esclavo => ACK (al DEC ')
//      Fin del comando (FF=OK, EE=Ret) (Esto no seria de mucha utilidad, porque puede retransmitir)
//        Respuesta Esclavo => ACK (al DEC ")

/**********************/
/* Ir a la coordenada */
/**********************/
// Uso: Ya puesto en estacion, sirve para situar al telescopio en alguna coordenada
#define SGOTO        0xC5
#define SEGON        0x08
#define SGOTO_SE     0xCD
#define SEGOFF       0x00
//      Comando              1 1 0 0  S 1 0 1 (S=seguimiento [1=ON, 2=OFF])
//        Respuesta Esclavo => ACK (Basura)
//      Coordenada AR h => [1 Byte]
//        Respuesta Esclavo => ACK (al comando)
//      Coordenada AR ' => [1 Byte]
//        Respuesta Esclavo => ACK (a AR h)
//      Coordenada AR " => [1 Byte]
//        Respuesta Esclavo => ACK (a AR ')
//      Coordenada DEC º => [1 Byte]
//        Respuesta Esclavo => ACK (al AR ")
//      Coordenada DEC ' => [1 Byte]
//        Respuesta Esclavo => ACK (al DEC º)
//      Coordenada DEC " => [1 Byte]
//        Respuesta Esclavo => ACK (al DEC ')
//      Fin del comando (FF=OK, EE=Ret) (Esto no seria de mucha utilidad, porque puede retransmitir)
//        Respuesta Esclavo => ACK (al DEC ")


//------------------------ Comandos de Lectura al esclavo

/*********************************/
/* Comando Leer estado del motor */
/*********************************/
// Uso: Por si el 128 necesita saber si se detuvo el motor.
#define RMOTST       0x11
//      Comando              0 0 0 1  0 0 0 1
//        Respuesta Esclavo => ACK (Basura)
//      Comando nuevamente del maestro (solo para forzar el envio de datos del esclavo)
//          Manda pasos AR [1 Byte] (0=stop, !=0 run)
//      Respuesta Master => ACK (Devuelve pasos AR)
//          Manda pasos DEC [1 Byte] (0=stop, !=0 run)
//      Respuesta Master => ACK (Devuelve pasos DEC)
//          Fin del comando (AA=OK, BB=Ret) (enviado por el esclavo) Confirma hasta Pasos H
//      Pide la ultima confirmacion => ACK (pasos DEC)
//          Fin del comando (AA=OK, BB=Ret) (enviado por el esclavo) Confirma Pasos L
//    ***  Si esta todo OK termina la comunicacion

/*******************/
/* Leer coordenada */
/*******************/
// Uso: Para saber en que coordenada ecuatorial esta situado el telescopio
#define RPOS          0x25
//      Comando              0 0 1 0  0 1 0 1
//        Respuesta Esclavo => ACK (Basura)
//      Comando nuevamente del maestro (solo para forzar el envio de datos del esclavo)
//          Manda AR H [1 Byte]
//      Respuesta Master => ACK (Devuelve AR H)
//          Manda AR M [1 Byte]
//      Respuesta Master => ACK (Devuelve AR M)
//          Manda AR S [1 Byte]
//      Respuesta Master => ACK (Devuelve AR S)
//          Manda DEC D [1 Byte]
//      Respuesta Master => ACK (Devuelve DEC D)
//          Manda DEC M [1 Byte]
//      Respuesta Master => ACK (Devuelve DEC M)
//          Manda DEC S [1 Byte]
//      Respuesta Master => ACK (Devuelve DEC S)
//          Fin del comando (AA=OK, BB=Ret) (enviado por el esclavo) Confirma hasta DEC M
//      Pide la ultima confirmacion => ACK (DEC S)
//          Fin del comando (AA=OK, BB=Ret) (enviado por el esclavo) Confirma DEC S
//    ***  Si esta todo OK termina la comunicacion

void spi_send (unsigned char size, unsigned char cmd, ...);
spi_data spi_read(char);
void spi_init(void);

