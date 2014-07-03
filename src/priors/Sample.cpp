#include "LibPF/priors/Sample.h"

/*!
 * Constructor
 */
Sample::Sample(int nCP, int nDP)
{
  this->nCP = nCP;
  this->nCPdemi = nCP/2;
  this->nDP = nDP;

  CPsize = nCP * sizeof(double);
  demiCPsize = nCPdemi * sizeof(double);
  DPsize = nDP * sizeof(int);

  /* Allocation of continuous params */
  ContinusP = new double[nCP]; 

  /* Init du pointeur sur la deuxieme partie du vecteur */
  this->ContinusPcenter = this->ContinusP + this->nCPdemi;

  /* Allocation of discrets params */
  if(nDP) DiscretsP  = new int[nDP]; 
  else DiscretsP = NULL;

  /* Init of the idparent */
  idparent = -1;
}

/*!
 * Destructor
 */

Sample::~Sample()
{
  //cout << "Destruction de Sample" << endl;

  /* freeing ContinusP vector */
  delete [] ContinusP;

  /* freeing DiscretsP vector */
  if(nDP) delete [] DiscretsP;
}

/*!
 * Copy of a particule
 */

Sample Sample::operator = (const Sample &in)
{
  /* copie des parametres continus */
  memcpy(this->ContinusP,in.ContinusP,CPsize);
  
  /* copie des parametres discrets */
  if(nDP) memcpy(this->DiscretsP,in.DiscretsP,DPsize);
  
  //this->idparent = in.idparent;

  return *this;
}


void Sample::SplCpy(Sample* in)
{
  /* copie des parametres continus */
  memcpy(this->ContinusP,in->ContinusP,CPsize);
  
  /* copie des parametres discrets */
  if(nDP) memcpy(this->DiscretsP,in->DiscretsP,DPsize);
  
  //this->idparent = in.idparent;
}


void Sample::SplCpyPast(Sample* in)
{
  /* copie des parametres continus */
  memcpy(this->ContinusP+nCPdemi,in->ContinusP+nCPdemi,demiCPsize);
}


void Sample::ResetContinusP()
{ 
  memset(ContinusP,0,CPsize);
}


void Sample::ResetDiscretsP()
{ 
  memset(DiscretsP,0,DPsize); 
}


void Sample::Reset()
{ 
  ResetContinusP(); 
  ResetDiscretsP(); 
}



void Sample::Disp()
{
  if(nCP)
    {
      cout << "CP = [";
      for(int i=0;i<nCP;i++)
	cout << "  " << ContinusP[i];
      cout <<  "  ]" << endl;
    }

  if(nDP)
    {
      cout << "DP = [";
      for(int i=0;i<nDP;i++)
	cout << "  " << DiscretsP[i];
      cout << "  ]\n";
    }
  cout << endl;
}
