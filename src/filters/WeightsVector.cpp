#include "WeightsVector.h"
#include <string.h>
/*! \fn WeightsVector(int taille)
 *  \brief this is the vector constructor
 *  \param taille is the number of weights stored in the vector
 */

WeightsVector::WeightsVector(int taille)
{
  nbw=taille;
  vectorSize=nbw*sizeof(double);
  w = new double[nbw];
  cumulativew = new double[nbw];
  normalizedw = new double[nbw];
  normalized = false;
  wcumulcalculed = false;
  initVal = (double)1 / (double)nbw;
  sum = (double)0;    
  coeff = (double)0;   
}

/*! \fn ~WeightsVector()
 *  \brief class destructor
 */

WeightsVector::~WeightsVector()
{
  delete [] w;
  delete [] cumulativew;
  delete [] normalizedw;
}

/*! \fn void ResetVector()
 *  \brief this function initialise WeightsVector parameters 
 *  to be able to set new weights
 */
void WeightsVector::ResetVector()
{
  sum = (double)0;
  normalized = false;
  wcumulcalculed = false;
}

/*! \fn void ResetAllVector()
 *  \brief this function initialise WeightsVector parameters 
 *  to be able to set new weights
 */
void WeightsVector::ResetAllVector()
{
  sum = (double)0;
  normalized = false;
  wcumulcalculed = false;

  for(int i=0;i<nbw;i++)
    {
      w[i] = 0;
      normalizedw[i] = 0;
      cumulativew[i] = 0;
    }
}

/*! \fn void InitVector()
 *  \brief this function initialise the vector to initVal
 */
void WeightsVector::InitVector()
{
  double* ptr = w;
  double* ptrNW = normalizedw;
  int i=nbw;
  while(i--)
    {
      *ptr++=initVal;
      *ptrNW++=initVal;
    }
  sum=(double)1;
  normalized = true;
  wcumulcalculed = false;
}

/*! \fn void InitVector(double val)
 *  \brief this function initialise the vector to val
 *  \param val is the value used to initialise the vector
 */
void WeightsVector::InitVector(double val)
{
  double* ptr = w;
  int i=nbw;
  while(i--) *ptr++=val;
  sum=(double)val*(double)nbw;
  normalized = false;
  wcumulcalculed = false;
}

/*! \fn int NormVector()
 *  \brief this function normalize the vector by its sum
 */
int WeightsVector::NormVector()
{
  if(normalized) return 1;
  if(!sum) return 0;
  double* ptrNW = normalizedw; 
  double* ptrW = w; 
  int i=nbw;
  while(i--) *ptrNW++ = *ptrW++ / sum;
  normalized = true;
  sum=0;
  return 1;
}

/*! \fn int NormVector(WeightsVector *normw)
 *  \brief this function normalise the vector and puts the result in normw
 *  \return a pointer on the normalised weights vector
 */
int WeightsVector::NormVector(WeightsVector *normw)
{
  if(normalized)
    {
      memcpy(normw->getNormalizedWptr(),normalizedw,vectorSize);
      normw->normalized = true;
      normw->sum=0;
      return 1;
    }
  if(!sum) return 0;
  double* ptrIn = w;
  double* ptrOut = normw->getNormalizedWptr();
  int i=nbw;
  while(i--) *ptrOut++ = *ptrIn++/sum;
  normw->normalized = true;
  normw->sum=0;
  return 1;
}

/*! \fn int NormVector(double *normw)
 *  \brief this function normalise the vector and puts the result in normw
 *  \return a pointer on the normalised weights vector
 */
int WeightsVector::NormVector(double *normw)
{
  if(normalized)
    {
      memcpy(normw,normalizedw,vectorSize);
      return 1;
    }
  if(!sum) return 0;
  double* ptrIn = w;
  double* ptrOut = normw;
  int i=nbw;  
  while(i--) *ptrOut++ = *ptrIn++/sum;
  return 1;
}

/*! \fn int NormAndCumulVector()
 *  \brief this function normalize the vector by its sum and construct the cumulative weights vector
 */
int WeightsVector::NormAndCumulVector()
{
  if(wcumulcalculed && normalized) return 1;
  if(!sum) return 0;
  double* ptr = w; 
  double* ptrNW = normalizedw; 
  double* ptrCW = cumulativew;
  int i=nbw;
  /* on calcule le premier et on le mat dans la premiere case du vecteur cumule */
  *ptrNW = *ptr++ / sum;
  *ptrCW++ = *ptrNW++;      
  while(--i)
    {
      *ptrNW = *ptr++ / sum;
      *ptrCW++ = *(ptrCW-1) + *ptrNW++;
    }
  wcumulcalculed = true;
  normalized = true;
  sum=0;
  return 1;
}

/*! \fn int NormVector(WeightsVector *normw)
 *  \brief this function normalise the vector and the cumulative weights vector and puts the result in normw
 */
