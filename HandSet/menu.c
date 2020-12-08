/***************************************************************************/
/* Descripcion:                                                            *
/*  Contiene las funciones necesarias para definir un menu sobre un lcd    *
/*  grafico.                                                               *
/***************************************************************************/

#include "menu.h"
#include "lcd.h"
#include "teclado.h"
#include "images.h"
#include "font.h"
#include "util.h"
#include "rtc.h"
#include "spi_master.h"
#include "sd.h"
#include "anim.h"

volatile unsigned menu_pg; // Pagina del menu
volatile unsigned menu_opt;		// Opcion seleccionada (1-4)
volatile unsigned char menu_optchar; // Numero de caracter que el usuario carga
volatile signed char menu_aux[3];

/*-------------------------------------------------------------------------------*/
/* Invierte una linea                                                            */
/*-------------------------------------------------------------------------------*/
void inv_linea(unsigned char line, unsigned char ini_col, unsigned char end_col )
{
 unsigned char i;
 lcd_setcursor(ini_col,line);
 if (lcd_state==LCD_INV)
	{
	 for (i=ini_col; i<=end_col; i++)
		 lcd_wrdat((lcd_rdat()), 0); // Invierte una linea completa
	}
 else
	{
	 for (i=ini_col; i<=end_col; i++)
		 lcd_wrdat(~(lcd_rdat()), 0); // Invierte una linea completa
	}
}


/*-------------------------------------------------------------------------------*/
/* Navegacion dentro de un menu                                                  */
/*-------------------------------------------------------------------------------*/
unsigned char menu_navpg (unsigned char num_opt, unsigned char type_pg)
{// Se le pasa como parametro la tecla y el numero de paginas
 unsigned char tecla, dtecla;
 READ_KEY(tecla,dtecla);
 switch (tecla)
	{
	 case BUT_ENTER:
		if (type_pg==NAV_PG)
			menu_print(((menu_pg<<3) + menu_opt)); // Avanza de pagina
		return 1;
	 break;
	 case BUT_ESC:
		if (menu_pg!=PG_PRINC) // Ya que este no puede volverse hacia atras
			menu_print(menu_pg>>3);
		return 2;
	 break;
	 case BUT_DW: case BUT_UP:
		if (num_opt!=0)// Si no hay opciones, es pagina estatica
			mv_cursor(tecla, num_opt);
		return 3;
	 break;
	 default:
		return 0;
	 break;
	}
}

/*-------------------------------------------------------------------------------*/
/* Ingresa datos                                                                 */
/*-------------------------------------------------------------------------------*/
unsigned char menu_datain ( unsigned char cant_dat, char *sep)
{// Se le pasa como parametro la cantidad de datos (ingresados de a uno) y el separador
 unsigned char i, tecla, tecla_cod;
 unsigned char dec_aux;
 for (i=0;i<cant_dat;i++)
	{
	 READ_KEY(tecla,tecla_cod);
	 if (tecla_cod<10)
		{
		 lcd_putchar((tecla_cod+'0'));
		 if ((i&0x01)!=0) // Es igual que if(i%2!=0)
			{
			 menu_aux[(i/2)]=dec_aux+tecla_cod;
			 if (i!=(cant_dat-1))
				{
				 lcd_putchar(*sep);
				 sep++;
				}
			}
		 else
			 dec_aux=tecla_cod*10;
		}
	 else
		{
		 if (tecla_cod==10)
			return DAT_VA; // Enter Dato valido
		 else
			return DAT_NVA; // Cualquier otra es no valido.
		}
	}
 lcd_putchar(*sep);
 return DAT_VA; // Numero valido
}

/*-------------------------------------------------------------------------------*/
/* Imprime datos formateados en pantalla desde un vector                         */
/*-------------------------------------------------------------------------------*/
void menu_dataout ( unsigned char size, unsigned char sgn, char *sep,...)
{// Se le pasa como parametro la cantidad de datos (ingresados de a uno) y el separador
 // Los numeros son de dos cifras nada mas
 int i;
 int16_t val;
 unsigned char sign=0;
 va_list vl;
 va_start(vl,size);
 unsigned char p[10];
 for (i=0;i<size;i++)
		{
		 val=va_arg(vl, int16_t);
		 if (val<0)
			{
			 if ((sign==0)&&(sgn==PRINT_WSIGN))
				lcd_putchar('-');
			 p[i]=-val;
			 sign=1; // Solo el primero tiene signo
			}
		 else
			p[i]=val;
		}
	 if ((sign==0)&&(sgn==PRINT_WSIGN))
		lcd_putchar('+');
 for (i=0;i<size;i++)
	{
	 if ((p[i]<10) && (p[i]>=0))
		lcd_putchar('0');
	 int2str(p[i]);
	 if(i!=size-1)
		{
		 lcd_putchar(*sep);
		 sep++;
		}
	}
 lcd_putchar(*sep);
 va_end(vl);
}

