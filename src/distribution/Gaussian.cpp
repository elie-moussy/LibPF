#include "LibPF/distribution/Gaussian.h"

/*!
 * Constructor
 */
Gaussian::Gaussian(string nomfic, string dispdec)
{
  this->dispdec = dispdec;
  if(!Load(nomfic)) cout << this->dispdec << "|ERROR|--> Gaussian (Constructor) \t:\t error loading parameters from file [" << nomfic << "]" << endl;
}


int Gaussian::Load(string nomfic)
{
  /* ouverture */
  ifstream fichier;
  fichier.open(nomfic.c_str(), ios::in);
  if(fichier.bad())
    return 0;
  
  /* Lecture de la dimension */
  FindStr(fichier,"<dim>");
  if(fichier.eof()) {cout << this->dispdec << "|ERROR|--> Gaussian (Load) \t:\t <dim> not found" << endl; return 0;}
  fichier >> dim;
  
  /* Allocations */
  Alloc(dim);
  
  /* Lecture moyenne */
  FindStr(fichier,"<mean>");
  if(fichier.eof()) 
    {cout << this->dispdec << "|ERROR|--> DetectSimu<TSpl,TW,TCP> (Load) \t:\t <mean> not found" << endl; return 0;}
  for(int i=0;i<dim;i++) 
    fichier >> mean[i];
       
  /* Lecture covariance */
  FindStr(fichier,"<cov>");
  if(fichier.eof()) 
    {cout << this->dispdec << "|ERROR|--> DetectSimu<TSpl,TW,TCP> (Load) \t:\t <cov> not found" << endl; return 0;}
  for(int i=0;i<dim*dim;i++) 
    fichier >> cov[i];

 
  /* Calcul des autres parametres */
  CalcParams();

  fichier.close();

  return 1;
}

/*!
 * Constructor
 */
Gaussian::Gaussian(int dim, string dispdec)
{
  this->dispdec = dispdec;
  Alloc(dim);
}

/*!
 * Constructor with initialisation
 */
Gaussian::Gaussian(int dim, double* m, double* cov, string dispdec)
{
  this->dispdec = dispdec;
  
  /* Gaussian construction */
  Alloc(dim);

  /* Initialisation */
  Init(m,cov);
}

/*! \fn void Gaussian::Alloc(int dim)
 *  \brief gaussian allocator
 *  \param the gaussian size
 */
void Gaussian::Alloc(int dim)
{
  typedef mat_cell<double> cell;

  this->dim = dim;

  meanMat = cvCreateMat(dim,1,cell::cv_type);
  mean = cell::get_data(meanMat);
  vectMat = cvCreateMat(dim,1,cell::cv_type);
  vect = cell::get_data(vectMat);
  covMat = cvCreateMat(dim,dim,cell::cv_type);
  cov = cell::get_data(covMat);
  cholcovMat = cvCreateMat(dim,dim,cell::cv_type);
  cholcov = cell::get_data(cholcovMat);
  invcovMat = cvCreateMat(dim,dim,cell::cv_type);
  invcov = cell::get_data(invcovMat);

  /* Creation des matrices qui contiendront des valeurs aleatoires */
  RandVectorMat = cvCreateMat(dim,1,cell::cv_type);      
  RandVector = cell::get_data(RandVectorMat);

  detcov = 0;
  coeff = 0;
  covIsdiagonal=0;

  /* calcul du nombre d'octet de chaque vecteur et matrice */
  meanSize = dim*sizeof(double);
  covSize = dim*dim*sizeof(double);

  /* vecteurs utilises temporairement pour des evaluations */
  yprim = new double[this->dim];
  diff = new double[this->dim];
  
  /* mise a 0 des matrices et vecteurs */
  memset(mean,0,meanSize);
  memset(vect,0,meanSize);
  memset(cov,0,covSize);
  memset(cholcov,0,covSize);
  memset(invcov,0,covSize);
  
  /* Generateur aleatoire init en fonction de l'horloge */
  //rng_state = cvRNG(0xffffffff);
  rng_state = cvRNG((uint64)RDTSC());
}

/* 
   Operateur d'affectation
*/

