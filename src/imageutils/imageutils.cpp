#include "imageutils.h"



/* Exporte une image sous forme d'un ficher matlab */
void exportImg2Matlab(string fname, IplImage* imgProba)
{
  ofstream fic(fname.c_str(),ios::out|ios::trunc);
 
  fic << "img=[";
  for(int i=0;i<imgProba->height;i++)
    {
      for(int j=0;j<imgProba->width;j++)
	{
	  fic << (int)cvGetReal2D(imgProba,i,j) << " ";
	  //fic << (int)(imgProba->imageData[i*imgProba->widthStep+j]) << " ";
	  //val = imgProba->imageData[j*imgProba->widthStep+i];
	  //fic << val << " ";
	}
      
      fic << ";" << endl;
    }
  fic << "];";
  fic.close();
}
/*
  On trouve ici des fonctions utilitaire pour les images du genre 
  seuillage, calcul de gradient mais on touve aussi des fonction de 
  clipping pour les points ou des lignes
*/


/** Seuillage d'une image **/
void Seuille(IplImage *img,IplImage *seuillee,int seuil)
{
  CvSize s=cvGetSize(img);
  int r,c;
  CvScalar val;

  cvZero(seuillee);
  
  for( r = 0; r < s.height; r++ )
      for( c = 0; c < s.width; c++ )
	{
	  val=cvGet2D(img,r,c);
	  if(val.val[0]>seuil) cvSet2D(seuillee,r,c,cvScalar(255));
	}
}


/**
   Ré échantillonnage d'une image avec un facteur factor
*/
int DownSample(IplImage *src, IplImage *dst, int factor)
{
  int pasx,pasy;
  int SIZEX,SIZEY;
  int x,y;
  unsigned char *ptsrc, *ptdst;

  SIZEY = src->height/factor;
  SIZEX = src->width/factor;

  pasy = cvRound((double)src->height / (double)SIZEY);
  pasx = cvRound((double)src->width / (double)SIZEX);

  ptsrc = (unsigned char *)src->imageData;
  ptdst = (unsigned char *)dst->imageData;

  //Balayage du resultat
  for(int j=0;j<SIZEY;j++)
    for(int i=0;i<SIZEX;i++)
    {
      // Calcul du point le plus proche:
      x = cvRound( (double)i * pasx );
      y = cvRound( (double)j * pasy );

      x = x < (3*src->width)-1 ? x : (3*src->width)-1;
      y = y < (3*src->height)-1 ? y : (3*src->height)-1;

      //printf("%d\t%d --> %d\t%d\n",i,j,x,y);

      //nvalue = (int)( ((unsigned char *)imW)[di*y+x] );
     
      //R
      *(ptdst+j*dst->widthStep+3*i) = *(ptsrc+y*src->widthStep+3*x);
      //G
      *(ptdst+j*dst->widthStep+3*i+1) = *(ptsrc+y*src->widthStep+3*x+1);
      //B
      *(ptdst+j*dst->widthStep+3*i+2) = *(ptsrc+y*src->widthStep+3*x+2);
    }
  return 1;
}


