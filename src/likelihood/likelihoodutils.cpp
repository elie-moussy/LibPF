#include "LibPF/likelihood/likelihoodutils.h"


// template<typename TW, typename TO> struct EdgeMesuredst
// {
//   static TW calc(MultiBSpline2D* spline, TO* img_mesure, float coeff, float scale=1)
//   { cout << "|ERROR|--> EdgeMesuredst \t:\t Not defined for this Observation type" << endl; return 0;}
  
//   static TW calc(MultiBSpline2D* spline, TO* img_mesure, TO* img_mask, float ro, float coeff, float scale=1)
//   { cout << "|ERROR|--> EdgeMesuredst \t:\t Not defined for this Observation type" << endl; return 0;}
// };

// template<typename TW> struct EdgeMesuredst<TW,IplImage>
// {
//   static TW calc(MultiBSpline2D* spline, IplImage* img_mesure, float coeff, float scale=1)
//   { return  CalcEdgeMesuredst<TW>(spline,img_mesure,coeff,scale); }

//   static TW calc(MultiBSpline2D* spline, IplImage* img_mesure, IplImage* img_mask, float ro, float coeff, float scale=1)
//   { return  CalcEdgeMesuredst<TW>(spline,img_mesure,img_mask,ro,coeff,scale); }
// };

/** Fonction de mesure qui calcule une distance en utilisant la normale aux contours de la spline (comme fait BLAKE) */
double CalcEdgeMesuredst(MultiBSpline2D* spline, IplImage* img_mesure, float coeff, float scale)
{
  CvPoint pt,pt1,pt2;
  CvLineIterator li_interne,li_externe;
  int count1=0,count2=0,count;
  double prob=1;
  Point2D p1,p2;
  int dx,dy;
  double sumdist=0;

  //IplImage* ttmp = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,3);
  //cvZero(ttmp);
  //spline->draw(ttmp,CV_RGB(255,0,0));

  for(register int i=0;i<spline->numpoints;i++)
    {
      pt.x = (int)(spline->points[i].myx);
      pt.y = (int)(spline->points[i].myy);
      dx = (int)(spline->normals[i].myx*SEARCHDISTANCE*scale);
      dy = (int)(spline->normals[i].myy*SEARCHDISTANCE*scale);
      pt1.x = (int)(spline->points[i].myx+dx);
      pt1.y = (int)(spline->points[i].myy+dy);
      pt2.x = (int)(spline->points[i].myx-dx);
      pt2.y = (int)(spline->points[i].myy-dy);


      //cout << "Testing (" << pt2.x << "," << pt2.y << ") -- (" << pt.x << "," << pt.y << ") -- (" << pt1.x << "," << pt1.y << ")" << endl;

      if (ClipLine(1,1,img_mesure->width-2,img_mesure->height-2,&pt,&pt1))
	count1 = cvInitLineIterator(img_mesure,pt,pt1,&li_interne,8);
      else count1=0;

      if (ClipLine(1,1,img_mesure->width-2,img_mesure->height-2,&pt,&pt2))
	count2 = cvInitLineIterator(img_mesure,pt,pt2,&li_externe,8);
      else count2=0;

      //cvLine(ttmp,pt,pt1,CV_RGB(0,255,0));
      //cvLine(ttmp,pt,pt2,CV_RGB(0,0,255));

      count = count1 > count2 ? count2 : count1;

      //cout << "count = " << count << endl;
      register int d;
      int found=0;
      for(d=0 ;(d<count)&&(!found);d++)
	{
	  if((*(li_interne.ptr) >0) || (*(li_externe).ptr>0))
	    {
	      found = 1;
	    }
	  CV_NEXT_LINE_POINT(li_interne);
	  CV_NEXT_LINE_POINT(li_externe);
	}
      if(found) sumdist+=(d*d);
      else sumdist+=MAXDISTCARRE*scale;
    }

  //Show("sample",ttmp,0);
  //cvWaitKey(50);
  //cvReleaseImage(&ttmp);

  //cout << "sumdist avant =  " << sumdist << endl;

  /* On normalise en fonction du nombre de lignes */
  sumdist*=(200.0/spline->numpoints);

#ifdef KEEPDIST
  /* Stockage de la distance */
  likelihoodDist = sqrt(sumdist);
#endif


  //cout << "nb pts =  " << spline->numpoints << endl;
  //cout << "sumdist apres =  " << sumdist << endl;

  /* calcul du poids */
  prob=exp(coeff * sumdist);

  //printf("%e\n",prob);
  return prob; 
}


