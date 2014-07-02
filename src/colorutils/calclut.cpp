#include "calclut.h"

/************************************** Calcul fausses couleurs ****************************************/

void calcul_fcol(short *lutR,short *lutG,short *lutB)
{
  int i;

  for(i=0;i<256;i++)
    {
      lutR[i]=(i*99) % 256;
      lutG[i]=(i*133) % 256;
      lutB[i]=(i*199) % 256;
    }

}

double DCalcFcol(int i)
{
  int r=0,g=0,b=0;

  r=(i*99) % 256;
  g=(i*133) % 256;
  b=(i*199) % 256;
  return CV_RGB_OLD(r,g,b);
}

CvScalar SCalcFcol(int i)
{
  int r=0,g=0,b=0;
  
  r = (i*99) % 256;
  g = (i*133) % 256;
  b = (i*199) % 256;
  return CV_RGB(r,g,b);
}

//Calcule l'indice correspondant a la couleur CvScalar
int CalcIndSFcol(CvScalar coul,int nbcoulmax)
{
  int i;
  CvScalar tmp;

  for(i=0;i<=nbcoulmax;i++)
    {
      tmp = SCalcFcol(i);
      if(ScalarEqual(tmp,coul)) return i;
    }
  return -1;
}

//Calcule l'indice correspondant a la couleur double
int CalcIndDFcol(double coul,int nbcoulmax)
{
  int i;
  for(i=0;i<=nbcoulmax;i++)
      if(DCalcFcol(i)==coul) return i;
  
  return -1;
}


/********************************************************************
 calcul des lut pour afficher la teinte
--------------------------------------------------------------------*/

double DCalcTeint(int i)
{
  int r=0,g=0,b=0;

  if((i>=0)&&(i<42))
    {
      r = 0;
      g = 252 - i * 6;
      b = 252;
    }
  if((i>=42)&&(i<84))
    {
      r = (i - 42) * 6;
      g = 0;
      b = 252;
    }
  if((i>=84)&&(i<126))
    {
      r = 252;
      g = 0;
      b = 252 - (i - 84) * 6;
    }
  if((i>=126)&&(i<168))
    {
      r = 252;
      g = (i - 126) * 6;
      b = 0;
    }
  if((i>=168)&&(i<210))
    {
      r = 252 - (i - 168) * 6;
      g = 252;
      b = 0;
    }
  if((i>=210)&&(i<253))
    {
        r = 0;
	g = 252;
        b = (i - 210) * 6;
    }
  if(i==253)
    {
      /* noir */
      r = 0;
      g = 0;
      b = 0;
    }
  if(i==254)
    {
      /* gris */
      r = 128;
      g = 128;
      b = 128;
    }
  if(i==255)
    {  
      /* blanc */
      r = 255;
      g = 255;
      b = 255;
    }

  return CV_RGB_OLD(r,g,b);
}

CvScalar SCalcTeint(int i)
{
  int r=0,g=0,b=0;

  if((i>=0)&&(i<42))
    {
      r = 0;
      g = 252 - i * 6;
      b = 252;
    }
  if((i>=42)&&(i<84))
    {
      r = (i - 42) * 6;
      g = 0;
      b = 252;
    }
  if((i>=84)&&(i<126))
    {
      r = 252;
      g = 0;
      b = 252 - (i - 84) * 6;
    }
  if((i>=126)&&(i<168))
    {
      r = 252;
      g = (i - 126) * 6;
      b = 0;
    }
  if((i>=168)&&(i<210))
    {
      r = 252 - (i - 168) * 6;
      g = 252;
      b = 0;
    }
  if((i>=210)&&(i<253))
    {
        r = 0;
	g = 252;
        b = (i - 210) * 6;
    }
  if(i==253)
    {
      /* noir */
      r = 0;
      g = 0;
      b = 0;
    }
  if(i==254)
    {
      /* gris */
      r = 128;
      g = 128;
      b = 128;
    }
  if(i==255)
    {  
      /* blanc */
      r = 255;
      g = 255;
      b = 255;
    }

  return CV_RGB(r,g,b);
} 

/** verifie l'egalitee de deux scalars **/
int ScalarEqual(CvScalar s1,CvScalar s2)
{
  //printf("Comparaison %f %f  %f %f  %f %f\n",s1.val[0],s2.val[0],s1.val[1],s2.val[1],s1.val[2],s2.val[2]);
  if((s1.val[0]!=s2.val[0])||(s1.val[1]!=s2.val[1])||(s1.val[2]!=s2.val[2])) return 0;
  else return 1;
}