/**
   Attention cette fonction ne fonctionne que pour une image sur 3 plans 
   elle effectue une moyenne dans un carre 2x2 sur les 3 plans
*/
void ImgPPPAverage2x2(IplImage * src, IplImage * dst)
{
  float val[3];
  int I1,I2,J1,J2;
  unsigned char * ptR, *ptG, *ptB;
  unsigned char * ptdR, *ptdG, *ptdB;

  ptR = (unsigned char *)src->imageData;
  ptG = (unsigned char *)src->imageData+1;
  ptB = (unsigned char *)src->imageData+2;
  ptdR = (unsigned char *)dst->imageData;
  ptdG = (unsigned char *)dst->imageData+1;
  ptdB = (unsigned char *)dst->imageData+2;

  for(int j=0;j<src->height;j+=2)
    for(int i=0;i<src->width;i+=2)
      {
	I1 = 3*i;
	I2 = 3*(i+1);

	J1 = j*dst->widthStep;
	J2 = (j+1)*dst->widthStep;

	val[0] = (*(ptR+J1+I1)*0.25)+(*(ptR+J1+I2)*0.25)+(*(ptR+J2+I1)*0.25)+(*(ptR+J2+I2)*0.25);
	val[1] = (*(ptG+J1+I1)*0.25)+(*(ptG+J1+I2)*0.25)+(*(ptG+J2+I1)*0.25)+(*(ptG+J2+I2)*0.25);
	val[2] = (*(ptB+J1+I1)*0.25)+(*(ptB+J1+I2)*0.25)+(*(ptB+J2+I1)*0.25)+(*(ptB+J2+I2)*0.25);

	*(ptdR+J1+I1) = *(ptdR+J1+I2) = *(ptdR+J2+I1) = *(ptdR+J2+I2) = (unsigned char)val[0];
	*(ptdG+J1+I1) = *(ptdG+J1+I2) = *(ptdG+J2+I1) = *(ptdG+J2+I2) = (unsigned char)val[1];
	*(ptdB+J1+I1) = *(ptdB+J1+I2) = *(ptdB+J2+I1) = *(ptdB+J2+I2) = (unsigned char)val[2];

      }


}


/**
   Attention cette fonction ne fonctionne que pour une image sur 1 plan
   elle effectue une moyenne dans un carre 2x2
*/
void ImgPAverage2x2(IplImage * src, IplImage * dst)
{
  unsigned char *ptsrc,*ptdst;//, *ptsrcL2,  *ptdstL2;
  int i,j;
  float moy;
  int ligne;

  ptsrc = (unsigned char *)src->imageData;
  ptdst = (unsigned char *)dst->imageData;
  ligne = src->widthStep;

  for(j=0;j<src->height;j+=2,ptsrc+=ligne,ptdst+=ligne)
    {
      /* Parcours de la ligne complete de 2 en 2 */
      for(i=0;i<src->widthStep;i+=2,ptsrc+=2,ptdst+=2)
	{
	  moy = ( (*ptsrc)*0.25 )+( (*(ptsrc+1))*0.25 )+( (*(ptsrc+ligne))*0.25 )+( (*(ptsrc+ligne+1))*0.25 );
	  // if(moy>200)
	  // 	    *ptdst = *(ptdst+1) = *(ptdst+ligne) = *(ptdst+ligne+1) = 255;
	  // 	  else
	  // 	    *ptdst = *(ptdst+1) = *(ptdst+ligne) = *(ptdst+ligne+1) = 0;
	  *ptdst = *(ptdst+1) = *(ptdst+ligne) = *(ptdst+ligne+1) = (unsigned char)moy;
	}
      
    }
}