/** Fonction de mesure qui calcule une distance en utilisant la normale aux contours de la spline et appliquant une
    penalite ro en fonction du masque img_mask */

double CalcEdgeMesuredst(MultiBSpline2D* spline, IplImage* img_mesure, IplImage* img_mask, float ro, float coeff, float scale)
{
  CvPoint pt,pt1,pt2;
  CvLineIterator li_interne,li_externe;
  CvLineIterator mask_li_interne,mask_li_externe;
  int count1=0,count2=0,count;
  double prob=1;
  Point2D p1,p2;
  int dx,dy;
  double sumdist=0;

  for(register int i=0;i<spline->numpoints;i++)
    {
      pt.x = (int)(spline->points[i].myx);
      pt.y = (int)(spline->points[i].myy);
      dx = (int)(spline->normals[i].myx*SEARCHDISTANCE*scale);
      dy = (int)(spline->normals[i].myy*SEARCHDISTANCE*scale);
      pt1.x = (int)(spline->points[i].myx+dx);
      pt1.y = (int)(spline->points[i].myy+dy);
      pt2.x = (int)(spline->points[i].myx-dx);
      pt2.y = (int)(spline->points[i].myy-dy);

      if (ClipLine(1,1,img_mesure->width-2,img_mesure->height-2,&pt,&pt1))
	{
	  count1 = cvInitLineIterator(img_mesure,pt,pt1,&li_interne,8);
	  cvInitLineIterator(img_mask,pt,pt1,&mask_li_interne,8);
	}
      else count1=0;

      if (ClipLine(1,1,img_mesure->width-2,img_mesure->height-2,&pt,&pt2))
	{
	  count2 = cvInitLineIterator(img_mesure,pt,pt2,&li_externe,8);
	  cvInitLineIterator(img_mask,pt,pt2,&mask_li_externe,8);
	}
      else count2=0;

      count = count1 > count2 ? count2 : count1;

      register int d;
      int found=0;
      float penality=0;
      for(d=0 ;(d<count)&&(!found);d++)
	{
	  /* Point interne trouve et il n'est pas dans le masque (il ne bouge pas si c'est un masque flot optique) */
	  if(*(li_interne.ptr) > 0)
	    {
	      found = 1;
	      if(*(mask_li_interne.ptr) == 0) penality = ro; 		
	    }
	  
	  /* Point externe trouve et il n'est pas dans le masque (il ne bouge pas si c'est un masque flot optique) */
	  if(*(li_externe.ptr) > 0)
	    {
	      found += 2;
	      if((*(mask_li_externe.ptr) == 0))
		if(found==2) penality = ro; 
	      else 
		if(penality) penality = 0;
	    }
	  CV_NEXT_LINE_POINT(li_interne);
	  CV_NEXT_LINE_POINT(li_externe);
	  CV_NEXT_LINE_POINT(mask_li_interne);
	  CV_NEXT_LINE_POINT(mask_li_externe);
	}
      if(found) sumdist+=(d+penality)*(d+penality);
      else sumdist+=MAXDISTCARRE*scale;
    }

  /* Normalisation */
  sumdist*=(200.0/spline->numpoints);

#ifdef KEEPDIST
  /* Stockage de la distance */
  likelihoodDist = sqrt(sumdist);
#endif
  
  /* Calcul du poids */
  prob=exp(coeff * sumdist);

  //printf("%e\n",prob);
  return prob; 
}

/* Mesure basee sur une forme et une image de distance */

// template<typename double, typename TO> struct DistMesuredst
// {
//   static double calc(MultiBSpline2D* spline, TO* img_mesure, float coeff, float scale=1)
//   { cout << "|ERROR|--> DistMesuredst \t:\t Not defined for this Observation type" << endl; return 0;}
  
//   static double calc(MultiBSpline2D* spline, TO* img_mesure, TO* img_mask, float ro, float coeff)
//   { cout << "|ERROR|--> DistMesuredst \t:\t Not defined for this Observation type" << endl; return 0;}
// };

// template<typename double> struct DistMesuredst<double,IplImage>
// {
//   static double calc(MultiBSpline2D* spline, IplImage* img_mesure, float coeff, float scale=1)
//   { return  CalcDistMesuredst<double>(spline,img_mesure,coeff,scale); }