/*-------------------------------------------------------------------------------*/
/* Ingresa un nombre                                                             */
/*-------------------------------------------------------------------------------*/
unsigned char menu_namein (char * name)
{// Se le pasa como parametro un puntero al vector donde estará el nombre
 unsigned char tecla_ant=0xFF, tecla_cod, tecla_rep=0, tecla;
 unsigned char i=0xFF;
 if (menu_pg==PG_B_SSOL)
	 delay_ms_ck(1500); // Ingresa con nombres completos
 else
	 delay_ms_ck(1);
 while (1)
	{
	 READ_KEY(tecla,tecla_cod);
	 if (tecla_cod<10)
		{
		 if ((tecla_ant!=tecla_cod)||(END_TDELAY))
			{
			 tecla_ant=tecla_cod;
			 tecla_rep=0;
			 lcd_setcursor(CursorX+6,CursorY);
			 i++;
			 if (i>9)
				return DAT_NVA;
			}
		 else
			{
			 tecla_rep++;
			 if (((pgm_read_byte(&keyCell[tecla_cod][tecla_rep]))==0)||(tecla_rep==5))
				tecla_rep=0;
			}
		 rst_delay_ms();
		 name[i]=pgm_read_byte(&keyCell[tecla_cod][tecla_rep]);
		 lcd_putchar(name[i]);
		 lcd_setcursor(CursorX-6,CursorY);
		}
	else
		{
		 switch (tecla)
			{
			 case BUT_ENTER:
				name[i+1]='\0';
			 	return (i+1);
			 break;
			 case BUT_ESC:
				if (i==0xFF)
					return (BUT_ESC);
			 	return (BUT_ESC+1); // Siempre es mayor a 9
			 break;
			}
		}
	}
}

/*-------------------------------------------------------------------------------*/
/* Shiftea un dato de un lista                                                   */
/*-------------------------------------------------------------------------------*/
char menu_sfdat (char * val, unsigned char rettype, unsigned char size)
{
 unsigned char tecla, dtecla;
 int i=0;
 do
	{
	 READ_KEY(tecla,dtecla);
	 switch(tecla)
		{
		 case BUT_UP:
			 i++;
			 if (i==size)
				i=0;
		 break;
		 case BUT_DW:
			i--;
			if (i<0)
				i=size-1;
		 break;
		}
	 lcd_putchar(val[i]); // Si aprieta otra cosa, valida lo que estaba
	 lcd_setcursor(CursorX-6,CursorY);
	}
 while ((tecla==BUT_DW)||(tecla==BUT_UP));
 lcd_setcursor(CursorX+6,CursorY);
 if (rettype==RNUM)
	 return i;
 else
	 return val[i];
}

/*-------------------------------------------------------------------------------*/
/* Shiftea un elementos de un lista                                              */
/*-------------------------------------------------------------------------------*/
unsigned char menu_sfelm (const obj_ceqs *list, unsigned char size)
{
 unsigned char tecla, dtecla;
 signed char dir=1;;
 signed char i=0;
 while (1)
	{
	 do
		{
		 i=dir+i;
		 if (i==size)
			i=0;
		 if (i<0)
			i=size-1;
		 objb.ra.h = pgm_read_byte(&list[i].ra.h);
		 objb.ra.m = pgm_read_byte(&list[i].ra.m);
		 objb.ra.s = pgm_read_byte(&list[i].ra.s);
		 objb.dec.d = pgm_read_byte(&list[i].dec.d);
		 objb.dec.m = pgm_read_byte(&list[i].dec.m);
		 objb.dec.s = pgm_read_byte(&list[i].dec.s);
		}
	 while ((vis_obj(objb.ra,objb.dec))==OBJ_NVIS);
	 lcd_setcursor(STR_CUR,0);
	 lcd_string_P(list[i].name);
	 READ_KEY(tecla,dtecla);
	 switch(tecla)
		{
		 case BUT_RT:
			dir=1;
		 break;
		 case BUT_LT: // Si aprieta las direcciones, modifica y valida
			dir=-1;
		 break;
		 case BUT_ENTER:
			 strcpy_Pe(objb.name,list[i].name);
			 return i;
		 break;
		 case BUT_ESC:
			return size;
		 break;
		 default: // Si aprieta cualquier cosa, no cambia nada
			i=i-dir;
			 if (i==size)
				i=0;
			 if (i<0)
				i=size-1;
		 break;
		}
	}
}

