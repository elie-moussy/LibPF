#include "distribution/Gaussian1D.h"



Gaussian1D::Gaussian1D(string nomfic, string dispdec):Gaussian(nomfic,dispdec)
{
  if(this->dim!=1) cout << this->dispdec << "|ERROR|--> Gaussian1D (Constructor) \t:\t dim is not 1 check input datas" << endl;
  //cout << "Gaussian1D Allocation" << endl;
  this->deuxsigcarre = 2*this->cov[0]*this->cov[0];
}


Gaussian1D::Gaussian1D(int dim, string dispdec):Gaussian(dim,dispdec)
{
  if(dim!=1) cout << this->dispdec << "|ERROR|--> Gaussian1D (Constructor) \t:\t dim is not 1 check input datas" << endl;
  //cout << "Gaussian1D Allocation" << endl;
  this->deuxsigcarre = 2*this->cov[0]*this->cov[0];
}


Gaussian1D::Gaussian1D(int dim, double* m, double* c, string dispdec):Gaussian(dim,m,c,dispdec)
{
  if(this->dim!=1) cout << this->dispdec << "|ERROR|--> Gaussian1D (Constructor) \t:\t dim is not 1 check input datas" << endl;
  //cout << "Gaussian1D Allocation" << endl;
  this->deuxsigcarre = 2*this->cov[0]*this->cov[0];
}


double Gaussian1D::Eval(double *v)
{
  //cout << "Gaussian 1D : this->deuxsigcarre = " << this->deuxsigcarre << "  coeff = " << this->coeff << endl;
  //cout << "this->deuxsigcarre : " << this->deuxsigcarre << endl;
  double diff = (v[0]-this->mean[0]); 
  return this->coeff * exp(-(diff*diff)/this->deuxsigcarre);
  //return exp(-(diff*diff)/this->deuxsigcarre);
}



double Gaussian1D::Eval(double *v, double *m)
{
  double diff = (v[0]-m[0]);
  return this->coeff * exp(-(diff*diff)/this->deuxsigcarre);
  //return exp(-(diff*diff)/this->deuxsigcarre);
}



void Gaussian1D::Draw(double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = this->mean[0] + ( this->cholcov[0] * this->RandVector[0] );
}


void Gaussian1D::Draw(double* mm, double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = mm[0] + ( this->cholcov[0] * this->RandVector[0] );
}



void Gaussian1D::AddNoise(double* vin, double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = vin[0] + ( this->cholcov[0] * this->RandVector[0] );
}