//   static double calc(MultiBSpline2D* spline, IplImage* img_mesure, IplImage* img_mask, float ro, float coeff, float scale=1)
//   { return  CalcDistMesuredst<double>(spline,img_mesure,img_mask,ro,coeff,scale); }
// };

/** Fonction de mesure qui calcule une distance en utilisant directement les distances le long de la spline */

double CalcDistMesuredst(MultiBSpline2D* spline, IplImage* img_mesure, float coeff, float scale)
{
  double prob=1;
  double sumdist=0;
  double d;

  //IplImage* ttmp = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,3);
  //cvZero(ttmp);
  //spline->draw(ttmp,CV_RGB(255,0,0));

  /* Parcours de la spline avec mesure directe de la distance */
  for(register int i=0;i<spline->numpoints;i++)
    {
      /* Verification de la validite du point */
      //cout << "(" << spline->points[i].myx << "," << spline->points[i].myy << ")" << endl;
      if(spline->points[i].myx>=0 && spline->points[i].myx<img_mesure->width && spline->points[i].myy>=0 && spline->points[i].myy<img_mesure->height)
	{
	  //d=img_mesure->imageData[(int)(spline->points[i].myy)*img_mesure->widthStep + (int)(spline->points[i].myx)]; 	
	  d = cvGetReal2D(img_mesure, (int)(spline->points[i].myy),(int)(spline->points[i].myx));
	  //cout << "dist = " << d << endl;
	  if(d<SEARCHDISTANCE*scale) sumdist+=d*d;
	  else sumdist+=MAXDISTCARRE*scale;

	  //sumdist+=d*d;

	  //cvCircle(ttmp,cvPoint((int)(spline->points[i].myx),(int)(spline->points[i].myy)),2,CV_RGB(0,255,0),-1,8,0);
	}
      else
	sumdist+=MAXRO;
    }

  //cout << "min = " << min << "  max = " << max << endl;

  //Show("sample",ttmp,1);
  //cvReleaseImage(&ttmp);

  /* On normalise en fonction du nombre de points */
  sumdist*=(200.0/spline->numpoints);

#ifdef KEEPDIST
  /* Stockage de la distance */
  likelihoodDist = sqrt(sumdist);
#endif

  /* calcul du poids */
  prob=exp(coeff * sumdist);

  //printf("%e\n",prob);
  return prob; 
}


/** Fonction de mesure qui calcule une distance en utilisant directement les distances le long de la spline et 
    appliquant une penalite ro en fonction du masque img_mask */
double CalcDistMesuredst(MultiBSpline2D* spline, IplImage* img_mesure, IplImage* img_mask, float ro, float coeff, float scale)
{
  double prob=1;
  double sumdist=0;
  int pos;
  double d;

  //IplImage* ttmp = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,3);
  //cvZero(ttmp);
  //spline->draw(ttmp,CV_RGB(255,0,0));

  /* Parcours de la spline avec mesure directe de la distance */
  for(register int i=0;i<spline->numpoints;i++)
    {
      /* Verification de la validite du point */
      if(spline->points[i].myx>=0 && spline->points[i].myx<img_mesure->width && spline->points[i].myy>=0 && spline->points[i].myy<img_mesure->height)
	{
	  /* Calcul de l'indice courant */
	  pos = (int)(spline->points[i].myy)*img_mask->widthStep + (int)(spline->points[i].myx);

	  /* Ajoute la distance du point courant */
	  d = cvGetReal2D(img_mesure, (int)(spline->points[i].myy),(int)(spline->points[i].myx));
	  //d = img_mesure->imageData[pos]; 

	  /* Ajoute la penalite si ce point se trouve hors masque */
	  if(!img_mask->imageData[pos]) d+=ro;

	  if(d<SEARCHDISTANCE*scale) sumdist+=d*d;
	  else sumdist+=MAXDISTCARRE*scale;

	  //sumdist+=d*d;
	}
      else
	sumdist+=MAXRO;
    }

  //Show("sample",ttmp,0);
  //cvReleaseImage(&ttmp);

  /* On normalise en fonction du nombre de points */
  sumdist*=(200.0/spline->numpoints);

#ifdef KEEPDIST
  /* Stockage de la distance */
  likelihoodDist = sqrt(sumdist);
#endif

  /* calcul du poids */
  prob=exp(coeff * sumdist);

  //printf("%e\n",prob);
  return prob; 
}
