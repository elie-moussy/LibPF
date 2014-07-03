#include "LibPF/dynamics/ContDynamicRW.h"

ContDynamicRW::ContDynamicRW(string nomfic):ContDynamicModel()
{
  //cout << "Construction de ContDynamicRW a partir de " << nomfic << endl;  
  Load(nomfic);

  /* Generateur aleatoire en fonction de l'horloge */
  //rng_state = cvRNG(0xffffffff);
  rng_state = cvRNG((uint64)RDTSC());

  typedef mat_cell<double> cell;

  /* Creation des matrices qui contiendront des valeurs aleatoires */
  //   RandVector = cvCreateMat(this->nCP,1,cell::cv_type);
  //   ptRandVector = cell::get_data(RandVector);
  
  /* une seule case pour faire des tirs independants */
  RandVector = cvCreateMat(1,1,cell::cv_type);
  ptRand = cell::get_data(RandVector);
  
  /* calcul du coeff */
  this->CalcQSumln();

}

ContDynamicRW::~ContDynamicRW()
{
  /* Free memory */
  DeleteMatrix(this->DynMatrix);
  DeleteVector(this->NoiseVector);
  DeleteVector(this->CholNoiseVector);
  DeleteVector(this->InvNoiseVector);
  DeleteMatrix(this->NoiseMatrix);
  DeleteVector(this->tmpVector);
  //DeleteVector(this->bound_max_norm);
  //DeleteVector(this->bound_min);
}


//Chargement des modeles de dynamique
int ContDynamicRW::Load(string nomfic)
{
  ifstream fichier;

  fichier.open(nomfic.c_str(), ios::in);
  if(!fichier)
    {
      cout << "|ERROR|--> ContDynamicRW (Load) \t:\t ERROR Loading file" << endl;
      return 0;
    }

  cout << "--> ContDynamicRW (Load) \t:\t Loading nCP ...";
  FindStr(fichier,"<nCP>");
  if(fichier.eof()) {cout << endl << "|ERROR|--> ContDynamicRW (Load) \t:\t <nCP> not found" << endl; return 0;}
 
  //Lecture de la taille des matrices
  fichier >> this->nCP; 
  cout << "= " << this->nCP << endl;
  
  /* taille complete */
  this->dimDynMatrix = (this->nCP);
  
  /* dimension complete de la matrice de bruit de dynamique */
  this->dimNoiseVector = (this->nCP);
  this->dimNoiseMatrix = (this->nCP);
  
  /* taille memoire des differentes matrices */
  this->sizeDynMatrix= this->dimDynMatrix*this->dimDynMatrix*sizeof(double);
  this->sizeNoiseVector= this->dimNoiseVector*sizeof(double);
  this->sizeNoiseMatrix= this->dimNoiseMatrix*this->dimNoiseMatrix*sizeof(double);
  
  //Allocations
  cout << "--> ContDynamicRW (Load) \t:\t Matrix allocation ..." << endl;
  
  /* matrice de dynamique (identitee) */
  this->DynMatrix=AllocMatrix<double>(this->dimDynMatrix,this->dimDynMatrix);
  for(int i=0;i<this->nCP;i++) this->DynMatrix[i*this->nCP+i] = 1;
  
  /* matrice globale de bruit de dynamique (diagonale) */
  this->NoiseVector=AllocVector<double>(this->dimNoiseVector);
  this->CholNoiseVector=AllocVector<double>(this->dimNoiseVector);
  this->InvNoiseVector=AllocVector<double>(this->dimNoiseVector);
  this->NoiseMatrix=AllocMatrix<double>(this->dimNoiseVector,this->dimNoiseVector);
  
  //cout << "globalPRanges ici = " << globalPRanges << endl;
  //getchar();
  //this->bound_max_norm=AllocVector<float>(this->nCP);
  //this->bound_min=AllocVector<float>(this->nCP);
  //for(int i=0;i<this->nCP;i++) 
  //  {
  //    this->bound_max_norm[i] = globalContPRanges.getMax(i) - globalContPRanges.getMin(i);
  //    this->bound_min[i] = globalContPRanges.getMin(i);
  //  }

  /* vecteur temporaire pour le calcul de la prediction */
  this->tmpVector = AllocVector<double>(this->nCP);
  this->sizetmpVector = this->nCP*sizeof(double);

  cout << "--> ContDynamicRW (Load) \t:\t Matrix allocation OK" << endl;

  cout << "--> ContDynamicRW (Load) \t:\t Loading dynamic noise matrix ..." << endl;
  FindStr(fichier,"<ContDynNoiseMatr>");
  if(fichier.eof()) {cout << endl << "|ERROR|--> ContDynamicRW (Load) \t:\t <ContDynNoiseMatr> not found" << endl; return 0;}
  
  int l;
  this->coeffSumlnQii = 0;
  for(int i=0;i<this->nCP;i++)
    {
      l = i*this->nCP;
      for(int j=0;j<this->nCP;j++)
	{
	  fichier >> this->NoiseMatrix[l+j];
	  if(i==j)
	    {
	      this->NoiseVector[i] = this->NoiseMatrix[l+j];
	      this->CholNoiseVector[i] = sqrt(this->NoiseVector[i]);
	      if(this->NoiseVector[i]) this->InvNoiseVector[i] = 1.0/this->NoiseVector[i];
	      this->coeffSumlnQii+=log(this->NoiseVector[i]);
	    }
	}
    }
  
  /* On charge le coeff betaU */   
  //   cout << "--> ContDynamicRW (Load) \t:\t Loading betaU coefficicent ..." << endl;
  //   FindStr(fichier,"<betaU>");
  //   if(fichier.eof()) {cout << endl << "|ERROR|--> ContDynamicRW (Load) \t:\t <betaU> not found" << endl; return 0;}
  //   fichier >> this->betaU;
  
  //   this->un_moins_betaU = 1.0 - this->betaU;
  
  // /* Calcul du coeff associe a la loi uniforme */
  //   this->Unipds=this->betaU;
  //   for(int i=0;i<this->nCP;i++) 
  //     this->Unipds*=1.0/this->bound_max_norm[i];    

  /* calcul de 1.0/ (2* pi^(n/2)) avec n = 2 * nCP */
  this->deuxpicoeff = 1.0/(pow(2*PI,this->nCP));
  
  /* on precalcule le facte -0.5 pour l'expo plus loin */
  this->coeffSumlnQii*=-0.5;
  
  cout << "*****************************************************" << endl;
  
  cout << "--> ContDynamicRW (Load) \t:\t Gaussian allocation ..." << endl;
  
  //Allocation de la gaussienne avec initialisation
  this->DynNoiseGauss = GaussianAlloc(this->dimNoiseVector,NULL,this->NoiseMatrix); 
      
  cout << "--> ContDynamicRW (Load) \t:\t Gaussian allocation ...done" << endl;

  /* fermeture du fichier */
  fichier.close();

  return 1;
}
  
