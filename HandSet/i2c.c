/***************************************************************************/
/* Descripcion:                                                            *
/*  Contiene las funciones básicas para comunicar mediante I2C al          *
/*  ATmega128 con cualquier periferico                                     *
/***************************************************************************/

#include "i2c.h"

//************************************************
//TWI initialize
// bit rate:18 (freq: 88Khz @8MHz)
//************************************************
void i2c_init(void)
{
 TWCR= 0x00; //disable twi
 TWBR= 0x28; //set bit rate
 TWSR= 0x00; //set prescale
 //TWCR= 0x44; //enable twi
}


//*************************************************
//Function to start i2c communication
//*************************************************
unsigned char i2c_start(int type)
{
 TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); 	//Send START condition
 while (!END_I2C);   		//Wait for TWINT flag set. This indicates that the
							//START condition has been transmitted
 if ((TWSR & MSK_TWSR) == type)			//Check value of TWI Status Register
    return (I2C_OK);
 else
	{
	 i2c_stop();
	 return (I2C_ERROR);
	}
}

//**************************************************
//Function to transmit address of the slave
//*************************************************
unsigned char i2c_sendAddress(unsigned char address)
{
 unsigned char STATUS;
   
 if ((address & 0x01) == 0) 
    STATUS = MT_SLA_ACK;
 else
    STATUS = MR_SLA_ACK; 
   
 TWDR = address; 
 TWCR = (1<<TWINT)|(1<<TWEN);	   //Load SLA_W into TWDR Register. Clear TWINT bit
   		  			 				   //in TWCR to start transmission of address
 while (!END_I2C);	   //Wait for TWINT flag set. This indicates that the
   		 		   					   //SLA+W has been transmitted, and
									   //ACK/NACK has been received.
 if ((TWSR & MSK_TWSR) == STATUS)	   //Check value of TWI Status Register
    return(I2C_OK);
 else 
	{
	 i2c_stop();
	 return (I2C_ERROR);
	}
}

//**************************************************
//Function to transmit a data byte
//*************************************************
unsigned char i2c_sendData(unsigned char data)
{
 TWDR = data; 
 TWCR = (1<<TWINT) |(1<<TWEN);	   //Load SLA_W into TWDR Register. Clear TWINT bit
   		  			 				   //in TWCR to start transmission of data
 while (!END_I2C);	   //Wait for TWINT flag set. This indicates that the
   		 		   					   //data has been transmitted, and
									   //ACK/NACK has been received.
 if ((TWSR & MSK_TWSR) == MT_DATA_ACK)   //Check value of TWI Status Register
    return (I2C_OK);
 else
	{
	 i2c_stop();
	 return (I2C_ERROR);
	}
}

//*****************************************************
//Function to receive a data byte and send ACKnowledge
//*****************************************************
unsigned char i2c_receiveData_ACK(void)
{
 TWCR = (1<<TWEA)|(1<<TWINT)|(1<<TWEN);
  
 while (!END_I2C);	   	   //Wait for TWINT flag set. This indicates that the
   		 		   					   //data has been received
 if ((TWSR & MSK_TWSR) != MR_DATA_ACK)    //Check value of TWI Status Register
 	{
	 i2c_stop();
	 return (I2C_ERROR);
	}

 return(TWDR);
}

//******************************************************************
//Function to receive the last data byte (no acknowledge from master
//******************************************************************
unsigned char i2c_receiveData_NACK(void)
{
//  unsigned char data;
  
  TWCR = (1<<TWINT)|(1<<TWEN);
  
  while (!(TWCR & (1<<TWINT)));	   	   //Wait for TWINT flag set. This indicates that the
   		 		   					   //data has been received
  if ((TWSR & MSK_TWSR) != MR_DATA_NACK)    //Check value of TWI Status Register
	{
	 i2c_stop();
	 return (I2C_ERROR);
	}

 return(TWDR);
}
//**************************************************
//Function to end the i2c communication
//*************************************************   	
void i2c_stop(void)
{
  TWCR =  (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);	  //Transmit STOP condition
}  

