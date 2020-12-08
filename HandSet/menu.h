#include "config.h"
#include "stdarg.h"
#include "calc.h"

#define END_CUR		127 // cantidad de columnas a invertir
#define STR_CUR		57

#define FOUND		1
#define NOTFOUND	0

#define TRACK_ON	1
#define TRACK_OFF	0
#define RECALC		1 // Con recalculo de la posicion
#define NO_RECALC	0 // Sin recalculo de la posicion

#define T_REPKEY	2000 // Mili-segundos de tiempo muerto para repetir tecla

#define DAT_VA		1 // Flag = Dato ingresado valido
#define DAT_NVA		0 // Flag = Dato ingresado no valido
#define DAT_CANC	11 // Flag = Dato ingresado cancelado

#define NAV_PG		1 // Tipo de pagina = Navegacion
#define CONF_PG		0 // Tipo de pagina = Configuracion
#define NAVSB_PG	2 // Tipo de pagina = Navegacion de sub-opciones

#define ERROR_TXT(txt,line,col)		lcd_setcursor(col,line);lcd_string_P(PSTR(txt));sound_error();delay_ms(1000);

#define OPT2LINE(opt)	(opt<<1)-2 // Traduce el valor de la opcion a la linea (va de 2 en 2))

// Paginas del menu
#define PG_PRINC	0b0000000000000000 // Menu Principal				0.0.0.0

#define PG_BUSQ		0b0000000000000001 // 	Busqueda:					1.0.0.0 
	#define PG_B_SSOL	0b0000000000001001 // 	Sistema Solar:				1.1.0.0
		#define PG_B_SSOLE	0b0000000001001001 // 		Objeto encontrado		1.1.1.0
			#define PG_B_SSOLB	0b0000001001001001 // 		Objeto encontrado		1.1.1.0
	#define PG_B_CE		0b0000000000001010
		#define PG_B_MES	0b0000000001010001 // 	Messier						1.2.0.0
			#define PG_B_MESE	0b0000001010001001 // 		Objeto encontrado		1.2.1.0
				#define PG_B_MESB	0b0001010001001001 // 		Objeto encontrado		1.2.1.0

		#define PG_B_IC		0b0000000001010010 // 	IC						
			#define PG_B_ICE	0b0000001010010001 // 		Objeto encontrado		1.3.1.0
				#define PG_B_ICB	0b0001010010001001 // 		Objeto encontrado		1.3.1.0
	
		#define PG_B_NGC	0b0000000001010011 // 	NGC							1.4.0.0
			#define PG_B_NGCE	0b0000001010011001 // 		Objeto encontrado		1.4.1.0
				#define PG_B_NGCB	0b0001010011001001 // 		Objeto encontrado		1.4.1.0

	#define PG_B_OC		0b0000000000001011
		#define PG_B_OC1	0b0000000001011001
			#define PG_B_OC1E	0b0000001011001001
				#define PG_B_OC1B	0b0001011001001001
		#define PG_B_OC2	0b0000000001011010
			#define PG_B_OC2E	0b00000010110100001
				#define PG_B_OC2B	0b0001011010001001

		#define PG_B_OC3	0b0000000001011011
			#define PG_B_OC3E	0b00000010110110001
				#define PG_B_OC3B	0b0001011011001001

		#define PG_B_OC4	0b0000000001011100
			#define PG_B_OC4E	0b00000010111000001
				#define PG_B_OC4B	0b0001011100001001

#define PG_CONF		0b0000000000000010 // 	Configuracion				3.0.0.0
	#define PG_C_PGE	0b0000000000010001 // 		Coordenadas geograficas	3.1.0.0
	#define PG_C_TIM	0b0000000000010010 // 		Fecha y Hora			3.2.0.0 
	#define PG_C_ALM		0b0000000000010011 // 		Alineacion				3.3.0.0
		#define PG_C_ALM1	0b0000000010011001 // 		Alineacion Manual		3.4.1.0
		#define PG_C_ALM2	0b0000000010011010 // 		Alineacion Manual		3.4.2.0
		#define PG_C_ALM3	0b0000000010011011 // 		Alineacion Manual		3.4.3.0

		#define PG_C_ALE1	0b0000000010011100 // 		Alineacion				3.3.1.0
		#define PG_C_ALE2	0b0000000010011101 // 		Alineacion				3.3.2.0
		#define PG_C_ALE3	0b0000000010011110 // 		Alineacion				3.3.2.0
		#define PG_C_ALE4	0b0000000010011111 // 		Alineacion				3.3.2.0

#define PG_C_STR	0b0000000010011011 // 3.3.3.0

#define PG_UTIL		0b0000000000000011
	#define PG_U_MODM		0b0000000000011001
		#define PG_U_MODMB		0b0000000011001001
	#define PG_U_PC			0b0000000000011010
		#define PG_U_LPC			0b0000000011010001 // Linkeado a la PC
	#define PG_U_PIN		0b0000000000011011
		#define PG_U_PINB		0b0000000011011001

// Cantidad total de opciones por pagina (COMPLETAR!!)
#define OPT_PRINC	3
#define OPT_CONF	3
#define OPT_BUSQ	3

#define OPT_CTIM	2
#define OPT_CPGE	2
#define OPT_CALI	3

#define RNUM		0
#define RCHAR		1

#define PRINT_WSIGN 1
#define PRINT_NSIGN 0


// Varibles globales visibles al sistema
extern volatile unsigned menu_pg; // Pagina del menu
extern volatile unsigned menu_opt;		// Opcion seleccionada (1-4)
extern volatile unsigned char menu_optchar; // Numero de caracter que el usuario carga
extern volatile signed char menu_aux[3]; // variable auxiliar para cargar los datos

// Prototipos de funciones
void inv_linea(unsigned char, unsigned char, unsigned char);
void mv_cursor(unsigned char, unsigned char);
void menu_print(unsigned);
unsigned char menu_navpg (unsigned char, unsigned char);
unsigned char menu_datain ( unsigned char, char *);
char menu_sfdat(char *,unsigned char, unsigned char);
void menu_dataout(unsigned char, unsigned char, char *,...);
unsigned char menu_namein(char *);
unsigned char menu_sfelm(const obj_ceqs *,unsigned char);
obj_ceqs find_obj(char *, unsigned char);
obj_ceqs find_obj_in_file(const char*, char *, unsigned char);
unsigned char menu_goto (obj_ceqs, unsigned char, unsigned char);