//Prediction de la partie continue d'une particule en fonction de la dynamique avec ajout du bruit

// void ContDynamicRW::ContDrawFromDynWithNoise(Sample* Spin, Sample* Spout)
// {
//   /* Tir du mode de propagation */
//   this->val = cvRandReal(&rng_state);

//   if(this->val>this->betaU)
//     {
//       //cout << "RW" << endl;
//       /* Tir random walk */
//       this->DynNoiseGauss->Draw(Spin->ContinusP,Spout->ContinusP);
//     }
//   else
//     {
//       cout << "Uni" << endl;
//       /* Tir uniforme */
//       for(int i=0;i<this->nCP;i++)
// 	{
// 	  //cout << "param " << i << " : " << this->bound_min[i] << " + ranb * " << this->bound_max_norm[i] << endl;
// 	  Spout->ContinusP[i] = this->bound_min[i] + (cvRandReal( &this->rng_state ) * this->bound_max_norm[i]);
// 	}
//       Spout->Disp();
//     }
// }

//Prediction de la partie continue d'une particule en fonction de la dynamique sans bruit
void ContDynamicRW::ContDrawFromDynWithOutNoise(Sample* Spin, Sample* Spout)
{
  /* recopie du resultat de la prediction du vecteur d'etat */
  if(Spin!=Spout)
    memcpy(Spout->ContinusP,Spin->ContinusP,this->sizetmpVector);
}

void ContDynamicRW::Disp()
{ 
  cout << "-------------------------" << endl;
  cout << "|  Random Walk dynamic  |" << endl;
  cout << "-------------------------" << endl;
 
  //cout << "--> ContDynamicRW (Disp) \t:\t Dynamic Type = " << ContDynamicType.s() << endl;
  
  /* display matrice de dynamique */
  cout << "Dynamic Matrix (dim = " << this->dimDynMatrix << " x " << this->dimDynMatrix << ")" << endl;
  DispMatrix<double>(DynMatrix,this->dimDynMatrix,this->dimDynMatrix);
  cout << endl;

  /* display matrice de bruit de dynamique */
  cout << "Dynamic Noise Vector (dim = 1  x " << this->dimNoiseVector << ")" << endl;
  DispVector<double>(this->NoiseVector,this->dimNoiseVector);
  cout << "Chol Dynamic Noise Vector (dim = 1  x " << this->dimNoiseVector << ")" << endl;
  DispVector<double>(this->CholNoiseVector,this->dimNoiseVector); 
  cout << "Inv Dynamic Noise Vector (dim = 1  x " << this->dimNoiseVector << ")" << endl;
  DispVector<double>(this->InvNoiseVector,this->dimNoiseVector);
  cout << "Dynamic Noise Matrix (dim = " << this->dimNoiseVector  << "  x " << this->dimNoiseVector << ")" << endl;
  DispMatrix<double>(this->NoiseMatrix,this->dimNoiseMatrix,this->dimNoiseMatrix);
  cout << endl;

  /* display gaussienne */
  DynNoiseGauss->Disp("Dynamic Noise Gaussian"); 
}

void ContDynamicRW::CalcQSumln()
{
  this->coeffQ=0;
  for(int i=0;i<this->dimNoiseVector;i++)
    this->coeffQ+=log(this->NoiseVector[i]);
}
