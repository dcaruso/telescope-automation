/***************************************************************************/
/* Descripcion:                                                            *
/*  Contiene las funciones basicas para controlar un motor paso a paso.    *
/*  Los estados del motor los saca de la tabla step.                       *
/*  El timer 0 modo CTC controla velocidad avance y torque AR              *
/*  El timer 2 modo CTC controla velocidad avance y torque DEC             *
/*  El timer 1 modo CTC controla el torque, con OCR1A controla la AR y con *
/*  OCR1B controla la DEC                                                  *
/***************************************************************************/
#include "motor.h"
#include <math.h>
#include <avr/pgmspace.h>

// Codigos de cada paso en half step. Los guarda en ROM.
// Las bobinas se activan con CEROS '0'
static const char __attribute__ ((progmem)) stepTableDEC[10]={0x0A,0x0B,0x09,0x0D,0x05,0x07,0x06,0x0E,0x0A,0x0B};
static const char __attribute__ ((progmem)) stepTableAR[10] ={0xD0,0xC0,0xE0,0xA0,0xB0,0x30,0x70,0x50,0xD0,0xC0}; // Alreves



volatile signed char tStepAR;
volatile signed char tStepDEC;
volatile int presAR;  // Prescaler AR
volatile int RpresAR;  // Recarga Prescaler AR
volatile int presDEC; // Prescaler DEC
volatile int RpresDEC; // Recarga Prescaler DEC
volatile unsigned char track_AR=TRACK_OFF;
volatile unsigned long stepsAR;
volatile unsigned long stepsDEC;
volatile long lstm;
volatile unsigned char flag_sdtime=0;
volatile unsigned char flag_dctime=0;
volatile unsigned frecAR;
volatile unsigned frecDEC;
volatile int trq_AR=FREC2TOR(1),trq_DEC=FREC2TOR(1);
volatile int frecAR_r=1,frecDEC_r=1;
volatile unsigned char tel_side=PRE_MER; // Antes del meridiano


/*------------------------------------------------*/
/* Rutina de Inicializacion del control del motor */
/*------------------------------------------------*/
void motor_init (void)
{
 wMOT_SIG=(pgm_read_byte(&stepTableDEC[0]))|(pgm_read_byte(&stepTableAR[0])); //Fijo en UN ESTADO
 dMOT_SIG=0xFF; // Todos como salida
 set_bit(bAR_EN,wMOT_ENA);
 set_bit(bAR_EN,dMOT_ENA);
 set_bit(bDEC_EN,wMOT_ENA);
 set_bit(bDEC_EN,dMOT_ENA);
 // Timer 0 - AR
 TIFR0  = 0x00;
 TCCR0B = (1<<CS00) | (1<<CS01); // Arranca el timer, prescaler 64
 RpresAR=0;
 OCR0A = TIMSID;
 TCCR0A = (1<<WGM01); // CTC mode Timer 0
 TIMSK0 = (1<<OCIE0A); // Interrupciones habilitadas

 // Timer 2 - DEC
 TIFR2  = 0x00;
 TCCR2A = (1<<WGM21); // CTC mode Timer 0
 TIMSK2 = (1<<OCIE2A); // Interrupciones habilitadas
 TCCR2B = 0;
 OCR2A = TIMSID;

 // Timer 1 - Control de torque
 // Frecuencia de PWM = 19531,25 Hz
 // Modo Fast PWM
 TCNT1 = 0;
 OCR1A = FREC2TOR(0);
 OCR1B = FREC2TOR(0);
 TCCR1A = (1<<WGM11)|(1<<WGM10); // Timer en 10 bit
 TCCR1B = (1<<WGM12)|(1<<CS10); // Prescaler en 1
 TIMSK1 = (1<<OCIE1A)| (1<<OCIE1B)| (1<<TOIE1);
}

