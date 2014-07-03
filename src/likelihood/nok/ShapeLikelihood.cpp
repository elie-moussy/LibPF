#include "LibPF/likelihood/nok/ShapeLikelihood.h"

 /* Constructeur */
ShapeLikelihood::ShapeLikelihood(enum MesType selmode, ImgProducer* imgB):Likelihood(imgB)
{

  /* Initialisation du mode de fonctionnement */
  mode = selmode;

  /* Chargement du producteur d'images (en fonction du mode) si il n'a pas ete passe en parametre */
  if(!this->imgBank)
    {
      cout << "--> ShapeLikelihood (CO) \t:\t local ImgProducer allocation\n";
      switch(mode)
	{
// 	case EDGE: this->imgBank = new ImgProducerEDGE();
// 	  break;

// 	case MEDGE: this->imgBank = new ImgProducerMEDGE();
// 	  break;

// 	case DIST: this->imgBank = new ImgProducerDIST();
// 	  break;

// 	case MDIST: this->imgBank = new ImgProducerMDIST();
// 	  break;

	default: cout << "|ERROR|--> ShapeLikelihood (CO) \t:\t bad mode !\n";
	}
    }
  else  cout << "--> ShapeLikelihood (CO) \t:\t using external ImgProducer\n";
}

  /* Destructeur */
ShapeLikelihood::~ShapeLikelihood()
{

}

/* Fonction de calcul de la vraisemblance */
double ShapeLikelihood::calc(Sample* spl)
{
      switch(mode)
	{
	case EDGE: 
	  break;

	case MEDGE:
	  break;

	case DIST: 
	  break;

	case MDIST:
	  break;

	default: cout << "|ERROR|--> ShapeLikelihood (CA) \t:\t bad mode !\n";
	}
}

/* Mesure de forme basee sur les contours */
double ShapeLikelihood::CalcEdgeMesuredst(MultiBSpline2D * spline, IplImage* img_mesure, 
					  float coeff, float scale)
{

}

/* Mesure de forme basee sur les contours avec masque */
double ShapeLikelihood::CalcEdgeMesuredst(MultiBSpline2D * spline, IplImage* img_mesure, 
			 IplImage* img_mask, float ro, 
			 float coeff=MOINS_UN_SUR_DEUX_SIGMA_CARRE,float scale=1);

/* Mesure de vraisemblance a partir de l'image de distance */
double ShapeLikelihood::CalcDistMesuredst(MultiBSpline2D * spline, IplImage* img_mesure, 
			 float coeff=MOINS_UN_SUR_DEUX_SIGMA_CARRE, float scale=1);

/* Mesure de vraisemblance a partir de l'image de distance avec masque */
double ShapeLikelihood::CalcDistMesuredst(MultiBSpline2D * spline, IplImage* img_mesure, IplImage* img_mask, 
			 float ro, float coeff=MOINS_UN_SUR_DEUX_SIGMA_CARRE, float scale=1);