Gaussian &Gaussian::operator=(const Gaussian &source)
{
    if (&source != this)
    {
      /* copie moyenne */
      memcpy(mean,source.mean,source.meanSize);

      /* copie covariance */
      memcpy(cov,source.cov,source.covSize);

      /* copie chol covariance */
      memcpy(cholcov,source.cholcov,source.covSize);

      /* copie inv covariance */
      memcpy(invcov,source.invcov,source.covSize);

      /* copie determinant */
      detcov = source.detcov;
      detcholcov = source.detcholcov;

      /* copie coeff */
      coeff = source.coeff;

      /* normalement pas utile */
      covIsdiagonal = source.covIsdiagonal;
    }
    return *this;
}

/*!
 * Destructor
 */
Gaussian::~Gaussian()
{
  //cout << "Destruction de Gaussian" << endl;
  cvReleaseMat(&meanMat);
  cvReleaseMat(&covMat);
  cvReleaseMat(&cholcovMat);
  cvReleaseMat(&invcovMat);

  delete [] yprim;
  delete [] diff;
}


/*!
 * Initialisation of the gaussian
 */
void Gaussian::Init(double* m, double* c)
{
  /* moyenne */
  if(m) memcpy(mean,m,meanSize);
  
  /* covariance */
  if(c) memcpy(cov,c,covSize);
  
  /* calcul des autres parametres a partir des nouvelles donnees */
  CalcParams();
}

/*! \fn void Gaussian::Init(Gaussian* gin)
 *  \brief initialisation of the gaussian with mean m and covariance cov
 *  \param m the initial mean vector
 *  \param c the initial covariance matrix  
 */
void Gaussian::Init(Gaussian* gin)
{
  /* moyenne */
  memcpy(this->mean,gin->mean,this->meanSize);
  
  /* covariance */
  memcpy(this->cov,gin->cov,this->covSize);
  
  /* calcul des autres parametres a partir des nouvelles donnees */
  CalcParams();
}


void Gaussian::CalcParams()
{
   /* Teste si la matrice est diagonale */
  this->covIsdiagonal=isDiagonal<double>(cov,dim);
  
   /* calcul de cholcov */
  if(covIsdiagonal)
     {
       double* ptcholcov = cholcov;
       double* ptcov = cov;
       double* ptinvcov = invcov;
       for(int i=0;i<dim;i++,ptcholcov+=dim,ptcov+=dim,ptinvcov+=dim)
	 {
	   /* chol */
	   ptcholcov[i] = sqrt(ptcov[i]);

	   /* inverse */
	   if(ptcov[i]) ptinvcov[i] = 1.0/ptcov[i];
	 }
     }
  else
    {
      cout << this->dispdec ;
      printf("|WARNING|--> Gaussian (Init) \t:\t CHOLESKY DECOMPOSITION ONLY DEFINED FOR SYMETRIC POSITIVE SEMI DEFINITE MATRIX\n");
      if(!Cholesky<double>(cov,cholcov,dim))
	 printf("|ERROR|--> Gaussian (Init) \t:\t CHOLESKY DECOMPOSITION : Matrix must be positive definite\n");

      /* inverse covariance */
      cvInvert(covMat,invcovMat);
    }
    
  /* determinant */
  detcov = cvDet(covMat);
  
  /* coeff */
  if(dim>1) coeff = 1.0/(pow(DEUXPI,dim/2)*sqrt(detcov));
  else 
    {
      coeff = 1.0/(cov[0]*sqrt(DEUXPI));
      this->deuxsigcarre = 2 * cov[0] * cov[0];
    }
  

  //force coeff a 1 pour obtenir un resultat entre 0 et 1
  //coeff = 1;

}

/* mise a jour des parametres a partir de cholcov */
/* Methode bourrain qui est couteuse */
void Gaussian::UpdateAllFromCholcov()
{
  /* Teste si la matrice est diagonale */
  this->covIsdiagonal=isDiagonal<double>(cov,dim);

  /* on calcule P a partir de cholcov */
  /* Pour passer de chol(P) a P (si cholcov est triangulaire SUP alors P = cholcov'*cholcov si INF alors P = cholcov*cholcov' */
  SquareMatrixMultXXprim(this->cholcov,this->cov,this->dim);

  /* on met a jour l'inverse */
  cvInvert(covMat,invcovMat);

  /* on met a jour le determinant */
  detcov = cvDet(covMat); 

  if(dim>1) coeff = 1.0/(pow(DEUXPI,dim/2)*sqrt(detcov));
  else 
    coeff = 1.0/(cov[0]*sqrt(DEUXPI));
}

