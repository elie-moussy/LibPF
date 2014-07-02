#include "shape.h"

/*
 * Constructeur de la classe
 * a partir d'un nombre de points fait une allocation de la liste des coordonnes des
 * points de la forme et des vecteurs de rayons et angles
 */
GlobalShapeDesc::GlobalShapeDesc(int nb)
{
  /* recopie du nombre de points */
  nbpts = nb;

  /* init du pointeur de fin de points */
  ptfin = 0;

  /* alloc de la liste de points */
  ptlist = (CvPoint*)calloc(nbpts,sizeof(CvPoint));
  
  /* points de reference */
  refid=0;

  center.x=0;
  center.y=0;
  
  /* alloc des vecteurs de forme */
  rayons_reference = (double*)calloc(nbpts,sizeof(double));
  angles_reference = (double*)calloc(nbpts,sizeof(double)); 
  rayons = (double*)calloc(nbpts,sizeof(double));
  angles = (double*)calloc(nbpts,sizeof(double));
}


GlobalShapeDesc::~GlobalShapeDesc()
{
  delete ptlist;
  delete rayons;
  delete angles;
}

/*
 * Ajout d'un nouveau point dans la liste des points
 */
void GlobalShapeDesc::Add(CvPoint *ptnew)
{
  ptlist[ptfin].x = ptnew->x;
  ptlist[ptfin].y = ptnew->y;
  ptfin++;
}


/*
 * Calcul de la representation de la forme a partir d'une liste de points 
 */
void GlobalShapeDesc::CalcGlobalShapeDescRef(CvPoint *center, CvPoint *pts)
{
  CalcShape(center,pts,rayons_reference,angles_reference,nbpts);  
}

void GlobalShapeDesc::CalcGlobalShapeDescRef(CvPoint *pts)
{
  CalcShape(pts,pts+1,rayons_reference,angles_reference,nbpts);  
}

/*
 * Calcul de la representation de la forme a partir de la liste de points connus 
 */
void GlobalShapeDesc::CalcGlobalShapeDescRef()
{
  CalcShape(ptlist,ptlist+1,rayons_reference,angles_reference,nbpts);
}
/*
 * Calcul de la representation de la forme a partir d'une liste de points 
 */
void GlobalShapeDesc::CalcGlobalShapeDesc(CvPoint *pts)
{
  CalcShape(pts,pts+1,rayons,angles,nbpts);  
}
void GlobalShapeDesc::CalcGlobalShapeDesc(CvPoint *pts,int *mask)
{
  CalcShape(pts,pts+1,rayons,angles,nbpts,mask);  
}

void GlobalShapeDesc::CalcGlobalShapeDesc(CvPoint *center,CvPoint *pts,int *mask)
{
  CalcShape(center,pts,rayons,angles,nbpts,mask);  
}

/* Fonctions d'affichage des courbes rayon et angles */

void GlobalShapeDesc::DispShapeDescRef()
{
  CvScalar color = CV_RGB(255,0,0);
  int sizeray = 256;
  int sizeangl = 256;
  int sizex=512;
  double factorx=(double)sizex/(double)nbpts;
  double anglfactor = (double)sizeangl/DEUXPI;

  IplImage *img_disp_rayons=cvCreateImage(cvSize((int)(nbpts*factorx),(int)(sizeray)),IPL_DEPTH_8U,3);
  IplImage *img_disp_angles=cvCreateImage(cvSize((int)(nbpts*factorx),(int)(sizeangl)),IPL_DEPTH_8U,3);
  cvZero(img_disp_rayons);
  cvZero(img_disp_angles);

  CvPoint debray,finray;
  CvPoint debangl,finangl;

  debray.x = 0;
  debray.y = sizeray-(int)(rayons_reference[0]*sizeray); 
  debangl.x = 0;
  debangl.y = sizeangl-(int)(angles_reference[0]*anglfactor);
  cvCircle(img_disp_rayons,debray,2,CV_RGB(0,0,255),-1,8,0);
  cvCircle(img_disp_angles,debangl,2,CV_RGB(0,0,255),-1,8,0);
  for(int i=1;i<nbpts;i++)
    {
      finray.x = (int)(i*factorx);
      finray.y = sizeray-(int)(rayons_reference[i]*sizeray);
      finangl.x = (int)(i*factorx);
      finangl.y = sizeangl-(int)(angles_reference[i]*anglfactor);

      cvLine(img_disp_rayons,debray,finray,color,1,8,0);
      cvLine(img_disp_angles,debangl,finangl,color,1,8,0);

      cvCircle(img_disp_rayons,finray,1,CV_RGB(0,255,0),-1,8,0);
      cvCircle(img_disp_angles,finangl,1,CV_RGB(0,255,0),-1,8,0);
      
      debray = finray;
      debangl = finangl;
    }

  cvNamedWindow("RAYONS REF",0);
  cvNamedWindow("ANGLES REF",0);
  cvShowImage("RAYONS REF",img_disp_rayons);
  cvShowImage("ANGLES REF",img_disp_angles);
  cvWaitKey(0);
  cvReleaseImage(&img_disp_rayons);
  cvReleaseImage(&img_disp_angles);
}

