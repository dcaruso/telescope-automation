// Header de motor.c

#include "config.h"
#include "calc.h"
#include <avr/pgmspace.h>

#define FULL_STEP	1
#define HALF_STEP	2
#define STEPS		200 // 360/1.8
#define M_AR		1
#define M_DEC		2

#define INIT_LSTM	0
#define LIM_LSTM	((6.0*3600.0)/CTE_AR_SG)
#define LIM_DECM	90.0*3600.0
#define LIM_ARM		12.0*3600.0

#define NO_CHG		0
#define WAIT_CHG	1

#define POST_MER	1 // AR+++
#define PRE_MER		0 // AR---
#define HSUR		1
#define HNOR		0

#define TRACK_ON	1
#define TRACK_OFF	0

#define CTE_AR_SG		0.505618095 //(precision 2^(-20)) Constante de Ascensión recta
#define CTE_DEC_SG		7.739287376 //(precision 2^(-20)) Constante de Declinación

#define MOTDEC_RUN	TCCR2B&((1<<CS20)|(1<<CS21)|(1<<CS22))
#define MOTAR_RUN	TCCR0B&((1<<CS00)|(1<<CS01)|(1<<CS02))

#define wrAR_ST(state)		(wMOT_SIG&mMOT_DEC)|(state)
#define wrDEC_ST(state)		(wMOT_SIG&mMOT_AR)|(state)

// PARA CRISTAL EXTERNO 20MHz

// La tierra gira 24hs de AR (360º=86400"), cada 23h 56' 4" (=86164")
// Si se mueve 0.956669047" de AR por 1 paso, el tiempo entre pasos (tp) sera:
// tp=0.956669047*86164/86400 = 0.954055923" (seg)
// Tomo una velocidad maxima de 2000x (donde x=vel siderea). Esto lo hago a paso completo
// La ecuacion de demora seria: td = (Ta+1) Pr Po / fc (donde fc=20MHz)
// Po=1000 Pr=64 Ta+1=149 step=2092
// Si Po = 1000 => td=t_sidereo para avances de 1/2 paso => vel=vel_siderea
// Si Po = 1 => td=t_sidereo/1000 , a 1 paso por ciclo => vel=vel_siderea*2000
// step=2092, significa que cada 2092 ciclos tengo que agregar 1/2 paso de deriva, a velocidad siderea
// Este 1/2 paso se agrega si me muevo a favor del giro terrestre y se resta si va en contra?
// Las interrupciones serán cada t= (Ta+1) * Pr /fc = 476.8 useg que son 9536 ciclos

#define TIMSID				148
#define STEPERR				2092

#define LIMC_SIDT			1000
#define FREC2TIM(frec)		(1000/frec)

#define MINTORQ				500
#define FREC2TOR(frec)		((frec<<1)+MINTORQ)

#define MAX_VEL				250
#define MIN_VEL				50
#define LMAX_VEL			250UL
#define LIM_STMOT			25L

// Prototipos de funciones
void motor_ARrun (int , long);
void motor_DECrun (int , long);
void motor_goto(unsigned char, unsigned char, c_eqs);
void motor_init(void);

extern volatile unsigned long stepsAR;
extern volatile unsigned long stepsDEC;
extern volatile float ftpos_dec,ftpos_ra;
extern volatile float ra_seg, dec_seg;
extern volatile unsigned char flag_sdtime;
extern volatile unsigned char flag_dctime;
extern volatile unsigned char track_AR;
extern volatile signed char tStepAR;
extern volatile unsigned frecAR;
extern volatile unsigned frecDEC;
extern volatile long lstm;
extern volatile int RpresAR;  // Recarga Prescaler AR
extern volatile int RpresDEC; // Recarga Prescaler DEC
extern volatile unsigned char tel_side; // Antes del meridiano
extern volatile unsigned char hemisf;

