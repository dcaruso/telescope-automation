// Header de calc.c

#include "config.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define DEGS  180/M_PI   // convert radians to degrees
#define RADS  M_PI/180   // convert degrees to radians
#define EPS   1.0e-5    // machine error constant

#define LIM_RA		6.0  // Limite de movimiento en AR
#define EXT_DEC		30.0 // Grados de declinacion agregados

#define OBJ_VIS 0	// flag que indica si el objeto es visible
#define OBJ_NVIS 1	// flag que indica si el objeto no es visible

#define NO_PARK		0xAA
#define PARKING		0x55

typedef struct  // Coordenadas en grados y ubicacion
{
 unsigned char d; // grados °
 unsigned char m; // minutos '
 char s; // Lado (N, S, E, O)
}c_geoi;


typedef struct
{
 unsigned char h; // Hours
 unsigned char m; // Minuts
 unsigned char s; // Seconds
 signed char hh; // Huso horario
} time;

typedef struct
{
 unsigned char y; // Year
 unsigned char m; // Month
 unsigned char d; // Day
} date;

typedef struct  // Coordenadas ecuatoriales
{
 float ra;   // Ascencion Recta
 float dec;  // Declinacion
 float rvec; // Distancia al objeto
}c_eq;

typedef struct // Objetos celestes
{
 c_eq c; // Coordenadas
 char name[10];
}obj_cel;

typedef struct // Objetos celestes
{
 signed char d;
 signed char m;
 signed char s;
}c_esfd;

typedef struct // Objetos celestes
{
 unsigned char h;
 unsigned char m;
 unsigned char s;
}c_esfra;

typedef struct // Objetos celestes
{
 c_esfra ra;
 c_esfd dec;
 char name[12];
}obj_ceqs;

typedef struct // Objetos celestes
{
 c_esfra ra;
 c_esfd dec;
}c_eqs;

typedef struct  // Elementos de la orbita
{
 float a; // semi-major axis [AU]
 float e; // eccentricity of orbit
 float i; // inclination of orbit [deg]
 float o; // longitude of the ascending node [deg]
 float w; // longitude of perihelion [deg]
 float l; // mean longitude [deg]
}orbit;

static const char __attribute__ ((progmem)) ssol_elem[10][9] = {{'L','U','N','A', 0 , 0 , 0 , 0 ,0},
															    {'M','E','R','C','U','R','I','O',0},
															    {'V','E','N','U','S', 0 , 0 , 0 ,0},
															    {'S','O','L', 0 , 0 , 0 ,0 , 0 , 0},
															    {'M','A','R','T','E', 0 , 0 , 0 ,0},
															    {'J','U','P','I','T','E','R', 0 ,0},
															    {'S','A','T','U','R','N','O', 0 ,0},
															    {'U','R','A','N','O', 0 , 0 , 0 ,0},
															    {'N','E','P','T','U','N','O', 0 ,0},
															    {'P','L','U','T','O','N', 0 , 0 ,0}};


#define NUMSTAR		20
static const obj_ceqs __attribute__ ((progmem)) list_star[NUMSTAR] = {
// Cercanas al polo Norte
 {.ra.h=14,.ra.m=15,.ra.s=39,.dec.d=+19,.dec.m=10,.dec.s=44,.name="Arturo     \0"},
 {.ra.h=18,.ra.m=36,.ra.s=56,.dec.d=+38,.dec.m=47,.dec.s=03,.name="Vega       \0"},
 {.ra.h=05,.ra.m=16,.ra.s=41,.dec.d=+45,.dec.m=59,.dec.s=50,.name="Capella    \0"},
 {.ra.h=07,.ra.m=39,.ra.s=18,.dec.d=+05,.dec.m=13,.dec.s=25,.name="Procion    \0"},
 {.ra.h=05,.ra.m=55,.ra.s=10,.dec.d=+07,.dec.m=24,.dec.s=26,.name="Betelgeuse \0"},
 {.ra.h=04,.ra.m=35,.ra.s=55,.dec.d=+16,.dec.m=30,.dec.s=32,.name="Aldebaran  \0"},
 {.ra.h=07,.ra.m=45,.ra.s=19,.dec.d=+28,.dec.m=01,.dec.s=32,.name="Pollux     \0"},
 {.ra.h=10,.ra.m=8,.ra.s=22,.dec.d=+11,.dec.m=58,.dec.s=3,.name="Regulo     \0"},
 {.ra.h=19,.ra.m=50,.ra.s=47,.dec.d=+8,.dec.m=52,.dec.s=8,.name="Altair     \0"},
 // Cercanas al polo Sur
 {.ra.h=6,.ra.m=45,.ra.s=9,.dec.d=-16,.dec.m=-43,.dec.s=-5,.name="Sirio      \0"},
 {.ra.h=5,.ra.m=14,.ra.s=32,.dec.d=-8,.dec.m=-12,.dec.s=-6,.name="Rigel      \0"},
 {.ra.h=01,.ra.m=37,.ra.s=43,.dec.d=-57,.dec.m=-14,.dec.s=-13,.name="Achernar   \0"},
 {.ra.h=14,.ra.m=03,.ra.s=49,.dec.d=-60,.dec.m=-22,.dec.s=-23,.name="Hadar      \0"},
 {.ra.h=12,.ra.m=26,.ra.s=36,.dec.d=-63,.dec.m=-05,.dec.s=-56,.name="Acrux      \0"},
 {.ra.h=16,.ra.m=29,.ra.s=25,.dec.d=-26,.dec.m=-25,.dec.s=-56,.name="Antares    \0"},
 {.ra.h=13,.ra.m=25,.ra.s=12,.dec.d=-11,.dec.m=-9, .dec.s=-41,.name="Spica      \0"},
 {.ra.h=22,.ra.m=57,.ra.s=39,.dec.d=-29,.dec.m=-37,.dec.s=-21,.name="Fomalhaut  \0"},
 {.ra.h=9,.ra.m=13,.ra.s=12,.dec.d=-69,.dec.m=-43,.dec.s=-3,.name="Miaplacidus\0"},
 {.ra.h=14,.ra.m=39,.ra.s=36,.dec.d=-60,.dec.m=-49,.dec.s=-23,.name="Alfa Cen   \0"},
 {.ra.h=6,.ra.m=23,.ra.s=57,.dec.d=-52,.dec.m=-41,.dec.s=-44,.name="Canopus    \0"}
};


// Prototipos de las funciones
float dm2real(c_geoi);
int abs_floor(float);
float mod2pi(float);
float true_anomaly(float, float);
void find_elem(orbit *, int, float);
float jd2000(void);
float jdfecha(void);
obj_ceqs get_coord(int); // (coor eq elemento, numero de planeta, dias J2000)
unsigned char vis_obj(c_esfra, c_esfd);
float local_sidtime(float);
c_eqs initial_pos(void);
c_eq geo2top (c_eq, float, float);
float c_geoi2deg (c_geoi);

// Para el calculo de la Luna
float PrimerGiro(float);
obj_ceqs get_moon(void);

c_esfd convert_dec(float);
c_esfra convert_ra(float);

// Variables globales
extern date fecha;
extern time t;
extern c_geoi lat, lon;
extern obj_ceqs objb;
extern c_eqs tpos;
extern int hmar;
extern volatile unsigned char park_flag;