void GlobalShapeDesc::DispShapeDesc()
{
  CvScalar color = CV_RGB(255,0,0);
  int sizeray = 256;
  int sizeangl = 256;
  int sizex=512;
  double factorx=(double)sizex/(double)nbpts;
  double anglfactor = (double)sizeangl/DEUXPI;

  IplImage *img_disp_rayons=cvCreateImage(cvSize((int)(nbpts*factorx),(int)(sizeray)),IPL_DEPTH_8U,3);
  IplImage *img_disp_angles=cvCreateImage(cvSize((int)(nbpts*factorx),(int)(sizeangl)),IPL_DEPTH_8U,3);
  cvZero(img_disp_rayons);
  cvZero(img_disp_angles);

  CvPoint debray,finray;
  CvPoint debangl,finangl;

  debray.x = 0;
  debray.y = sizeray-(int)(rayons[0]*sizeray); 
  debangl.x = 0;
  debangl.y = sizeangl-(int)(angles[0]*anglfactor);
  cvCircle(img_disp_rayons,debray,2,CV_RGB(0,0,255),-1,8,0);
  cvCircle(img_disp_angles,debangl,2,CV_RGB(0,0,255),-1,8,0);
  for(int i=1;i<nbpts;i++)
    {
      finray.x = (int)(i*factorx);
      finray.y = sizeray-(int)(rayons[i]*sizeray);
      finangl.x = (int)(i*factorx);
      finangl.y = sizeangl-(int)(angles[i]*anglfactor);

      cvLine(img_disp_rayons,debray,finray,color,1,8,0);
      cvLine(img_disp_angles,debangl,finangl,color,1,8,0);

      cvCircle(img_disp_rayons,finray,1,CV_RGB(0,255,0),-1,8,0);
      cvCircle(img_disp_angles,finangl,1,CV_RGB(0,255,0),-1,8,0);
      
      debray = finray;
      debangl = finangl;
    }

  cvNamedWindow("RAYONS",0);
  cvNamedWindow("ANGLES",0);
  cvShowImage("RAYONS",img_disp_rayons);
  cvShowImage("ANGLES",img_disp_angles);
  cvWaitKey(0);
  cvReleaseImage(&img_disp_rayons);
  cvReleaseImage(&img_disp_angles);
}



/* Calcul de la similarite entre la reference et la forme correspondant a la liste de points en parametres */
double GlobalShapeDesc::GlobalShapeDescCompar(int *mask, int nbpts)
{
  double distray=0.0,distangl=0.0;
  int cpt=0;

  for(int i=0;i<nbpts;i++)
    {
      if(mask[i])
	{
	  //printf("R = %f     %f\n",rayons_reference[i],rayons[i]);
	  distray += sqrt(rayons_reference[i]*rayons[i]);
	  distangl += sqrt(angles_reference[i]*angles[i]);
	  cpt++;
	}
    }
  
  //printf("\nDistances : dR = %f  dA = %f\navec matching = %f \n",1.0-distray,1.0-distangl,1.0-((double)cpt/(double)nbpts));
  //printf("Prob = %f\n",exp(-(3.0-distray-distangl-((double)cpt/(double)nbpts))/0.04));

  return exp(-(3.0-distray-distangl-((double)cpt/(double)nbpts))/0.04);
}


/* Calcul de la similarite entre la reference et la forme correspondant a la liste de points en parametres */
double GlobalShapeDesc::GlobalShapeDescCompar2(int *mask)
{
  double distray=0.0,distangl=0.0;
  int cpt=0;

  for(int i=0;i<nbpts;i++)
    {
      if(mask[i])
	{
	  //printf("R = %f     %f\n",rayons_reference[i],rayons[i]);
	  distray += pow(rayons_reference[i]-rayons[i],2);
	  distangl += pow(angles_reference[i]-angles[i],2);
	  cpt++;
	}	
    }
  
  printf("\nDistances : dR = %f  dA = %f\n",1.0/(distray/(double)cpt),1.0/(distangl/(double)cpt));

}