/*-------------------------------------------------------------------------------*/
/* Mueve el cursor invirtiendo una linea                                         */
/*-------------------------------------------------------------------------------*/
void mv_cursor(unsigned char tecla, unsigned char end_opt)
{
 if (tecla==BUT_UP || tecla==BUT_DW)
	{
	 inv_linea(OPT2LINE(menu_opt), STR_CUR, END_CUR); // Borra la opcion actual para pasar a la siguiente
	 if (tecla==BUT_UP)
		{
		 if (menu_opt==1) // limite superior
			 menu_opt=end_opt;
		 else
		 	 menu_opt--;
		}
	 else
		{
		 if (menu_opt==end_opt) // limite inferior
			 menu_opt=1;
		 else
			 menu_opt++;
		}
	 inv_linea(OPT2LINE(menu_opt), STR_CUR, END_CUR); // Invierte la siguiente linea
	}
}

unsigned char menu_goto (obj_ceqs obj_c, unsigned char tracking, unsigned char recal)
{
 unsigned char flag=0;
 park_flag=NO_PARK;
 lcd_setcursor(STR_CUR+1,0);
 lcd_string(obj_c.name);
 lcd_setcursor(STR_CUR+5,5);
 lcd_string_P(PSTR("Loading"));
 start_anim(CursorX,CursorY,350,ANIM_LOADING2);
 spi_send(6,SGOTO_SE,obj_c.ra.h,obj_c.ra.m,obj_c.ra.s,obj_c.dec.d,obj_c.dec.m,obj_c.dec.s);
 do
	{// Espera a que termine de posicionarse
	 sdata=spi_read(RMOTST);
	 if ((sdata.buf[0]==0) && (sdata.buf[1]==0)&&(flag==0))
		{
		 flag=1;
		 sound_ok();
		 stop_anim();
		 if (tracking==TRACK_ON)
			{
			 lcd_setcursor(STR_CUR+5,5);
			 lcd_string_P(PSTR("Siguiendo "));
			 inv_linea(5,STR_CUR+4,CursorX+1);
			 if (recal==RECALC)
				return FOUND;
			}
		 else
			 return FOUND;
		}
	 delay_ms(100); // Demora con numero primo para impedir que queden en fase con el pwm
	 sdata=spi_read(RPOS);
	 tpos.ra.h=sdata.buf[0];
	 tpos.ra.m=sdata.buf[1];
	 tpos.ra.s=sdata.buf[2];
	 tpos.dec.d=sdata.buf[3];
	 tpos.dec.m=sdata.buf[4];
	 tpos.dec.s=sdata.buf[5];
	 lcd_setcursor(STR_CUR+7,2);
	 menu_dataout(3,PRINT_NSIGN,&sym_ra[0],tpos.ra.h,tpos.ra.m,tpos.ra.s);
	 lcd_setcursor(STR_CUR+1,4);
	 menu_dataout(3,PRINT_WSIGN,&sym_dec[0],tpos.dec.d,tpos.dec.m,tpos.dec.s);
	}
 while ((codKey!=BUT_ESC));
 stop_anim();
 codKey=0;
 spi_send(2,SMOVMOT,MOTSTOP,MOTSTOP); // Con ESCAPE detiene al motor y vuelve a la pagina anterior
 return NOTFOUND;
}

/*-------------------------------------------------------------------------------*/
/* Busca los objetos celestes por nombre de una lista, segun la pagina que se    */
/* encuentre                                                                     */
/*-------------------------------------------------------------------------------*/
obj_ceqs find_obj(char * name, unsigned char size)
{
 obj_ceqs r;
 r.name[0]='0';
 unsigned char n;
 switch (menu_pg)
	{
	 case PG_B_SSOL:
		for (n=0;n<10;n++)
			{
			 if ((strncmp_Pe (name,ssol_elem[n],size))==0)
				{
				 if (n==0)
					 r = get_moon();
				 else
					 r = get_coord(n-1);
				 break;
				}
			}
	 break;
	 case PG_B_IC:
		r=find_obj_in_file("IC.csv",name,size);
	 break;
	 case PG_B_NGC:
		r=find_obj_in_file("NGC.csv",name,size);
	 break;
	 case PG_B_MES:
		r=find_obj_in_file("MESSIER.csv",name,size);
	 break;
	}
 return r;
}

