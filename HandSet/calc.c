/***************************************************************************/
/* Descripcion:                                                            */
/*  Contiene las rutinas necesarias para el calculo de la posicion de los  */
/*  planetas en el AVR atmega128.                                          */
/***************************************************************************/

#include "calc.h"
#include "util.h"

//-------------
//* Funciones *
//-------------
// convert angle (deg, min) to degrees as real
float dm2real(c_geoi c)
{
 float rv;
 if (c.d < 0)
   rv = c.d - 1.0*c.m/60;
 else
   rv = c.d + 1.0*c.m/60;
 return rv;
}

// return the integer part of a number
int abs_floor(float x)
{
 int r;
 if (x>=0.0)
    r = floor(x);
 else
    r = ceil(x);
 return r;
}

// return an angle in the range 0 to 2pi radians
float mod2pi(float x)
{
 float b = x/(2*M_PI);
 float a = (2*M_PI)*(b - abs_floor(b));
 if (a < 0)
    a = (2*M_PI) + a;
 return a;
}

// compute the true anomaly from mean anomaly using iteration
//  M - mean anomaly in radians
//  e - orbit eccentricity
float true_anomaly(float M, float e)
{
 float V, E1;

 // initial approximation of eccentric anomaly
 float E;
 E = M + e*sin(M)*(1.0 + e*cos(M));
 do                                   // iterate to improve accuracy
   {
    E1 = E;
    E = E1-(E1-e*sin(E1)-M)/(1-e*cos(E1));
   }
 while (fabs(E-E1)>EPS); // Debe ser EPS

 // convert eccentric anomaly to true anomaly
 V = 2*atan(sqrt((1 + e)/(1 - e))*tan(0.5*E));

 if (V<0)
    V = V + (2*M_PI);      // modulo 2pi

 return V;
}


// Busca el planeta y carga sus elementos
// i= numero de planeta
// d= dias respecto a J2000
// p= structura orbit del planeta en cuestion
void find_elem(orbit *p, int i, float d)
{
 float siglo = d/36525; // centuries since J2000

 switch (i)
   {
    case 0: // Mercury
       p->a = 0.38709893 + 0.00000066*siglo;
       p->e = 0.20563069 + 0.00002527*siglo;
       p->i = ( 7.00487  -  23.51*siglo/3600)*RADS;
       p->o = (48.33167  - 446.30*siglo/3600)*RADS;
       p->w = (77.45645  + 573.57*siglo/3600)*RADS;
       p->l = mod2pi((252.25084 + 538101628.29*siglo/3600)*RADS);
       break;
    case 1: // Venus
       p->a = 0.72333199 + 0.00000092*siglo;
       p->e = 0.00677323 - 0.00004938*siglo;
       p->i = (  3.39471 -   2.86*siglo/3600)*RADS;
       p->o = ( 76.68069 - 996.89*siglo/3600)*RADS;
       p->w = (131.53298 - 108.80*siglo/3600)*RADS;
       p->l = mod2pi((181.97973 + 210664136.06*siglo/3600)*RADS);
       break;
    case 2: // Earth/Sun
       p->a = 1.00000011 - 0.00000005*siglo;
       p->e = 0.01671022 - 0.00003804*siglo;
       p->i = (  0.00005 -    46.94*siglo/3600)*RADS;
       p->o = (-11.26064 - 18228.25*siglo/3600)*RADS;
       p->w = (102.94719 +  1198.28*siglo/3600)*RADS;
       p->l = mod2pi((100.46435 + 129597740.63*siglo/3600)*RADS);
       break;
    case 3: // Mars
       p->a = 1.52366231 - 0.00007221*siglo;
       p->e = 0.09341233 + 0.00011902*siglo;
       p->i = (  1.85061 -   25.47*siglo/3600)*RADS;
       p->o = ( 49.57854 - 1020.19*siglo/3600)*RADS;
       p->w = (336.04084 + 1560.78*siglo/3600)*RADS;
       p->l = mod2pi((355.45332 + (68905103.78*siglo)/3600)*RADS);
       break;
    case 4: // Jupiter
       p->a = 5.20336301 + 0.00060737*siglo;
       p->e = 0.04839266 - 0.00012880*siglo;
       p->i = (  1.30530 -    4.15*siglo/3600)*RADS;
       p->o = (100.55615 + 1217.17*siglo/3600)*RADS;
       p->w = ( 14.75385 +  839.93*siglo/3600)*RADS;
       p->l = mod2pi((34.40438 + 10925078.35*siglo/3600)*RADS);
       break;
    case 5: // Saturn
       p->a = 9.53707032 - 0.00301530*siglo;
       p->e = 0.05415060 - 0.00036762*siglo;
       p->i = (  2.48446 +    6.11*siglo/3600)*RADS;
       p->o = (113.71504 - 1591.05*siglo/3600)*RADS;
       p->w = ( 92.43194 - 1948.89*siglo/3600)*RADS;
       p->l = mod2pi((49.94432 + 4401052.95*siglo/3600)*RADS);
       break;
    case 6: // Uranus
       p->a = 19.19126393 + 0.00152025*siglo;
       p->e =  0.04716771 - 0.00019150*siglo;
       p->i = (  0.76986  -    2.09*siglo/3600)*RADS;
       p->o = ( 74.22988  - 1681.40*siglo/3600)*RADS;
       p->w = (170.96424  + 1312.56*siglo/3600)*RADS;
       p->l = mod2pi((313.23218 + 1542547.79*siglo/3600)*RADS);
       break;
    case 7: // Neptune
       p->a = 30.06896348 - 0.00125196*siglo;
       p->e =  0.00858587 + 0.00002510*siglo;
       p->i = (  1.76917  -   3.64*siglo/3600)*RADS;
       p->o = (131.72169  - 151.25*siglo/3600)*RADS;
       p->w = ( 44.97135  - 844.43*siglo/3600)*RADS;
       p->l = mod2pi((304.88003 + 786449.21*siglo/3600)*RADS);
       break;
    case 8: // Pluto
       p->a = 39.48168677 - 0.00076912*siglo;
       p->e =  0.24880766 + 0.00006465*siglo;
       p->i = ( 17.14175  +  11.07*siglo/3600)*RADS;
       p->o = (110.30347  -  37.33*siglo/3600)*RADS;
       p->w = (224.06676  - 132.25*siglo/3600)*RADS;
       p->l = mod2pi((238.92881 + 522747.90*siglo/3600)*RADS);
       break;
   }
}


