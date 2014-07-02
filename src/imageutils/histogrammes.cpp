//Fonctions relatives aux histogrammes
#include "imageutils/histogrammes.h"



/*********************************************************************************************
 Auteur : Ludovic Brèthes

 Description : Affichage de l'histogramme 2D
 Entrées : histo   = histogramme a afficher
           fen     = nom de la fenetre
           scale   = echelle 
           lutR lutV lutB   = luts d'affichage
 Sorties : Visualisation d'une image de l'histogramme
**********************************************************************************************/
void Hist2DDisplay(CvHistogram *histo,const char *fen,int scale)
{
  int h,s;
  float max_value;
  int hist_size[2];

  cvGetDims( histo->bins, hist_size );

  IplImage* hist_img = cvCreateImage( cvSize(hist_size[0]*scale,hist_size[1]*scale), 8, 3 );

  cvGetMinMaxHistValue( histo, 0, &max_value, 0, 0 );
  cvZero( hist_img );
  for( h = 0; h < hist_size[0]; h++ )
    {
      for( s = 0; s < hist_size[1]; s++ )
	{
	  float bin_val = cvQueryHistValue_2D( histo, h, s );
	  if(bin_val)
	    {
	      int intensity = cvRound(bin_val*255/max_value);
	      cvRectangle( hist_img, cvPoint( h*scale, s*scale ),cvPoint( (h+1)*scale - 1, (s+1)*scale - 1),CV_RGB(intensity,intensity,intensity),CV_FILLED );
	    }
	  
	}
    }
  cvNamedWindow(fen,0);
  cvShowImage(fen,hist_img);

  cvReleaseImage(&hist_img);
  return;
}

void Hist2DDisplayZoom(CvHistogram *histo,const char *fen, int scale,int coul,int autozoom)
{
  int hist_size[2];
  int h,s;
  float max_value;
  int xmin=0,ymin=0,xmax=255,ymax=255;
  int posx,posy,delta;

  cvGetDims( histo->bins, hist_size );

  IplImage* hist_img = cvCreateImage( cvSize(hist_size[0]*scale,hist_size[1]*scale), 8, 3 );
  
  cvGetMinMaxHistValue( histo, 0, &max_value, 0, 0 );
  cvZero( hist_img );

  //Recherche coordonnées min et max pour zoom
  if(autozoom)
    Hist2DPosMinMaxBin(histo,&xmin,&ymin,&xmax,&ymax);
  
  //Traitement de l'affichage
  for( h = 0; h < hist_size[0]; h++ )
    {
      for( s = 0; s < hist_size[1]; s++ )
	{
	  float bin_val = cvQueryHistValue_2D(histo,h,s);
	  if(bin_val)
	    {
	      int intensity = cvRound(bin_val*255/max_value);
	      if((xmax-xmin)>(ymax-ymin))
		{
		  posx = cvRound(((double)h-(double)xmin)/(double)(xmax-xmin)*255*scale);
		  posy =  cvRound((double)(s-xmin)/(double)(xmax-xmin)*255*scale);
		  delta = cvRound( (double)(255*scale)/(double)(xmax-xmin));
		}
	      else
		{
		  posx = cvRound(((double)h-(double)ymin)/(double)(ymax-ymin)*255*scale);
		  posy =  cvRound((double)(s-ymin)/(double)(ymax-ymin)*255*scale);
		  delta = cvRound( (double)(255*scale)/(double)(ymax-ymin));
		}
	      switch(coul)
		{
		case GRIS:
		  cvRectangle(hist_img,cvPoint(posx,posy),cvPoint(posx+delta,posy+delta),CV_RGB(intensity,intensity,intensity),CV_FILLED,8,0);
		  break;
		case FCOL:
		  cvRectangle(hist_img,cvPoint(posx,posy),cvPoint(posx+delta,posy+delta),SCalcFcol(intensity),CV_FILLED,8,0);
		  break;
		case TEINTE:
		  cvRectangle(hist_img,cvPoint(posx,posy),cvPoint(posx+delta,posy+delta),SCalcTeint(intensity),CV_FILLED,8,0);
		  break;
		}
	    }
	}
    }
  cvNamedWindow(fen,0);
  cvShowImage(fen,hist_img);
  // cvSaveImage("/home/lbrethes/dev/src/images/capture/cluster.jpg",hist_img);
  cvReleaseImage(&hist_img);
  return;
}

