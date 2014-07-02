/***************************************************************************
                          bspline2d.cpp  -  description
                             -------------------
    begin                : Thu Jul 25 2002
    copyright            : (C) 2002 by Paulo Menezes
    email                : pm@deec.uc.pt
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "bspline2d.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#define EPAISSEUR 1

BSpline2D::BSpline2D(int deg, int nknots, double * inknots, Point2D * pts)
{
  intnum=nknots-2;
  density=5;
  degree = deg;
  knot_nr=nknots;
  knots = new double[knot_nr];
  for (int i=0;i<knot_nr;i++)
    knots[i]=inknots[i];
  
  controlpts= new Point2D[knot_nr];
  //controlpts2= new Point2D[knot_nr];
  ctrlpts = new CvPoint[knot_nr];
  for (int i=0;i<knot_nr;i++){
    controlpts[i]=pts[i]; // copy the control points  
  }
  coeffx= new double[knot_nr];
  coeffy= new double[knot_nr]; 
  points = new Point2D[intnum*(density+1)];
  normals = new Vector2D[intnum*(density+1)];  
  numpoints=intnum*(density+1);
  
  this->pts = (CvPoint*)calloc(NBMAXPTS,sizeof(CvPoint));



  nbparts = 6;

  if(nbparts)
    {
      nbctrlpts_bypart = (int*)calloc(nbparts,sizeof(int));
      nbctrlpts_bypart[0] = 13;
      nbctrlpts_bypart[1] = 6;
      nbctrlpts_bypart[2] = 5;
      nbctrlpts_bypart[3] = 4;
      nbctrlpts_bypart[4] = 5;
      nbctrlpts_bypart[5] = 3;
      
      numctrlpts_bypart = (int **)calloc(nbparts,sizeof(int*));
      for(int i=0;i<nbparts;i++)
	numctrlpts_bypart[i] = (int*)calloc(nbctrlpts_bypart[i],sizeof(int));
      
      //numctrlpts_bypart[0] = {0,7,8,14,15,20,21,27,28,32,33,34,35};
      //numctrlpts_bypart[1] = {1,2,3,4,5,6};
      //numctrlpts_bypart[2] = {9,10,11,12,13};
      //numctrlpts_bypart[3] = {16,17,18,19};
      //numctrlpts_bypart[4] = {22,23,24,25,26};
      //numctrlpts_bypart[5] = {29,30,31};


    }

}

BSpline2D::~BSpline2D(){
}

/** No descriptions */
void BSpline2D::genpoints()
{
  int i,ii,l2;
  float u;
  
  l2=intnum+degree-1;
  numpoints=0;

  for (i=degree-1; i<l2; i++)
    {
      if(knots[i+1]>knots[i])
	for(ii=0; ii<=density; ii++)
	  {
	    u=knots[i]+ii*(knots[i+1]-knots[i])/density;
	    points[numpoints]=deboor2D(u,i);
	    numpoints+=1;
	  }
    }

  //Generation des normales aux segments
  gennormals();
}


/** For each of the generated points it creates a vector that is
    normal to the spline   */ 
void BSpline2D::gennormals()
{
  Vector2D v;
  for (int i=1;i<numpoints-1;i++)
    {
      v = points[i+1] - points[i-1];
      normals[i] = v.normal();
      //printf("Point %d  vecteur = %f %f  normale = %f %f\n",i,v.myx,v.myy,normals[i].myx,normals[i].myy);
    }
  v=points[1]-points[0];
  normals[0]=v.normal();
  v=points[numpoints-1]-points[numpoints-2];
  normals[numpoints-1]=v.normal();
  
}

/** this method generates new control points translating, rotating and
    scaling the base ones  */ 
void BSpline2D::transform(double x, double y, double theta, double scale)
{
  Point2D tmp;

  //Translation  
  Point2D transl(x,y);

  Posmin.x = 1000;
  Posmin.y = 1000;
  Posmax.x = -1;
  Posmax.y = -1;
  //int j=0;
  for (int i=0;i<knot_nr;i++)
    {
      //if( (i<22)||(i>26) )//&&(i<29))||(i>31) )
	//{
	  tmp=controlpts[i];
	  //mise a l'echelle
	  tmp=scale*tmp;
	  //rotation
	  if (theta!=0.0)
	    tmp.rotate(theta);
	  
	  //controlpts2[i]=tmp+transl;
	  ctrlpts[i].x=((int)(tmp.myx+transl.myx));
	  ctrlpts[i].y=((int)(tmp.myy+transl.myy));
	  
	  //Recup des min et max
	  if(ctrlpts[i].x<Posmin.x) Posmin.x=ctrlpts[i].x;
	  else
	    if(ctrlpts[i].x>Posmax.x) Posmax.x=ctrlpts[i].x;
	  if(ctrlpts[i].y<Posmin.y) Posmin.y=ctrlpts[i].y;
	  else
	    if(ctrlpts[i].y>Posmax.y) Posmax.y=ctrlpts[i].y;
	  
	  //  controlpts2[i]=controlpts[i];       // virer ça
	  //j++;
	  //	}
    }

//intnum=j-2;

  //Generation des points a partir des points de controle
  genpoints();
}

