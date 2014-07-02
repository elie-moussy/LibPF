#include "shapedesc/shapeutils.h"

/*
  Fonction de calcul du rayon et angles entre un point de ref
  et une liste de points
*/
void CalcShape(CvPoint *ptref, CvPoint *pts, double *ray, double *angl, int nbpts)
{
  double raymax=0.0;
#ifndef NORMALIZE
  double anglmin=7.0;
#endif
  double anglesum=0.0;
  double dx,dy;
  

  memset(ray,0,sizeof(double)*nbpts);
  memset(angl,0,sizeof(double)*nbpts);

  /* Pour chaque point calculer le rayon et l'angle */
  for(int i=0;i<nbpts;i++)
    {
      dx = pts[i].x-ptref->x;
      dy = pts[i].y-ptref->y;

      /* Rayon */
      ray[i] = sqrt((dx*dx)+(dy*dy));
#ifndef NORMALIZE
      if(ray[i] > raymax) raymax=ray[i];
#else
      raymax+=ray[i];
#endif
      /* Angle */
      if(dx>0)
	{
	  //dx>0
	  if(dy<0)
	    {
	      //dx>0 && dy<0
	      angl[i] = atan(dy/dx)+(2*PI);
	    }
	  else
	    {
	      //dx>0 && dy>=0
	      angl[i] = atan(dy/dx);
	    }
	}
      else
	if(!dx)
	  {
	    //dx=0
	    if(dy<0)
	      {
		//dx=0 && dy<0
		angl[i] = TROISPISURDEUX;
	      }
	    else
	      {
		//dx=0 && dy>=0
		angl[i] = PISURDEUX;
	      }
	  }
	else
	  {
	    //dx<0
	    angl[i] = atan(dy/dx)+PI;
	  }
#ifndef NORMALIZE
      if(angl[i]<anglmin) anglmin = angl[i];
#else
      anglesum+=angl[i];
#endif
    }

  /* Normalisation du rayon et shift des angles */
  //double raynorm = 1.0/raymax;
  for(register int i=0;i<nbpts;i++)
    {
      ray[i]/=raymax;
#ifndef NORMALIZE
      angl[i]-=anglmin;
#else
     angl[i]/=anglesum;
#endif
    }
}


void CalcShape(CvPoint *ptref, CvPoint *pts, double *ray, double *angl, int nbpts, int *mask)
{
  double raymax=0.0;
#ifndef NORMALIZE
  double anglmin=7.0;
#endif
  double dx,dy;
  double anglesum=0.0;

  memset(ray,0,sizeof(double)*nbpts);
  memset(angl,0,sizeof(double)*nbpts);

  /* Pour chaque point calculer le rayon et l'angle */
  for(int i=0;i<nbpts;i++)
    {
      if(mask[i])
	{
	  dx = pts[i].x-ptref->x;
	  dy = pts[i].y-ptref->y;
	  
	  /* Rayon */
	  ray[i] = sqrt((dx*dx)+(dy*dy));
#ifndef NORMALIZE	
	  if(ray[i] > raymax) raymax=ray[i];
#else
	  raymax+=ray[i];
#endif
	  
	  /* Angle */
	  if(dx>0)
	    {
	      //dx>0
	      if(dy<0)
		{
		  //dx>0 && dy<0
		  angl[i] = atan(dy/dx)+(2*PI);
		}
	      else
		{
		  //dx>0 && dy>=0
		  angl[i] = atan(dy/dx);
		}
	    }
	  else
	    if(!dx)
	      {
		//dx=0
		if(dy<0)
		  {
		    //dx=0 && dy<0
		    angl[i] = TROISPISURDEUX;
		  }
		else
		  {
		    //dx=0 && dy>=0
		    angl[i] = PISURDEUX;
		  }
	      }
	    else
	      {
		//dx<0
		angl[i] = atan(dy/dx)+PI;
	      }
#ifndef NORMALIZE
	  if(angl[i]<anglmin) anglmin = angl[i];
#else
	  anglesum+=angl[i];
#endif
	}
    }

  /* Normalisation du rayon et shift des angles */
  //double raynorm = 1.0/raymax;
  for(register int i=0;i<nbpts;i++)
    {
      if(mask[i])
	{
	  ray[i]/=raymax;
#ifndef NORMALIZE
	  angl[i]-=anglmin;
#else
	  angl[i]/=anglesum;
#endif
	}
    }
}
