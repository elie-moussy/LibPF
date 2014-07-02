#include "priors/PriorGauss.h"

PriorGauss::PriorGauss(string nomfic):Prior()
{
  if(!Load(nomfic)) cout << "|ERROR|--> PriorGauss (Constructor) \t:\t error loading parameters from file" << endl;
}

PriorGauss::~PriorGauss()
{

}

int PriorGauss::Load(string nomfic)
{
  //gPrior = GaussianAlloc(nomfic);
  //gPrior->Disp();
  ifstream fichier;
  string tmp;
  int dim;
  double* mm;
  double* cc;

  fichier.open(nomfic.c_str(), ios::in);
  if(!fichier)
    {
      cout << "|ERROR|--> PriorGauss \t:\t Opening file [" << nomfic << "] ERROR" << endl;
      return 0;
    }
  
  /* lecture de la dimension de la gaussienne */
  FindStr(fichier,"<nCP>");
  if(fichier.eof()) { cout << "|ERROR|--> PriorGauss \t:\t <nCP> not found" << endl; return 0;}
  fichier >> dim;

  /* Alloc des moy et cov temporaires */
  mm = new double[dim];
  cc = new double[dim*dim];

  /* lecture de la moyenne */
  FindStr(fichier,"<PriorGaussMean>");
  if(fichier.eof()) { cout << "|ERROR|--> PriorGauss \t:\t <PriorGaussMean> not found" << endl; return 0;}
  for(int i=0;i<dim;i++)
    fichier >> mm[i];

  /* lecture de la covariance */
  FindStr(fichier,"<PriorGaussCov>");
  if(fichier.eof()) { cout << "|ERROR|--> PriorGauss \t:\t <PriorGaussCov> not found" << endl; return 0;}
  for(int i=0;i<dim*dim;i++)
    fichier >> cc[i];

  /* Alloc de la gaussienne */
  gPrior = GaussianAlloc(dim,mm,cc);

  /* Liberation memoire */
  delete [] mm;
  delete [] cc;
  
  /* fermeture du fichier */
  fichier.close();

  return 1;
}

void PriorGauss::Draw(Sample* s)
{
  /* Trace sur une particule */
  gPrior->Draw(s->ContinusP);
}

void PriorGauss::Draw(double* mm, Sample* s)
{
  /* Trace sur une particule */
  gPrior->Draw(mm,s->ContinusP);
}

double PriorGauss::Eval(Sample* s)
{
  return gPrior->Eval(s->ContinusP);
}

void PriorGauss::Disp()
{
  cout << "Gaussian prior" << endl;
  gPrior->Disp();
}