/** Draws the spline with the current transformation on the image */ 
void BSpline2D::draw(IplImage * img,CvScalar color, int fill)
{

  if(fill)
    {
      //Trace rempli
      for (int i=0;i<numpoints;i++)
	{
	  pts[i].x = (int)points[i].myx;
	  pts[i].y = (int)points[i].myy;
	}      
      cvFillPoly(img,&pts,&numpoints,1,color,8,0 );
    }
  else
    {
      //Trace ligne
      for (int i=1;i<numpoints;i++)
	{
    //cvLine(img,cvPoint(cvRound(points[i-1].myx),cvRound(points[i-1].myy)),cvPoint(cvRound(points[i].myx),cvRound(points[i].myy)),color,2);
	  cvLine(img,cvPoint((int)points[i-1].myx,(int)points[i-1].myy),cvPoint((int)points[i].myx,(int)points[i].myy),color,EPAISSEUR);
	}
    } 
  
}

/** Draw normals */
void BSpline2D::draw_normals(IplImage * img,int size,CvScalar color)
{
  Point2D p1,p2;

  for(int i=0;i<numpoints;i++)
    {
      p1 = points[i]+(normals[i]*size);
      p2 = points[i]-(normals[i]*size);

      cvLine(img,cvPoint((int)p1.myx,(int)p1.myy),cvPoint((int)p2.myx,(int)p2.myy),color,EPAISSEUR,8,0);
      //cvLine(img,cvPoint((int)points[i].myx,(int)points[i].myy),cvPoint((int)p1.myx,(int)p1.myy),CV_RGB(0,0,255),2,8,0);
      //cvLine(img,cvPoint((int)points[i].myx,(int)points[i].myy),cvPoint((int)p2.myx,(int)p2.myy),CV_RGB(255,0,0),2,8,0);
    }



}

/* Trace les points de controle */
void BSpline2D::draw_ctrlpoints(IplImage * img,CvScalar color)
{
  /* Trace des Points de Controle */
  for (int i=0;i<knot_nr;i++)
    {
      cvCircle(img,ctrlpts[i],2,color,-1,8,0);
    }
}

/* Trace les zones de la main */
void BSpline2D::draw_parts(IplImage * img)
{ 
  int nb;

  nb=4;

  //Paume
  pts[0] = ctrlpts[0];
  pts[1] = ctrlpts[8];
  pts[2] = ctrlpts[32];
  pts[3] = ctrlpts[35];
  cvFillPoly(img,&pts,&nb,1,SCalcFcol(2),8,0 );

  nb = 5;

  //Pouce
  pts[0] = ctrlpts[0];
  pts[1] = ctrlpts[3];
  pts[2] = ctrlpts[4];
  pts[3] = ctrlpts[5];
  pts[4] = ctrlpts[7];
  cvFillPoly(img,&pts,&nb,1,SCalcFcol(1),8,0 );

  //Index
  pts[0] = ctrlpts[8];
  pts[1] = ctrlpts[10];
  pts[2] = ctrlpts[11];
  pts[3] = ctrlpts[12];
  pts[4] = ctrlpts[14];
  cvFillPoly(img,&pts,&nb,1,SCalcFcol(3),8,0 );

  //Majeur
  pts[0] = ctrlpts[15];
  pts[1] = ctrlpts[17];
  pts[2] = ctrlpts[18];
  pts[3] = ctrlpts[19];
  pts[4] = ctrlpts[20];
  cvFillPoly(img,&pts,&nb,1,SCalcFcol(4),8,0 );

  //Anulaire
  pts[0] = ctrlpts[21];
  pts[1] = ctrlpts[23];
  pts[2] = ctrlpts[24];
  pts[3] = ctrlpts[25];
  pts[4] = ctrlpts[27];
  cvFillPoly(img,&pts,&nb,1,SCalcFcol(5),8,0 );

  //Oriculaire
  pts[0] = ctrlpts[28];
  pts[1] = ctrlpts[29];
  pts[2] = ctrlpts[30];
  pts[3] = ctrlpts[31];
  pts[4] = ctrlpts[32];
  cvFillPoly(img,&pts,&nb,1,SCalcFcol(6),8,0 );


  cvRectangle(img, Posmin, Posmax, SCalcFcol(7),EPAISSEUR,8,0);


}



