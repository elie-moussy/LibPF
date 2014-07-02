#include "shapedesc/ShapeModels.h"


/**
   Auteur : Ludovic Brèthes

   Description : Constructeur de la classe
   \param dirname : repertoire contenant des modeles de forme
   \param modelpref : prefixe des noms de fichiers contenant des modeles
   \param modelext : extention des fichiers modeles
*/
ShapeModels::ShapeModels(const char *splinefic, const char * splineconfig, const char * splineext)
{
  char modelfile[256];
  char modelfileconfig[256];

  /* Recopie des parametres */
  strcpy(this->splinefic,splinefic);
  strcpy(this->splineconfig,splineconfig);
  strcpy(this->splineext,splineext);

  /* Init des autres parametres */
 
  couleur = CV_RGB(255,0,0);

  sprintf(modelfile,"%s.%s",splinefic,splineext);
  sprintf(modelfileconfig,"%s.%s",splineconfig,splineext);
  
  models = new MultiBSpline2D(modelfile,modelfileconfig);;
  nbmodels = models->nbconfigs;

  /********************* Construction de la signature initiale de la forme **********************************/
  models->transform(160,120,0,3,0);
  //this->ViewModel();

  glsd = new GlobalShapeDesc(models->numpoints);
  liste = (CvPoint*)calloc(models->numpoints,sizeof(CvPoint));  

  CvPoint cog,cmin,cmax;
  cmin.x = 400;
  cmin.y = 400;
  cmax.x = -1;
  cmax.y = -1;
  for(int i=0;i<models->numpoints;i++)
    {
      liste[i].x = (int)(models->points[i].myx);
      liste[i].y = (int)(models->points[i].myy);      
      if(liste[i].y<cmin.y) cmin.y = liste[i].y;
      if(liste[i].y>cmax.y) cmax.y = liste[i].y;
      if(liste[i].x<cmin.x) cmin.x = liste[i].x;
      if(liste[i].x>cmax.x) cmax.x = liste[i].x;
    }
  cog.x = (cmin.x+cmax.x)/2;
  cog.y = (cmin.y+cmax.y)/2;
  //glsd->CalcGlobalShapeDescRef(&cog,liste);
  //glsd->DispShapeDescRef();
}

/**
   Auteur : Ludovic Brèthes

   Description : Destructeur de la classe
*/
ShapeModels::~ShapeModels()
{

  delete models;

}


MultiBSpline2D * ShapeModels::GetShapeModels()
{
  return models;
}

/**
   Auteur : Ludovic Brèthes

   Description : Methode de visualisation d'un modele de la liste ou de tous
   \param num : numero de modele a visualiser, si num==-1, on affiche tous les modeles
*/
void ShapeModels::ViewModel(int num, char *nomfen)
{
  IplImage *imgview;
  int width=200;
  int height=200;
  int i;

  if(num!=-1) 
    {
      imgview = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
      cvZero(imgview);
      models->transform(width/2,height/2,0,4,num);
      models->draw(imgview,couleur);
    }
  else
    {
      imgview = cvCreateImage(cvSize(nbmodels*width,height),IPL_DEPTH_8U,3);
      cvZero(imgview);

      for(i=0;i<nbmodels;i++)
	{
	  models->transform(i*width+width/2,height/2,0,4,i);
	  models->draw(imgview,couleur);

	}
    }

  cvNamedWindow(nomfen,0);
  cvShowImage(nomfen,imgview);
  cvWaitKey(0);

  cvReleaseImage(&imgview);
  cvDestroyWindow(nomfen);
}

void ShapeModels::ViewModel2(int num, char *nomfen)
{
  IplImage *imgview;
  int width=200;
  int height=200;
  int i;

  if(num!=-1) 
    {
      imgview = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
      cvSet(imgview,cvScalar(255,255,255));
      models->transform(width/2,height/2,0,4,num);
      models->draw(imgview,CV_RGB(0,0,0));
    }
  else
    {
      imgview = cvCreateImage(cvSize(nbmodels*width,height),IPL_DEPTH_8U,3);
      cvSet(imgview,cvScalar(255,255,255));

      for(i=0;i<nbmodels;i++)
	{
	  models->transform(i*width+width/2,height/2,0,4,i);
	  models->draw(imgview,CV_RGB(0,0,0));

	}
    }

  cvSaveImage("modeles.jpg",imgview);
  cvNamedWindow(nomfen,0);
  cvShowImage(nomfen,imgview);
  cvWaitKey(0);

  cvReleaseImage(&imgview);
  cvDestroyWindow(nomfen);
}


void ShapeModels::DrawModel(IplImage *img,int num, double x, double y, double theta, double scale, CvScalar coul)
{
  models->transform(x,y,theta,scale,num);
  models->draw(img,coul);
}