// Numero de dias desde J2000 (Jan 1.5, 2000)
float jd2000()
{
 float h= t.h+(t.m *(1.0/60))-t.hh+(t.m *(1.0/3600));
 return 367.0*(fecha.y+2000)
       - floor(7*((fecha.y+2000) + floor((fecha.m + 9)/12))/4)
       + floor(275*fecha.m/9) + fecha.d
       - 730531.5
       + h/24;
}

// compute RA, DEC, and distance of planet-p for day number-d
// result returned in structure obj in degrees and astronomical units
obj_ceqs get_coord(int pn) // (coor eq elemento, numero de planeta, dias J2000)
{
 orbit p;
 orbit e;
 obj_ceqs obj;
 float me, ve, re;
 float xe,ye,ze;
 float xeq,yeq,zeq;
 float mp, vp, rp;
 float xh,yh,zh;
 float xg,yg,zg;
 float ecl;
 float d=jd2000();
 find_elem(&p, pn, d);

 find_elem(&e, 2, d);

 // position of Earth in its orbit
 me = mod2pi(e.l - e.w);
 ve = true_anomaly(me, e.e);
 re = e.a*(1 - e.e*e.e)/(1 + e.e*cos(ve));

 // heliocentric rectangular coordinates of Earth
 xe = re*cos(ve + e.w);
 ye = re*sin(ve + e.w);
 ze = 0.0;

 // position of planet in its orbit
 mp = mod2pi(p.l - p.w);
 vp = true_anomaly(mp, p.e);
 rp = p.a*(1 - p.e*p.e)/(1 + p.e*cos(vp));

 // heliocentric rectangular coordinates of planet
 xh = rp*(cos(p.o)*cos(vp + p.w - p.o) - sin(p.o)*sin(vp + p.w - p.o)*cos(p.i));
 yh = rp*(sin(p.o)*cos(vp + p.w - p.o) + cos(p.o)*sin(vp + p.w - p.o)*cos(p.i));
 zh = rp*(sin(vp + p.w - p.o)*sin(p.i));

 if (pn == 2) // earth --> compute sun
 {
  xh = 0;
  yh = 0;
  zh = 0;
 }

 // convert to geocentric rectangular coordinates
 xg = xh - xe;
 yg = yh - ye;
 zg = zh - ze;

 // rotate around x axis from ecliptic to equatorial coords
 ecl = 23.439281*RADS;            //value for J2000.0 frame
 xeq = xg;
 yeq = yg*cos(ecl) - zg*sin(ecl);
 zeq = yg*sin(ecl) + zg*cos(ecl);

 // find the RA and DEC from the rectangular equatorial coords
 obj.ra   = convert_ra(mod2pi(atan2(yeq, xeq))*12/M_PI);
 obj.dec  = convert_dec(atan(zeq/sqrt(xeq*xeq + yeq*yeq))*DEGS);
 strcpy_Pe (obj.name,ssol_elem[pn+1]);
 return obj;
}