/*--------------------------------------------------------------*/
/* Rutina de Movimiento del motor (para busqueda y seguimiento) */
/*--------------------------------------------------------------*/
void motor_goto(unsigned char speed, // Velocidad de seguimiento, la de busqueda es cte
				unsigned char track, // Seguimiento
				c_eqs coord) // Coordenadas)
{
 long st_AR,st_DEC;
 float err_dec,err_ra, aux_ra;
 // Calculo el error al objeto deseado
 err_dec = (convert_dec2fs(coord.dec))-ftpos_dec;
 err_ra = (convert_ra2fs(coord.ra))-ftpos_ra;
 track_AR=track;
 // Ajuste del salto 0h->23h => Busca la minima distancia
 // El calculo se tiene en cuenta para la estrategia, aunque su resultado no se usa.
 aux_ra = err_ra;
 if (aux_ra>(LIM_ARM)) // 12h
	 aux_ra=aux_ra-(LIM_ARM*2); //-24h
 if (aux_ra<(-LIM_ARM))
	 aux_ra=aux_ra+(LIM_ARM*2); //+24h

 // Calculo para el cambio de estrategia
 st_AR = (aux_ra/CTE_AR_SG);
 if (((st_AR+lstm)>((long)LIM_LSTM)) || ((st_AR+lstm)<((long)(-(LIM_LSTM))))) // Cambia el sentido de busqueda
	{ // Debe recalcular los errores
	 if  (((ftpos_dec==(-LIM_DECM))||(ftpos_dec==(LIM_DECM)))) // Si esta en el POLO
		{
		 if (tel_side==POST_MER)// Cambia a PRE_MER
			{
			 ftpos_ra = ftpos_ra - LIM_ARM; // Lo pasa del otro lado del meridiano
			 if (ftpos_ra < 0.0)
				ftpos_ra+=LIM_ARM*2;
			 tel_side=PRE_MER;
			 err_ra = err_ra + LIM_ARM;
			}
		 else
			{
			 ftpos_ra = ftpos_ra + LIM_ARM; // Lo pasa del otro lado del meridiano
			 if (ftpos_ra > (LIM_ARM*2))
				ftpos_ra+=LIM_ARM*2;
			 tel_side=POST_MER;
			 err_ra = err_ra - LIM_ARM;
			}
		 if (err_ra>(LIM_ARM)) // 12h
			 err_ra=err_ra-(LIM_ARM*2); //-24h
		 if (err_ra<(-LIM_ARM))
			 err_ra=err_ra+(LIM_ARM*2); //+24h
		}
	 else
		{ // No está en el polo
		 err_dec= - err_dec - 2*(LIM_DECM-fabs(ftpos_dec));// Esto vale para el polo sur
		 if (err_ra<0)
			 err_ra = err_ra + LIM_ARM;
		 else
			 err_ra = err_ra - LIM_ARM;
		}
	}
 else
	 err_ra = aux_ra; // Se recupera lo que se haya calculado antes

// Si pide una posicion distinta de RA la calcula
 if ((err_ra>=CTE_AR_SG)||(err_ra<=-CTE_AR_SG))
	{
	 st_AR = (err_ra/CTE_AR_SG);
	 // Pregunta si pasa el meridiano => es necesario cambiar el mecanismo
	 if (st_AR<0)
		 motor_ARrun(-speed, -(st_AR));
	 else
		 motor_ARrun(speed, (st_AR));
	}
 else
	{
	 if (track)
		 motor_ARrun(-1, 0); // Activa el seguimiento
	}

 if ((err_dec>=CTE_DEC_SG)||(err_dec<=-CTE_DEC_SG))
	{
	 st_DEC = (err_dec/CTE_DEC_SG);
	 if (st_DEC<0)
		 motor_DECrun(-speed, -(st_DEC));
	 else
		 motor_DECrun(speed, (st_DEC));
	}
}