void Hist1DDisplay(CvHistogram *histo,char *fen,int scale,CvScalar couleur)
{
  int h_bins=NBBINS;
  int s_bins=NBBINS;
  int p;
  float max_value;
  CvPoint prec = cvPoint(0,(NBBINS-1)*scale);
  
  IplImage* hist_img = cvCreateImage( cvSize(h_bins*scale,s_bins*scale), 8, 3 );
  
  cvGetMinMaxHistValue( histo, 0, &max_value, 0, 0 );
  cvZero( hist_img );
  for( p = 0; p < h_bins; p++ )
    {
      float bin_val = cvQueryHistValue_1D(histo,p);
      if(bin_val!=-1)
	{
	  cvLine(hist_img,prec,cvPoint(p*scale,(int)(NBBINS-1-bin_val)*scale),couleur,1,8,0);
	  prec.x = p*scale;
	  prec.y = (int)(NBBINS-1-bin_val)*scale;
	}
    }
  cvNamedWindow(fen,0);
  cvShowImage(fen,hist_img);
  cvReleaseImage(&hist_img);
  return;
}
 
void Hist1DDisplayBarre(CvHistogram *histo,char *fen,int scale,CvScalar couleur)
{
  int h_bins=NBBINS;
  int s_bins=NBBINS;
  int p;
  float max_value;
  //CvPoint prec = cvPoint(0,(NBBINS-1)*scale);
  CvPoint Axe;

  IplImage* hist_img = cvCreateImage( cvSize(h_bins*scale,s_bins*scale), 8, 3 );
  
  cvGetMinMaxHistValue( histo, 0, &max_value, 0, 0 );
  cvZero( hist_img );
  for( p = 0; p < h_bins; p++ )
    {
      float bin_val = cvQueryHistValue_1D(histo,p);
      if(bin_val!=-1)
	{
	  Axe.x = p*scale;
	  Axe.y = (NBBINS-1)*scale;
	  cvLine(hist_img,Axe,cvPoint(p*scale,(int)(NBBINS-1-bin_val)*scale),couleur,1,8,0);
	  //prec.x = p*scale;
	  //prec.y = (NBBINS-1-bin_val)*scale;
	}
    }
  cvNamedWindow(fen,0);
  cvShowImage(fen,hist_img);
  cvReleaseImage(&hist_img);
  return;
}


IplImage * Hist1D2IplImage(CvHistogram *histo,double scale,CvScalar couleur)
{
  int h_bins=NBBINS;
  int s_bins=NBBINS;
  int p;
  float max_value;
  //CvPoint prec = cvPoint(0,(NBBINS-1)*scale);
  CvPoint Axe;

  IplImage* hist_img = cvCreateImage( cvSize((int)(h_bins*scale),(int)(s_bins*scale)), 8, 3 );
  
  cvGetMinMaxHistValue( histo, 0, &max_value, 0, 0 );
  cvZero( hist_img );
  for( p = 0; p < h_bins; p++ )
    {
      float bin_val = cvQueryHistValue_1D(histo,p);
      if(bin_val!=-1)
	{
	  Axe.x = cvRound(p*scale);
	  Axe.y = cvRound((NBBINS-1)*scale);
	  cvLine(hist_img,Axe,cvPoint((int)(p*scale),(int)((NBBINS-1-bin_val)*scale)),couleur,1,8,0);
	  //prec.x = p*scale;
	  //prec.y = (NBBINS-1-bin_val)*scale;
	}
    }
  return hist_img;
}

