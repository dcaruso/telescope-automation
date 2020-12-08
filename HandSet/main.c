/***************************************************************************/
/* Descripcion:                                                            *
/*  Programa principal, desde aca se llaman a todas las distintas rutinas, *
/*  segun lo que se quiera hacer. La funcion principal (main), debe ser de *
/*  caracter funcional y de facil lectura. Es decir, que solo debe         *
/*  contener, en el mejor caso, llamados a funciones y decisiones simples. *
/*  Aca se intenta implementar un prototipo del PAD. Simplemente se        *
/*  inicializan todos los perifericos y luego se espera a que se pulse una *
/*  tecla, para realizar una accion. Se pueden mostrar distintos carteles  *
/*  segun la tecla apretada y el motor debera hacer distintos movimientos. *
/***************************************************************************/

#include "config.h"

#include "images.h"
#include "lcd.h"
#include "font.h"
#include "teclado.h"
#include "util.h"
#include "menu.h"
#include "rtc.h"
#include "i2c.h"
#include "spi_master.h"
// SD
#include "sd_raw.h"
// PC
#include "uart.h"
#include "anim.h"


//----------------
//* Var globales *
//----------------

date fecha;
time t;
c_geoi lat;
c_geoi lon;
obj_ceqs objb; // Coordenadas y nombre del objeto buscado o a buscar
int hmar;
volatile unsigned char park_flag;
c_eqs tpos; // Posiciones actuales del telescopio (inicio AR=0h 0m, DEC=+-90º 0m)

