#include "imgproducer/ImgProducer.h"
#include <opencv2/legacy/legacy.hpp>

/* Constructeur */
ImgProducer::ImgProducer(int w, int h)
{
  /* Affectation des dimensions de reference des images traitees */
  width = w;
  height = h;
  
  /* Init du nombre d'images traitees */
  imgCpt = 0; 

  /* Reservation/allocation des images et parametres necessaires */
  cout << "--> ImgProducer (CO) \t:\t Images allocations\n";
  alloc();
}

/* Destructeur */
ImgProducer::~ImgProducer()
{
  /* Liberation des images */
  for(int i=0;i<img.size();i++)
    {
      cout << "Free " << imgNM[i] << " image" << endl;
      cvReleaseImage(&(img[i]));
      cvReleaseImage(&(imgPrec[i]));
    }     
}


/* Ajout d'une image et allocation de celle ci */
int ImgProducer::imgRegister(const char* nm, int w, int h, int depth, int plane)
{ 
  if(imgID[nm]||(!imgNM.empty()&&imgNM[0]==nm))
    {
      cout << "|ERROR|--> ImgProducer (RE) \t:\t " << nm << " label is already used ! \n";
      cout << "                            \t \t If you need the same image use imgGetPtr(\"" << nm ;
      cout << "\") or change your label\n";
      return -1;
    }

  /* Id de la nouvelle image */
  int id=img.size();
  
  //cout << "Create " << nm << " with ID = " << img.size() << endl;
  
  /* Ajout dans la map de la correspondance indice nom */
  imgID[nm]=id;
  
  /* Ajout du nom dans le vecteur de noms */
  imgNM.push_back(nm);
  
  /* Allocation de l'image et ajout dans le vecteur */
  IplImage* imgTmp = cvCreateImage(cvSize(w,h),depth,plane);
  img.push_back(imgTmp);

  /* Image precedente */
  IplImage* imgTmpPrec = cvCreateImage(cvSize(w,h),depth,plane);
  imgPrec.push_back(imgTmpPrec);

  string dispStr;
  FormatStr(imgNM[id],dispStr,10);

  cout << "Create [ " << id << " , " << dispStr <<  " , " << imgTmp->width << "x" << imgTmp->height <<  " , " << imgTmp->nChannels << " ]" << endl;
  
  return imgID[nm];
}

void ImgProducer::disp(int calcall)
{
  string dispStr;

  /* Affichage de l'image RGB de reference */
  Show("RGB",imgSRC,0,0,0);
  
  if(calcall)
    process();

  for(int i=0;i<img.size();i++)
    {      
      FormatStr(imgNM[i],dispStr,10);
      cout << "|\t" << i << "\t|\t" << dispStr <<  "\t|\t" << img[i]->width << "x" << img[i]->height <<  "\t|\t" << img[i]->nChannels << "\t|" << endl;
      Show((char*)imgNM[i].c_str(),img[i],0);
    }

  cvWaitKey(0);
}

/* Execution de tous les traitements d'images */
void ImgProducer::process(IplImage* imgIn)
{
  cout << "Processing all images\n";
  
  /* init image */
  if(imgIn) set(imgIn);
  
  /* execution de tous les traitements */
  calcGRAY();
  calcDIFFABS();
  calcPYRDOWN();
  calcEDGE();
  calcSKINPROB();
  calcFLOWMASK();
}

/* Alloc de tous les elements necessaires */
void ImgProducer::alloc()
{
  /* Allocation des modules de traitement */
  allocGRAY();
  allocDIFFABS();
  allocPYRDOWN();
  allocFGRAY();
  allocEDGE();
  allocSKINPROB();
  allocFLOWMASK();

  /* Allocation du vecteur flag */
  imgOK=new int[img.size()];
  size=sizeof(int)*img.size();
  memset(imgOK,0,size);
}


/*********************************************************************/
/*                    TRAITEMENTS D'IMAGES                           */
/*********************************************************************/



/**********************************************************************/
/* Calcul image en niveau de gris                                     */
/**********************************************************************/
void ImgProducer::allocGRAY()
{
  /* Niveaux de gris */
  idGRAY = imgRegister("GRAY",width,height,IPL_DEPTH_8U,1);
}

void ImgProducer::calcGRAY()
{
  cvCvtColor(imgSRC,img[idGRAY],CV_BGR2GRAY);
  imgOK[idGRAY] = 1;
}


/*******************************************************************/
/* Calcul image de difference absolue (en niveau de gris)          */
/*******************************************************************/
void ImgProducer::allocDIFFABS()
{
  /* Difference absolue en niveaux de gris */
  idDIFFABS = imgRegister("DIFFABS",width,height,IPL_DEPTH_8U,1);
}

void ImgProducer::calcDIFFABS()
{
  cvAbsDiff(imgGRAY(),imgGRAYPrec(),img[idDIFFABS]);
  imgOK[idDIFFABS] = 1;
}


/**********************************************************************/
/* Calcul image filtree par sous echantillonnage                      */
/**********************************************************************/
void ImgProducer::allocPYRDOWN
()
{
  /* RGB sous echantillonnee */
  idPYRDOWN = imgRegister("PYRDOWN",width/2,height/2,IPL_DEPTH_8U,3);
}

