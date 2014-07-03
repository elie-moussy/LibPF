#include "LibPF/distribution/GaussianDiag2D.h"

GaussianDiag2D::GaussianDiag2D(string nomfic, string dispdec):Gaussian(nomfic,dispdec)
{
  this->covIsdiagonal = 1;
  //cout << "GaussianDiag2D Allocation" << endl;
}


GaussianDiag2D::GaussianDiag2D(int dim, string dispdec):Gaussian(dim,dispdec)
{
  this->covIsdiagonal = 1;
  //cout << "GaussianDiag2D Allocation" << endl;
}


GaussianDiag2D::GaussianDiag2D(int dim, double* m, double* c, string dispdec):Gaussian(dim,m,c,dispdec)
{
  this->covIsdiagonal = 1;
  //cout << "GaussianDiag2D Allocation" << endl;
}


double GaussianDiag2D::Eval(double *v)
{
  double diff[2];
  double res;

  diff[0] = v[0]-this->mean[0];
  diff[1] = v[1]-this->mean[1];

  //cout << "coeff = " << this->coeff << " diff =  " << diff[0] << "  " << diff[1] << endl;

  res = this->coeff*exp(-0.5 * (this->invcov[0]*diff[0]*diff[0] + this->invcov[3]*diff[1]*diff[1]) );
  //res = exp(-0.5 * (invcov[0]*diff[0]*diff[0] + invcov[3]*diff[1]*diff[1]) );

  return res;
}



double GaussianDiag2D::Eval(double *v, double *m)
{
  double diff[2];
  double res;

  diff[0] = v[0]-m[0];
  diff[1] = v[1]-m[1];


  // cout << "Params " << endl;
//   DispVector<T>(m,4);
//   DispVector<T>(v,4);


//   DispVector<T>(diff,2);
//   cout << "coeff = " << this->coeff << endl;
//   DispMatrix<T>(this->invcov,2,2);
  
  res = this->coeff*exp(-0.5 * (this->invcov[0]*diff[0]*diff[0] + this->invcov[3]*diff[1]*diff[1] ) );
  //res = exp(-0.5 * (invcov[0]*diff[0]*diff[0] + invcov[3]*diff[1]*diff[1] ) );
  return res;
}



void GaussianDiag2D::Draw(double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = this->mean[0] + this->cholcov[0]*this->RandVector[0];
  vout[1] = this->mean[1] + this->cholcov[3]*this->RandVector[1];
}


void GaussianDiag2D::Draw(double* mm, double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = mm[0] + this->cholcov[0]*this->RandVector[0];
  vout[1] = mm[1] + this->cholcov[3]*this->RandVector[1];
}


void GaussianDiag2D::AddNoise(double* vin, double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = vin[0] + this->cholcov[0]*this->RandVector[0];
  vout[1] = vin[1] + this->cholcov[3]*this->RandVector[1];
}
