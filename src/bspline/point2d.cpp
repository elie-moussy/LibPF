/***************************************************************************
                          point2d.cpp  -  description
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

#include "bspline/point2d.h"


Point2D::Point2D()
{
  myx=0.0;
  myy=0.0;
}

Point2D::~Point2D()
{

}

Point2D::Point2D(double ix,double iy)
{
  myx=ix;myy=iy;
}

Vector2D operator - (const Point2D &a,const  Point2D & b)
{
  double x,y;
  x=a.myx-b.myx;
  y=a.myy-b.myy;
  return Vector2D(x,y);
}

/** multiply a point by a scalar */
Point2D operator * (const Point2D & pt,const double &val){
  Point2D tmp;
  tmp.myx=pt.myx*val;
  tmp.myy=pt.myy*val;
  return tmp;
}

/** multiply a point by a scalar */
Point2D operator * (const double& a,const Point2D &b)
{
  return b*a;
}


//------------------------------------------

Vector2D::~Vector2D()
{

}

/** returns a vector that is normal to the input one */
Vector2D Vector2D::normal()
{
#define NORMALIZEVEC
#ifdef NORMALIZEVEC 
  double mag;
  mag=sqrt(myx*myx+myy*myy);
  
  return Vector2D(-myy/mag,myx/mag);
#else
  return Vector2D(-myy,myx);
#endif    
}

/** Rotate around origin */
void Point2D::rotate(double angle)
{
  double st=sin(angle);
  double ct=cos(angle);
  double xrot=ct*(myx)-st*(myy);
  double yrot=st*(myx)+ct*(myy);
  myx=xrot;
  myy=yrot;
}

/* Rotate around a point */
void Point2D::rotate(double angle, double xc, double yc)
{
  double st=sin(angle);
  double ct=cos(angle);
  double xnorm=myx-xc;
  double ynorm=myy-yc;
  double xrot=ct*(xnorm)-st*(ynorm);
  double yrot=st*(xnorm)+ct*(ynorm);
  myx=xrot+xc;
  myy=yrot+yc;
}
void Point2D::rotate(double angle,Point2D pt)
{
  double st=sin(angle);
  double ct=cos(angle);
  double xnorm=myx-pt.myx;
  double ynorm=myy-pt.myy;
  double xrot=ct*(xnorm)-st*(ynorm);
  double yrot=st*(xnorm)+ct*(ynorm);
  myx=xrot+pt.myx;
  myy=yrot+pt.myy;
}

/** translate a point */
Point2D Point2D::operator +(const Point2D &pt)
{
  Point2D tmp;
  tmp.myx=myx+pt.myx;
  tmp.myy=myy+pt.myy;
  return tmp;
}