/*! \fn void SetMeanVector(float *m)
 *  \brief This function set mean equal to mean vector
 *  there is not copy of the values but only pointer copy
 *  that makes mean to point on the vector m datas
 *  \param m the vector of datas
 */
void Gaussian::SetMeanVector(double* m)
{
  mean = m;
}

/*! \fn void SetMean(float *m)
 *  \brief This function set mean values equal to mean vector
 *  values copying the datas
 *  \param m the vector of datas
 */

void Gaussian::SetMean(double* m)
{
  memcpy(mean,m,meanSize);
}

/*! \fn void SetMean(float *m, int size)
 *  \brief This function set mean values equal to mean vector
 *  values copying size datas
 *  \param m the vector of datas
 *  \param size is the memory size to copy
 */
void Gaussian::SetMean(double* m, int size)
{
  memcpy(mean,m,size);
}


/*! \fn void SetCovVector(float *c)
 *  \brief This function set cov equal to cov matrix
 *  there is not copy of the values but only a pointer copy
 *  that makes cov to point on the vector c datas
 *  \param c the vector of datas
 */
void Gaussian::SetCovVector(double* c)
{
  cov = c;

  //mise a jour des autres parametres
  CalcParams();
}

/*! \fn void SetCov(float *c)
 *  \brief This function set cov values equal to c matrix
 *  values copying the datas
 *  \param c the matrix of datas
 */
void Gaussian::SetCov(double* c)
{
  memcpy(cov,c,covSize);

  //mise a jour des autres parametres
  CalcParams();
}

/*!
 * Display the gaussian with message label
 */
void Gaussian::Disp(char *label)
{
  cout << this->dispdec << label << endl;
  Disp();
}


/*!
 * Display the gaussian
 */

void Gaussian::Disp()
{
  cout << this->dispdec << "Gaussian dim = " << this->dim << endl;
  cout << this->dispdec << "Mean : ";
  DispVector<double>(mean,dim,this->dispdec);
  cout << this->dispdec << "Cov : " << endl;
  DispMatrix<double>(cov,dim,dim,this->dispdec);
  cout << this->dispdec << "cholCov : " << endl;
  DispMatrix<double>(cholcov,dim,dim,this->dispdec); 
  cout << this->dispdec << "invCov : " << endl;
  DispMatrix<double>(invcov,dim,dim,this->dispdec);
  cout << "Coeff = " << this->coeff << endl;
  cout << "Det = " << this->detcov << endl;

  cout << endl;
}

/*! \fn Gaussian* Gaussian::ExtractContigousSubGauss(int nbparam, int startp, int endp)
 *  \brief Extraction of a sub part of the gaussian from contigous parameters
 *  \param nbparam is the number of parameters to extract
 *  \param startp is the first parameter position
 *  \param endp is the last parameter position
 *  \return a gaussian smaller that this one containing the gaussian sub part
 */
Gaussian* Gaussian::ExtractContigousSubGauss(int nbparam, int startp, int endp)
{
  int *parlist = new int[nbparam];

  /* init liste des parametres a extraire */
  for(int i=0,j=startp;j<=endp;i++,j++)
    parlist[i] = j;

  /* Extraction */
  Gaussian* gtmp = ExtractSubGauss(nbparam,parlist);

  /* Liberation de la memoire */
  delete [] parlist;

  return gtmp;
}

/*! \fn Gaussian* Gaussian::ExtractSubGauss(int nbparam, int *parlist)
 *  \brief Extraction of a sub part of the gaussian
 *  \param nbparam is the number of parameters to extract
 *  \param parlist is the parameter list
 *  \return a gaussian smaller that this one containing the gaussian sub part
 */
