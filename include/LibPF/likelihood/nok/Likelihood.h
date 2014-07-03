#ifndef LIKELIHOOD_H
#define LIKELIHOOD_H

#include <opencv2/core/core.hpp>
#include <math.h>
#include <iostream>
#include "LibPF/priors/Sample.h"
#include "LibPF/imgproducer/ImgProducer.h"


/*********************************************************************/
/*                          CONSTANTES                               */
/*********************************************************************/
/* Intervient dans le calcul de la vraissemblance avec les contours */
#define SIGMA 24.0
#define MOINS_UN_SUR_DEUX_SIGMA_CARRE -0.00086806
//#define SIGMA 7.0
//#define MOINS_UN_SUR_DEUX_SIGMA_CARRE -0.01020408

/* largeur de recherche des points de contours */
#define SEARCHDISTANCE 5

/* C'est la penalite lorsque le point de contour n'est pas trouve */
#define MAXDISTCARRE 100

/* Penalite max pour un point hors de l'image */
#define MAXRO 100

/*********************************************************************/
/*                      CLASSE LIKELIHOOD                            */
/*********************************************************************/
/* Interface pour le calcul de vraissemblance dans les targets */
class Likelihood
{

public:

  /* Producteur d'images */
  /* 
     Attention a ce que le imgBank soit un producteur d'images
     coherent avec le mode de fonctionnement (ex : il doit fournir 
     une image masque flot optique pour la mesure MEDGE) 
  */
  ImgProducer* imgBank;

  /* Flag indiquant si le ImgProducer est local ou non */
  bool localImgBank;

  /* Constructeur */
  Likelihood(ImgProducer* imgB=NULL);

  /* Destructeur */
  ~Likelihood();

  /* Methode effectuant le calcul de vraisemblance */
  virtual double calc(Sample* spl)=0;

};
#endif