/// Calculo de la LUNA
float PrimerGiro (float x)
{
 x = x - 360 * floor (x/360);
 return x;
}


float c_geoi2deg (c_geoi l)
{
 float x = l.d + 1.0*l.m /60;
 if ((l.s == 'S') || (l.s == 'O'))
	 x=-x;
 return x;
}

float local_sidtime (float T)
{
float DD;
float B, L;// RA, DC;
float ST, SA;
float HR, SS;

//Latitud//

B = c_geoi2deg(lat);

//Longitud//
L = c_geoi2deg(lon);
//fecha juliana//
HR = t.h-t.hh + 1.0*t.m/60.0 + (t.s *(1.0/3600.0));
DD=fecha.d+HR/24;
SS= 6.6460656 + 2400.051*T +0.00002581*T*T;
//Tiempo sidéreo a Greenwich//
ST =(SS/24-floor(SS/24))*24.0;

//tiempo sidéreo a local//
SA=ST+(DD-floor(DD))*24.0*1.002737908;
SA=SA+L/15;
if (SA<0)
	SA=SA+24;
if (SA>=24)
	SA=SA-24;
//printf ("\n LST=%f\n",SA);
return SA;
}

// Corrector por paralaje
c_eq geo2top (c_eq elm, float P, float T)
{
c_eq topelm;
float B;
float ST, SA, H;
float DY, SS, CA, D, R, P1, P2, U;

//Latitud//
B=dm2real(lat);
if (lat.s=='S')
	B=-B;
SA = local_sidtime(T);
//Angulo horario//
H=SA-elm.ra;
if (H < 0)
	H = H + 24;

//correccion por paralaje//
B=B*M_PI/180;
U =atan(.996647 * tan(B));
P1 = .996647 * sin(U) + hmar * sin(B) / 6378140;
P2 = cos(U) + hmar * cos(B) / 6378140;
P=P*M_PI/180;
R = 1 / sin(P);
CA = atan(P2 * sin(H*M_PI/12) / (R * cos(elm.dec) - P2 * cos(H*M_PI/12)));
CA = CA * 12 / M_PI;
topelm.ra = elm.ra - CA;
topelm.dec = DEGS*(atan(cos((H+CA)*M_PI/12)*(R *sin(elm.dec)-P1)/(R*cos(elm.dec)*cos(H*M_PI/12)-P2)));

return topelm;
}

float jdfecha ()
{
 float HR = t.h-t.hh;
 float YY = fecha.y+2000;
 float JD;
 float J1;
 signed char S,A;

 HR = HR + (t.m *(1.0/60)) +(t.s *(1.0/3600));
 JD = (-1) * floor(7 * (floor((fecha.m + 9) / 12) + YY) / 4);
 S = 1;
 if ((fecha.m - 9)<0) S=-1;
	A = abs(fecha.m - 9);

 J1 = floor(YY + S * floor(A / 7));
 J1 = -1 * floor((floor(J1 / 100) + 1) * 3 / 4);
 JD = JD + floor(275 * fecha.m / 9) + fecha.d + J1
- 693991.5  + 367 * YY + (HR / 24);
 return JD;
}

