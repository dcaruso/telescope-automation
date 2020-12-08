/***************************************************************************/
/* Descripcion:                                                            *
/*  Define las funciones basicas del lcd grafico de 128x64 (ancho x largo) *
/*  Este lcd esta dividdo en dos partes que controlan 64x64 pixeles.       *
/*  A su vez, cada una se divide en 8 filas de 8 pixeles. A la hora de     *
/*  escribir el mismo, uno escribe de a columnas de 8 pixeles y se pueden  *
/*  setear metodos de auto-avance en el sentido de X para incrementar un   *
/*  pixel luego de cada escritura.                                         *
/*  La idea con esto es que desde el main, uno pueda escribir strings,     *
/*  ubicar el cursor donde quiera y todo se resuelva en forma invisible    *
/*  para que eso pase.                                                     *
/***************************************************************************/

#include "lcd.h"
#include "font.h"
#include "images.h"
#include "util.h"

volatile unsigned char CursorX, CursorY;
volatile unsigned char lcd_state=LCD_CLEAR;
//**********************************
//** Seteo la posicion del cursor **
//**********************************
void lcd_setcursor (unsigned char Columna, unsigned char Linea)
{
 CursorX = Columna;
 CursorY = Linea;
 if (Columna > 63)
   {
    set_bit(DER, LCD_SIDE);
    clr_bit(IZQ, LCD_SIDE);
    Columna -=64;
   }
 else
   {
    set_bit(IZQ, LCD_SIDE);
    clr_bit(DER, LCD_SIDE);
   }
 lcd_wrcom(X_INI | Columna);
 lcd_wrcom(Y_INI | Linea);
}

//**********************************
//** Escribo un comando en el LCD **
//**********************************
void lcd_wrcom(unsigned char com)
{
 clr_bit(RW_O,LCD_wCTRL);
 clr_bit(DI_O,LCD_wCTRL);
 set_bit(E_O,LCD_wCTRL);
 wDATO(com);
 clr_bit(E_O,LCD_wCTRL);    // Valida el comando.
 delay_5us;
}

//***********************************
//** Lee un dato cargado en el LCD **
//***********************************
unsigned char lcd_rdat(void)
{
 unsigned char rdata;
 if (CursorX==64) // Esta del otro lado
    lcd_setcursor(64,CursorY);  // Pasa para el otro lado
 LCD_dDATO=0x00;// sBUS(0x00); // Los pone como entradas
 LCD_wDATO=0xFF; // Pull-ups para leer NUEVO
 set_bit(RW_O,LCD_wCTRL);
 set_bit(DI_O,LCD_wCTRL);
 set_bit(E_O,LCD_wCTRL);
 delay_5us;

 rdata=LCD_rDATO; //rDATO;      // Dato a descartar
 clr_bit(E_O,LCD_wCTRL);    // Valida el comando.
/* set_bit(RW_O,LCD_wCTRL);*/
/* set_bit(DI_O,LCD_wCTRL);*/
 set_bit(E_O,LCD_wCTRL);
 delay_5us;
 rdata=LCD_rDATO; //rDATO;      // Dato a descartar
 clr_bit(E_O,LCD_wCTRL);    // Valida el comando.
 LCD_dDATO=0xFF;//sBUS(0xFF); // Todo como salida otra vez.
 delay_5us;
 lcd_setcursor(CursorX, CursorY);
 return rdata;
}

//*******************************
//** Escribo un dato en el LCD **
//*******************************
void lcd_wrdat(unsigned char data, unsigned char read)
{
 if (CursorX==64) // Esta del otro lado
    lcd_setcursor(64,CursorY);  // Pasa para el otro lado
 if (CursorX==128)
    lcd_setcursor(0,CursorY+1); // Siguiente Linea, si se zarpa
 if (read==1)
    data|=lcd_rdat();
 clr_bit(RW_O,LCD_wCTRL);
 set_bit(DI_O,LCD_wCTRL);
 set_bit(E_O,LCD_wCTRL);
 if (lcd_state==LCD_INV)
	 data=~data;
 wDATO(data);
 clr_bit(E_O,LCD_wCTRL);    // Valida el comando.
 delay_5us;
 CursorX++;
}


