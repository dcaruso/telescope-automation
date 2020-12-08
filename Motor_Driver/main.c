/***************************************************************************/
/* Descripcion:                                                            *
/*  Programa principal, este describe el funcionamiento del driver del     *
/*  motor. De aca se llaman a las distintas funciones.                     *
/***************************************************************************/

#include "config.h"
#include "motor.h"
#include "spi_slave.h"

volatile c_eqs tpos={.ra.h=0,.ra.m=0,.ra.s=0,.dec.d=0,.dec.m=0,.dec.s=0};
volatile float ftpos_dec,ftpos_ra;
volatile float ra_seg, dec_seg;

volatile c_eqs obj={.ra.h=12,.ra.m=0,.ra.s=0,.dec.d=90,.dec.m=0,.dec.s=0};
volatile unsigned char hemisf;

int main(void)
{
 unsigned char flag_sdtime_r=0, flag_dctime_r=0;
 unsigned c_sidtime=0,c_dectime=0;
 unsigned frecAR_r=1,frecDEC_r=1;
 unsigned long stepDECLIM = 0,stepARLIM = 0;
 motor_init(); // Inicializa los motores
 spi_init();   // Inicializa la comunicacion SPI
 sei();
 ftpos_dec=0.0;
 ftpos_ra=0.0;
 while (1)
	{
	 if ((spi_dint.cmd!=SPI_NOCMD)&&(spi_state==SPI_IDLE))
		{
		 switch (spi_dint.cmd)
			{ // Aca irian las asignaciones a las variables y los llamados a las funciones
			 case SMOVMOT:
				 switch (spi_dint.buf[0])
					{
					 case (MOTRUN|DIRNEG):
						 motor_ARrun(-MIN_VEL,0);
					 break;
					 case (MOTRUN|DIRPOS):
						 motor_ARrun(MIN_VEL,0);
					 break;
					 default:
						 motor_ARrun(0,0);
					 break;
					}
				 switch (spi_dint.buf[1])
					{
					 case (MOTRUN|DIRNEG):
						 motor_DECrun(-MIN_VEL,0);
					 break;
					 case (MOTRUN|DIRPOS):
						 motor_DECrun(MIN_VEL,0);
					 break;
					 default:
						 motor_DECrun(0,0);
					 break;
					}
				track_AR=0;
			 break;
			 case SPOSORG:
				tpos.ra.h=spi_dint.buf[0];
				tpos.ra.m=spi_dint.buf[1];
				tpos.ra.s=spi_dint.buf[2];
				tpos.dec.d=spi_dint.buf[3];
				tpos.dec.m=spi_dint.buf[4];
				tpos.dec.s=spi_dint.buf[5];
				ftpos_dec=convert_dec2fs(tpos.dec);
				ftpos_ra=convert_ra2fs(tpos.ra);
				ra_seg=0.0;
				dec_seg=0.0;
				lstm =0;
				if (tpos.dec.d==-90) // Hemisferio SUR
					{
					 hemisf=HSUR;
					 tel_side=PRE_MER;
					}
				else
					{
					 tel_side=POST_MER;
					 hemisf=HNOR;
					}
			 break;
			 case SPOSTEL:
				tpos.ra.h=spi_dint.buf[0];
				tpos.ra.m=spi_dint.buf[1];
				tpos.ra.s=spi_dint.buf[2];
				tpos.dec.d=spi_dint.buf[3];
				tpos.dec.m=spi_dint.buf[4];
				tpos.dec.s=spi_dint.buf[5];
				ftpos_dec=convert_dec2fs(tpos.dec);
				ftpos_ra=convert_ra2fs(tpos.ra);
				ra_seg=0.0;
				dec_seg=0.0;
			 break;
			 case SGOTO:
				obj.ra.h=spi_dint.buf[0];
				obj.ra.m=spi_dint.buf[1];
				obj.ra.s=spi_dint.buf[2];
				obj.dec.d=spi_dint.buf[3];
				obj.dec.m=spi_dint.buf[4];
				obj.dec.s=spi_dint.buf[5];
				motor_goto(MAX_VEL,TRACK_OFF,obj);
			 break;
			 case SGOTO_SE:
				obj.ra.h=spi_dint.buf[0];
				obj.ra.m=spi_dint.buf[1];
				obj.ra.s=spi_dint.buf[2];
				obj.dec.d=spi_dint.buf[3];
				obj.dec.m=spi_dint.buf[4];
				obj.dec.s=spi_dint.buf[5];
				motor_goto(MAX_VEL,TRACK_ON,obj); // Siempre hace seguimiento
			 break;
			 case SCONFM:
			 break;
			}
		 spi_dint.cmd=SPI_NOCMD;
		}
	 if (ftpos_ra>=86400.0)
		ftpos_ra-=86400.0;
	 if (ftpos_ra<0)
		ftpos_ra+=86400.0;
	 tpos.dec = convert_f2dec(ftpos_dec);
	 tpos.ra = convert_f2ra(ftpos_ra);

	 if (!track_AR)
		{ // Ajuste por rotación de la Tierra
		 if (flag_sdtime!=flag_sdtime_r)
			{
			 flag_sdtime_r=flag_sdtime;
			 c_sidtime++;
			 // Analiza donde se encuentra el telescopio según el meridiano del lugar
			 if (c_sidtime>=LIMC_SIDT)
				{
				 TIMSK0 = 0; // Enmascara la interrupcion, para no mandarse macanas
				 c_sidtime=0;
				 ftpos_ra=ftpos_ra+CTE_AR_SG; // Ajusta la posicion segun el corrimiento que tuvo
				 if (stepsAR!=0)
					stepsAR-=tStepAR; // Agrega un paso extra si va en contra al sidereo, resta uno si va a favor
									 // Se toma al movimiento sidereo como positivo
				 TIMSK0 = (1<<OCIE0A); //Libera la interrupcion
				}
			}
		}
	else
		 c_sidtime=0;
	}
 return 0;
}