/*------------------------------------------------*/
/* Rutina de Movimiento del motor                 */
/*------------------------------------------------*/
// Activa a un motor determinado a velocidad fija
// Tipos de activacion
// steps = 0 => Giro Continuo
// frec = 0 => Detiene al motor (fmax=n veces velocidad siderea)
void motor_ARrun (int frec, long steps)
{
 tStepAR=1;
 if (frec<0)
	{
	 frec =-frec;
	 tStepAR=-1;
	}
 if (frec!=0) // = Stop
	{
	 if (steps>MAX_VEL*2)
		frecAR=frec;
	 else
		frecAR=steps/2;
	if (steps!=0)
		{
		 trq_AR=FREC2TOR(1); // Ajusta el torque segun la frecuencia
		 RpresAR = FREC2TIM(1); // calcula el tiempo maximo
		}
	else
		{
		 trq_AR=FREC2TOR(frec); // Ajusta el torque segun la frecuencia
		 RpresAR = FREC2TIM(frec); // calcula el tiempo maximo
		}
	 presAR =0;
	 TCNT0 = 0; // Reinicia el timer
	 stepsAR=steps;
	}
 else
	{
	 trq_AR = FREC2TOR(0); // Cuando esta detenido deja una baja tension
	 stepsAR = 0;
	 RpresAR=0;
	 frecAR_r=0;
	}
}

void motor_DECrun (int frec, long steps)
{
 tStepDEC=1;
 if (frec<0)
	{
	 frec =-frec;
	 tStepDEC=-1;
	}
 if (tel_side==POST_MER)
	tStepDEC=-tStepDEC;
 if (frec!=0) // = Stop
	{
	 if ((steps>MAX_VEL*2)||(steps==0))
		frecDEC=frec;
	 else
		frecDEC=steps/2;
	 if (steps!=0)
		{
		 trq_DEC=FREC2TOR(1); // la frecuencia va de 1 a 1000
		 RpresDEC = FREC2TIM(1); // calcula el tiempo maximo
		}
	 else
		{
		 trq_DEC=FREC2TOR(frec); // la frecuencia va de 1 a 1000
		 RpresDEC = FREC2TIM(frec); // calcula el tiempo maximo
		}
	 presDEC =0;
	 TCNT2 = 0; // Reinicia el timer
	 stepsDEC=steps;
	 TCCR2B = (1<<CS22); // Arranca el timer, prescaler 64
	}
 else
	{
	 trq_DEC = FREC2TOR(0); // Cuando esta detenido deja una baja tension
	 stepsDEC = 0;
	 TCCR2B =0;
	 frecDEC_r=0;
	}
}

/*------------------------------------------------*/
/* INTERRUPCION TIMER 0, MOTOR AR                 */
/*------------------------------------------------*/
// Controla el avance en el motor de AR
volatile signed char indexAR=0;
volatile signed char indexDEC=0;
ISR (TIMER0_COMPA_vect)
{
 flag_sdtime=~(flag_sdtime);
 if (RpresAR!=0)
	{
	 if (presAR==0)
		{
		 if (indexAR>7) // Se pasa de la secuencia maxima
			 indexAR=0;
		 wMOT_SIG=(pgm_read_byte(&stepTableAR[indexAR]))|(pgm_read_byte(&stepTableDEC[indexDEC]));
		 indexAR = indexAR + tStepAR;
		 if (indexAR < 0)
			 indexAR=7;
			 lstm += tStepAR;
		 if (stepsAR!=0) // Si es mov por pasos
			{
			 stepsAR--; // Aplica un paso
			 if (tStepAR>0)
				 ra_seg=ra_seg+CTE_AR_SG;
			 else
				 ra_seg=ra_seg-CTE_AR_SG;
			 if ((fabs(ra_seg))>=1.0)
				{
				 ftpos_ra=ftpos_ra+roundf(ra_seg);
				 ra_seg=ra_seg-roundf(ra_seg);
				}
			 if (stepsAR<frecAR)
				 frecAR_r=frecAR_r-1;
			 else
				{
				 if (frecAR_r<frecAR)
					 frecAR_r=frecAR_r+1; // Rampa de subida
				}
			 trq_AR=FREC2TOR(frecAR_r); // Ajusta el torque segun la frecuencia
			 RpresAR = FREC2TIM(frecAR_r); // calcula el tiempo maximo
			 if (stepsAR==0)
				{
				 trq_AR = FREC2TOR(1); // Como baja la velocidad, baja el torque
				 if (track_AR)
					{// Una vez que llego, activa el seguimiento
					 RpresAR=FREC2TIM(1);
					 tStepAR = -1; // Half Step, gira hacia coordenadas menores
					}
				 else
					RpresAR=0;
				}
			}
		 presAR=RpresAR;
		}
	 presAR=presAR-1;
	}
}