/********************************** Fonctions de Traitement des histogrammes **************************************/

//Dynamise l'histogramme en etirant ses valeurs entre 0 et 255 
void Hist2DDyn(CvHistogram *histo,int ech)
{
  float min,max;
  double scale;

  //recuperation du min et du max
  cvGetMinMaxHistValue(histo,&min,&max);

  //printf("min = %f \t max = %f\n",min,max);
  if(min!=0)
    {
      /* il faut aussi translater l'histogramme on appelle donc la fonction
	 qui fait translation et etirement */
      Hist2DDyn2(histo,ech);
      return;
    }
  
  scale = (double)ech/(double)max;
  //printf("Min = %f Max = %f Scale = %f\n",min,max,scale);
  cvScale(histo->bins,histo->bins,scale);
}



//Etire les valeurs de l'histogramme entre 0 et ech
void Hist1DDyn(CvHistogram *histo,int ech)
{
  
  float min,max;
  double scale;

  //recuperation du min et du max
  cvGetMinMaxHistValue(histo,&min,&max);

  scale = (double)ech/(double)max;
  //printf("min = %f  max = %f  Scale = %f\n",min,max,scale);
  cvScale(histo->bins,histo->bins,scale);
}

/** Determine les positions min et max de l'histogramme **/
void Hist2DPosMinMaxBin(CvHistogram *histo,int *xmin,int *ymin,int *xmax,int *ymax)
{
  int h_bins=NBBINS;
  int s_bins=NBBINS;
  int h,s;

  *xmin=256;
  *ymin=256;
  *xmax=-1;
  *ymax=-1;

  for( h = 0; h < h_bins; h++ )
      for( s = 0; s < s_bins; s++ )
	{
	  float bin_val = cvQueryHistValue_2D( histo, h, s );
	  if(bin_val)
	    {
	      if(h<*xmin) *xmin=h;
	      if(h>*xmax) *xmax=h;
	      
	      if(s<*ymin) *ymin=s;
	      if(s>*ymax) *ymax=s;
	    }
	}
}

/** Fonction qui rempli une matrice correspondant aux valeurs de l'histogramme **/
void Hist2DGetBins(CvHistogram *histo,CvMat **out)
{
  int h,s;

  for( h = 0; h < NBBINS; h++ )
    {
      for( s = 0; s < NBBINS; s++ )
	{
	  double bin_val = cvQueryHistValue_2D( histo, h, s );
	  cvmSet(*out,h,s,bin_val);	  
	}
    }
  
}

/** Fonction qui rempli l'histogramme avec les valeurs d'une matrice **/
void Hist2DSetBins(CvMat *mat, CvHistogram *out)
{
  int h,s;
  int hist_size[2];

  cvGetDims( mat, hist_size );

  for( h = 0; h < hist_size[1]; h++ )
    {
      for( s = 0; s < hist_size[0]; s++ )
	{	  
	  double bin_val = cvmGet(mat,h,s);
	  cvSet2D(out->bins,h,s,cvScalar(bin_val));
	}
    }
  
}


/** Fonction de Dilatation d'un Histogramme 2D **/
void Hist2DDilate(CvHistogram *histo,int taille_se)
{
  CvMat * mat;
  int hist_size[2];
  IplConvKernel* B=NULL;

  cvGetDims( histo->bins, hist_size );
  mat=cvCreateMat(hist_size[0],hist_size[1],CV_32F);

  cvCopy(histo->bins,mat);
  if(taille_se>3) B = cvCreateStructuringElementEx(taille_se,taille_se,taille_se/2,taille_se/2,CV_SHAPE_RECT,NULL);
  cvDilate(mat,mat,B);
  cvCopy(mat,histo->bins);
  if(B) cvReleaseStructuringElement(&B);
  cvReleaseMat(&mat);
}

