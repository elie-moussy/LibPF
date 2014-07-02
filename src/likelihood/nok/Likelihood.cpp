#include "Likelihood.h"

Likelihood::Likelihood(ImgProducer* imgB)
{
  /* Recopie du ImgProducer (NULL par defaut) */
  this->imgBank = imgB;  
  if(imgBank) this->localImgBank=true;
  else this->localImgBank=false;
  cout << "--> Likelihood (CO) \t:\t new likelihood object\n";
}

Likelihood::~Likelihood()
{
  if(localImgBank) delete imgBank;
}
