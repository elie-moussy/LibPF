#include "LibPF/imageutils/displayutils.h"


void Show( char* nm,IplImage* imgview, int pause)
{
  cvNamedWindow(nm,0);
  cvShowImage(nm,imgview);
  if(pause) cvWaitKey(0);
}
void Show( char* nm,IplImage* imgview, int x, int y, int pause)
{
  Show(nm,imgview,0);
  cvMoveWindow(nm,x,y);
  if(pause) cvWaitKey(0);
}

void DispIma(IplImage *img, char *fen, int pause)
{
  cvNamedWindow(fen,0);
  cvShowImage(fen,img);
  if(pause) cvWaitKey(0);
}

void DispIma(IplImage *img, char *fen, int x, int y,int pause)
{
  cvNamedWindow(fen,0);
  cvMoveWindow(fen,x,y);
  cvShowImage(fen,img);
  if(pause) cvWaitKey(0);
}

/** Affichage d'une image en fausse couleur **/
void DispFcol(IplImage *img, char *fen)
{
  int i,j;
  CvSize t = cvGetSize(img);
  IplImage *fcol = cvCreateImage(t,8,3);
  double coul;

  //printf("height = %d  width = %d\n",t.height,t.width);
  //cvWaitKey(0);
  
  for(i=0;i<t.height;i++)
    for(j=0;j<t.width;j++)
      {
	coul = cvGetReal2D(img,i,j);
	//printf("Coul = %f\n",coul);
	if(coul<255) cvSet2D(fcol,i,j,SCalcFcol((int)coul));
 	else cvSet2D(fcol,i,j,cvScalar(255,255,255));

      }
  cvNamedWindow(fen,0);
  cvShowImage(fen,fcol);

  //if(pause) cvWaitKey(0);

  cvReleaseImage(&fcol);
}




void DispResizedFcol(IplImage *img, char *fen,int height, int width)
{
  int i,j;
  CvSize t = cvGetSize(img);
  IplImage *fcol = cvCreateImage(t,8,3);
  double coul;

  //printf("height = %d  width = %d\n",t.height,t.width);
  //cvWaitKey(0);
  
  for(i=0;i<t.height;i++)
    for(j=0;j<t.width;j++)
      {
	coul = cvGetReal2D(img,i,j);
	//printf("Coul = %f\n",coul);
	cvSet2D(fcol,i,j,SCalcFcol((int)coul));
      }
  cvNamedWindow(fen,0);
  cvShowImage(fen,fcol);
  cvResizeWindow(fen,width,height);  
  cvReleaseImage(&fcol);
}