/** Fonction d'Erosion d'un Histogramme 2D **/
void Hist2DErode(CvHistogram *histo,int taille_se)
{
  CvMat * mat;
  int hist_size[2];
  IplConvKernel* B=NULL;

  cvGetDims( histo->bins, hist_size );
  mat=cvCreateMat(hist_size[0],hist_size[1],CV_32F);

  cvCopy(histo->bins,mat);
  if(taille_se>3) B = cvCreateStructuringElementEx(taille_se,taille_se,taille_se/2,taille_se/2,CV_SHAPE_RECT,NULL);
  cvErode(mat,mat,B);
  cvCopy(mat,histo->bins);
  if(B) cvReleaseStructuringElement(&B);
  cvReleaseMat(&mat);
}

/* Fonction qui fait une fermetture de taille nb*/
void Hist2DClose(CvHistogram *histo,int taille_se, int nb)
{
  int i;
  CvMat * mat;
  int hist_size[2];
  IplConvKernel* B=NULL;

  cvGetDims( histo->bins, hist_size );

  mat=cvCreateMat(hist_size[0],hist_size[1],CV_32F);
  cvCopy(histo->bins,mat);
  if(taille_se>3) B = cvCreateStructuringElementEx(taille_se,taille_se,taille_se/2,taille_se/2,CV_SHAPE_RECT,NULL);

  for(i=0;i<nb;i++)
    cvDilate(mat,mat,B);
  for(i=0;i<nb;i++)
    cvErode(mat,mat,B);

  cvCopy(mat,histo->bins);
  if(B) cvReleaseStructuringElement(&B);
  cvReleaseMat(&mat);
}

/* Fonction qui fait une ouverture de taille nb*/
void Hist2DOpen(CvHistogram *histo,int taille_se, int nb)
{
  int i;
  CvMat * mat;
  int hist_size[2];
  IplConvKernel* B=NULL;

  cvGetDims( histo->bins, hist_size );

  mat=cvCreateMat(hist_size[0],hist_size[1],CV_32F);
  cvCopy(histo->bins,mat);
  if(taille_se>3) B = cvCreateStructuringElementEx(taille_se,taille_se,taille_se/2,taille_se/2,CV_SHAPE_RECT,NULL);

  for(i=0;i<nb;i++)
    cvErode(mat,mat,B);
  for(i=0;i<nb;i++)
    cvDilate(mat,mat,B);

  cvCopy(mat,histo->bins);
  if(B) cvReleaseStructuringElement(&B);
  cvReleaseMat(&mat);
}

/** Dilatation de l'histogramme 1D **/
void Hist1DDilate(CvHistogram *hist,int taille_es)
{
  int j=0;
  
  //Element structurant
  int deb=0;
  int mil=(taille_es-1)/2;
  int fin=taille_es-1;
  double max=0;
  CvScalar val;
  CvHistogram *htmp=NULL;

  cvCopyHist(hist,&htmp);
  cvClearHist(hist);
  if(taille_es % 2 == 0) 
    {
      //ES pair
      mil++;      
    }

  //printf("taille es = %d\n",taille_es);
  for(deb=0,fin=taille_es-1;fin<256;deb++,mil++,fin++)
    {
      max=0;
      for(j=deb;j<=fin;j++)
	{
	  val = cvGet1D(htmp->bins,j);
	  if(val.val[0]>max) max=val.val[0];
	}
	cvSet1D(hist->bins,mil,cvScalar(max));
    }

  cvReleaseHist(&htmp);
}



void Hist2DCalcRapport(CvHistogram *hist_peau, CvHistogram *hist_total, CvHistogram *histo)
{
  int hist_size[2];
  int i,j;
 
  cvGetDims( hist_peau->bins, hist_size );


  for(j=0;j<hist_size[1];j++)
    for(i=0;i<hist_size[0];i++)
      {
	double binval_total = cvQueryHistValue_2D( hist_total, i, j );
	if(binval_total)
	  {
	    double binval_peau = cvQueryHistValue_2D( hist_peau, i, j );
	    double ratio = binval_peau / binval_total;
	    cvSetReal2D( histo->bins, i, j, ratio );
	  }
	else cvSetReal2D( histo->bins, i, j, 0 );
      }

  Hist2DDyn2(histo, 255.0);

}

