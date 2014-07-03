#include "LibPF/distribution/Gaussian4D.h"

Gaussian4D::Gaussian4D(string nomfic, string dispdec):Gaussian(nomfic,dispdec)
{
  if(this->dim!=4) cout << dispdec << "|ERROR|--> Gaussian4D (Constructor) \t:\t dim is not 4 check input datas" << endl;
  //cout << "Gaussian4D Allocation" << endl;
}

Gaussian4D::Gaussian4D(int dim, string dispdec):Gaussian(dim,dispdec)
{
  if(dim!=4) cout << dispdec << "|ERROR|--> Gaussian4D (Constructor) \t:\t dim is not 4 check input datas" << endl;
  //cout << "Gaussian4D Allocation" << endl;
}

Gaussian4D::Gaussian4D(int dim, double* m, double* c, string dispdec):Gaussian(dim,m,c,dispdec)
{
  if(dim!=4) cout << dispdec << "|ERROR|--> Gaussian4D (Constructor) \t:\t dim is not 4 check input datas" << endl;
  //cout << "Gaussian4D Allocation" << endl;
}

double Gaussian4D::Eval(double *v)
{
  double diff[4];
  double res;

  diff[0] = v[0]-this->mean[0];
  diff[1] = v[1]-this->mean[1];
  diff[2] = v[2]-this->mean[2];
  diff[3] = v[3]-this->mean[3];

  res = this->coeff*exp(-0.5 * (this->invcov[0]*diff[0]*diff[0] + this->invcov[5]*diff[1]*diff[1] + this->invcov[10]*diff[2]*diff[2] + this->invcov[15]*diff[3]*diff[3]
			  + (this->invcov[1]+this->invcov[4])*diff[0]*diff[1] + (this->invcov[2]+this->invcov[8])*diff[0]*diff[2] + (this->invcov[3]+this->invcov[12])*diff[0]*diff[3]
			  + (this->invcov[6]+this->invcov[9])*diff[1]*diff[2] + (this->invcov[7]+this->invcov[13])*diff[1]*diff[3] + (this->invcov[11]+this->invcov[14])*diff[2]*diff[3]));

  //   res = exp(-0.5 * (this->invcov[0]*diff[0]*diff[0] + this->invcov[5]*diff[1]*diff[1] + this->invcov[10]*diff[2]*diff[2] + this->invcov[15]*diff[3]*diff[3]
  // 			  + (this->invcov[1]+this->invcov[4])*diff[0]*diff[1] + (this->invcov[2]+this->invcov[8])*diff[0]*diff[2] + (this->invcov[3]+this->invcov[12])*diff[0]*diff[3]
  // 			  + (this->invcov[6]+this->invcov[9])*diff[1]*diff[2] + (this->invcov[7]+this->invcov[13])*diff[1]*diff[3] + (this->invcov[11]+this->invcov[14])*diff[2]*diff[3]));
  return res;
}

double Gaussian4D::Eval(double *v, double *m)
{
  double diff[4];
  double res;
  
  diff[0] = v[0]-m[0];
  diff[1] = v[1]-m[1];
  diff[2] = v[2]-m[2];
  diff[3] = v[3]-m[3];

  res = this->coeff*exp(-0.5 * (this->invcov[0]*diff[0]*diff[0] + this->invcov[5]*diff[1]*diff[1] + this->invcov[10]*diff[2]*diff[2] + this->invcov[15]*diff[3]*diff[3]
			  + (this->invcov[1]+this->invcov[4])*diff[0]*diff[1] + (this->invcov[2]+this->invcov[8])*diff[0]*diff[2] + (this->invcov[3]+this->invcov[12])*diff[0]*diff[3]
			  + (this->invcov[6]+this->invcov[9])*diff[1]*diff[2] + (this->invcov[7]+this->invcov[13])*diff[1]*diff[3] + (this->invcov[11]+this->invcov[14])*diff[2]*diff[3]));
  
  //   res = exp(-0.5 * (this->invcov[0]*diff[0]*diff[0] + this->invcov[5]*diff[1]*diff[1] + this->invcov[10]*diff[2]*diff[2] + this->invcov[15]*diff[3]*diff[3]
  // 			  + (this->invcov[1]+this->invcov[4])*diff[0]*diff[1] + (this->invcov[2]+this->invcov[8])*diff[0]*diff[2] + (this->invcov[3]+this->invcov[12])*diff[0]*diff[3]
  // 			  + (this->invcov[6]+this->invcov[9])*diff[1]*diff[2] + (this->invcov[7]+this->invcov[13])*diff[1]*diff[3] + (this->invcov[11]+this->invcov[14])*diff[2]*diff[3]));
  return res;
}

