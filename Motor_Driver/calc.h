// Header de calc.c

#include "config.h"

#define DEGS  180/M_PI   // convert radians to degrees
#define RADS  M_PI/180   // convert degrees to radians
#define EPS   1.0e-5    // machine error constant

#define OBJ_VIS 0	// flag que indica si el objeto es visible
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
}c_eqs;


c_esfra convert_f2ra(float);
c_esfd convert_f2dec(float);
float convert_dec2fs (c_esfd);
float convert_ra2fs (c_esfra);

// Variables globales
extern volatile c_eqs tpos;

