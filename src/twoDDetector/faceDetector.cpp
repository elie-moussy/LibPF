#include "twoDDetector/faceDetector.h"
#include <time.h>

/* Constructeur */
faceDetector::faceDetector(string file, ImgProducer* imgB):twoDDetector(file,imgB)
{
  this->detectorID = 0;

  double cov[4]={0,0,0,0};

  /* Lecture des parametres dans le fichier */
  ifstream fichier;
  
  fichier.open(file.c_str(), ios::in);
  if(!fichier)
    {
      cout << "|ERROR|--> faceDetector \t:\t Opening file [" << file << "] ERROR" << endl;
      return;
    }

  /* Lecture de la covariance associee au melange de gaussienne */
  FindStr2(fichier,"<DetectorCov>");
  if(!fichier.eof()) 
    { 
      cout << "--> twoDDetector \t:\t Loading covariance" << endl;
      fichier >> cov[0];
      fichier >> cov[1];
      fichier >> cov[2];
      fichier >> cov[3];
    }

  /* Sous echantillonnage */
  PyrDown=0;
  FindStr2(fichier,"<DetectorDownsample>");
  if(!fichier.eof()) 
    { 
      fichier >> PyrDown;
      cout << "--> twoDDetector \t:\t downsample = " << PyrDown << endl;
    }
 
  /* fermeture du fichier */
  fichier.close();

  /* Appel de la fonction d'init */
  init(cov);
}

faceDetector::faceDetector(int w, int h, ImgProducer* imgB, double* cov, int downsample):twoDDetector(w,h,imgB)
{
  this->detectorID = 0;
  /* Demi image ou non */
  PyrDown = downsample;

  /* Appel de la fonction d'init */
  init(cov);
}

void faceDetector::init(double* cov)
{
  /* allocation du storage */
  storage = cvCreateMemStorage(0);
  
  /* creation de la cascade de classifieur pour la detection de visages */
  newFD();

  /* Init du scale et allocation de l'image sous echantillonnee */
  if(PyrDown)
    {
      printf("--> faceDetector (Constructor) \t:\t faceDetector with image pyrdown\n");
      //this->imgIN = this->imgBank->getImgPtr("PYRDOWN");
      scaleFactor = 2;      
    }
  else
    {
      printf("--> faceDetector (Constructor) \t:\t faceDetector without image pyrdown\n");
      //this->imgIN = this->imgBank->imgRGB;
      scaleFactor =1;
    }

  /* Allocation d'une mixture de gaussiennes */
  this->gm = new GaussianMixture(NBMAXFACES,2,NULL,cov);

  detectionsAJeterSize = 256*sizeof(int);

  printf("--> faceDetector (Constructor) \t:\t Constructor done...\n");
}

/* Destructeur */
faceDetector::~faceDetector()
{
  cvReleaseMemStorage(&storage);
  cvReleaseHaarClassifierCascade(&cascade);
}

