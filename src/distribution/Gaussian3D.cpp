#include "distribution/Gaussian3D.h"


Gaussian3D::Gaussian3D(string nomfic, string dispdec):Gaussian(nomfic,dispdec)
{
  if(this->dim!=3) cout << dispdec << "|ERROR|--> Gaussian3D (Constructor) \t:\t dim is not 3 check input datas" << endl;
  //cout << "Gaussian3D Allocation" << endl;
}


Gaussian3D::Gaussian3D(int dim, string dispdec):Gaussian(dim,dispdec)
{
  if(dim!=3) cout << dispdec << "|ERROR|--> Gaussian3D (Constructor) \t:\t dim is not 3 check input datas" << endl;
  //cout << "Gaussian3D Allocation" << endl;
}

Gaussian3D::Gaussian3D(int dim, double* m, double* c, string dispdec):Gaussian(dim,m,c,dispdec)
{
  if(dim!=3) cout << dispdec << "|ERROR|--> Gaussian3D (Constructor) \t:\t dim is not 3 check input datas" << endl;
  //cout << "Gaussian3D Allocation" << endl;
}

double Gaussian3D::Eval(double *v)
{
  double diff[3];
  double res;
  
  diff[0] = v[0]-this->mean[0];
  diff[1] = v[1]-this->mean[1];
  diff[2] = v[2]-this->mean[2];
  
  res = this->coeff*exp(-0.5 * (this->invcov[0]*diff[0]*diff[0] + this->invcov[4]*diff[1]*diff[1] + this->invcov[8]*diff[2]*diff[2]
				+ (this->invcov[1]+this->invcov[3])*diff[0]*diff[1] + (this->invcov[2]+this->invcov[6])*diff[0]*diff[2] 
				+ (this->invcov[5]+this->invcov[7])*diff[1]*diff[2]));
  
  //res = exp(-0.5 * (invcov[0]*diff[0]*diff[0] + invcov[4]*diff[1]*diff[1] + invcov[8]*diff[2]*diff[2]
  //+ (invcov[1]+invcov[3])*diff[0]*diff[1] + (invcov[2]+invcov[6])*diff[0]*diff[2] 
  //+ (invcov[5]+invcov[7])*diff[1]*diff[2]));  
  
  return res;
}

double Gaussian3D::Eval(double *v, double *m)
{
  double diff[3];
  double res;

  diff[0] = v[0]-m[0];
  diff[1] = v[1]-m[1];
  diff[2] = v[2]-m[2];

  res = this->coeff*exp(-0.5 * (this->invcov[0]*diff[0]*diff[0] + this->invcov[4]*diff[1]*diff[1] + this->invcov[8]*diff[2]*diff[2]
				+ (this->invcov[1]+this->invcov[3])*diff[0]*diff[1] + (this->invcov[2]+this->invcov[6])*diff[0]*diff[2] 
				+ (this->invcov[5]+this->invcov[7])*diff[1]*diff[2]));
  
  //res = exp(-0.5 * (invcov[0]*diff[0]*diff[0] + invcov[4]*diff[1]*diff[1] + invcov[8]*diff[2]*diff[2]
  //+ (invcov[1]+invcov[3])*diff[0]*diff[1] + (invcov[2]+invcov[6])*diff[0]*diff[2] 
  //+ (invcov[5]+invcov[7])*diff[1]*diff[2]));  
  
  return res;
}

void Gaussian3D::Draw(double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = this->mean[0] + this->cholcov[0]*this->RandVector[0] +  this->cholcov[1]*this->RandVector[1] +  this->cholcov[2]*this->RandVector[2];
  vout[1] = this->mean[1] + this->cholcov[3]*this->RandVector[0] +  this->cholcov[4]*this->RandVector[1] +  this->cholcov[5]*this->RandVector[2];
  vout[2] = this->mean[2] + this->cholcov[6]*this->RandVector[0] +  this->cholcov[7]*this->RandVector[1] +  this->cholcov[8]*this->RandVector[2];
}

void Gaussian3D::Draw(double* mm, double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = mm[0] + this->cholcov[0]*this->RandVector[0] +  this->cholcov[1]*this->RandVector[1] +  this->cholcov[2]*this->RandVector[2];
  vout[1] = mm[1] + this->cholcov[3]*this->RandVector[0] +  this->cholcov[4]*this->RandVector[1] +  this->cholcov[5]*this->RandVector[2];
  vout[2] = mm[2] + this->cholcov[6]*this->RandVector[0] +  this->cholcov[7]*this->RandVector[1] +  this->cholcov[8]*this->RandVector[2];
}

void Gaussian3D::AddNoise(double* vin, double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = vin[0] + this->cholcov[0]*this->RandVector[0] +  this->cholcov[1]*this->RandVector[1] +  this->cholcov[2]*this->RandVector[2];
  vout[1] = vin[1] + this->cholcov[3]*this->RandVector[0] +  this->cholcov[4]*this->RandVector[1] +  this->cholcov[5]*this->RandVector[2];
  vout[2] = vin[2] + this->cholcov[6]*this->RandVector[0] +  this->cholcov[7]*this->RandVector[1] +  this->cholcov[8]*this->RandVector[2];
}
