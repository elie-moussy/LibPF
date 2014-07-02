#ifndef SHAPELIKELIHOOD_H
#define SHAPELIKELIHOOD_H

#include "bspline/multibspline2d.h"
#include "imageutils/imageutils.h"
#include "miscelleanous/externParams.h"
#include "Likelihood.h"

/* Interface pour le calcul de vraissemblance dans les targets */
class ShapeLikelihood : public Likelihood
{
public:

  /* Mode de mesure */
  enum MesType {EDGE,MEDGE,DIST,MDIST} mode;

  /* Constructeur */
  ShapeLikelihood(enum MesType selmode, ImgProducer* imgB=NULL);

  /* Destructeur */
  ~ShapeLikelihood();

  /* Fonction de calcul de la vraisemblance */
  double calc(Sample* spl);

  /* Mesure de forme basee sur les contours */
  double CalcEdgeMesuredst(MultiBSpline2D * spline, IplImage* img_mesure, 
			   float coeff=MOINS_UN_SUR_DEUX_SIGMA_CARRE, float scale=1);

  /* Mesure de forme basee sur les contours avec masque */
  double CalcEdgeMesuredst(MultiBSpline2D * spline, IplImage* img_mesure, 
			   IplImage* img_mask, float ro, 
			   float coeff=MOINS_UN_SUR_DEUX_SIGMA_CARRE,float scale=1);

  /* Mesure de vraisemblance a partir de l'image de distance */
  double CalcDistMesuredst(MultiBSpline2D * spline, IplImage* img_mesure, 
			   float coeff=MOINS_UN_SUR_DEUX_SIGMA_CARRE, float scale=1);

  /* Mesure de vraisemblance a partir de l'image de distance avec masque */
  double CalcDistMesuredst(MultiBSpline2D * spline, IplImage* img_mesure, IplImage* img_mask, 
			   float ro, float coeff=MOINS_UN_SUR_DEUX_SIGMA_CARRE, float scale=1);

};
#endif
