#include "priors/PriorGaussMixture.h"

PriorGaussMixture::PriorGaussMixture(string nomfic):Prior()
{
  if(!Load(nomfic)) 
    cout << "|ERROR|--> PriorGaussMixture (Constructor) \t:\t error loading parameters from file" << endl;
}
  
PriorGaussMixture::~PriorGaussMixture()
{

}

int PriorGaussMixture::Load(string nomfic)
{
  gmPrior = new GaussianMixture(nomfic);
  //gmPrior->Disp();
  return 1;
}

void PriorGaussMixture::Draw(Sample* s)
{
  /* Trace sur une particule */
  gmPrior->Draw(s->ContinusP);
}

void PriorGaussMixture::Draw(double* mm, Sample* s)
{
  /* Trace sur une particule */
  gmPrior->Draw(mm,s->ContinusP);
}

double PriorGaussMixture::Eval(Sample* s)
{
  return gmPrior->Eval(s->ContinusP);
}

void PriorGaussMixture::Disp()
{
  cout << "Gaussian Mixture prior" << endl;
  gmPrior->Disp();
}