#include "sd_raw.h"
#include "partition.h"
#include "fat.h"
#include "fat_config.h"
/*-------------------------------------------------------------------------------*/
/* Busca los objetos celestes de un archivo                                      */
/*-------------------------------------------------------------------------------*/
obj_ceqs find_obj_in_file(const char* file, char *name, unsigned char size)
{
 obj_ceqs r={.name[0]='0'};
 struct partition_struct* partition = partition_open(sd_raw_read, sd_raw_read_interval,0,0,0);
 if (!partition)
	{
 /* If the partition did not open, assume the storage device is a "superfloppy", i.e. has no MBR. */
	 partition = partition_open(sd_raw_read,sd_raw_read_interval,0,0,-1);
	 if(!partition)
		{
		 lcd_string_P(PSTR("Partition failed\n"));
		 return r;
		}
	}
	 /* open file system */
 struct fat_fs_struct* fs = fat_open(partition);
 if(!fs)
	{
	 lcd_string_P(PSTR("Filesystem failed\n"));
	 return r;
	}

	/* open root directory */
 struct fat_dir_entry_struct directory;
 fat_get_dir_entry_of_path(fs, "/", &directory);
 struct fat_dir_struct* dd = fat_open_dir(fs, &directory);
 if (!dd)
	{
	 lcd_string_P(PSTR("Root directory failed\n"));
	 return r;
	}

// Read file data
 struct fat_dir_entry_struct dir_entry;
 struct fat_file_struct* fd;
 char buffer[25];
// unsigned char n,i=0;
 int32_t offset_file;
 int32_t file_point;
 int32_t file_pos;
 char *paux;
 intptr_t count;
 fd= open_file_in_dir(fs, dd,file); // Probar con dir_entry->long_name
 if (!fd)
	{
	 lcd_string_P(PSTR("Archivo invalido\n"));
	 return r;
	}
 while(fat_read_dir(dd, &dir_entry))
	{
	 if (strcmp(dir_entry.long_name, file)==0)
		{
		 file_point=(dir_entry.file_size>>1)/25;
		 break;
		}
	}

 offset_file= file_point*25; // Arranca siempre por la mitad
 file_pos = offset_file;
 int compare;
 unsigned char comp_size;
 fat_seek_file(fd, &offset_file, FAT_SEEK_CUR);
 unsigned char it_end=0;
 while((count=fat_read_file(fd, buffer, sizeof(buffer)))>0)
	{
	 // Ajuste de linea
	 offset_file = strchr_e(buffer,'\n')-count;  // Ajusta al siguiente fin de línea
	 file_pos += offset_file + count + 25;
	 if (file_pos> dir_entry.file_size)
		file_pos = dir_entry.file_size;
	 fat_seek_file(fd, &offset_file, FAT_SEEK_CUR);
	 if((count=fat_read_file(fd, buffer, sizeof(buffer)))<=0)
		 break;
	 // Comparacion
	 compare = strncmp(name,buffer,size);
	 comp_size = strchr_e(buffer,' ');
	 // Debe verificar si esta comparando contra el nombre válido
	 if (comp_size>(size+1))
		 compare = -1; // Está pasado
	 if (comp_size<(size+1))
		 compare = 1; // Está mas atrás

	 if (compare==0)
		{ // Lo encuentra
		 paux = strtok( buffer," ");    // Primera llamada => Primer token
		 strcpy(r.name,paux);

		 paux = strtok(NULL," ");
		 r.ra.h=strtolong(paux);

		 paux = strtok(NULL," ");
		 r.ra.m=strtolong(paux);

		 paux = strtok(NULL," ");
		 r.ra.s=strtolong(paux);

		 paux = strtok(NULL," ");
		 if (paux[0]=='+')
			{
			 r.dec.d=1;
			 r.dec.m=1;
			 r.dec.s=1;
			}
		 else
			{
			 r.dec.d=-1;
			 r.dec.m=-1;
			 r.dec.s=-1;
			}

		 paux = strtok(NULL," ");
		 r.dec.d=strtolong(paux)*r.dec.d;

		 paux = strtok(NULL," ");
		 r.dec.m=strtolong(paux)*r.dec.m;

		 paux = strtok(NULL," ");
		 r.dec.s=strtolong(paux)*r.dec.s;
		 break;
		}
	 if (file_point>1)
		 file_point= file_point>>1;
	 else
		{
		 file_point= 1;
		 it_end=it_end+1;
		 if (it_end==10)
			break;
		}
	 if (compare>0)
		{// Esta mas adelante
		 offset_file=file_point*25;
		}
	 else
	 	{// Esta mas atras
		 offset_file=-(file_point*25);
	 	}
	 offset_file-=30; // Reacomoda para dejar desde la última línea leida
	 file_pos+= offset_file; // Prepara la posicion a la que va a apuntar

	 if (file_pos > dir_entry.file_size) // 16 es el tamaño mínimo
		{
		 file_pos -= 35;
		 offset_file -= 35;
		}
	 fat_seek_file(fd, &offset_file, FAT_SEEK_CUR);
	}
 fat_close_file(fd);
 fat_close_dir(dd);
 fat_close(fs);
 partition_close(partition);
 return r;
}