obj_ceqs get_moon ()
{
 c_eq moon;
 obj_ceqs moon_r;

 float JD;
 float T,T2,T3;
 float L1,M,M1,D,F,OM,EX;
 float S;
 float L,B,W1,W2,BT,P,LM,R,Z,OB;

//	<!-- POSICION DE LA LUNA   -->
 JD = jdfecha();
 T = (JD) / 36525;
 T2 = T * T;
 T3 = T2 * T;
 L1 = 270.434164 + 481267.8831 * T - .001133 * T2 + .0000019 * T3;
 M = 358.475833 + 35999.0498 * T - .00015 * T2 - .0000033 * T3;
 M1 = 296.104608 + 477198.8491 * T + .009192 * T2 + .0000144 * T3;
 D = 350.737486 + 445267.1142 * T - .001436 * T2 + .0000019 * T3;
 F = 11.250889 + 483202.0251 * T - .003211 * T2 - .0000003 * T3;
 OM = 259.183275 - 1934.142 * T + .002078 * T2 + .0000022 * T3;
 OM = OM * RADS;

//	<!--TERMINOS ADITIVOS -->
 L1 = L1 + .000233 * sin((51.2 + 20.2 * T) * RADS);
 S = .003964 * sin((346.56 + 132.87 * T - .0091731 * T2) * RADS);
 L1 = L1 + S + .001964 * sin(OM);
 M = M - .001778 * sin((51.2 + 20.2 * T) * RADS);
 M1 = M1 + .000817 * sin((51.2 + 20.2 * T) * RADS);
 M1 = M1 + S + .002541 * sin(OM);
 D = D + .002011 * sin((51.2 + 20.2 * T) * RADS);
 D = D + S + .001964 * sin(OM);
 F = F + S - .024691 * sin(OM);
 F = F - .004328 * sin(OM + (275.05 - 2.3 * T) * RADS);
 EX = 1 - .002495 * T - .00000752 * T2;
 OM = OM *DEGS;
 L1 = PrimerGiro(L1);
 M = PrimerGiro(M);
 M1 = PrimerGiro(M1);
 D = PrimerGiro(D);
 F = PrimerGiro(F);
 OM = PrimerGiro(OM);
 M = M *RADS;
 M1 = M1 *RADS;
 D = D *RADS;
 F = F *RADS;

//<!--CALCULO DE LA LONGITUD-->
L=L1+ 6.28875 * sin(M1) + 1.274018 * sin(2 * D - M1) + .658309 * sin(2 * D)
	+ .213616 * sin(2 * M1) - EX * .185596 * sin(M) - .114336 * sin(2 * F)
	+ .058793 * sin(2 * D - 2 * M1) + EX * .057212 * sin(2 * D - M - M1) + .05332 * sin(2 * D + M1)
	+ EX * .045874 * sin(2 * D - M) + EX * .041024 * sin(M1 - M) - .034718 * sin(D)
	- EX * .030465 * sin(M + M1) + .015326 * sin(2 * D - 2 * F) - .012528 * sin(2 * F + M1)
	- .01098 * sin(2 * F - M1) + .010674 * sin(4 * D - M1) + .010034 * sin(3 * M1)
	+ .008548 * sin(4 * D - 2 * M1) - EX * .00791 * sin(M - M1 + 2 * D) - EX * .006783 * sin(2 * D + M)
	+ .005162 * sin(M1 - D) + EX * .005 * sin(M + D) + EX * .004049 * sin(M1 - M + 2 * D)
	+ .003996 * sin(2 * M1 + 2 * D) + .003862 * sin(4 * D) + .003665 * sin(2 * D - 3 * M1)
	+ EX * .002695 * sin(2 * M1 - M) + .002602 * sin(M1 - 2 * F - 2 * D) + EX * .002396 * sin(2 * D - M - 2 * M1)
	- .002349 * sin(M1 + D) + EX * EX * .002249 * sin(2 * D - 2 * M) - EX * .002125 * sin(2 * M1 + M)
	- EX * EX * .002079 * sin(2 * M) + EX * EX * .002059 * sin(2 * D - M1 - 2 * M) - .001773 * sin(M1 + 2 * D - 2 * F)
	+ EX * .00122 * sin(4 * D - M - M1) - .00111 * sin(2 * M1 + 2 * F) + .000892 * sin(M1 - 3 * D)
	- EX * .000811 * sin(M + M1 + 2 * D) + EX * .000761 * sin(4 * D - M - 2 * M1) + EX * EX*.000717 * sin(M1 - 2 * M)
	+ EX * EX * .000704 * sin(M1 - 2 * M - 2 * D) + EX * .000693 * sin(M - 2 * M1 + 2 * D) + EX * .000598 * sin(2 * D - M - 2 * F)
	+ .00055 * sin(M1 + 4 * D) + .000538 * sin(4 * M1) + EX * .000521 * sin(4 * D - M) + .000486 * sin(2 * M1 - D)
	- .001595 * sin(2 * F + 2 * D);

//<!--CALCULO DE LA LATITUD-->
B = 5.128189 * sin(F) + .280606 * sin(M1 + F) + .277693 * sin(M1 - F) + .173238 * sin(2 * D - F);
	+ .055413 * sin(2 * D + F - M1) + .046272 * sin(2 * D - F - M1) + .032573 * sin(2 * D + F);
	+ .017198 * sin(2 * M1 + F) + 9.266999E-03 * sin(2 * D + M1 - F) + .008823 * sin(2 * M1 - F);
	+ EX * .008247 * sin(2 * D - M - F) + .004323 * sin(2 * D - F - 2 * M1) + .0042 * sin(2 * D + F + M1);
	+ EX * .003372 * sin(F - M - 2 * D) + EX * .002472 * sin(2 * D + F - M - M1) + EX * .002222 * sin(2 * D + F - M);
	+ .002072 * sin(2 * D - F - M - M1) + EX * .001877 * sin(F - M + M1) + .001828 * sin(4 * D - F - M1);
	- EX * .001803 * sin(F + M) - .00175 * sin(3 * F) + EX * .00157 * sin(M1 - M - F) - .001487 * sin(F + D) - EX * .001481 * sin(F + M + M1)
	+ EX * .001417 * sin(F - M - M1) + EX * .00135 * sin(F - M) + .00133 * sin(F - D)
	+ .001106 * sin(F + 3 * M1) + .00102 * sin(4 * D - F) + .000833 * sin(F + 4 * D - M1)
	+ .000781 * sin(M1 - 3 * F) + .00067 * sin(F + 4 * D - 2 * M1) + .000606 * sin(2 * D - 3 * F)
	+ .000597 * sin(2 * D + 2 * M1 - F) + EX * .000492 * sin(2 * D + M1 - M - F) + .00045 * sin(2 * M1 - F - 2 * D)
	+ .000439 * sin(3 * M1 - F) + .000423 * sin(F + 2 * D + 2 * M1) + .000422 * sin(2 * D - F - 3 * M1)
	- EX * .000367 * sin(M + F + 2 * D - M1) - EX * .000353 * sin(M + F + 2 * D) + .000331 * sin(F + 4 * D)
	+ EX * .000317 * sin(2 * D + F - M + M1) + EX * EX * .000306 * sin(2 * D - 2 * M - F) - .000283 * sin(M1 + 3 * F);

W1 = .0004664 * cos(OM * RADS);
W2 = .0000754 * cos((OM + 275.05 - 2.3 * T) * RADS);
BT = B * (1 - W1 - W2);

//<!--CALCULO DEL PARALAJE-->
 P = .950724 + .051818 * cos(M1) + .009531 * cos(2 * D - M1) + .007843 * cos(2 * D) + .002824 * cos(2 * M1) + .000857 * cos(2 * D + M1)
	+ EX * .000533 * cos(2 * D - M) + EX * .000401 * cos(2 * D - M - M1) + .000173 * cos(3 * M1) + .000167 * cos(4 * D - M1) - EX * .000111 * cos(M)
	+ .000103 * cos(4 * D - 2 * M1) - .000084 * cos(2 * M1 - 2 * D) - EX * .000083 * cos(2 * D + M) + .000079 * cos(2 * D + 2 * M1)
	+ .000072 * cos(4 * D) + EX * .000064 * cos(2 * D - M + M1) - EX * .000063 * cos(2 * D + M - M1)
	+ EX * .000041 * cos(M + D) + EX * .000035 * cos(2 * M1 - M) - .000033 * cos(3 * M1 - 2 * D)
	- .00003 * cos(M1 + D) - .000029 * cos(2 * F - 2 * D) - EX * .000029 * cos(2 * M1 + M)
	+ EX * EX * .000026 * cos(2 * D - 2 * M) - .000023 * cos(2 * F - 2 * D + M1) + EX * .000019 * cos(4 * D - M - M1);

 B = BT * RADS;
 LM = L * RADS;
//	<!--transformacion  Eclipticas-Ecuatoriales-->
 Z = (JD - 0.5) / 365.2422;
 OB=23.452294 -Z*(.46845 + 5.9E-07 *Z)/3600;

 OB = OB * RADS;
 moon.dec =asin(sin(B) * cos(OB) + cos(B) * sin(OB) * sin(LM));
 moon.ra =acos(cos(B) * cos(LM) / cos(moon.dec));

 if (LM > M_PI)
	moon.ra = 2 * M_PI - moon.ra;

 moon.ra*=12/M_PI;
 moon=geo2top(moon,P,T);
 moon_r.ra = convert_ra(moon.ra);
 moon_r.dec= convert_dec(moon.dec);

 strcpy_Pe (moon_r.name,ssol_elem[0]);

 return moon_r;
}

