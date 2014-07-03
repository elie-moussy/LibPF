#include "LibPF/distribution/GaussianDiagND.h"

GaussianDiagND::GaussianDiagND(string nomfic, string dispdec):Gaussian(nomfic,dispdec)
{
  this->covIsdiagonal = 1;
  cout << dispdec << "GaussianDiagND Allocation" << endl;
}


GaussianDiagND::GaussianDiagND(int dim, string dispdec):Gaussian(dim,dispdec)
{
  this->covIsdiagonal = 1;
  cout << dispdec << "GaussianDiagND Allocation" << endl;
}


GaussianDiagND::GaussianDiagND(int dim, double* m, double* c, string dispdec):Gaussian(dim,m,c,dispdec)
{
  this->covIsdiagonal = 1;
  cout << dispdec << "GaussianDiagND Allocation" << endl;
}


double GaussianDiagND::Eval(double *v)
{
  double res=0.0;
  int i;
  double* ptinvcov;

  /* Calcul de la difference a la moyenne */
  for(i=0;i<this->dim;i++)
    this->vect[i] = v[i]-this->mean[i];
  
  /* Calcul de la vraissemblance */
  for(i=0, ptinvcov = this->invcov ; i<this->dim ; i++, ptinvcov += this->dim)
    res+= ptinvcov[i]*this->vect[i]*this->vect[i];
  
  return this->coeff*exp(-0.5*res);
  //return exp(-0.5*res);

  return res;
}

double GaussianDiagND::Eval(double *v, double *m)
{
  double res=0.0;
  int i;
  double* ptinvcov;

  /* Calcul de la difference a la moyenne */
  for(i=0;i<this->dim;i++)
    this->vect[i] = v[i]-m[i];
  
  for(i=0, ptinvcov = this->invcov ; i<this->dim ; i++, ptinvcov += this->dim)
    res+= ptinvcov[i]*this->vect[i]*this->vect[i];

  return this->coeff*exp(-0.5*res);
  //return exp(-0.5*res);

  return res;
}

void GaussianDiagND::Draw(double* vout)
{
  int i;
  double* ptcholcov;
  
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  
  /* Tir autour de la moyenne mean avec une covariance cov */    
  for(i=0, ptcholcov = this->cholcov ; i<this->dim ; i++, ptcholcov += this->dim)
    vout[i] = ptcholcov[i]*this->RandVector[i] + this->mean[i];      
}

void GaussianDiagND::Draw(double* mm, double* vout)
{
  int i;
  double* ptcholcov;
  
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  
  /* Tir autour de la moyenne mean avec une covariance cov */    
  for(i=0, ptcholcov = this->cholcov ; i<this->dim ; i++, ptcholcov += this->dim)
    vout[i] = ptcholcov[i]*this->RandVector[i] + mm[i];      
}

void GaussianDiagND::AddNoise(double* vin, double* vout)
{
  int i;
  double* ptcholcov;
  
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));

  /* Tir autour de la moyenne mean avec une covariance cov */    
  for(i=0, ptcholcov = this->cholcov ; i<this->dim ; i++, ptcholcov += this->dim)
    vout[i] = ptcholcov[i]*this->RandVector[i] + vin[i];    
}
