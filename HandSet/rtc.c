/***************************************************************************/
/* Descripcion:                                                            *
/*  Rutinas de control del RTC                                             *
/***************************************************************************/

#include "rtc.h"
#include "i2c.h"
#include "menu.h"
#include "lcd.h"
#include "anim.h"
#include "teclado.h"
#include "spi_master.h"


void rtc_init(void)
{
 i2c_init();
 rtc_out1hz();
 set_bit(ISC21,EICRA); // por flanco de subida
 set_bit(ISC20,EICRA); 
 clr_bit(INT1HZ,P_dRTC); // Pone como entrada la interrupcion
 set_bit(INT1HZ,P_wRTC); // Pull-up interno a la entrada de interrupcion
 set_bit(INTF2,EIFR); // borra el flag de interrupcion
 set_bit(INT2,EIMSK); // habilita la interrupcion ext 0
}

unsigned char rtc_out1hz(void)
{
 i2c_start(START);
 i2c_sendAddress(DS1307_W);
 i2c_sendData(0x07);
 i2c_sendData(0x10);
 i2c_stop();
 return I2C_OK;
}

ISR(INT2_vect)
{
 unsigned char xc_bk=CursorX,yc_bk=CursorY;
 lcd_setcursor(4,7);
 if(rtc_read()==I2C_ERROR)
 	lcd_string_P(PSTR("Err RTC"));
 if ((t.s&0x01))
	 menu_dataout(2,PRINT_NSIGN,": ",t.h,t.m);
 else
	 menu_dataout(2,PRINT_NSIGN,"  ",t.h,t.m);
 lcd_setcursor(xc_bk,yc_bk);
 set_bit(INTF2,EIFR);
}

//***********************************************************************
//Function to read RTC time
//***********************************************************************    
unsigned char rtc_read(void)
{
 unsigned char data;

 if ((i2c_start(START))==I2C_ERROR)
	 return I2C_ERROR;

 if ((i2c_sendAddress(DS1307_W))==I2C_ERROR)
	 return I2C_ERROR;

 if ((i2c_sendData(0x00))==I2C_ERROR)
	 return I2C_ERROR; 

 if ((i2c_start(REPEAT_START))==I2C_ERROR)
	 return I2C_ERROR;

 if ((i2c_sendAddress(DS1307_R))==I2C_ERROR)
	 return I2C_ERROR;
 
 data=i2c_receiveData_ACK();
 if (data==I2C_ERROR) 
	 return I2C_ERROR;
 t.s = BCD2HEX(data);
 
 data=i2c_receiveData_ACK();
 if (data==I2C_ERROR) 
	 return I2C_ERROR;
 t.m = BCD2HEX(data);
 
 data=i2c_receiveData_ACK();
 if (data==I2C_ERROR) 
	 return I2C_ERROR;
 t.h = BCD2HEX(data);
 
 data=i2c_receiveData_ACK();
 if (data==I2C_ERROR) 
	 return I2C_ERROR;
 
 data=i2c_receiveData_ACK();
 if (data==I2C_ERROR) 
	 return I2C_ERROR;
 fecha.d = BCD2HEX(data);
 
 data=i2c_receiveData_ACK();
 if (data==I2C_ERROR) 
	 return I2C_ERROR;
 fecha.m = BCD2HEX(data);
 
 data=i2c_receiveData_NACK();
 if (data==I2C_ERROR) 
	 return I2C_ERROR;
 fecha.y = BCD2HEX(data);

 i2c_stop();
 return (I2C_OK);
}


//******************************************************************
//Function to write new time in the RTC 
//******************************************************************   
unsigned char rtc_writeTime(unsigned char sec, unsigned char min,unsigned char hour)
{
 i2c_start(START);
 i2c_sendAddress(DS1307_W);
 i2c_sendData(0x00);
 i2c_sendData(HEX2BCD(sec));
 i2c_sendData(HEX2BCD(min));
 i2c_sendData(HEX2BCD(hour));
 i2c_stop();
 return (I2C_OK);
}

//******************************************************************
//Function to write new date in the RTC 
//******************************************************************   
unsigned char rtc_writeDate(unsigned char day, unsigned char month, unsigned char year)
{
 i2c_start(START);
 i2c_sendAddress(DS1307_W);
 i2c_sendData(0x04);
 i2c_sendData(HEX2BCD(day));   // Dia
 i2c_sendData(HEX2BCD(month)); // Mes
 i2c_sendData(HEX2BCD(year));  // Año
 i2c_stop();
 return (I2C_OK);
}

//***********************************************************************
//Function to write back-up into RTC
//***********************************************************************    
unsigned char rtc_wrbkp(void)
{
 i2c_start(START);
 i2c_sendAddress(DS1307_W);
 i2c_sendData(0x08);
 i2c_sendData(t.hh);		//08 - Huso Horario
 i2c_sendData(lat.d);		//09 - Latitud (Grados)
 i2c_sendData(lat.m);		//10 - Latitud (Minutos)
 i2c_sendData(lat.s);		//11 - Latitud (Hemisferio)
 i2c_sendData(lon.d);		//12 - Longitud (Grados)
 i2c_sendData(lon.m);		//13 - Longitud (Minutos)
 i2c_sendData(lon.s);		//14 - Longitud (Este/Oeste)
 i2c_sendData(hmar);		//15 - Altura sobre el mar (hmar)
 i2c_sendData(park_flag);	//16 - Parking Flag
 i2c_stop();
 return (I2C_OK);
}

//***********************************************************************
//Function to read back-up into RTC
//***********************************************************************    
unsigned char rtc_rdbkp(void)
{
 i2c_start(START);
 i2c_sendAddress(DS1307_W);
 i2c_sendData(0x08);
 i2c_start(REPEAT_START);
 i2c_sendAddress(DS1307_R);
 t.hh= i2c_receiveData_ACK();			//08 - Huso Horario
 lat.d = i2c_receiveData_ACK();			//09 - Latitud (Grados)
 lat.m = i2c_receiveData_ACK();			//10 - Latitud (Minutos)
 lat.s = i2c_receiveData_ACK();			//11 - Latitud (Hemisferio)
 if (lat.s!='N')
	lat.s='S';		// Si no hay nada cargado, por defecto le pone SUR
 lon.d = i2c_receiveData_ACK();			//12 - Longitud (Grados)
 lon.m = i2c_receiveData_ACK();			//13 - Longitud (Minutos)
 lon.s = i2c_receiveData_ACK();			//14 - Longitud (Este/Oeste)
 if (lon.s!='E')
	lon.s='O';		// Si no hay nada cargado, por defecto le pone Oeste
 hmar = i2c_receiveData_ACK();			//15 - Altura sobre el mar (hmar)
 park_flag = i2c_receiveData_NACK();		//16 - Parking Flag
 i2c_stop();
 if (park_flag==PARKING)
	{
	 tpos=initial_pos();
	 spi_send(6,SPOSORG,tpos.ra.h,tpos.ra.m,tpos.ra.s,tpos.dec.d,0,0);
	}
 else 
	{
	 menu_print(PG_C_ALM);
	}
 return (I2C_OK);
}