/*------------------------------------------------*/
/* INTERRUPCION TIMER 2, MOTOR DEC                */
/*------------------------------------------------*/
// Controla el avance en el motor de DEC
ISR (TIMER2_COMPA_vect)
{
 flag_dctime=~(flag_dctime);
 if (presDEC==0)
	{
	 if (indexDEC>7) // Se pasa de la secuencia maxima
		 indexDEC=0;
	 wMOT_SIG=(pgm_read_byte(&stepTableAR[indexAR]))|(pgm_read_byte(&stepTableDEC[indexDEC]));
	 indexDEC = indexDEC - tStepDEC;
	 if (indexDEC < 0)
		 indexDEC=7;
	 if (stepsDEC!=0) // Si es mov por pasos
		{
		 stepsDEC--; // Aplica un paso
		 if (((tel_side==PRE_MER)&&(tStepDEC>0))||((tel_side==POST_MER)&&(tStepDEC<0)))
			 dec_seg=dec_seg+CTE_DEC_SG;
		 else
			 dec_seg=dec_seg-CTE_DEC_SG;
		 if ((fabs(dec_seg))>=1.0)
			{
			 ftpos_dec=ftpos_dec+roundf(dec_seg);
			 dec_seg=dec_seg-roundf(dec_seg);
			}
		 if (((ftpos_dec==(-LIM_DECM))||(ftpos_dec==(LIM_DECM))))
			{
			 if (tel_side==POST_MER) // Pasa del POST al PRE_MER
				{
				 ftpos_ra=ftpos_ra-LIM_ARM;
				 tel_side=PRE_MER;
				}
			 else
				{
				 ftpos_ra=ftpos_ra+LIM_ARM;
				 tel_side=POST_MER;
				}
			}
		 if (stepsDEC<frecDEC)
			 frecDEC_r=frecDEC_r-1;
		 else
			{
			 if (frecDEC_r<frecDEC)
				 frecDEC_r=frecDEC_r+1; // Rampa de subida
			}
		 trq_DEC=FREC2TOR(frecDEC_r); // Ajusta el torque segun la frecuencia
		 RpresDEC = FREC2TIM(frecDEC_r); // calcula el tiempo maximo

		 if (stepsDEC==0)
			{
			 TCCR2B = 0; // Para el timer 2, no avanza mas.
			 trq_DEC=FREC2TOR(0);
			}
		}
	 presDEC=RpresDEC;
	}
 presDEC=presDEC-1;
}


// Control de torque
// Ascension Recta
ISR (TIMER1_COMPA_vect)
{
 wMOT_SIG=wMOT_SIG|mMOT_AR; // Apaga AR y deja solo DEC
}

// Declinacion
ISR (TIMER1_COMPB_vect)
{
 wMOT_SIG=wMOT_SIG|mMOT_DEC; // Apaga DEC y deja solo AR
}

// Restablece los valores, asi se reinicia el ciclo de modulacion PWM
ISR (TIMER1_OVF_vect)
{
 wMOT_SIG=(pgm_read_byte(&stepTableAR[indexAR]))|(pgm_read_byte(&stepTableDEC[indexDEC]));
 OCR1A=trq_AR;
 OCR1B=trq_DEC;
}

