#include "LibPF/distribution/Gaussian2D.h"

Gaussian2D::Gaussian2D(string nomfic, string dispdec):Gaussian(nomfic,dispdec)
{
  if(this->dim!=2) cout << this->dispdec << "|ERROR|--> Gaussian2D (Constructor) \t:\t dim is not 2 check input datas" << endl;
  //cout << "Gaussian2D Allocation" << endl;
}


Gaussian2D::Gaussian2D(int dim, string dispdec):Gaussian(dim,dispdec)
{
  if(dim!=2) cout << this->dispdec << "|ERROR|--> Gaussian2D (Constructor) \t:\t dim is not 2 check input datas" << endl;
  //cout << "Gaussian2D Allocation" << endl;
}


Gaussian2D::Gaussian2D(int dim, double* m, double* c, string dispdec):Gaussian(dim,m,c,dispdec)
{
  if(dim!=2) cout << this->dispdec << "|ERROR|--> Gaussian2D (Constructor) \t:\t dim is not 2 check input datas" << endl;
  //cout << "Gaussian2D Allocation" << endl;
}


double Gaussian2D::Eval(double *v)
{
  double diff[2];
  double res;

  diff[0] = v[0]-this->mean[0];
  diff[1] = v[1]-this->mean[1];

  res = this->coeff*exp(-0.5 * (this->invcov[0]*diff[0]*diff[0] + this->invcov[3]*diff[1]*diff[1] + (this->invcov[1]+this->invcov[2])*diff[0]*diff[1]) );
  //res = exp(-0.5 * (invcov[0]*diff[0]*diff[0] + invcov[3]*diff[1]*diff[1] + (invcov[1]+invcov[2])*diff[0]*diff[1]) );

  return res;
}

double Gaussian2D::Eval(double *v, double *m)
{
  double diff[2];
  double res;

  diff[0] = v[0]-m[0];
  diff[1] = v[1]-m[1];
  
  res = this->coeff*exp(-0.5 * (this->invcov[0]*diff[0]*diff[0] + this->invcov[3]*diff[1]*diff[1] + (this->invcov[1]+this->invcov[2])*diff[0]*diff[1]) );
  //res = exp(-0.5 * (invcov[0]*diff[0]*diff[0] + invcov[3]*diff[1]*diff[1] + (invcov[1]+invcov[2])*diff[0]*diff[1]) );
  return res;
}

void Gaussian2D::Draw(double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = this->mean[0] + this->cholcov[0]*this->RandVector[0] +  this->cholcov[1]*this->RandVector[1];
  vout[1] = this->mean[1] + this->cholcov[2]*this->RandVector[0] +  this->cholcov[3]*this->RandVector[1];
}

void Gaussian2D::Draw(double* mm, double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = mm[0] + this->cholcov[0]*this->RandVector[0] +  this->cholcov[1]*this->RandVector[1];
  vout[1] = mm[1] + this->cholcov[2]*this->RandVector[0] +  this->cholcov[3]*this->RandVector[1];
}

void Gaussian2D::AddNoise(double* vin, double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = vin[0] + this->cholcov[0]*this->RandVector[0] +  this->cholcov[1]*this->RandVector[1];
  vout[1] = vin[1] + this->cholcov[2]*this->RandVector[0] +  this->cholcov[3]*this->RandVector[1];
}