/* Execution de la detection de visage */
GaussianMixture* faceDetector::process(int offsetX, int offsetY, double scale)
{
  CvRect face_rect;
  CvRect resized_rect;
  CvPoint face_center;
  
  clock_t start_time, end_time;

  /* RAZ du storage */
  cvClearMemStorage( storage );
  
  /* Detection des visages */
 		start_time = clock();
  //faces = cvHaarDetectObjects(imgIN, cascade, storage, 1.05, 2, CV_HAAR_DO_CANNY_PRUNING,cvSize(10,10));
  //faces = cvHaarDetectObjects(imgBank->imgPYRDOWN(), cascade, storage, 1.05, 2, CV_HAAR_DO_CANNY_PRUNING,cvSize(10,10));
  //printf("faceDetector --> %d x %d\n",imgBank->imgPYRDOWN()->width, imgBank->imgPYRDOWN()->height);
  faces = cvHaarDetectObjects(imgBank->imgRGB(), cascade, storage, 1.2, 2, CV_HAAR_DO_CANNY_PRUNING,cvSize(10,10));
  //printf("faceDetector --> %d x %d\n",imgBank->imgRGB()->width, imgBank->imgRGB()->height);
		end_time = clock();
		double diff = (double)(end_time - start_time) /(double)CLOCKS_PER_SEC; // en s
		printf("%s::cvHaarDetectObjects (scaleFactor : %d)\t processing time : %lf ms\n", __FILE__, scaleFactor, diff*1000.0);
  /* init du nb de visages detectes */
  nbdetected = 0;
  detectedROI.clear();
  detectedCENTER.clear();
  
  /* Construction de la liste des centres de visages et filtrage par couleur peau si demande */
  for(int i = 0 ; i < faces->total; i++ )
    {
      /* extraction des visages */
      face_rect = *(CvRect*)cvGetSeqElem(faces,i);
      
      if(PyrDown)
	{
	  /* remise a l'echelle du rectangle */
	  resized_rect.x = scaleFactor*face_rect.x;
	  resized_rect.y = scaleFactor*face_rect.y;
	  resized_rect.width = scaleFactor*face_rect.width;
	  resized_rect.height = scaleFactor*face_rect.height;

	  detectedROI.push_back(resized_rect);

	  /* Je calcule le centre du visage */
	  face_center.x = resized_rect.x + (int)(resized_rect.width*0.5);
	  face_center.y = resized_rect.y + (int)(resized_rect.height*0.5);
	}
      else
	{
	  detectedROI.push_back(face_rect);

	  /* Je calcule le centre du visage */
	  face_center.x = face_rect.x + (int)(face_rect.width*0.5);
	  face_center.y = face_rect.y + (int)(face_rect.height*0.5);
	}
      
      /* Ajout du centre detecte dans la liste */
      detectedCENTER.push_back(face_center);

      /* mise a jour de la mixture */
      gm->glist[nbdetected]->mean[0] = offsetX + face_center.x;
      gm->glist[nbdetected]->mean[1] = offsetY + face_center.y;
	  	  
      /*on incremente pour passer au visage suivant */
      nbdetected++;	
    }
  gm->curnb = nbdetected;
  //cout << nbdetected << " detected faces\n";
  return gm;
}

/*! \fn  void newFD(void)
 *  \brief Haar classifier cascade allocation and loading of the cascade from a xml file
 */
void faceDetector::newFD(void)
{
  char nomfic[256];
  sprintf(nomfic,"%s/detectors/haarcascades/haarcascade_frontalface_alt.xml",icuConfigPath.c_str());
  cascade = (CvHaarClassifierCascade*)cvLoad(nomfic, 0, 0, 0 );
}

/* Elimine les detections superposees. IL RESTE A AJOUTER LA MISE A JOUR DU MELANGE DE GAUSSIENNE */
void faceDetector::cleanSuperposedFaces(int coeff)
{
  /* Raz des flag a jeter */
  memset(detectionsAJeter,0,detectionsAJeterSize);

  /* Parcours des detections pour selectionner celles a eliminer */
  for (int i=0; i<nbdetected; i++)
    for (int j=i+1; j<nbdetected; j++)
      {
	if ( abs(detectedCENTER[i].x-detectedCENTER[j].x) < detectedROI[i].width/coeff
	     && abs(detectedCENTER[i].y-detectedCENTER[j].y) < detectedROI[i].height/coeff )
	  {
	    if (detectedROI[i].width < detectedROI[j].width)
	      detectionsAJeter[j] = 1;
	    else
	      detectionsAJeter[i] = 1;
	  }
      }
  
  /* Elimination des elements */
  vector<CvRect>::iterator curROI;
  vector<CvPoint>::iterator curCENTER;
  int i;
  for(i=nbdetected-1,curROI=detectedROI.end(),curCENTER=detectedCENTER.end();curROI!=detectedROI.begin();curROI--,curCENTER--,i--)
     {
       if(detectionsAJeter[i]) 
 	{
 	  detectedROI.erase(curROI);
 	  detectedCENTER.erase(curCENTER);
 	  nbdetected--;
 	}
     }

 //  int nberased=0;
//   for(int i=nbdetected-1;i>=0;i--) {
//      if(detectionsAJeter[i]) 
// 	{
// 	  detectedROI.erase(detectedROI[i]);
// 	  detectedCENTER.erase(detectedCENTER[i]);
// 	  nberased++;
// 	}
//   }
//   nbdetected-=nberased;
}
