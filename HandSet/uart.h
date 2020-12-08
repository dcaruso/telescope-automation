// Header de uart.c
#include "config.h"

//Set the baud rate of the communication
//#define BAUD 9600
//#define MYUBRR F_CPU/16/BAUD-1
#define BAUD 9600UL
#define UBRRVAL (F_CPU/(BAUD*16)-1)

#define UART_SIZE	6
#define MAX_COM		2

#define COM_ERR		0
#define COM_IDLE	1		// Espera ':'
#define COM_PAR0	2		// Recive el primer parametro
#define COM_PAR1	3		// Recive el segundo parametro, se ahi en adelante ya no importa
#define COM_REST	4		// Resto de la info

// Lista de comandos
#define NO_CMDPC	0
#define GET_RA		('G'+'R')
#define GET_DEC		('G'+'D')

#define GOTO_RA		('S'+'r')
#define GOTO_DEC	('S'+'d')
#define MOV_START	('M'+'S')
#define STOP		('Q')

 // Modos de respuesta del HandSet a la PC
#define AUTO		0
#define MANUAL		1

// Tipo de variable para comunicacion con PC
typedef struct
{
 unsigned char cmd;     // Comando
 signed char sign;
 unsigned char data[UART_SIZE]; // Buffer de datos
}uart_data;



//void uart_printchar(char, FILE *);
void uart_init (void);
void uart_send(unsigned char, char *,...);
void uart_TxByte(char) ;
void autoresp_init(void);

extern volatile unsigned char com_state;
extern volatile uart_data pc[MAX_COM];
extern volatile unsigned char ind_pcread,ind_pcwrite, pc_overwr;
extern volatile unsigned char respondPC;


