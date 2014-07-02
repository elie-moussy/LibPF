#include "twoDDetector/motionBlobDetector.h"

/* Constructeur */
motionBlobDetector::motionBlobDetector(string file, ImgProducer* imgB):twoDDetector(file,imgB)
{
  this->detectorID = 2;
  double cov[4]={0,0,0,0};

  /* Lecture des parametres dans le fichier */
  ifstream fichier;
  
  fichier.open(file.c_str(), ios::in);
  if(!fichier)
    {
      cout << "|ERROR|--> motionBlobDetector \t:\t Opening file [" << file << "] ERROR" << endl;
      return;
    }

  /* Lecture de la covariance associee au melange de gaussienne */
  FindStr2(fichier,"<DetectorCov>");
  if(!fichier.eof()) 
    { 
      cout << "--> motionBlobDetector \t:\t Loading covariance" << endl;
      fichier >> cov[0];
      fichier >> cov[1];
      fichier >> cov[2];
      fichier >> cov[3];
    }

  /* Lectutre du facteur d'echelle */
  factor=10;
  FindStr2(fichier,"<DetectorScaleFactor>");
  if(!fichier.eof()) fichier >> factor;
  cout << "--> motionBlobDetector \t:\t Scale factor = " << factor << endl;
  
  /* Lecture du seuil proba peau */
  SeuilProba = 10;
  FindStr2(fichier,"<DetectorThreshold>");
  if(!fichier.eof()) fichier >> SeuilProba;
  cout << "--> motionBlobDetector \t:\t Likelihood Threshold = " << SeuilProba << endl;
  
  /* Lecture des seuils Haut et Bas hysteresis */
  SeuilProbaBas = 20;
  SeuilProbaHaut = 100;
  FindStr2(fichier,"<DetectorHysteresisThreshold>");
  if(!fichier.eof())
    {
      fichier >> SeuilProbaBas;
      fichier >> SeuilProbaHaut;
    }  
  cout << "--> motionBlobDetector \t:\t Hysteresis Threshold = " << SeuilProbaBas << " - " << SeuilProbaHaut << endl;
      
  /* fermeture du fichier */
  fichier.close();

  /* Appel de la fonction d'init */
  init(cov);
}


motionBlobDetector::motionBlobDetector(int w, int h, ImgProducer* imgB, double* cov, int fact,int th,int thB,int thH):twoDDetector(w,h,imgB)
{
  this->detectorID = 2;
  SeuilProba=th;
  SeuilProbaBas=thB;
  SeuilProbaHaut=thH;
  factor=fact;
  init(cov);
} 

void motionBlobDetector::init(double* cov)
{
  /* Calcul factor */
  ffactor = 1.0/(double)this->factor;
  imgSZ=cvSize(width/factor,height/factor);

  /* Alloc images */
  img_proba = cvCreateImage(imgSZ,IPL_DEPTH_8U,1);
  img_reduced = cvCreateImage(imgSZ,IPL_DEPTH_8U,1);
  img_reduced_prec = cvCreateImage(imgSZ,IPL_DEPTH_8U,1);
  img_mask_haut = cvCreateImage(imgSZ,IPL_DEPTH_8U,1);
  img_mask_bas = cvCreateImage(imgSZ,IPL_DEPTH_8U,1);
  img_regions = cvCreateImage(imgSZ,IPL_DEPTH_8U,1);
  img_tmp = cvCreateImage(imgSZ,IPL_DEPTH_8U,1);
  img_tmp2 = cvCreateImage(imgSZ,IPL_DEPTH_8U,1);

  /* Alloc structures */
  storage = cvCreateMemStorage(0);
  contour = 0;

  /* Allocation d'une mixture de gaussiennes */
  this->gm = new GaussianMixture(NBMAXBLOBS,2,NULL,cov);
  printf("--> motionBlobDetector (Constructor) \t:\t Constructor done...\n");
}

motionBlobDetector::~motionBlobDetector()
{
  cvReleaseImage(&img_reduced);
  cvReleaseImage(&img_reduced_prec);
  cvReleaseImage(&img_mask_haut);
  cvReleaseImage(&img_mask_bas);
  cvReleaseImage(&img_regions);
  cvReleaseImage(&img_tmp);
  cvReleaseImage(&img_tmp2);
  cvReleaseImage(&img_proba);
  delete this->gm;
}

/* Extraction des blobs */
GaussianMixture* motionBlobDetector::process(int offsetX, int offsetY, double scale)
{
  int coul,i;
  CvSeq *ptc;
  CvRect blob_rect;
  CvPoint blob_center; 
 
  /* Sous echantillonnage de l'image d'entree */
  //nearest-neigboor interpolation
  cvResize(imgBank->imgGRAY(),img_reduced,CV_INTER_NN);
  cvResize(imgBank->imgGRAYPrec(),img_reduced_prec,CV_INTER_NN);
  cvAbsDiff(img_reduced,img_reduced_prec,img_proba);

  /* Reduction du bruit par moyenne avec un bloc 2x2 */
  ImgPAverage2x2(img_proba,img_proba);
  
  /************************* Selection des blobs en fonction d'un seuil ******************************/

  /* Seuillage de l'image de proba pour ne garder que les pixels de probabilité très fiable */
  cvThreshold(img_proba,img_mask_haut,SeuilProbaHaut,255,CV_THRESH_BINARY);

  /* Seuillage de l'image pour obtenir un masque correspondant a tout les pixels qui doivent
     etre elimines */
  cvThreshold(img_proba,img_mask_bas,SeuilProbaBas,255,CV_THRESH_BINARY_INV);

  /* Elimination des pixels de proba < SeuilBas */
  cvSet(img_proba,cvScalar(0),img_mask_bas);

  /* Regroupement des pixels en regions connexes */
  cvFindContours( img_proba, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );

  /* Filtrage des blobs et calcul des rectangles englobant */
  nbdetected=0;
  detectedROI.clear();
  detectedCENTER.clear();

  for(ptc = contour, i=0 ; ptc != 0; ptc = ptc->h_next, i++ )
    {
      coul = nbdetected+1;

      /* tracé de la région */
      cvZero(img_tmp);
      cvDrawContours(img_tmp, ptc, CV_RGB(coul,coul,coul), CV_RGB(coul,coul,coul), -1, CV_FILLED, 8 );

      /* Teste si la region doit être gardée en fonction du masque */
      cvAnd(img_tmp,img_mask_haut,img_tmp2);

      if((cvCountNonZero(img_tmp2)!=0)&&(ptc->total>5))
	{
	  /* Le blob est conserve, on calcule le rectangle associe */
	  blob_rect = cvBoundingRect(ptc,1);

	  /* Mise a l'echelle */
	  blob_rect.x *= factor;
	  blob_rect.y *= factor;
	  blob_rect.width *= factor;
	  blob_rect.height *= factor;
	  
	  /* Ajout du rectangle dans la liste */
	  detectedROI.push_back(blob_rect);

	  /* Ajout du centre dans la liste */
	  blob_center.x = blob_rect.x + (int)(blob_rect.width*0.5);
	  blob_center.y = blob_rect.y + (int)(blob_rect.height*0.5);
	  
	  /* Ajout du centre detecte dans la liste */
	  detectedCENTER.push_back(blob_center);

	  /* mise a jour de la mixture */
	  gm->glist[nbdetected]->mean[0] = offsetX + blob_center.x;
	  gm->glist[nbdetected]->mean[1] = offsetY + blob_center.y;
 
	  /* On incremente pour passer a la region suivante */
	  nbdetected++;	
	}
    }
  gm->curnb = nbdetected;
  return gm;
}
