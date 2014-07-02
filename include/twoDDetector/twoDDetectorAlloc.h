#ifndef TWODDETECTORALLOC_H
#define TWODDETECTOTALLOC_H

#include <string>
using std::string;

#include "miscelleanous/externParams.h"

#include "twoDDetector.h"
#include "faceDetector.h"
#include "skinBlobDetector.h"
#include "motionBlobDetector.h"
#include "motionDetector.h"
#include "inputDetector.h"//mod

#define DETECTFACE 0
#define DETECTSKINBLOB 1
#define DETECTMOTIONBLOB 2
#define DETECTMOTION 3
#define DETECTINPUT 4//mod

inline twoDDetector* twoDDetectorAlloc(int choix, string file, ImgProducer* imgB=NULL)
{
  twoDDetector* newDetector;

  switch(choix)
    {
    case DETECTFACE:
      {
	newDetector = new faceDetector(file,imgB);
      }
      break;

    case DETECTSKINBLOB:
      {
	newDetector = new skinBlobDetector(file,imgB);
      }
      break;

    case DETECTMOTIONBLOB:
      {
	newDetector = new motionBlobDetector(file,imgB);
      }
      break;

    case DETECTMOTION:
      {
	newDetector = new motionDetector(file,imgB);
      }
      break;
      //mod
    case DETECTINPUT:
    {
    newDetector = new inputDetector(file, imgB);
    }
    break;
      //endmod
    default :
      {
	cout << "|ERROR|--> twoDDetectorAlloc \t:\t Bad twoDDetector number !!!!" << endl;
	newDetector = NULL;
      }
    }

  /* Mise a jour de l'ID de la dynamique */
  if(newDetector) newDetector->detectorID = choix;

  return newDetector;
}

/* Fonction qui retourne un objet prior selon le choix */
inline twoDDetector* twoDDetectorAlloc(string file, ImgProducer* imgB=NULL)
{
  ifstream fichier;
  string tmp;
  
  fichier.open(file.c_str(), ios::in);
  if(!fichier)
    {
      cout << "|ERROR|--> twoDDetectorAlloc \t:\t Opening file [" << file << "] ERROR" << endl;
      return 0;
    }
  
  /* lecture du type de dynamique a charger */
  FindStr(fichier,"<DetectorType>");
  if(fichier.eof()) { cout << "|ERROR|--> twoDDetectorAlloc \t:\t <DetectorType> not found" << endl; return 0;}
  fichier >> tmp;
  
  /* fermeture du fichier */
  fichier.close();
  
  /* selection du type de dynamique */
  twoDDetectorType = tmp;

  /* Construction à partir du PriorType contenu dans le fichier */
  return twoDDetectorAlloc(twoDDetectorType.i(), file,imgB);
}

#endif
