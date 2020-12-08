// Header de i2c.c

#include "config.h"

#define	 START				0x08
#define  REPEAT_START		0x10
#define  MT_SLA_ACK			0x18
#define  MT_SLA_NACK		0x20
#define  MT_DATA_ACK		0x28
#define  MT_DATA_NACK		0x30
#define  MR_SLA_ACK			0x40
#define  MR_SLA_NACK		0x48
#define  MR_DATA_ACK		0x50
#define  MR_DATA_NACK		0x58
#define  ARB_LOST			0x38

#define  MSK_TWSR           0xF8

#define  I2C_ERROR			0x7E
#define  I2C_OK             0

#define  END_I2C            (TWCR & (1<<TWINT))

#define  DS1307_W			0xD0 // Dirección de escritura del RTC
#define  DS1307_R			0xD1 // Dirección de lectura del RTC

void i2c_init(void);
unsigned char i2c_start(int);
unsigned char i2c_sendAddress(unsigned char);
unsigned char i2c_sendData(unsigned char);
unsigned char i2c_receiveData_ACK(void);
unsigned char i2c_receiveData_NACK(void);
void i2c_stop(void);