int main(void)
{
 unsigned char tecla, dtecla,n;
 signed char aux;
 const obj_ceqs *list;
 clr_bit(SRE,MCUCR); // Deshabilita External Memory Interface (Libera el PORTA)
 lcd_init();
 uart_init();
 spi_init();
 sound_init();
 key_init();
 sei();
 lcd_setcursor(3,0);
 lcd_string_P(PSTR("Bienvenido!"));
 lcd_setcursor(5,3);
 lcd_string_P(PSTR("    Pulse ENTER"));
 inv_linea(3,64,CursorX+1);
 lcd_string_P(PSTR("\n    para comenzar"));

 do
	{
	 READ_KEY(tecla,dtecla);
	}
 while (tecla!=BUT_ENTER); // Esto prende el equipo
 lcd_clr();

// Inicio de la presentacion.
 start_anim(42,0,70,ANIM_UTN);
 sound_ok();
 delay_ms(1000);

 // Prueba del backligth
 set_bit(LCD_BKL,LCD_wBKL);
 lcd_setcursor(0,0);
 lcd_bmp(&p00000_Advertencia[0]); //Warning, no apunte al SOL!
 delay_ms(4000);
 clr_bit(LCD_BKL,LCD_wBKL);

// Verifica la existencia de la SD
 if (!sd_raw_init())
	{
	 lcd_clr();
	 lcd_string_P(PSTR("\nMemoria SD\n No encontrada\n"));
	 delay_ms(2000);
	}

 rtc_init();
 rtc_read();
 menu_print(PG_PRINC);
 rtc_rdbkp(); // Actualiza las variables según el rtc
 sound_ok();
 while(1)
	{
	 switch (menu_pg)
		{
//- Pantalla Principal
		 case PG_PRINC: // 0.0.0.0
			menu_navpg(OPT_PRINC,NAV_PG);
		 break;
//- Pantalla de Busqueda General
		 case PG_BUSQ: // 1.0.0.0
			menu_navpg(OPT_BUSQ,NAV_PG);
		 break;
//- Pantalla de Busqueda Catalogos Estelares
		 case PG_B_CE:
			menu_navpg(3,NAV_PG);
		 break;
//- Pantalla de Busqueda Otros Catalogos
		 case PG_B_OC: // Falta levantar de la memoria otros catalogos
			menu_navpg(4,NAV_PG);
		 break;
//- Pantallas de Busquedas de Elementos Varios
		 case PG_B_SSOL: case PG_B_IC: case PG_B_NGC: case PG_B_MES:
			n = menu_namein(objb.name);
			if (n!=DAT_NVA && n<10)
				{
				 lcd_setcursor(STR_CUR+2,2);
				 lcd_string_P(PSTR("Loading"));
				 start_anim(STR_CUR+44,2,350,ANIM_LOADING2);
				 objb=find_obj(objb.name,n);
				 stop_anim();
				 if (vis_obj(objb.ra,objb.dec)!=OBJ_VIS)
					{ERROR_TXT("No visible",0,STR_CUR);}
				 else
					{
					 if (objb.name[0]=='0')
						{ ERROR_TXT("Invalido",0,STR_CUR);}
					 else
						menu_pg= (menu_pg<<3) + 1; // Avanza de pagina
					}
				}
			else
				{
				 if (n==DAT_NVA)
					 {ERROR_TXT("Invalido",0,STR_CUR);}
				 if (n==BUT_ESC)
					menu_pg= (menu_pg>>3); // Vuelve atras
				}
			menu_print(menu_pg);
		 break;
//- Pantalla Elementos encontrados, espera validación
		 case PG_B_MESE: case PG_B_ICE: case PG_B_SSOLE: case PG_B_NGCE: case PG_B_OC1E: case PG_B_OC2E: case PG_B_OC3E: case PG_B_OC4E:
			menu_navpg(0,NAV_PG);
		 break;
//- Pantalla Elementos en proceso de busqueda
		 case PG_B_MESB: case PG_B_ICB: case PG_B_SSOLB: case PG_B_NGCB: case PG_B_OC1B: case PG_B_OC2B: case PG_B_OC3B: case PG_B_OC4B: case PG_U_MODMB:// case PG_TOURB:

			menu_goto(objb,TRACK_ON, NO_RECALC);
			menu_print((menu_pg>>3));
			codKey=0;
		 break;

//- Pantalla de Configuración
		 case PG_CONF: // 3.0.0.0
		 	menu_navpg(OPT_CONF,NAV_PG);
			rtc_wrbkp();
		 break;
		case PG_C_TIM: // 3.2.0.0
			 if (menu_navpg(OPT_CTIM,CONF_PG)==1) // Pregunta si pulso enter
				{
				 lcd_setcursor(STR_CUR+2,OPT2LINE(menu_opt));
				 if (menu_opt==1) // Fecha
					{ // Carga los valores a los auxiliares, para cuando salga
					 menu_aux[2] = fecha.y;menu_aux[1]=fecha.m;menu_aux[0]=fecha.d;
					 if((menu_datain(6,"// "))==DAT_VA) // (cant datos, separador, )
						{
						 if ((menu_aux[2]<100)&&(menu_aux[1]<13)&&(menu_aux[0]<31))
							{
							 fecha.y=menu_aux[2];fecha.m=menu_aux[1];fecha.d=menu_aux[0];
							 rtc_writeDate(menu_aux[0],menu_aux[1],menu_aux[2]);
							}
						 else
							{ERROR_TXT("Invalido",0,STR_CUR);}
						}
					}
				 else // Hora
					{ // Carga los valores a los auxiliares, para cuando salga
					 menu_aux[0]=t.h;menu_aux[1]=t.m;
					 if((menu_datain(4,": "))==DAT_VA) // (cant datos, separador, )
						{
						 if ((menu_aux[0]<24)&&(menu_aux[1]<60))
							{
							 t.h=menu_aux[0];t.m=menu_aux[1]; // CUANDO FUNCIONE EL RTC, sacar esta linea
							 rtc_writeTime(0,menu_aux[1],menu_aux[0]);
							 lcd_putchar(' ');
							 if (t.hh>0)
								 menu_aux[2]= 1-((menu_sfdat("+-",RNUM,2))<<1); // Lo usa de backUP
							 else
								 menu_aux[2]= ((menu_sfdat("-+",RNUM,2))<<1)-1; // Lo usa de backUP
							 if((menu_datain(2," "))==DAT_VA) // (cant datos, separador, )
								{
								 if (menu_aux[0]<13)
									 t.hh=menu_aux[0]*menu_aux[2];
								 else
									 {ERROR_TXT("Invalido",1,STR_CUR);}
								}
							}
						 else
							{ERROR_TXT("Invalido",1,STR_CUR);}
						}
					}
				 menu_print(menu_pg);// Reimprime la pantalla para borrar
				}
		 break;
		 case PG_C_PGE:
			 if (menu_navpg(OPT_CPGE,CONF_PG)==1) // Pregunta si pulso enter
				{
				 lcd_setcursor(STR_CUR+2,OPT2LINE(menu_opt));
				 if (menu_opt==1) // Latitud
					{
					 menu_aux[0] = lat.d;menu_aux[1]=lat.m;
					 if((menu_datain(4,&sym_dec[0]))==DAT_VA) // 128=º
						{
						 if ((menu_aux[0]<90)&&(menu_aux[1]<60))
							{
							 lcd_string_P(PSTR(" "));
							 lat.d=menu_aux[0];lat.m=menu_aux[1];
							 if (lat.s=='N')
								 lat.s= menu_sfdat("NS",RCHAR,2);
							 else
								 lat.s= menu_sfdat("SN",RCHAR,2);
							}
						 else
							{ERROR_TXT("Invalido",0,STR_CUR);}
						}
					}
				 else
					{
					 menu_aux[0] = lon.d;menu_aux[1]=lon.m;
					 while(1)
						{
						 READ_KEY(tecla,dtecla);
						 if (dtecla<2)
							{
							 menu_aux[2]=dtecla*100;
							 lcd_putchar(dtecla+'0');
							 break;
							}
						 if ((tecla!=BUT_ENTER)&&(tecla!=BUT_ESC))
							break;
						}
					 if ((tecla!=BUT_ENTER)&&(tecla!=BUT_ESC))
						{
						 if((menu_datain(4,&sym_dec[0]))==DAT_VA) // (cant datos, separador, )
							{
							 if (((menu_aux[0]+menu_aux[2])<180)&&(menu_aux[1]<59))
								{
								 lcd_string_P(PSTR(" "));
								 lon.d=menu_aux[0]+menu_aux[2];lon.m=menu_aux[1];
								 if (lon.s=='O')
									 lon.s= menu_sfdat("OE",RCHAR,2);
								 else
								 lon.s= menu_sfdat("EO",RCHAR,2);
								}
							 else
								{ERROR_TXT("Invalido",1,STR_CUR);}
							}
						}
					}
				 menu_print(menu_pg);
				}
		break;
		case PG_C_ALM: // Es una presentacion a la problematica
			menu_navpg(0,NAV_PG);
		break;
		case PG_C_ALM1: case PG_C_ALM2: case PG_C_ALM3:
			READ_KEY(tecla,dtecla);
			switch (tecla)
				{
				 case BUT_ENTER:
					if (menu_pg==PG_C_ALM3)
						{
						// Termina la alineacion manual, establece la posicion de inicio
						 tpos = initial_pos();
						 spi_send(6,SPOSORG,tpos.ra.h,tpos.ra.m,tpos.ra.s,tpos.dec.d,tpos.dec.m,tpos.dec.s);
						}
					menu_pg++;
				 break;
				 case BUT_ESC:
					if (menu_pg==PG_C_ALM1)
						menu_pg=PG_C_ALM;
					else
						menu_pg--;
				 break;
				}
			menu_print(menu_pg);
		break;

		case PG_C_ALE1:
			list =&list_star[0];
			if ((n=menu_sfelm(list,NUMSTAR))!=NUMSTAR)
				 menu_print(PG_C_ALE2);
			else
				menu_print(PG_CONF);
		break;
		case PG_C_ALE2:
			 if (menu_goto(objb,TRACK_OFF,NO_RECALC)==FOUND)
				 menu_print(PG_C_ALE3);
			 else
				 menu_print(PG_C_ALE1);
			n=0;
		break;

		case PG_C_ALE3:
			 lcd_setcursor(STR_CUR+2,1);
			 READ_KEY(tecla,dtecla);
			 menu_print(menu_pg);
			 if (n==tecla) // Si repite la tecla, cancela el movimiento
				{
				 spi_send(2,SMOVMOT,MOTSTOP,MOTSTOP);
				 n=0;
				}
			 else
				{
				 switch (tecla)
					{
					 case BUT_UP:
						 spi_send(2,SMOVMOT,MOTSTOP,MOTRUN|DIRPOS);
						 inv_linea(1,87,93);
					 break;
					 case BUT_DW:
						 spi_send(2,SMOVMOT,MOTSTOP,MOTRUN|DIRNEG);
						 inv_linea(3,87,93);
					 break;
					 case BUT_LT:
						 spi_send(2,SMOVMOT,MOTRUN|DIRNEG,MOTSTOP);
						 inv_linea(2,79,85);
					 break;
					 case BUT_RT:
						 spi_send(2,SMOVMOT,MOTRUN|DIRPOS,MOTSTOP);
						 inv_linea(2,95,101);
					 break;
					 case BUT_ENTER:
						 menu_print(PG_C_ALE4);
						 spi_send(2,SMOVMOT,MOTSTOP,MOTSTOP);
					 break;
					 case BUT_ESC:
						 menu_print(PG_C_ALE1);
						 spi_send(2,SMOVMOT,MOTSTOP,MOTSTOP);
					 break;
					 default:
						 spi_send(2,SMOVMOT,MOTSTOP,MOTSTOP);
					 break;
					}
				 n=tecla;
				}
		break;
		case PG_C_ALE4:
			READ_KEY(tecla,dtecla);
			switch (tecla)
				{
				 case BUT_ENTER:
					 menu_print(PG_C_ALE1);
				 break;
				 case BUT_ESC:
					menu_print(PG_CONF);
				 break;
				}
		break;


		case PG_UTIL:
			 sdata=spi_read(RPOS);
			 tpos.ra.h=sdata.buf[0];
			 tpos.ra.m=sdata.buf[1];
			 tpos.ra.s=sdata.buf[2];
			 tpos.dec.d=sdata.buf[3];
			 tpos.dec.m=sdata.buf[4];
			 tpos.dec.s=sdata.buf[5];
			 objb.ra=tpos.ra;
			 objb.dec=tpos.dec; // Lo hace por que es necesario para MOD MAN
			 menu_navpg(3,NAV_PG);
		break;
		case PG_U_MODM:// Aun no va a ninguna posicion
			 dtecla=menu_navpg(3,CONF_PG); // La usa de back-up
			 if (dtecla==1) // Pregunta si pulso enter
				{
				 switch (menu_opt)
					{
					 case 1:  // Carga AR
						 lcd_setcursor(STR_CUR+8,OPT2LINE(menu_opt));
						 if((menu_datain(6,&sym_ra[0]))==DAT_VA) // (cant datos, separador, )
							{objb.ra.h=menu_aux[0];objb.ra.m=menu_aux[1];objb.ra.s=menu_aux[2];}
					 break;
					 case 2: // Carga la DEC
						 lcd_setcursor(STR_CUR+2,OPT2LINE(menu_opt));
						 if ((objb.dec.d<0)||(objb.dec.m<0)||(objb.dec.s<0))
							 aux= (menu_sfdat("-+",RNUM,2)<<1)-1; // Lo usa de backUP
						 else
							 aux= 1-((menu_sfdat("+-",RNUM,2))<<1); // Lo usa de backUP
						 if((menu_datain(6,&sym_dec[0]))==DAT_VA) // (cant datos, separador, )
							{objb.dec.s=menu_aux[2]*aux;objb.dec.m=menu_aux[1]*aux;objb.dec.d=menu_aux[0]*aux;}
					 break;
					 case 3: // Ir a la coordenada
						 if (vis_obj(objb.ra,objb.dec)!=OBJ_VIS)
							{ERROR_TXT("No visible",0,STR_CUR);}
						 else
							{
							 strcpy_Pe(objb.name,PSTR("Desconocido\0"));
							 menu_pg=PG_U_MODMB;
							}
					 break;
					}
				 menu_print(menu_pg);
				}
		break;
		case PG_U_PIN:
			tpos=initial_pos();
			spi_send(6,SGOTO_SE,tpos.ra.h,tpos.ra.m,tpos.ra.s,tpos.dec.d,tpos.dec.m,tpos.dec.s);
			do
				{
				 delay_ms(100);
				 sdata=spi_read(RMOTST);
				 tpos.ra.h=sdata.buf[0];
				 tpos.ra.m=sdata.buf[1];
				 tpos.ra.s=sdata.buf[2];
				 tpos.dec.d=sdata.buf[3];
				 tpos.dec.m=sdata.buf[4];
				 tpos.dec.s=sdata.buf[5];
				}
			while ((codKey!=BUT_ESC)&&((sdata.buf[0]!=0)||(sdata.buf[1]!=0)));
			stop_anim();
			if (codKey!=BUT_ESC)
				{
				 park_flag=PARKING;
				 rtc_wrbkp();
				 menu_print(PG_U_PINB);
				}
			else
				{
				 spi_send(2,SMOVMOT,MOTSTOP,MOTSTOP); // Con ESCAPE detiene al motor y vuelve a la pagina anterior
				 sound_error();
				 menu_print(PG_UTIL);
				}
			codKey=0;
		break;
		case PG_U_PINB:
			sound_ok();
			delay_ms(1000);
			menu_print(PG_UTIL);
		break;
		case PG_U_PC:
			aux= 0;
			n=0;
			respondPC=MANUAL;
			while (codKey!=BUT_ESC)
				{
				 if (n!=0)
					{
					 sdata=spi_read(RMOTST);
					 if ((sdata.buf[0]==0) && (sdata.buf[1]==0))
						{
						 sound_ok();
						 stop_anim();
						 lcd_setcursor(0,2);
						 lcd_string_P(PSTR("Siguiendo    "));
						 inv_linea(2,0,CursorX+1);
						 n=0;
						}
					}
				 if ((ind_pcread!=ind_pcwrite)&&(com_state==COM_IDLE))
					{
					 ind_pcread++;
					 if (ind_pcread==MAX_COM)
						ind_pcread=0;
					 switch (pc[ind_pcread].cmd)
						{
						 case GET_RA:
							 sdata=spi_read(RPOS);
							 tpos.ra.h=sdata.buf[0];
							 tpos.ra.m=sdata.buf[1];
							 tpos.ra.s=sdata.buf[2];
							 tpos.dec.d=sdata.buf[3];
							 tpos.dec.m=sdata.buf[4];
							 tpos.dec.s=sdata.buf[5];
							 uart_send(3,"::#",tpos.ra.h,tpos.ra.m,tpos.ra.s);
							 if (aux==0)
								 menu_print(PG_U_LPC);
							 aux=1;
						 break;
						 case GET_DEC:
							 sdata=spi_read(RPOS);
							 tpos.ra.h=sdata.buf[0];
							 tpos.ra.m=sdata.buf[1];
							 tpos.ra.s=sdata.buf[2];
							 tpos.dec.d=sdata.buf[3];
							 tpos.dec.m=sdata.buf[4];
							 tpos.dec.s=sdata.buf[5];
							 if ((tpos.dec.d<0)||(tpos.dec.m<0)||(tpos.dec.s<0))
								 uart_TxByte('-');
							 else
								 uart_TxByte('+');
							 uart_send(3,&sym_decPC[0],abs(tpos.dec.d),abs(tpos.dec.m),abs(tpos.dec.s));
						 break;
						 case GOTO_RA:
							objb.ra.h=pc[ind_pcread].data[0];
							objb.ra.m=pc[ind_pcread].data[1];
							objb.ra.s=pc[ind_pcread].data[2];
							uart_TxByte('1'); // Retorna valido
						 break;
						 case GOTO_DEC:
							objb.dec.d=pc[ind_pcread].sign*pc[ind_pcread].data[0];
							objb.dec.m=pc[ind_pcread].sign*pc[ind_pcread].data[1];
							objb.dec.s=pc[ind_pcread].sign*pc[ind_pcread].data[2];
							uart_TxByte('1'); // Retorna valido
						 break;
						 case MOV_START:
							menu_print(PG_U_LPC);
							if (vis_obj(objb.ra,objb.dec)==OBJ_VIS)
								{
								 uart_TxByte('0'); // Dice que es válido el movimiento
								 park_flag=NO_PARK;
								 spi_send(6,(SGOTO_SE),objb.ra.h,objb.ra.m,objb.ra.s,objb.dec.d,objb.dec.m,objb.dec.s);
								 lcd_string_P(PSTR("Posicionando"));
								 start_anim(74,2,300,ANIM_LOADING1);
								 lcd_string_P(PSTR("\nRA: "));
								 menu_dataout(3,PRINT_NSIGN,&sym_ra[0],objb.ra.h,objb.ra.m,objb.ra.s);
								 lcd_string_P(PSTR("\nDE:"));
								 menu_dataout(3,PRINT_WSIGN,&sym_dec[0],objb.dec.d,objb.dec.m,objb.dec.s);
								 sound_ok();
								 n=1;
								}
							else
								{
								 uart_TxByte('1'); // No es válido el movimiento
								 ERROR_TXT("No visible",2,0)
								}
						 break;
						 case STOP:
							spi_send(2,SMOVMOT,MOTSTOP,MOTSTOP);
							menu_print(PG_U_LPC);
							lcd_string_P(PSTR("Telescopio detenido"));
						 break;
						}
					 pc[ind_pcread].cmd=NO_CMDPC;
					}
				}
			 codKey=0;
			 respondPC=AUTO;
			 menu_print(PG_UTIL);
		break;
		}
	}
 return 0;
}


