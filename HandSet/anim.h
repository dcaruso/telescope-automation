// Header de anim.c

#include "config.h"

#define ANIM_LOADING1	0x01
#define ANIM_LOADING2	0x02
#define ANIM_LOADING3	0x03
#define ANIM_UTN		0x04

typedef struct  // Coordenadas en grados y ubicacion
{
 unsigned char name; // Nombre de la animacion
 unsigned char state; // Estado
 unsigned char xpos; // Posicion X
 unsigned char ypos; // Posicion X 
}s_anim;

void start_anim (unsigned char, unsigned char, unsigned, unsigned char);

void stop_anim (void);