int WeightsVector::NormAndCumulVector(WeightsVector* normw)
{
  if(wcumulcalculed && normalized)
    {
      memcpy(normw->getNormalizedWptr(),normalizedw,vectorSize);
      memcpy(normw->getCumulativeWptr(),cumulativew,vectorSize);
      normw->wcumulcalculed = true;
      normw->normalized = true;
      normw->sum=0;
      return 1;
    }
  if(!sum) return 0;
  double* ptr = w; 
  double* ptrCWout = normw->getCumulativeWptr();
  double* ptrWout = normw->getNormalizedWptr();
  int i=nbw;
  /* on calcule le premier et on le mat dans la premiere case du vecteur cumule */
  *ptrWout = *ptr++/sum;
  *ptrCWout++ = *ptrWout++;      
  while(--i)
    {
      *ptrWout = *ptr++/sum;      
      *ptrCWout++ = *(ptrCWout-1) + *ptrWout++;
    }
  normw->wcumulcalculed = true;
  normw->normalized = true;
  normw->sum=0;

  return 1;
}

/*! \fn int CalcCumulativeW()
 *  \brief this function compute cumulative weights vector
 */
int WeightsVector::CalcCumulativeW()
{
  if(wcumulcalculed) return 1;
  double* ptrCW = cumulativew;
  int i=nbw;
  if(!normalized)
    {
      if(!sum) return 0;
      double* ptrW = w;
      *ptrCW++ = *ptrW++/sum;      
      while(--i) *ptrCW++ = *(ptrCW-1) + *ptrW++/sum;
      wcumulcalculed = true;
    }
  else
    {
      double* ptrW = normalizedw;
      *ptrCW++ = *ptrW++;      
      while(--i) *ptrCW++ = *(ptrCW-1) + *ptrW++;
      wcumulcalculed = true;
    }
  return 1;
}

/*! \fn int CalcCumulativeW(WeightsVector* normw)
 *  \brief this function compute cumulative weights vector
 */
int WeightsVector:: CalcCumulativeW(WeightsVector* normw)
{  
  if(wcumulcalculed)
    {
      memcpy(normw->getCumulativeWptr(),cumulativew,vectorSize);
      normw->wcumulcalculed = true;
      return 1;
    }

  double* ptrCW = normw->getCumulativeWptr();
  int i=nbw;

  if(!normalized)
    {
      if(!sum) return 0;
      double* ptrW = w;
      *ptrCW++ = *ptrW++/sum;      
      while(--i) *ptrCW++ = *(ptrCW-1) + *ptrW++/sum;
      normw->wcumulcalculed = true;
    }
  else
    {
      double* ptrW = normalizedw;
      *ptrCW++ = *ptrW++;      
      while(--i) *ptrCW++ = *(ptrCW-1) + *ptrW++;
      normw->wcumulcalculed = true;
    }
  return 1;
}

/*! \fn int CalcCumulativeW(double *normw)
 *  \brief this function compute cumulative weights vector
 */
int WeightsVector:: CalcCumulativeW(double *normw)
{  
  if(wcumulcalculed)
    {
      memcpy(normw,cumulativew,vectorSize);
      return 1;
    } 
  double* ptrCW = normw;
  int i=nbw;

  if(!normalized)
    {
      if(!sum) return 0;
      double* ptrW = w;
      *ptrCW++ = *ptrW++/sum;      
      while(--i) *ptrCW++ = *(ptrCW-1) + *ptrW++/sum;      
    }
  else
    {
      double* ptrW = normalizedw;
      *ptrCW++ = *ptrW++;      
      while(--i) *ptrCW++ = *(ptrCW-1) + *ptrW++;      
    }
  return 1;
}

/*! \fn double* getNormalizedWptr()
 *  \brief this function compute the normalised weights if it is not done and return
 *  a pointer on the datas
 *  \return the pointer on the normalised weights
 */
double* WeightsVector::getNormalizedWptr()
{
  /* teste si le vecteur de poids est normalise si non on le calcule */
  if(!normalized) NormVector();

  /* retourne le pointeur sur le vecteur de poids normalise */
  return normalizedw;
}

/*! \fn double* getCumulativeWptr()
 *  \brief this function compute the cumulative weights if it is not done and return
 *  a pointer on the datas
 *  \return the pointer on the cumulative weights
 */
double* WeightsVector::getCumulativeWptr()
{
  /* teste si le vecteur de poids cumules est calcule si non on le calcule */
  if(!wcumulcalculed) CalcCumulativeW();    
  
  /* retourne le pointeur sur le vecteur de poids cumules */
  return cumulativew;
}

/*! \fn void DispW()
 *  \brief this function display the weights vector
 */
void WeightsVector::DispW()
{
  int i=nbw;
  double* ptr = w;
  cout << "Weights vector :\n";
  while(i--) cout << *ptr++ << " ";
  cout << endl << "Sum = " << sum << endl;
}

/*! \fn void DispCumulativeW()
 *  \brief this function display the cumulative weights vector
 */
void WeightsVector::DispCumulativeW()
{
  int i=nbw;
  double* ptr = cumulativew;
  cout << "Cumulative Weights vector :" << endl;
  while(i--) cout << *ptr++ << " ";
  cout << endl;
}

/*! \fn void DispNormalizedW()
 *  \brief this function display the cumulative weights vector
 */
void WeightsVector::DispNormalizedW()
{
  int i=nbw;
  double* ptr = normalizedw;
  cout << "Normalized Weights vector :" << endl;
  while(i--) cout << *ptr++ << " ";
  cout << endl;
}

/*! \fn void Disp()
 *  \brief this function display all the vectors
 */
void WeightsVector::Disp()
{
  DispW();
  if(wcumulcalculed) DispCumulativeW();
  if(normalized) DispNormalizedW();
}
