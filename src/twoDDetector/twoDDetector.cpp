#include "LibPF/twoDDetector/twoDDetector.h"

twoDDetector::twoDDetector(string file,ImgProducer* imgB)
{
  /* Lecture dans le fichier des dimensions des images */
  ifstream fichier;
  
  fichier.open(file.c_str(), ios::in);
  if(!fichier)
    {
      cout << "|ERROR|--> twoDDetector \t:\t Opening file [" << file << "] ERROR" << endl;
      return;
    }

  /* Lecture de la largeur d'image */
  FindStr(fichier,"<img_width>");
  if(fichier.eof()) { cout << "|ERROR|--> twoDDetector \t:\t <img_width> not found" << endl; return;}
  fichier >> width;

  /* Lecture de la hauteur d'image */
  FindStr(fichier,"<img_height>");
  if(fichier.eof()) { cout << "|ERROR|--> twoDDetector \t:\t <img_height> not found" << endl; return;}
  fichier >> height;
  
  /* fermeture du fichier */
  fichier.close();

  /* Init des differents parametres */
  detectedROI.clear();
  detectedCENTER.clear();
  detectorID=-1;

  /* Si ImgProducer est passe en parametre on fait l'affectation 
     sinon on cree un ImgProducer pour la detection de visages */
  if(imgB)
    {
      cout << "--> twoDDetector (Constructor) \t:\t using external ImgProducer\n";
      this->imgBank = imgB;
    }
  else
    {
      cout << "--> twoDDetector (Constructor) \t:\t using internal ImgProducer\n";
      this->imgBank = new ImgProducer(this->width,this->height);
    }
  nbdetected=0;
}

twoDDetector::twoDDetector(int w, int h, ImgProducer* imgB)
{
  /* Recopie des dimensions des images */
  width=w;
  height=h;

  /* Init des differents parametres */
  detectedROI.clear();
  detectedCENTER.clear();
  detectorID=-1;

  /* Si ImgProducer est passe en parametre on fait l'affectation 
     sinon on cree un ImgProducer pour la detection de visages */
  if(imgB)
    {
      cout << "--> twoDDetector (Constructor) \t:\t using external ImgProducer\n";
      this->imgBank = imgB;
    }
  else
    {
      cout << "--> twoDDetector (Constructor) \t:\t using internal ImgProducer\n";
      this->imgBank = new ImgProducer(this->width,this->height);
    }
  nbdetected=0;
}

twoDDetector::~twoDDetector()
{

}

/* Affiche les ROI */
void twoDDetector::dispROI(IplImage* imgRES, int d, CvScalar coul)
{
  vector<CvRect>::const_iterator cur;
  for(cur=detectedROI.begin();cur!=detectedROI.end();cur++)
    {
      cvRectangle(imgRES,cvPoint(cur->x,cur->y),
		  cvPoint((cur->x+cur->width),(cur->y+cur->height)),
		  coul,d);
    }
}

/* Affiche les Centres */
void twoDDetector::dispCENTER(IplImage* imgRES, int d, CvScalar coul)
{
  vector<CvPoint>::const_iterator cur;
  for(cur=detectedCENTER.begin();cur!=detectedCENTER.end();cur++)
    {
      cvCircle(imgRES,*cur,d,coul,-1,8,0);
    }
}