void Gaussian4D::Draw(double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));

  vout[0] = this->mean[0] + this->cholcov[0]*this->RandVector[0] +  this->cholcov[1]*this->RandVector[1] +  this->cholcov[2]*this->RandVector[2] +  this->cholcov[3]*this->RandVector[3];
  vout[1] = this->mean[1] + this->cholcov[4]*this->RandVector[0] +  this->cholcov[5]*this->RandVector[1] +  this->cholcov[6]*this->RandVector[2] +  this->cholcov[7]*this->RandVector[3];
  vout[2] = this->mean[2] + this->cholcov[8]*this->RandVector[0] +  this->cholcov[9]*this->RandVector[1] +  this->cholcov[10]*this->RandVector[2] +  this->cholcov[11]*this->RandVector[3];
  vout[3] = this->mean[3] + this->cholcov[12]*this->RandVector[0] +  this->cholcov[13]*this->RandVector[1] +  this->cholcov[14]*this->RandVector[2] +  this->cholcov[15]*this->RandVector[3]; 
}

void Gaussian4D::Draw(double* mm, double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = mm[0] + this->cholcov[0]*this->RandVector[0] +  this->cholcov[1]*this->RandVector[1] +  this->cholcov[2]*this->RandVector[2] +  this->cholcov[3]*this->RandVector[3];
  vout[1] = mm[1] + this->cholcov[4]*this->RandVector[0] +  this->cholcov[5]*this->RandVector[1] +  this->cholcov[6]*this->RandVector[2] +  this->cholcov[7]*this->RandVector[3];
  vout[2] = mm[2] + this->cholcov[8]*this->RandVector[0] +  this->cholcov[9]*this->RandVector[1] +  this->cholcov[10]*this->RandVector[2] +  this->cholcov[11]*this->RandVector[3];
  vout[3] = mm[3] + this->cholcov[12]*this->RandVector[0] +  this->cholcov[13]*this->RandVector[1] +  this->cholcov[14]*this->RandVector[2] +  this->cholcov[15]*this->RandVector[3]; 
}

void Gaussian4D::AddNoise(double* vin, double* vout)
{
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  vout[0] = vin[0] + this->cholcov[0]*this->RandVector[0] +  this->cholcov[1]*this->RandVector[1] +  this->cholcov[2]*this->RandVector[2] +  this->cholcov[3]*this->RandVector[3];
  vout[1] = vin[1] + this->cholcov[4]*this->RandVector[0] +  this->cholcov[5]*this->RandVector[1] +  this->cholcov[6]*this->RandVector[2] +  this->cholcov[7]*this->RandVector[3];
  vout[2] = vin[2] + this->cholcov[8]*this->RandVector[0] +  this->cholcov[9]*this->RandVector[1] +  this->cholcov[10]*this->RandVector[2] +  this->cholcov[11]*this->RandVector[3];
  vout[3] = vin[3] + this->cholcov[12]*this->RandVector[0] +  this->cholcov[13]*this->RandVector[1] +  this->cholcov[14]*this->RandVector[2] +  this->cholcov[15]*this->RandVector[3];
}