/*-------------------------------------------------------------------------------*/
/*Imprime la pagina del menu que indica la variable "pag" e inicializa todas las */
/*variables de la pagina                                                         */
/*-------------------------------------------------------------------------------*/
void menu_print(unsigned pag)
{
 clr_bit(INT2,EIMSK); // deshabilita la muestra con el RTC
 menu_pg=pag;
 menu_opt = 1;
 stop_anim();
 lcd_setcursor(0,0);
 switch (pag)
	{
	 case PG_PRINC: //menu principal
		 lcd_bmp(&p00000_Menu[0]); // Si la imagen es de 128x64 no hace falta borrar la pantalla.
	 break;

	 case PG_BUSQ: // Pantalla de elementos a buscar
		 lcd_bmp(&p00001_Busqueda[0]); // Si la imagen es de 128x64 no hace falta borrar la pantalla.
	 break;
		case PG_B_SSOL: // Sistema Solar - aqui carga el nombre del objeto
			 lcd_bmp(&p00011_Busqueda[0]);
			 lcd_setcursor(STR_CUR,0);
		break;
		case PG_B_CE: // Catalogos Estelares - Lista de catalogos disponibles
			 lcd_bmp(&p00012_Busqueda[0]);
		break;
			case PG_B_MES:
				 lcd_bmp(&p00121_Busqueda[0]);
				 lcd_setcursor(STR_CUR,0);
			break;
			case PG_B_IC:
				 lcd_bmp(&p00122_Busqueda[0]);
				 lcd_setcursor(STR_CUR,0);
			break;
			case PG_B_NGC:
				 lcd_bmp(&p00123_Busqueda[0]);
				 lcd_setcursor(STR_CUR,0);
			break;
		case PG_B_OC:
			lcd_bmp(&p00013_Busqueda[0]);
			// Aca deberia levantar los nombres y cargarlos en una matriz
		break;
			case PG_B_OC1: case PG_B_OC2: case PG_B_OC3: case PG_B_OC4:
				lcd_bmp(&p0013x_Busqueda[0]);
			break;
			case PG_B_SSOLE: case PG_B_ICE: case PG_B_MESE: case PG_B_NGCE: case PG_B_OC1E: case PG_B_OC2E: case PG_B_OC3E: case PG_B_OC4E: // Pagina que muestra las coordenadas del objeto a ver.
				lcd_bmp(&p01xx1_Busqueda[0]);
				lcd_setcursor(STR_CUR,0);
				lcd_string(objb.name);
				lcd_setcursor(STR_CUR+6,2);
				menu_dataout(3,PRINT_NSIGN,&sym_ra[0],objb.ra.h,objb.ra.m,objb.ra.s);
				lcd_setcursor(STR_CUR,4);
				menu_dataout(3,PRINT_WSIGN,&sym_dec[0],objb.dec.d,objb.dec.m,objb.dec.s);
			break;
				case PG_B_SSOLB: case PG_B_ICB: case PG_B_MESB: case PG_B_NGCB: case PG_B_OC1B: case PG_B_OC2B: case PG_B_OC3B: case PG_B_OC4B: case PG_U_MODMB: //case PG_TOURB:
					lcd_bmp(&p01xx1_Busqueda[0]);

				break;
	 case PG_CONF: //Pantalla de configuración
		 lcd_bmp(&p00002_Configuracion[0]); // Si la imagen es de 128x64 no hace falta borrar la pantalla.
	 break;

		 case PG_C_TIM: //Pantalla de Fecha y Hora
			 lcd_bmp(&p00022_Fechayhora[0]); // Si la imagen es de 128x64 no hace falta borrar la pantalla.
			 lcd_setcursor(STR_CUR+2,0); // Fecha
			 menu_dataout(3,PRINT_NSIGN,"// ", fecha.d,fecha.m, fecha.y);
			 inv_linea(0,STR_CUR,END_CUR); // Resalta la opcion de la fecha
			 lcd_setcursor(STR_CUR+2,2); // Hora
			 menu_dataout(2,PRINT_NSIGN,": ",t.h,t.m);
			 menu_dataout(1,PRINT_WSIGN," ",t.hh);
		 break;

		 case PG_C_PGE: //Pantalla de Coordenadas geograficas
			 lcd_bmp(&p00021_Posiciongeografica[0]); // Si la imagen es de 128x64 no hace falta borrar la pantalla.
			 lcd_setcursor(STR_CUR+2,0); // Fecha
			 menu_dataout(2,PRINT_NSIGN,&sym_dec[0],lat.d,lat.m);
			 lcd_putchar(' ');
			 lcd_putchar(lat.s);
			 inv_linea(0,STR_CUR,END_CUR);
			 lcd_setcursor(STR_CUR+2,2);
			 if (lon.d<100)
				lcd_putchar('0');
			 menu_dataout(2,PRINT_NSIGN,&sym_dec[0],lon.d,lon.m);
			 lcd_putchar(' ');
			 lcd_putchar(lon.s);
		 break;
		 case PG_C_ALE1:
			 lcd_bmp(&p00234_Alineacion[0]);
		 break;
			 case PG_C_ALE2:
				lcd_bmp(&p01xx1_Busqueda[0]);
				lcd_setcursor(3,0);
				lcd_string_P(PSTR("Estrella"));
			 break;
			 case PG_C_ALE3:
				 lcd_bmp(&p00235_Alineacion[0]);
				 lcd_setcursor(STR_CUR+2,0);
				 lcd_string(objb.name);
			 break;
			 case PG_C_ALE4:
				 lcd_bmp(&p00236_Alineacion[0]);
			 break;
		 case PG_C_ALM:
			 lcd_bmp(&p00023_Alineacion[0]); // Deberia ser una pantalla de presentacion a la cuestion
		 break;
			 case PG_C_ALM1:
				 lcd_bmp(&p00231_Alineacion[0]);
			 break;
			 case PG_C_ALM2:
				 lcd_bmp(&p00232_Alineacion[0]);
			 break;
			 case PG_C_ALM3:
				 lcd_bmp(&p00233_Alineacion[0]);
			 break;
	 case PG_UTIL:
		lcd_bmp(&p00003_Utilidades[0]);
	 break;
		 case PG_U_MODM:
			 lcd_bmp(&p00031_Modomanual[0]);
			 lcd_setcursor(STR_CUR+8,0);
			 menu_dataout(3,PRINT_NSIGN,&sym_ra[0],objb.ra.h,objb.ra.m,objb.ra.s);
			 inv_linea(0,STR_CUR,END_CUR);
			 lcd_setcursor(STR_CUR+2,2);
			 menu_dataout(3,PRINT_WSIGN,&sym_dec[0],objb.dec.d,objb.dec.m,objb.dec.s);
		 break;
		 case PG_U_PC:
			lcd_clr();
			lcd_setcursor(5,3);
			lcd_string_P(PSTR(" Conectando "));
			start_anim(CursorX,CursorY,300,ANIM_LOADING1);
			lcd_setcursor(64,7);
			lcd_bmp(&pesc_msj[0]);
		 break;
			 case PG_U_LPC:
				stop_anim();
				lcd_clr();
				lcd_setcursor(2,0);
				lcd_string_P(PSTR("Control desde PC"));
				inv_linea(0,0,127);
				lcd_setcursor(64,7);
				lcd_bmp(&pesc_msj[0]);
				lcd_setcursor(0,2);
			 break;
		 case PG_U_PIN:
			lcd_bmp(&p00033_Posiciondeinicio[0]);
			start_anim(119,3,300,ANIM_LOADING1);
		 break;
			 case PG_U_PINB:
				lcd_bmp(&p00331_Posiciondeinicio[0]);
			 break;
	}
 set_bit(INTF2,EIFR); // borra el flag de interrupcion
 set_bit(INT2,EIMSK); // habilita la muestra con el RTC
}