/* Trace les zones de la main */
void BSpline2D::draw_mask(IplImage * img)
{ 
  int nb;

  //RAZ zone de la main
  cvRectangle(img, Posmin, Posmax, cvScalar(0),EPAISSEUR,8,0);


  nb=4;

  //Paume
  pts[0] = ctrlpts[0];
  pts[1] = ctrlpts[8];
  pts[2] = ctrlpts[32];
  pts[3] = ctrlpts[35];
  cvFillPoly(img,&pts,&nb,1,cvScalar(1),8,0 );

  nb = 5;

  //Pouce
  pts[0] = ctrlpts[0];
  pts[1] = ctrlpts[3];
  pts[2] = ctrlpts[4];
  pts[3] = ctrlpts[5];
  pts[4] = ctrlpts[7];
  cvFillPoly(img,&pts,&nb,1,cvScalar(2),8,0 );

  //Index
  pts[0] = ctrlpts[8];
  pts[1] = ctrlpts[10];
  pts[2] = ctrlpts[11];
  pts[3] = ctrlpts[12];
  pts[4] = ctrlpts[14];
  cvFillPoly(img,&pts,&nb,1,cvScalar(3),8,0 );

  //Majeur
  pts[0] = ctrlpts[15];
  pts[1] = ctrlpts[17];
  pts[2] = ctrlpts[18];
  pts[3] = ctrlpts[19];
  pts[4] = ctrlpts[20];
  cvFillPoly(img,&pts,&nb,1,cvScalar(4),8,0 );

  //Anulaire
  pts[0] = ctrlpts[21];
  pts[1] = ctrlpts[23];
  pts[2] = ctrlpts[24];
  pts[3] = ctrlpts[25];
  pts[4] = ctrlpts[27];
  cvFillPoly(img,&pts,&nb,1,cvScalar(5),8,0 );

  //Oriculaire
  pts[0] = ctrlpts[28];
  pts[1] = ctrlpts[29];
  pts[2] = ctrlpts[30];
  pts[3] = ctrlpts[31];
  pts[4] = ctrlpts[32];
  cvFillPoly(img,&pts,&nb,1,cvScalar(6),8,0 );

}


