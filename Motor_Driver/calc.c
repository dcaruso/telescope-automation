/***************************************************************************/
/* Descripcion:                                                            *
/*  Contiene las funciones necesarias para realizar las conversiones de    *
/*  coordenadas del sistema ecuatorial                                     *
/***************************************************************************/

#include "calc.h"
#include <math.h>

// Convierte la ascensión recta de flotante a HH:MM:SS
c_esfra convert_f2ra(float x)
{
 c_esfra ra;
 x = x/3600.0;
 ra.h = floorf(x);
 x= 60.0*(x - ra.h);
 ra.m = floorf(x);
 x= 60.0*(x - ra.m);
 ra.s = floorf(x);

 return ra;
}

// Convierte la declinación de flotante a DD:MM:SS
c_esfd convert_f2dec(float x)
{
 c_esfd dec;
 x = x/3600.0;
 if (x<0)
	{
	 dec.d = -floorf(-x);
	 x= 60.0*(x - dec.d);
	 dec.m = -floorf(-x);
	}
 else
	{
	 dec.d = floorf(x);
	 x= 60.0*(x - dec.d);
	 dec.m = floorf(x);
	}
 x= 60.0*(x - dec.m);
 dec.s = floorf(x);
 return dec;
}

// Convierte la declinación de DD:MM:SS a flotante
float convert_dec2fs (c_esfd dec)
{
 float x;
 x = dec.d*3600.0+dec.m*60.0+dec.s*1.0;
 return x;
}

// Convierte la ascensión recta de HH:MM:SS a flotante
float convert_ra2fs (c_esfra ra)
{
 float x;
 x = ra.h*3600.0+ra.m*60.0+ra.s*1.0;
 return x;
}