void ImgProducer::calcPYRDOWN()
{
  /* Filtrage de l'image d'entree par sous echantillonnage */
  cvPyrDown(imgSRC,img[idPYRDOWN],CV_GAUSSIAN_5x5);
  imgOK[idPYRDOWN] = 1;
}


/**********************************************************************/
/* Calcul image de contours (canny)                                   */
/**********************************************************************/
void ImgProducer::allocEDGE()
{
  /* Contours */
  idEDGE = imgRegister("EDGE",width,height,IPL_DEPTH_8U,1);
}

void ImgProducer::calcEDGE()
{
  /* calcul des contours */
  cvCanny(imgFGRAY(),img[idEDGE],20,40);
  imgOK[idEDGE] = 1;
}

/**********************************************************************/
/* Calcul image GRAY filtree                                          */
/**********************************************************************/
void ImgProducer::allocFGRAY()
{
  /* Niveaux de gris sous echantillonne */
  idMGRAY = imgRegister("MGRAY",width/2,height/2,IPL_DEPTH_8U,1);
  
  /* Niveaux de gris apres filtrage PyrDown PyrUp */
  idFGRAY = imgRegister("FGRAY",width,height,IPL_DEPTH_8U,1);
}

void ImgProducer::calcFGRAY()
{
  ///* Conversion de l'image couleur sous echantillonnee en gris */
  //cvCvtColor(imgPYRDOWN(),img[idMGRAY],CV_BGR2GRAY);
  
  /* Sous echantillonnage de l'image GRAY */
  cvPyrDown(imgGRAY(),img[idMGRAY],CV_GAUSSIAN_5x5);

  /* Retour a la dimension correcte */
  cvPyrUp(img[idMGRAY],img[idFGRAY],CV_GAUSSIAN_5x5);

  imgOK[idMGRAY] = 1;
  imgOK[idFGRAY] = 1;
}

/**********************************************************************/
/* Calcul image de probabilite couleur peau                           */
/**********************************************************************/
void ImgProducer::allocSKINPROB()
{
  /* Image de proba couleur peau */
  idSKINPROB = imgRegister("SKINPROB",width,height,IPL_DEPTH_8U,1);

  /* Plans pour separation R,G,B */
  idP1=imgRegister("RGBP1",width,height,IPL_DEPTH_8U,1);
  idP2=imgRegister("RGBP2",width,height,IPL_DEPTH_8U,1);
  idP3=imgRegister("RGBP3",width,height,IPL_DEPTH_8U,1);
  
  images = new IplImage*[3];
  images[0] = img[idP1];
  images[1] = img[idP2];
  images[2] = img[idP3];

  /* Charge le modele de couleur */
  skinModel.load("skincolormodel_RGB");

  /* Seuillage par defaut */
  skinModel.threshold(50);

  /* Objet de segmentation couleur peau */
  //scs = new SkinColorSegmentation("RGB",width,height);

  /* Seuillage par defaut de l'histogramme */
  //scs->setThreshold(50);

}

void ImgProducer::calcSKINPROB()
{
  /* Separation en trois plans */
  cvSplit(imgSRC,img[idP3],img[idP2],img[idP1],NULL);
  
  /* Segmentation par projection de l'histogramme */
  cvCalcBackProject(images,img[idSKINPROB],skinModel.hist);

  imgOK[idSKINPROB] = 1;
}

/**********************************************************************/
/* Calcul masque flot optique                                         */
/**********************************************************************/
void ImgProducer::allocFLOWMASK() 
{ 
  /* Alloc des images */
  idGIMGX=imgRegister("GIMGX",width,height,IPL_DEPTH_32F,1);
  idGIMGY=imgRegister("GIMGY",width,height,IPL_DEPTH_32F,1);
  idFLOWMASK=imgRegister("FLOWMASK",width,height,IPL_DEPTH_8U,1);

  /* Init de constantes */ 
  xremainstep=img[idGIMGX]->widthStep-width*4;
  yremainstep=img[idGIMGY]->widthStep-width*4;
  mremainstep=img[idFLOWMASK]->widthStep-width;

  /* Init de certains parametres */
  lambda = 0.01;
  criteria.type = CV_TERMCRIT_ITER;
  criteria.max_iter = 3;
  
}

void ImgProducer::calcFLOWMASK() 
{ 
  /* Calcul du flot */
  cvCalcOpticalFlowHS(imgFGRAY(),imgFGRAYPrec(),0,img[idGIMGX],img[idGIMGY],lambda,criteria);

  /* Calcul du masque */
  int i,j;
  unsigned char  *mask;
  float * fx, * fy;
  fx=(float *)img[idGIMGX]->imageData;
  fy=(float *)img[idGIMGY]->imageData;
  mask=(unsigned char *)img[idFLOWMASK]->imageData;
  for(j=0;j<height;j++)
    {
      for(i=0;i<width;i++)
	{
	  if((fabs(*fx)>0.5)||(fabs(*fy)>0.5)) *mask=255;
	  else *mask=0;
	  fx++;
	  fy++;
	  mask++;
	}
      fx+=xremainstep;
      fy+=yremainstep;
      mask+=mremainstep;
    }

  imgOK[idGIMGX] = 1;  
  imgOK[idGIMGY] = 1;  
  imgOK[idFLOWMASK] = 1;
}