//****************************
//** Inicializacion del LCD **
//****************************
void lcd_init (void)
{
 LCD_wCTRL = 0x00;
 LCD_dCTRL = 0xFF;
 LCD_wDATO = 0x00;
 LCD_dDATO = 0xFF;
 set_bit(RST_LCD,LCD_wCTRL);
 lcd_selfont('m'); // Selecciona la tipografia media por defecto
 set_bit(DER,LCD_SIDE);
 clr_bit(IZQ,LCD_SIDE);
 clr_bit(E_O,LCD_wCTRL);
 lcd_wrcom(D_OFF); //apaga el display
 lcd_wrcom(X_INI); // Y address position 0.
 lcd_wrcom(Y_INI); // Page 0.
 lcd_wrcom(ST_LN); // Start line 0;
 lcd_wrcom(D_ON); // Enciende el lcd.
 clr_bit(DER,LCD_SIDE);
 set_bit(IZQ,LCD_SIDE);
 lcd_wrcom(D_OFF); //apaga el display
 lcd_wrcom(X_INI); // Y address position 0.
 lcd_wrcom(Y_INI); // Page 0.
 lcd_wrcom(ST_LN); // Start line 0;
 lcd_wrcom(D_ON); // Enciende el lcd.
 lcd_clr();
}

//*******************************
//** Borra la pantalla del LCD **
//*******************************
void lcd_clr(void)
{
 unsigned char y,x;
 lcd_setcursor(0,0);
 set_bit(IZQ,LCD_SIDE);
 set_bit(DER,LCD_SIDE);
 for (y=0;y<8;y++)
   {
    lcd_wrcom(Y_INI+y);
    while (CursorX<64)
       {
        lcd_wrdat(0,0); // Sin lectura previa
       }
    CursorX = 0;
   }
 lcd_setcursor(0,0);
}

//***********************************
//** Selecciona la Font que quiere **
//***********************************
// p= pequeña 3x6
// m= mediana 5x7
// g= grande 8x12
volatile PGM_P fonts;
void lcd_selfont (char typefont)
{
 switch (typefont)
   {
    case 'p':
//         fonts = &Font3x6[0];
    break;
    case 'm':
         fonts = &Font5x7[0];
    break;
    case 'g':
    break;
   }
}

//*************************
//** Putchar para el LCD **
//*************************
void lcd_putchar (char c)
{
 unsigned char col;
 int i, width, height;
 unsigned char data;
 width = pgm_read_byte(fonts);
 height = pgm_read_byte(fonts+1);
 for (col=0; col<width; col++)
    {
     i = (int)((((int)c)-32)*width+col+2);
     data = pgm_read_byte(fonts+i);
     lcd_wrdat(data,0); // anda // Direcciona columna por columna del caracter que desea ver
     if ((data==0x00)&&(c>='A')&&(c<='z')&&(col>1)) // Termino el caracter
     	break; // reduce los espacios entre caracteres
    }
 lcd_wrdat(0x00,0); // Espacio separador
}

//************************************
//** Copia una cadena de ROM al LCD **
//************************************
// El control del largo de la frase corre por cuenta del usuario
void lcd_string_P (PGM_P frase)
 {
  while (pgm_read_byte(frase)!='\0')
    {
     if (pgm_read_byte(frase)=='\n')
        lcd_setcursor(0,CursorY+1);  // Columna cero, siguiente linea
     else
       {
        if (pgm_read_byte(frase)=='\r')
           lcd_setcursor(0,CursorY);  // Retorno de carro
        else
           lcd_putchar(pgm_read_byte(frase));
       }
     frase++;
    }
 }

//************************************
//** Copia una cadena de RAM al LCD **
//************************************
// El control del largo de la frase corre por cuenta del usuario
void lcd_string (char *frase)
 {
  while ((*frase)!='\0')
    {
     if ((*frase)=='\n')
        lcd_setcursor(0,CursorY+1);  // Columna cero, siguiente linea
     else
       {
        if ((*frase)=='\r')
           lcd_setcursor(0,CursorY);  // Retorno de carro
        else
			{
			 if ((*frase)=='\t')
				lcd_setcursor(CursorX+4,CursorY);
			 else
				lcd_putchar((*frase));
			}
       }
     frase++;
    }
 }


//*************************************
//** Dibuja una imagen bmp en el LCD **
//*************************************
// Siempre se dibuja de izquierda a derecha. Desde una ubicacion definida previamente
void lcd_bmp (PGM_P image)
 {
  unsigned char x_ini=CursorX;
  unsigned char y_ini=CursorY;
  unsigned char width= pgm_read_byte(image++)+x_ini;
  unsigned char heigth= pgm_read_byte(image++)/8-1+y_ini;
  while (CursorX!=width || CursorY!=heigth)
     {
      if (CursorX==width)
         lcd_setcursor(x_ini,CursorY+1); // siguiente linea
      lcd_wrdat(pgm_read_byte(image++),0);
     }
  lcd_setcursor(0,0);
 }