Gaussian* Gaussian::ExtractSubGauss(int nbparam, int *parlist)
{
  /* test des parametres */
  if(nbparam>=dim) return NULL;

  /* Allocation des mean et cov temporaire */
  double* meantmp = new double[nbparam];
  double* covtmp = new double[nbparam*nbparam];
  int id;

  /* recuperation des moyennes */
  for(int i=0;i<nbparam;i++)
    {
      id = parlist[i];
      meantmp[i] = mean[id];

      /* recuperation des covariances */
      for(int j=0;j<nbparam;j++)
	{
	  covtmp[i*nbparam+j] = cov[id*dim+parlist[j]];
	}
    }

  /* creation de la nouvelle gaussienne */
  Gaussian* gtmp = GaussianAlloc(nbparam,meantmp,covtmp,this->dispdec);

  /* Liberation de la memoire */
  delete [] meantmp;
  delete [] covtmp;

  return gtmp;
}

/* Calcul de p a partir de sa racine */
double Gaussian::EvalFromCholCov(double *v, double* m)
{
  double* ptcholcov = this->cholcov;
  double prodcholcov = 1;
  double yprimy=0;
  
  /* calcul de y' = (v - mean)' / cholcov' */  
  for(int i=0;i<this->dim;i++)
    this->diff[i] = v[i] - m[i];
  
  SolveLowerSystem(this->yprim,this->cholcov,this->diff,1,this->dim);
  //SolveUpperSystem(this->yprim,this->cholcov,this->diff,1,this->dim);

  for(int i=0;i<this->dim;i++,ptcholcov+=dim)
    { 
      /* calcul de prod(cholcov_{ii}) */
      prodcholcov *= ptcholcov[i]; 

      /* calcul de y'y */   
      yprimy += this->yprim[i]*this->yprim[i];
    }

  /* calcul de p = coeffchol * exp(-0.5*y'*y) */
  return  1.0/prodcholcov * exp(-0.5*yprimy);

}

/* Calcul de p a partir de sa racine */
double Gaussian::lnEvalFromCholCov(double *v, double* m)
{
  double* ptcholcov = this->cholcov;
  double sumlncholcov = 0;
  double yprimy=0;
  
  /* calcul de y' = (v - mean)' / cholcov' */  
  for(int i=0;i<this->dim;i++)
    this->diff[i] = v[i] - m[i];
  
  SolveLowerSystem(this->yprim,this->cholcov,this->diff,1,this->dim);
  //SolveUpperSystem(this->yprim,this->cholcov,this->diff,1,this->dim);

  for(int i=0;i<this->dim;i++,ptcholcov+=dim)
    { 
      /* calcul de prod(cholcov_{ii}) */
      sumlncholcov += log(ptcholcov[i]); 

      /* calcul de y'y */   
      yprimy += this->yprim[i]*this->yprim[i];
    }

  /* calcul de p = exp( - sumlncholcov - (0.5*y'*y) */
  return  exp( -sumlncholcov - (0.5*yprimy) );
}

/* Fonction qui calcule le determinant de Sk (cholcov) car c'est utilise dans JMSUKF */
void Gaussian::CalcSkdet()
{
  double* ptcholcov = this->cholcov;
  
  this->detcholcov=1;
  for(int i=0;i<this->dim;i++,ptcholcov+=dim)
    this->detcholcov*=ptcholcov[i];
}

/* Fonction qui calcule le coefficient Sum(log(Sk_ii)) qui est utilise dans JMSUKF */
void Gaussian::CalcSkSumln()
{
  double* ptcholcov = this->cholcov;
  
  this->coeffcholcov=0;
  for(int i=0;i<this->dim;i++,ptcholcov+=dim)
    this->coeffcholcov+=log(fabs(ptcholcov[i]));
}

/* Fonction de recopie de certains parametres utilisee par JMSUKF */
void Gaussian::CopySk(Gaussian* gin)
{
  /* copie moyenne */
  memcpy( this->mean,gin->mean,gin->meanSize);

  /* copie chol covariance */
  memcpy( this->cholcov,gin->cholcov,gin->covSize);

  /* copie du coeff de Sk */
  this->coeffcholcov = gin->coeffcholcov;
}
