#include "GaussianND.h"

GaussianND::GaussianND(string nomfic, string dispdec):Gaussian(nomfic,dispdec)
{
  if(this->dim<5) cout << dispdec << "|WARNING|--> GaussianND (Constructor) \t:\t dim is lower than 5 so an optimized gaussian could be allocated" << endl;
  cout << dispdec << "--> GaussianND (Constructor) \t:\t GaussianND Allocation" << endl;
}


GaussianND::GaussianND(int dim, string dispdec):Gaussian(dim,dispdec)
{
  if(dim<5) cout << dispdec << "|WARNING|--> GaussianND (Constructor) \t:\t dim is lower than 5 so an optimized gaussian could be allocated" << endl;
  cout << dispdec << "--> GaussianND (Constructor) \t:\t GaussianND Allocation" << endl;
}


GaussianND::GaussianND(int dim, double* m, double* c, string dispdec):Gaussian(dim,m,c,dispdec)
{
  if(dim<5) cout << dispdec << "|WARNING|--> GaussianND (Constructor) \t:\t dim is lower than 5 so an optimized gaussian could be allocated" << endl;
  cout << dispdec << "--> GaussianND (Constructor) \t:\t GaussianND Allocation" << endl;
}


double GaussianND::Eval(double *v)
{
  double res=0.0;
  int id;
  
  /* Calcul de la difference a la moyenne */
  for(int i=0;i<this->dim;i++)
    this->vect[i] = v[i]-this->mean[i];
  
  /* Calcul de la vraissemblance */
  for(int i=0;i<this->dim;i++)
    {
      id = i*this->dim;
      for(int j=0;j<this->dim;j++)
	res+=this->vect[i]*this->invcov[id+j]*this->vect[j];	
    }
  return this->coeff*exp(-0.5*res);
  //return exp(-0.5*res);

  return res;
}



double GaussianND::Eval(double *v, double *m)
{
  double res=0.0;
  int id;
  
  /* Calcul de la difference a la moyenne */
  for(int i=0;i<this->dim;i++)
    this->vect[i] = v[i]-m[i];
  
  /* Calcul de la vraissemblance */
  for(int i=0;i<this->dim;i++)
    {
      id = i*this->dim;
      for(int j=0;j<this->dim;j++)
	res+=this->vect[i]*this->invcov[id+j]*this->vect[j];	
    }
  return this->coeff*exp(-0.5*res);
  //return exp(-0.5*res);

  return res;
}


void GaussianND::Draw(double* vout)
{
  int i,j;
  double* ptcholcov;

  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  
  /* Tir autour de la moyenne mean avec une covariance cov */    
  for(i=0,ptcholcov=this->cholcov;i<this->dim;i++,ptcholcov+=this->dim)
    {
      vout[i] = this->mean[i];
      for(j=0;j<this->dim;j++)
	vout[i]+=ptcholcov[j]*this->RandVector[j];      
    }
}

void GaussianND::Draw(double* mm, double* vout)
{
  int i,j;
  double* ptcholcov;

  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  
  /* Tir autour de la moyenne mean avec une covariance cov */    
  for(i=0,ptcholcov=this->cholcov;i<this->dim;i++,ptcholcov+=this->dim)
    {
      vout[i] = mm[i];
      for(j=0;j<this->dim;j++)
	vout[i]+=ptcholcov[j]*this->RandVector[j];      
    }
}

void GaussianND::AddNoise(double* vin, double* vout)
{
  int i,j;
  double* ptcholcov;
  
  /* Tirage aleatoire du bruit des parametres continus (m=0 sd=1) */
  cvRandArr(&this->rng_state,this->RandVectorMat,CV_RAND_NORMAL,cvRealScalar(0),cvRealScalar(1));
  
  /* Tir autour de la moyenne mean avec une covariance cov */    
  for(i=0,ptcholcov=this->cholcov;i<this->dim;i++,ptcholcov+=this->dim)
    {
      vout[i] = vin[i];
      for(j=0;j<this->dim;j++)
	vout[i]+=ptcholcov[j]*this->RandVector[j];      
    }
}
