#include "motionDetector.h"

/* Constructeur
   w : largeur des images d'entree
   h : hauteur des images d'entree
   imgB : producteur d'images (traitements)
   cov : matrice de covariance des elements du melange de gaussiennes
   downsample : si 1 alors on realise un filtrage Gaussien de l'image
   sinon on utilise l'image telle quelle
*/
motionDetector::motionDetector(string file, ImgProducer* imgB):twoDDetector(file,imgB)
{
  this->detectorID = 3;
  double cov[4]={0,0,0,0};
  
  /* Lecture des parametres dans le fichier */
  ifstream fichier;
  
  fichier.open(file.c_str(), ios::in);
  if(!fichier)
    {
      cout << "|ERROR|--> motionDetector \t:\t Opening file [" << file << "] ERROR" << endl;
      return;
    }

  /* Lecture de la covariance associee au melange de gaussienne */
  FindStr2(fichier,"<DetectorCov>");
  if(!fichier.eof()) 
    { 
      cout << "--> motionDetector \t:\t Loading covariance" << endl;
      fichier >> cov[0];
      fichier >> cov[1];
      fichier >> cov[2];
      fichier >> cov[3];
    }

  /* Parametres */
  nu=0;
  FindStr2(fichier,"<DetectorNu>");
  if(!fichier.eof()) fichier >> nu;

  pas=10;
  FindStr2(fichier,"<DetectorPas>");
  if(!fichier.eof()) fichier >> pas;

  rw=25;
  rh=29;
  FindStr2(fichier,"<DetectorRegSize>");
  if(!fichier.eof()) 
    {
      fichier >> rw;
      fichier >> rh;
    }

  tau=0;
  FindStr2(fichier,"<DetectorDistMax>");
  if(!fichier.eof()) fichier >> tau;
 
  /* fermeture du fichier */
  fichier.close();

  /* Appel de la fonction d'init */
  init(cov);
}

motionDetector::motionDetector(int w, int h, ImgProducer* imgB, double* cov,double tau_, int rw_, int rh_, int nu_, int pas_):twoDDetector(w,h,imgB)
{
  this->detectorID = 3;
  /* Recopie des parametres */
  width = w;
  height = h;
  nu = nu_;
  tau = tau_;
  rw = rw_;
  rh = rh_;
  pas = pas_;

  /* Alloc et init des parametres */
  init(cov);
}

/* Fonction d'init permet de faire l'alloc */
void motionDetector::init(double* cov)
{
  Rwidth = rw + nu;
  Rheight = rh + nu;
  demiRwidth = Rwidth/2;
  demiRheight = Rheight/2;
  demipas = pas / 2;

  /* Affichage des parametres */
  cout << "--> motionDetector (CO) \t:\t Image size : " << width << "x" << height << endl;
  cout << "--> motionDetector (CO) \t:\t nu = " << nu << endl;
  cout << "--> motionDetector (CO) \t:\t tau = " << tau << endl;
  cout << "--> motionDetector (CO) \t:\t Region size : " << Rwidth << "x" << Rheight << endl;
  cout << "--> motionDetector (CO) \t:\t Demi Region size : " << demiRwidth << "x" << demiRheight << endl;
  cout << "--> motionDetector (CO) \t:\t pas = " << pas << endl;
  cout << "--> motionDetector (CO) \t:\t demipas = " << demipas << endl;

  /* Alloc du calcul de distrib de mvt */
  md = new MotionDistribution(8,1,width,height);

  /* Init du nb de detections */
  nbdetected = 0;
  
  /* Allocation d'une mixture de gaussiennes */
  nbmaxmotionblobs = width/pas*height/pas;
  gm = new GaussianMixture(nbmaxmotionblobs,2,NULL,cov);
  gm->raz();

  detectedROI.clear();
  detectedCENTER.clear();

  printf("--> motionDetector (Constructor) \t:\t Constructor done...\n");
}

/* Destructeur */
motionDetector::~motionDetector()
{
  delete md;
  delete gm;
}

/* Fonction de detection */
GaussianMixture* motionDetector::process(int offsetX, int offsetY, double scale)
{
  int drw;
  int drh;
  int iw;
  int ih;
  double distance, incr;
  CvPoint motion_center;

  /* mise a l'echelle */
  drw = (int)(this->demiRwidth*scale);
  drh = (int)(this->demiRheight*scale);
  iw = (int)(this->width - drw);
  ih = (int)(this->height - drh);

  //cout << "zone : " << drh << "," << drw << endl;
  
  /* Calcul de la distribution du mouvement dans une zone */
  md->InitMask();
  gm->raz();
  nbdetected = 0;
  //detectedROI.clear();
  detectedCENTER.clear();  

  /* On traite l'image sous echantillonnee pour en extraire une mixture */
  for(int i=demipas;i<height;i+=pas)
    {
      for(int j=demipas;j<width;j+=pas)
	{
	  /* determine la region */	      
	  ptUL.x = (j > drw) ? (j - drw) : 0;
	  ptUL.y = (i > drh) ? (i - drh) : 0;
	  ptLR.x = (j < iw) ? (j + drw) : width;
	  ptLR.y = (i < ih) ? (i + drh) : height;
	
	  /* calcul de l'increment = 1.0 / nbpixels */
	  incr = 1.0 / (double)( (ptLR.x-ptUL.x)*(ptLR.y-ptUL.y) );

	  /* calcule l'histo */
	  md->CalcFromRect(0,imgBank->imgDIFFABS(),ptUL,ptLR,incr);
	
	  /* calcule la distance et seuillage */
	  distance = md->BhattaDistance();

	  /* Trace pour illustration simplement */
	  //double coul = 255.0*(1.0-distance);
	  //cvRectangle(this->tmpImg,ptUL,ptLR,cvScalar(coul,coul,coul),-1);

	  //cout << "dist = " << distance << endl;
	  if(distance < tau)
	    {   
	      motion_center.x = j;
	      motion_center.y = i;
	      
	      /* Ajout du centre detecte dans la liste */
	      detectedCENTER.push_back(motion_center);
	      
	      /* mise a jour de la mixture */
	      gm->glist[nbdetected]->mean[0] = offsetX + motion_center.x;
	      gm->glist[nbdetected]->mean[1] = offsetY + motion_center.y;
	      
	      /*on incremente pour passer au visage suivant */
	      nbdetected++;	
	    }   
	} 
    }
  gm->curnb = nbdetected;
  return gm;
}