/*
  Detecte les objets dans une image et extrait un contour par objet 
*/
void Contours(IplImage *img)
{
  CvMemStorage* storage = cvCreateMemStorage(0);
  CvSeq* contour = 0;
  IplImage* src;
  IplImage* dst = cvCreateImage( cvGetSize(src), 8, 3 );

  cvCopyImage(img,src);

  cvThreshold( src, src, 254, 255, CV_THRESH_BINARY_INV );
  cvNamedWindow( "Source", 0 );
  cvShowImage( "Source", src );
  
  cvFindContours( src, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
  cvZero( dst );
  
  for( ; contour != 0; contour = contour->h_next )
    {
      CvScalar color = CV_RGB( rand(), rand(), rand() );
      /* replace CV_FILLED with 1 to see the outlines */
      cvDrawContours( dst, contour, color, color, -1, CV_FILLED, 8 );
    }
  
  cvNamedWindow( "Components", 0 );
  cvShowImage( "Components", dst );
  

}


/**  Cohen-Sutherland algorithm for line segment clipping
     implementation 
     
     Given the image dimensions and the end points of a line segment,
     this method clips the line segment by adjusting the endpoints to
     the part of the segment that will be inside the image. In the
     case that the segment will have no points inside the image it
     will return false
*/
bool ClipLine(int right,int bottom,CvPoint *pt1,CvPoint * pt2)
{
  int x1 = pt1->x, y1 = pt1->y, x2 = pt2->x, y2 = pt2->y;
  // c1 tem um bit por cada caso
  // bit 0 set -> x1<0
  // bit 1 set -> x1>width-1
  // bit 2 set -> y1<0
  // bit 3 set -> y1>width-1
  
  int c1 = (x1 < 0) + (x1 > right) * 2 + (y1 < 0) * 4 + (y1 > bottom) * 8;
  
  // mesma coisa para c2 e x2,y2
  int c2 = (x2 < 0) + (x2 > right) * 2 + (y2 < 0) * 4 + (y2 > bottom) * 8;

  //printf("C : %d %d c1&c2 = %d  c1|c2 = %d\n",c1,c2,(c1 & c2),(c1 | c2));
  if( (c1 & c2) == 0 && (c1 | c2) != 0 )
    // não ultrapassam os mesmos limites, o q quer dizer que pelo menos parte esta
    // dentro da imagem.
    {
      int a;

      //printf("Clipping ...\n");
      
      if( c1 & 12 ) // ultrapassa o(s) limite(s) em y
	{
	  a = c1 < 8 ? 0 : bottom;
	  x1 += (int) (((int64) (a - y1)) * (x2 - x1) / (y2 - y1));
	  y1 = a;
	  c1 = (x1 < 0) + (x1 > right) * 2;
	}
      if( c2 & 12 )
	{
	  a = c2 < 8 ? 0 : bottom;
	  x2 += (int) (((int64) (a - y2)) * (x2 - x1) / (y2 - y1));
	  y2 = a;
	  c2 = (x2 < 0) + (x2 > right) * 2;
	}
      if( (c1 & c2) == 0 && (c1 | c2) != 0 )
	{
	  if( c1 )
	    {
	      a = c1 == 1 ? 0 : right;
	      y1 += (int) (((int64) (a - x1)) * (y2 - y1) / (x2 - x1));
	      x1 = a;
	      c1 = 0;
	    }
	  if( c2 )
	    {
	      a = c2 == 1 ? 0 : right;
	      y2 += (int) (((int64) (a - x2)) * (y2 - y1) / (x2 - x1));
	      x2 = a;
	      c2 = 0;
	    }
	}
      
      pt1->x = x1;
      pt1->y = y1;
      pt2->x = x2;
      pt2->y = y2;
    }
  
  return ( c1 | c2 ) == 0;
  
}


/* Clipping de la ligne dans une region delimitee par minx,miny,bottom, et right */
bool ClipLine(int minx, int miny, int right,int bottom,CvPoint *pt1,CvPoint * pt2)
{
  int x1 = pt1->x, y1 = pt1->y, x2 = pt2->x, y2 = pt2->y;
  // c1 tem um bit por cada caso
  // bit 0 set -> x1<0
  // bit 1 set -> x1>width-1
  // bit 2 set -> y1<0
  // bit 3 set -> y1>width-1
  
  int c1 = (x1 < minx) + (x1 > right) * 2 + (y1 < minx) * 4 + (y1 > bottom) * 8;
  
  // mesma coisa para c2 e x2,y2
  int c2 = (x2 < minx) + (x2 > right) * 2 + (y2 < miny) * 4 + (y2 > bottom) * 8;
  
  //printf("C : %d %d c1&c2 = %d  c1|c2 = %d\n",c1,c2,(c1 & c2),(c1 | c2));
  if( (c1 & c2) == 0 && (c1 | c2) != 0 )
    // não ultrapassam os mesmos limites, o q quer dizer que pelo menos parte esta
    // dentro da imagem.
    {
      int a;

      //printf("Clipping ...\n");
      
      if( c1 & 12 ) // ultrapassa o(s) limite(s) em y
	{
	  a = c1 < 8 ? minx : bottom;
	  x1 += (int) (((int64) (a - y1)) * (x2 - x1) / (y2 - y1));
	  y1 = a;
	  c1 = (x1 < minx) + (x1 > right) * 2;
	}
      if( c2 & 12 )
	{
	  a = c2 < 8 ? minx : bottom;
	  x2 += (int) (((int64) (a - y2)) * (x2 - x1) / (y2 - y1));
	  y2 = a;
	  c2 = (x2 < minx) + (x2 > right) * 2;
	}
      if( (c1 & c2) == 0 && (c1 | c2) != 0 )
	{
	  if( c1 )
	    {
	      a = c1 == 1 ? miny : right;
	      y1 += (int) (((int64) (a - x1)) * (y2 - y1) / (x2 - x1));
	      x1 = a;
	      c1 = 0;
	    }
	  if( c2 )
	    {
	      a = c2 == 1 ? miny : right;
	      y2 += (int) (((int64) (a - x2)) * (y2 - y1) / (x2 - x1));
	      x2 = a;
	      c2 = 0;
	    }
	}
      
      pt1->x = x1;
      pt1->y = y1;
      pt2->x = x2;
      pt2->y = y2;
    }
  return ( c1 | c2 ) == 0;
  
}


/* Fonction qui verifie la validite de parametres (position dans image)
   si le point est en dehors de l'image on modifie ses coordonnes pour le 
   mettre au bord de l'image
*/
void ClipPoint(CvPoint *pt,IplImage *img)
{
  //position en x
  if(pt->x<0) pt->x=0;
  else
    if(pt->x>img->width) pt->x = img->width-1;

  //position en y
  if(pt->y<0) pt->y=0;
  else
    if(pt->y>img->height) pt->y = img->height-1;
}

void ClipPoint(CvPoint *pt,int width, int height)
{
  //position en x
  if(pt->x<0) pt->x=0;
  else
    if(pt->x>width) pt->x = width-1;

  //position en y
  if(pt->y<0) pt->y=0;
  else
    if(pt->y>height) pt->y = height-1;
}

void ClipPoint(Point2D & pt,IplImage *img)
{
  //position en x
  if(pt.myx<0) pt.myx=0;
  else
    if(pt.myx>img->width) pt.myx = img->width-1;

  //position en y
  if(pt.myy<0) pt.myy=0;
  else
    if(pt.myy>img->height) pt.myy = img->height-1;
}


void ClipPoint(Point2D & pt,int width, int height)
{
  //position en x
  if(pt.myx<0) pt.myx=0;
  else
    if(pt.myx>width) pt.myx = width-1;

  //position en y
  if(pt.myy<0) pt.myy=0;
  else
    if(pt.myy>height) pt.myy = height-1;
}

/** Fonction qui calcule des points sur la normale a un segment passant par deux points */
int SegNormale(CvPoint pt1, CvPoint pt2, int taille, CvPoint *res)
{
  float m,norme;
  CvPoint2D32f N,V;

  //printf("%d %d  | %d %d\n",pt1.x,pt1.y,pt2.x,pt2.y);

  res[1].x = (int)((float)(pt1.x+pt2.x)/(float)2);
  res[1].y = (int)((float)(pt1.y+pt2.y)/(float)2);


  if (pt1.y == pt2.y)
    {  
      if(pt1.x == pt2.x) return 0;

      res[0].y = res[1].y - taille;
      res[2].y = res[1].y + taille;
      res[0].x = res[1].x;
      res[2].x = res[1].x;
    }
  else
    {
      m = -((pt2.x-pt1.x)/(pt2.y-pt1.y));
      
      N.x = res[1].x + 2*taille;
      N.y =  m * (N.x - res[1].x) + res[1].y;

 
      V.x = N.x- res[1].x;
      V.y = N.y- res[1].y;
      norme = sqrt(pow(V.x,2)+pow(V.y,2));

      V.x = V.x/norme;
      V.y = V.y/norme;

      // A + µ B
      res[0].x = (int)(res[1].x - taille*V.x);
      res[2].x = (int)(res[1].x + taille*V.x);
      res[0].y = (int)(res[1].y - taille*V.y);
      res[2].y = (int)(res[1].y + taille*V.y);
    }

  //printf("Points : pt1 %d %d  pt2 %d %d  milieu %d %d\t",pt1.x,pt1.y,pt2.x,pt2.y,milieu.x,milieu.y);

  return 1;
}


/** Fonction qui calcule des points sur la normale a un segment passant par deux points */
int SegNormale2(CvPoint pt1, CvPoint pt2, int taille, CvPoint *res)
{
  //CvPoint *res;
  float m,norme;
  CvPoint2D32f N,V;


  //res = (CvPoint*)malloc(3*sizeof(CvPoint));

  res[1].x = cvRound((float)(pt1.x+pt2.x)/(float)2);
  res[1].y = cvRound((float)(pt1.y+pt2.y)/(float)2);


  if (pt1.y == pt2.y)
    {  
      if(pt1.x == pt2.x) return 0;

      res[0].y = res[1].y - taille;
      res[2].y = res[1].y + taille;
      res[0].x = res[1].x;
      res[2].x = res[1].x;
    }
  else
    {
      m = -((pt2.x-pt1.x)/(pt2.y-pt1.y));
      
      N.x = res[1].x + 2*taille;
      N.y =  m * (N.x - res[1].x) + res[1].y;

 
      V.x = N.x- res[1].x;
      V.y = N.y- res[1].y;
      norme = sqrt(pow(V.x,2)+pow(V.y,2));

      V.x = V.x/norme;
      V.y = V.y/norme;

      // A + µ B

      res[0].x = cvRound(res[1].x - taille*V.x);
      res[2].x = cvRound(res[1].x + taille*V.x);
      res[0].y = cvRound(res[1].y - taille*V.y);
      res[2].y = cvRound(res[1].y + taille*V.y);
    }

  //printf("Points : pt1 %d %d  pt2 %d %d  milieu %d %d\t",pt1.x,pt1.y,pt2.x,pt2.y,milieu.x,milieu.y);

  return 1;
}

/** Calcul du gradient couleur **/
/*
  avec approche vectorielle Di Zenzo

  d² = p.cos²A + q.sin²A + 2t.sinA.cosA
  avec
  p = DRx² + DGx² + DBx²
  q = DRy² + DGy² + DBy²
  t = DRx.DRy + DGx.DGy + DBx.DBy
  
  d² maximale pour :
     A = 1/2.arctg(2t/(p-q))                                     {direction du gradient}
     |Gcoul|² = 1/2.[p + q + sqrt( (p+q)² - 4(pq-t²) )]             {module du gradient}
*/
void GradientCouleur(IplImage *img1,IplImage *img2,IplImage *img3, IplImage *edge)
{
  IplImage *DRx,*DRy,*DGx,*DGy,*DBx,*DBy;
  IplImage *p,*t,*q,*tmp,*sobeltmp;
  CvSize taille = cvGetSize(img1);
  

  DRx = cvCreateImage(taille,IPL_DEPTH_32F,1);
  DRy = cvCreateImage(taille,IPL_DEPTH_32F,1);
  DGx = cvCreateImage(taille,IPL_DEPTH_32F,1);
  DGy = cvCreateImage(taille,IPL_DEPTH_32F,1);
  DBx = cvCreateImage(taille,IPL_DEPTH_32F,1);
  DBy = cvCreateImage(taille,IPL_DEPTH_32F,1);
  p = cvCreateImage(taille,IPL_DEPTH_32F,1);
  t = cvCreateImage(taille,IPL_DEPTH_32F,1);
  q = cvCreateImage(taille,IPL_DEPTH_32F,1);
  tmp = cvCreateImage(taille,IPL_DEPTH_32F,1);
  sobeltmp = cvCreateImage(taille,IPL_DEPTH_16S,1);

  cvSobel(img1,sobeltmp,1,0);
  cvConvertScale(sobeltmp,DRx);
  cvSobel(img1,sobeltmp,0,1);
  cvConvertScale(sobeltmp,DRy);
  cvSobel(img2,sobeltmp,1,0);
  cvConvertScale(sobeltmp,DGx);
  cvSobel(img2,sobeltmp,0,1);
  cvConvertScale(sobeltmp,DGy);
  cvSobel(img3,sobeltmp,1,0);
  cvConvertScale(sobeltmp,DBx);
  cvSobel(img3,sobeltmp,0,1);
  cvConvertScale(sobeltmp,DBy);
  
  //Calc p
  cvPow(DRx,p,2);
  cvPow(DGx,tmp,2);
  cvAdd(tmp,p,p);
  cvPow(DBx,tmp,2);
  cvAdd(tmp,p,p);

  //Calc q
  cvPow(DRy,q,2);
  cvPow(DGy,tmp,2);
  cvAdd(tmp,q,q);
  cvPow(DBy,tmp,2);
  cvAdd(tmp,q,q);

  //Calc t
  cvMul(DRx,DRy,t);
  cvMul(DGx,DGy,tmp);
  cvAdd(tmp,t,t);
  cvMul(DBx,DBy,tmp);
  cvAdd(tmp,t,t);

  //Re utilisation des images DR* DG* et DB* pour le calcul final
  cvPow(t,DRx,2);  //t²
  cvMul(p,q,tmp);  //p*q
  cvSub(tmp,DRx,tmp);  //p*q-t²
  cvScale(tmp,DRx,4);  //4*(p*q-t²)
  cvAdd(p,q,tmp);
  cvPow(tmp,DGx,2);  //(p+q)²
  cvSub(DGx,DRx,tmp);  //(p+q)²-4*(p*q-t²)
  cvPow(tmp,DRx,0.5);    //sqrt((p+q)²-4*(p*q-t²))
  cvAdd(DRx,p,tmp);
  cvAdd(tmp,q,tmp);
  cvScale(tmp,tmp,0.5);
  cvPow(tmp,tmp,0.5);
  cvConvertScale(tmp,edge);
  
  cvReleaseImage(&DRx);
  cvReleaseImage(&DRy);
  cvReleaseImage(&DGx);
  cvReleaseImage(&DGy);
  cvReleaseImage(&DBx);
  cvReleaseImage(&DBy);
  cvReleaseImage(&p);
  cvReleaseImage(&q);
  cvReleaseImage(&t);
  cvReleaseImage(&tmp);
  cvReleaseImage(&sobeltmp);
}

// A = 1/2.arctg(2t/(p-q))
void GradientCouleurComplet(IplImage *img1,IplImage *img2,IplImage *img3, IplImage *edge, IplImage *dir)
{
  IplImage *DRx,*DRy,*DGx,*DGy,*DBx,*DBy;
  IplImage *p,*t,*q,*tmp,*sobeltmp;
  CvSize taille = cvGetSize(img1);
  

  DRx = cvCreateImage(taille,IPL_DEPTH_32F,1);
  DRy = cvCreateImage(taille,IPL_DEPTH_32F,1);
  DGx = cvCreateImage(taille,IPL_DEPTH_32F,1);
  DGy = cvCreateImage(taille,IPL_DEPTH_32F,1);
  DBx = cvCreateImage(taille,IPL_DEPTH_32F,1);
  DBy = cvCreateImage(taille,IPL_DEPTH_32F,1);
  p = cvCreateImage(taille,IPL_DEPTH_32F,1);
  t = cvCreateImage(taille,IPL_DEPTH_32F,1);
  q = cvCreateImage(taille,IPL_DEPTH_32F,1);
  tmp = cvCreateImage(taille,IPL_DEPTH_32F,1);
  sobeltmp = cvCreateImage(taille,IPL_DEPTH_16S,1);

  cvSobel(img1,sobeltmp,1,0);
  cvConvertScale(sobeltmp,DRx);
  cvSobel(img1,sobeltmp,0,1);
  cvConvertScale(sobeltmp,DRy);
  cvSobel(img2,sobeltmp,1,0);
  cvConvertScale(sobeltmp,DGx);
  cvSobel(img2,sobeltmp,0,1);
  cvConvertScale(sobeltmp,DGy);
  cvSobel(img3,sobeltmp,1,0);
  cvConvertScale(sobeltmp,DBx);
  cvSobel(img3,sobeltmp,0,1);
  cvConvertScale(sobeltmp,DBy);
  
  //Calc p
  cvPow(DRx,p,2);
  cvPow(DGx,tmp,2);
  cvAdd(tmp,p,p);
  cvPow(DBx,tmp,2);
  cvAdd(tmp,p,p);

  //Calc q
  cvPow(DRy,q,2);
  cvPow(DGy,tmp,2);
  cvAdd(tmp,q,q);
  cvPow(DBy,tmp,2);
  cvAdd(tmp,q,q);

  //Calc t
  cvMul(DRx,DRy,t);
  cvMul(DGx,DGy,tmp);
  cvAdd(tmp,t,t);
  cvMul(DBx,DBy,tmp);
  cvAdd(tmp,t,t);

  //Re utilisation des images DR* DG* et DB* pour le calcul final
  cvPow(t,DRx,2);  //t²
  cvMul(p,q,tmp);  //p*q
  cvSub(tmp,DRx,tmp);  //p*q-t²
  cvScale(tmp,DRx,4);  //4*(p*q-t²)
  cvAdd(p,q,tmp);
  cvPow(tmp,DGx,2);  //(p+q)²
  cvSub(DGx,DRx,tmp);  //(p+q)²-4*(p*q-t²)
  cvPow(tmp,DRx,0.5);    //sqrt((p+q)²-4*(p*q-t²))
  cvAdd(DRx,p,tmp);
  cvAdd(tmp,q,tmp);
  cvScale(tmp,tmp,0.5);
  cvPow(tmp,tmp,0.5);
  cvConvertScale(tmp,edge);
  

  //2*t mis dans t
  cvAdd(t,t,t);
  //p-q mis dans p
  //cvSub(p,q,p);
  //2t/(p-q) mis dans q
  //cvDiv(t,p,q);

  register unsigned char *ptdir,*ptp,*ptq,*ptt;
  ptdir = (unsigned char *)dir->imageData;
  ptt = (unsigned char *)t->imageData;
  ptp = (unsigned char *)p->imageData;
  ptq = (unsigned char *)q->imageData;

  for(register int i=0;i<dir->imageSize;i++)
    {
      *ptdir = (unsigned char)atan( 0.5*(double)*ptt/(double)(*ptp-*ptq) );
      ptdir++;
      ptt++;
      ptp++;
      ptq++;
    }


  cvReleaseImage(&DRx);
  cvReleaseImage(&DRy);
  cvReleaseImage(&DGx);
  cvReleaseImage(&DGy);
  cvReleaseImage(&DBx);
  cvReleaseImage(&DBy);
  cvReleaseImage(&p);
  cvReleaseImage(&q);
  cvReleaseImage(&t);
  cvReleaseImage(&tmp);
  cvReleaseImage(&sobeltmp);
}


/**
   Dilate the image
**/
void IplImageDilate(IplImage *img,int iter)
{
  IplConvKernel* B=NULL;

  //B = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_RECT,NULL);
  cvDilate(img,img,B,iter);
  //cvReleaseStructuringElement(&B);
}

/**
   Erode the image
**/
void IplImageErode(IplImage *img,int iter)
{
  IplConvKernel* B=NULL;

  //B = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_RECT,NULL);
  cvErode(img,img,B,iter);
  //cvReleaseStructuringElement(&B);
}

/** Gradient Morphologique **/
/*
  G = Dil(img) - Ero(img)
*/
void MorphoGrad(IplImage *imgin,IplImage *imgout)
{
  IplImage *tmp = cvCreateImage(cvGetSize(imgin),8,3);
  IplConvKernel* B = NULL; //cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_RECT,NULL);
  cvZero(tmp);
  cvMorphologyEx(imgin,imgout,tmp,B,CV_MOP_GRADIENT,1);
  //cvReleaseStructuringElement(&B);
  cvReleaseImage(&tmp);
}