/** Draws the spline with the current transformation on the image */ 
void BSpline2D::draw_ctrlpoints_old(IplImage * img,CvScalar color)
{

  /* Trace des Points de Controle */
  for (int i=0;i<knot_nr;i++)
    {
      cvCircle(img,cvPoint((int)controlpts2[i].myx,(int)controlpts2[i].myy),2,color,-1,8,0);
        
    }

  int nb;

  //Pouce
  nb = 5;
  pts[0].x = (int)controlpts2[0].myx;
  pts[0].y = (int)controlpts2[0].myy;
  pts[1].x = (int)controlpts2[3].myx;
  pts[1].y = (int)controlpts2[3].myy;
  pts[2].x = (int)controlpts2[4].myx;
  pts[2].y = (int)controlpts2[4].myy;
  pts[3].x = (int)controlpts2[5].myx;
  pts[3].y = (int)controlpts2[5].myy;
  pts[4].x = (int)controlpts2[7].myx;
  pts[4].y = (int)controlpts2[7].myy;
  cvFillPoly(img,&pts,&nb,1,SCalcFcol(1),8,0 );


  //Paume
  nb=4;
  pts[0].x = (int)controlpts2[0].myx;
  pts[0].y = (int)controlpts2[0].myy;
  pts[1].x = (int)controlpts2[8].myx;
  pts[1].y = (int)controlpts2[8].myy;
  pts[2].x = (int)controlpts2[32].myx;
  pts[2].y = (int)controlpts2[32].myy;
  pts[3].x = (int)controlpts2[35].myx;
  pts[3].y = (int)controlpts2[35].myy;
  cvFillPoly(img,&pts,&nb,1,SCalcFcol(2),8,0 );

  //Test d'utilsation des pointeurs
  //int *paume_list[4];



  //Index
  nb=5;
  pts[0].x = (int)controlpts2[8].myx;
  pts[0].y = (int)controlpts2[8].myy;
  pts[1].x = (int)controlpts2[10].myx;
  pts[1].y = (int)controlpts2[10].myy;
  pts[2].x = (int)controlpts2[11].myx;
  pts[2].y = (int)controlpts2[11].myy;
  pts[3].x = (int)controlpts2[12].myx;
  pts[3].y = (int)controlpts2[12].myy;
  pts[4].x = (int)controlpts2[14].myx;
  pts[4].y = (int)controlpts2[14].myy;
  cvFillPoly(img,&pts,&nb,1,SCalcFcol(3),8,0 );

  //Majeur
  pts[0].x = (int)controlpts2[15].myx;
  pts[0].y = (int)controlpts2[15].myy;
  pts[1].x = (int)controlpts2[17].myx;
  pts[1].y = (int)controlpts2[17].myy;
  pts[2].x = (int)controlpts2[18].myx;
  pts[2].y = (int)controlpts2[18].myy;
  pts[3].x = (int)controlpts2[19].myx;
  pts[3].y = (int)controlpts2[19].myy;
  pts[4].x = (int)controlpts2[20].myx;
  pts[4].y = (int)controlpts2[20].myy;
  cvFillPoly(img,&pts,&nb,1,SCalcFcol(4),8,0 );

  //Anulaire
  pts[0].x = (int)controlpts2[21].myx;
  pts[0].y = (int)controlpts2[21].myy;
  pts[1].x = (int)controlpts2[23].myx;
  pts[1].y = (int)controlpts2[23].myy;
  pts[2].x = (int)controlpts2[24].myx;
  pts[2].y = (int)controlpts2[24].myy;
  pts[3].x = (int)controlpts2[25].myx;
  pts[3].y = (int)controlpts2[25].myy;
  pts[4].x = (int)controlpts2[27].myx;
  pts[4].y = (int)controlpts2[27].myy;
  cvFillPoly(img,&pts,&nb,1,SCalcFcol(5),8,0 );

  //Oriculaire
  pts[0].x = (int)controlpts2[28].myx;
  pts[0].y = (int)controlpts2[28].myy;
  pts[1].x = (int)controlpts2[29].myx;
  pts[1].y = (int)controlpts2[29].myy;
  pts[2].x = (int)controlpts2[30].myx;
  pts[2].y = (int)controlpts2[30].myy;
  pts[3].x = (int)controlpts2[31].myx;
  pts[3].y = (int)controlpts2[31].myy;
  pts[4].x = (int)controlpts2[32].myx;
  pts[4].y = (int)controlpts2[32].myy;
  cvFillPoly(img,&pts,&nb,1,SCalcFcol(6),8,0 );


  cvRectangle(img, Posmin, Posmax, SCalcFcol(7),EPAISSEUR,8,0);


}

/** Draws a mask corresponding to the curent transformation state */
void BSpline2D::DrawMask(IplImage * img)
{

  //Trace rempli
  for (int i=0;i<numpoints;i++)
    {
      pts[i].x = (int)points[i].myx;
      pts[i].y = (int)points[i].myy;
    }      
  cvFillPoly(img,&pts,&numpoints,1,cvScalar(255),8,0 );
}

/** ONLY FOR DEGREE 2 */
inline Point2D BSpline2D::deboor2D(double u, int interval){
  register int j;
  double t1,t2;
  
  for (j=interval-1; j<=interval+1; j++)
    {
      coeffx[j]=ctrlpts[j].x;
      coeffy[j]=ctrlpts[j].y;
    }
  j=interval+1;
  {
    t1=  (knots[j+1] - u )/(knots[j+1]-knots[j-1]);
    t2=  1.0-t1;
    
    coeffx[j]=t1* coeffx[j-1]+t2* coeffx[j];
    coeffy[j]=t1* coeffy[j-1]+t2* coeffy[j];
  }
  j--;
  {
    t1=  (knots[j+1] - u )/(knots[j+1]-knots[j-1]);
    t2=  1.0-t1;
    
    coeffx[j]=t1* coeffx[j-1]+t2* coeffx[j];
    coeffy[j]=t1* coeffy[j-1]+t2* coeffy[j];
  }   
  j=interval+1;
  {
    t1=  (knots[j] - u )/(knots[j]-knots[j-1]);
    t2=  1.0-t1;
    
    coeffx[j]=t1* coeffx[j-1]+t2* coeffx[j];
    coeffy[j]=t1* coeffy[j-1]+t2* coeffy[j];
  }
  
  return Point2D(coeffx[interval+1],coeffy[interval+1]);
}