void Hist2DDyn2(CvHistogram *histo, float ech)
{
  int hist_size[2];
  int i,j;
  float min,max;

  //recuperation du min et du max
  //cvGetMinMaxHistValue(histo,&min,&max);
  //printf("min = %f\t max = %f\n",min,max); 

  cvGetDims( histo->bins, hist_size );
  
  min = 9999;
  max=-1;
  for(j=0;j<hist_size[1];j++)
    for(i=0;i<hist_size[0];i++)
      {
	float binval = cvQueryHistValue_2D( histo, i, j );
	if (binval)
	  {
	    if (binval<min) min = binval;
	    if(binval>max) max = binval;
	  }
      }

  if(max<=0) return;
  for(j=0;j<hist_size[1];j++)
    for(i=0;i<hist_size[0];i++)
      {
	float binval = cvQueryHistValue_2D( histo, i, j );
	if(binval>0)
	  {
	    float newval = (binval-min)*ech/max;
	    cvSetReal2D( histo->bins, i, j, newval );
	  }
      }
}

/** Calcul de l'histogramme a partir de deux vecteurs **/
void CalcHistoObj(CvMat *mat1,CvMat *mat2,double taille, CvHistogram *histo, int dyn)
{

  int i,x,y;
  //double old;
  CvScalar sum,old;

  for(i=0;i<taille;i++)
    {
      x = (int)cvmGet(mat1,0,i);
      y = (int)cvmGet(mat2,0,i);
      old=cvGet2D(histo->bins,x,y);
      cvSet2D(histo->bins,x,y,cvScalar(old.val[0]+1));
    }
  //Normalisation
  if(dyn) Hist2DDyn(histo);
  return;
}

void CalcMatHisto(CvMat *mat1,CvMat *mat2,double taille, CvHistogram *histo, int dyn)
{

  int i,x,y;
  //double old;
  CvScalar sum,old;

  for(i=0;i<taille;i++)
    {
      x = (int)cvmGet(mat1,0,i);
      y = (int)cvmGet(mat2,0,i);
      old=cvGet2D(histo->bins,x,y);
      cvSet2D(histo->bins,x,y,cvScalar(old.val[0]+1));
    }
  //Normalisation
  if(dyn) Hist2DDyn(histo);
  return;
}

void CalcHistoPlanes(IplImage **planes,CvHistogram **out)
{
  int r,c;
  double x,y,bin;
  CvSize taille = cvGetSize(planes[0]);
  CvScalar sum;

  cvClearHist(*out);
  for(r=0;r<taille.height;r++)
    for(c=0;c<taille.width;c++)
      {
	x = cvGetReal2D(planes[0],r,c);
	y = cvGetReal2D(planes[1],r,c);
	if(x || y)
	  {
	    bin = cvGetReal2D((*out)->bins,(int)x,(int)y);
	    bin +=1;
	    cvSetReal2D((*out)->bins,(int)x,(int)y,bin);	
	  }
      }
  Hist2DDyn((*out));
}

/** Convertit un histogramme en une image **/
void Histo2Img(CvHistogram *histo,IplImage **img)
{
  int h_bins=NBBINS;
  int s_bins=NBBINS;
  int h,s;
  float max_value;
  
  *img = cvCreateImage( cvSize(h_bins,s_bins),8,1);
   
  cvGetMinMaxHistValue(histo,0,&max_value,0,0);
  cvZero(*img);
  for( h = 0; h < h_bins; h++ )
    {
      for( s = 0; s < s_bins; s++ )
	{
	  float bin_val = cvQueryHistValue_2D( histo, h, s );
	  int intensity = cvRound(bin_val*255/max_value);
	  cvSet2D(*img,s,h,cvScalar(intensity));
	}
    }
  return;
}