unsigned char vis_obj(c_esfra ra, c_esfd dec)
{
float ra_aux, dec_aux, lat_aux;
float limit, calc_aux;
lat_aux = (c_geoi2deg(lat))*RADS;   //conversion grados a radianes
dec_aux = (1.0*dec.d+dec.m/60.0+dec.s/3600.0)*RADS; //conversion de grados a radianes
if ((cos(dec_aux))==0.0)
	{ // Desea apuntar al polo, solo es valido si la latitud del lugar lo permite
	if (((lat_aux<0)&&(dec_aux>0))||((lat_aux>0)&&(dec_aux<0)))
		return OBJ_NVIS; // Sólo toma por visibles a las de la media esfera circunpolar
	return OBJ_VIS; // Apunta al polo
	}
if (cos(lat_aux)==0.0) // Si esta en el polo
	{ // Si se encuentra en el polo, puede observar hasta dec 0º
	if (((lat_aux<0)&&(dec_aux>0))||((lat_aux>0)&&(dec_aux<0)))
		return OBJ_NVIS; // Sólo toma por visibles a las de la media esfera circunpolar
	return OBJ_VIS;
	}
calc_aux = (sin(dec_aux)*sin(dec_aux))/(cos(lat_aux)*cos(lat_aux));
if (calc_aux>=1.0) // Elementos circunpolares
	{// Vale solo si la latitud coincide en signo con la declinacion que quiere verse
	if (((lat_aux<0)&&(dec_aux>0))||((lat_aux>0)&&(dec_aux<0)))
		return OBJ_NVIS; // Sólo toma por visibles a las de la media esfera circunpolar
	return OBJ_VIS;
	}
ra_aux = fabs((1.0*ra.h+ra.m/60.0+ra.s/3600.0) - (local_sidtime((jdfecha()) / 36525)));// Toma desde el meridiano local
if (ra_aux>12.0)
	ra_aux=24.0-ra_aux;
ra_aux = (15*ra_aux)*RADS;  //convierte horas a radianes
limit = fabs(asin((sqrt(1-calc_aux))/(cos(dec_aux))));
if (((12.0*15*RADS)-limit)<ra_aux)
	return OBJ_NVIS;
else
	return OBJ_VIS;
}

// Transforma el angulo horario de grados a horas
c_esfra convert_ra(float x)
{
 c_esfra ra;
 ra.h = floorf(x);
 x= 60.0*(x - ra.h);
 ra.m = floorf(x);
 x= 60.0*(x - ra.m);
 ra.s = roundf(x);

 return ra;
}

c_esfd convert_dec(float x)
{
 c_esfd dec;
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
 dec.s = roundf(x);
 return dec;
}

c_eqs initial_pos (void)
{
 c_eqs p_ini;
 float lst;
 lst = local_sidtime(((jdfecha()) / 36525));
 p_ini.dec.m=0;p_ini.dec.s=0;
 if (lat.s=='S')
	{
	 lst = lst - 6.0;
	 p_ini.dec.d=-90;
	}
 else
	{
	 lst = lst + 6.0;
	 p_ini.dec.d=90;
	}
 if (lst<0.0)
	lst=lst+24.0;
 if (lst>=24.0)
	lst=lst-24.0;
 p_ini.ra = convert_ra(lst);
 return p_ini;
}


