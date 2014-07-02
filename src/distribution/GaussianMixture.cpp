#include "distribution/GaussianMixture.h"

/*! \fn GaussianMixture::GaussianMixture(string nomfic)
 *  \brief Class constructor from a file
 *  \param nomfic the filename containing gaussian mixture
 */
GaussianMixture::GaussianMixture(string nomfic,string dispdec)
{
  this->dispdec = dispdec;
  if(!Load(nomfic)) cout << dispdec << "|ERROR|--> GaussianMixture (Constructor) \t:\t error loading parameters from file [" << nomfic << "]" << endl;
}


/*! \fn GaussianMixture::GaussianMixture(int nb, int dim)
 *  \brief Class constructor which only allocate ang initialise the class
 *  \param nb the number of gaussians in the mixture
 *  \param dim the gaussians dimension
 */
GaussianMixture::GaussianMixture(int nb,int dim,string dispdec)
{
  this->dispdec = dispdec;
  /* allocation */
  this->Alloc(nb,dim,NULL,NULL);
  
  this->dim = dim;
}

/*! \fn GaussianMixture(int nb, int dim, T *m, T *cov)
 *  \brief Class constructor
 *  \param nb the number of gaussians in the mixture
 *  \param dim the gaussian dimension
 *  \param m the initial mean vector
 *  \param c the initial covariance matrix
 */
GaussianMixture::GaussianMixture(int nb,int dim, double *m, double *cov,string dispdec)
{
  this->dispdec = dispdec;
  /* allocation */
  this->Alloc(nb,dim,m,cov);

  this->dim = dim;
}

/*! \fn GaussianMixture(int nb, Gaussian *g)
 *  \brief Class constructor
 *  \param nb the number of gaussians in the mixture
 *  \param g the gaussian used for gaussian mixture initialisation
 */
GaussianMixture::GaussianMixture(int nb, Gaussian *g)
{
  this->dispdec = g->dispdec;
  /* allocation */
  this->Alloc(nb,g->dim,g->mean,g->cov);

  this->dim = g->dim;
}

int GaussianMixture::Load(string nomfic)
{
  int ddim,nbgauss;
  ifstream fichier;
  double* mm;
  double* cc;
  char pattern[256];

  /* ouverture */
  fichier.open(nomfic.c_str(), ios::in);
  if(fichier.bad())
    return 0;
  
  /* Lecture du nombre de gaussiennes */
  FindStr(fichier,"<nb>");
  if(fichier.eof()) {cout << dispdec << "|ERROR|--> GaussianMixture (Load) \t:\t <nb> not found" << endl; return 0;}
  fichier >> nbgauss;  

  /* Lecture de la dimension des gaussiennes */
  FindStr(fichier,"<dim>");
  if(fichier.eof()) {cout << dispdec << "|ERROR|--> GaussianMixture (Load) \t:\t <dim> not found" << endl; return 0;}
  fichier >> ddim; 

  this->dim = ddim;

  /* Allocations d'un melange de gaussiennes sans init */
  Alloc(nbgauss,this->dim,NULL,NULL);

  /* Allocation de moyenne et covariance temp */
  mm = new double[this->dim];
  cc = new double[this->dim*this->dim];
  
  /*Lecture des gausiennes une par une */
  for(int n=0;n<nbgauss;n++)
    {
      /* Lecture moyenne */
      sprintf(pattern,"<mean_%d>",n);
      FindStr(fichier,pattern);
      if(fichier.eof()) {cout << dispdec << "|ERROR|--> GaussianMixture (Load) \t:\t " << pattern << " not found" << endl; return 0;}
      for(int i=0;i<this->dim;i++) 
	fichier >> mm[i];
      
      /* Lecture covariance */
      sprintf(pattern,"<cov_%d>",n);
      FindStr(fichier,pattern);
      if(fichier.eof()) {cout << dispdec << "|ERROR|--> GaussianMixture (Load) \t:\t " << pattern << " not found" << endl; return 0;}
      for(int i=0;i<this->dim*this->dim;i++) 
	fichier >> cc[i];

      /* Lecture coefficient */
      // sprintf(pattern,"<coeff_%d>",n);
      //       FindStr(fichier,pattern);
      //       if(fichier.eof()) {cout << "|ERROR|--> GaussianMixture (Load) \t:\t " << pattern << " not found" << endl; return 0;}
      //       fichier >> coeff[i];
 
      /* Init de la gaussienne i */
      glist[n]->Init(mm,cc);
    }

  fichier.close();

  delete [] mm;
  delete [] cc;

  return 1;
}

/*! \fn void Alloc(int nb, int dim)
 *  \brief Gaussian Mixture initialisations and allocations
 *  \param nb the number of gaussians in the mixture
 *  \param dim the gaussians dimension
 */
void GaussianMixture::Alloc(int nb, int dim, double *m, double *cov)
{
  //printf("nb = %d  et dim = %d\n",nb,dim);
  this->nb = this->curnb = nb;
  
  glist = new Gaussian*[nb];
  coeff = new double[nb];
  coeffCumul = new double[nb];
  
  this->areEquiprob = 1;
  this->EquiprobCoeff = 1.0/(double)nb;
  
  glist[0] = GaussianAlloc(dim,m,cov,dispdec);
  if(cov || m) glist[0]->Init(m,cov);
  coeff[0] = this->EquiprobCoeff;  
  coeffCumul[0] =  coeff[0];
  for(int i=1;i<nb;i++)
    {
      glist[i] = GaussianAlloc(dim,m,cov,dispdec);
      if(cov || m) glist[i]->Init(m,cov);
      coeff[i] = this->EquiprobCoeff;  
      coeffCumul[i] =  coeffCumul[i-1] + coeff[i];
    }
  
  /* Generateur aleatoire init en fonction de l'horloge */
  //rng_state = cvRNG(0xffffffff);
  rng_state = cvRNG((uint64)RDTSC());
}

/*!
 * Destructor
 */
GaussianMixture::~GaussianMixture()
{
  cout << "Destruction de GaussianMixture" << endl; 
  for(int i=0;i<nb;i++) {
    delete glist[i];
  }
  delete [] glist;
  delete [] coeff;
  delete [] coeffCumul;
}

/* cette fonction n'initialise pas la gaussienne mais copie simplement m et cov */
void GaussianMixture::add(double *m, double *cov)
{
  /* moyenne */
  if(m) memcpy(this->glist[this->curnb]->mean,m,this->glist[this->curnb]->meanSize);
  
  /* covariance */
  if(cov) memcpy(this->glist[this->curnb]->cov,cov,this->glist[this->curnb]->covSize);

  this->curnb++;

  /* mise a jour du coeff */
  this->EquiprobCoeff = 1.0/(double)curnb;
}

/*! \fn void Init(T *m, T *cov)
 *  \brief Initialisation of the gaussian with same mean and cov
 *  \param m the initial mean vector
 *  \param c the initial covariance matrix
 */
void GaussianMixture::Init(double *m, double *cov)
{
  /* Initialisation */
  for(int i=0;i<this->nb;i++)
    {
      this->glist[i]->Init(m,cov);
    }
}

/*! \fn void Init(Gaussian *g)
 *  \brief Initialization of the gaussian mixture with the gaussian g
 *  \param g the gaussian used for gaussian mixture initialisation
 */
void GaussianMixture::Init(Gaussian *g)
{
  this->Init(g->mean,g->cov);
}

/*! \fn void Transform(CvPoint *centerslist, int nbcenters)
 *  \brief This method is used to change the mixture 
 *  by positionning each gaussian of the mixture on the centers
 *  in parameter. This method is usable only with 2D gaussian mixture
 *  \param centerslist is the list of the centers on which the gaussian must be positionned
 *  \param nbcenters is the number of centers and must be lowed than nb
   */
void GaussianMixture::Transform(CvPoint *centerslist, int nbcenters)
{
  /* mise a jour du nombre de gausiennes et du coefficient d'equiprobabilite */
  curnb = nbcenters;
  EquiprobCoeff = 1.0/(double)curnb;
  
  /* placement des gaussiennes sur les centres detectes */
  for(int i=0;i<curnb;i++)
    {
      glist[i]->mean[0] += centerslist[i].x;
      glist[i]->mean[1] += centerslist[i].y;
    }
}

/*! \fn void Eval(T *v)
 *  \brief This method evaluate the vector v in the gaussian mixture
 *  \param v the vector to evaluate
 *  \return a TEV corresponding to the likelihood of the vector
 */
double GaussianMixture::Eval(double *v)
{
  if(areEquiprob) return EvalEquiProb(v);
  else return EvalWeighted(v);
}

/*! \fn void EvalEquiProb(T *v)
 *  \brief This method evaluate the vector v in the gaussian mixture
 *  where each gaussian are equiprobable
 *  \param v the vector to evaluate
 *  \return a TEV corresponding to the likelihood of the vector
 */
double GaussianMixture::EvalEquiProb(double *v)
{
  double sum=0.0;

  //cout << "Equiprob avec curnb = " << curnb << " et EquiprobCoeff = " << EquiprobCoeff << endl;

  /* Calcul des probabilites pour chaque gaussienne */
  for(int i=0;i<curnb;i++)
    {      
      //cout << "Eval " << i << " : " << glist[i]->mean[0] << " / " << v[0] << " = " << glist[i]->Eval(v) << endl;
      
      sum+=glist[i]->Eval(v);
    }

  return 1.0/curnb*sum;
}
  
/*! \fn void EvalWeighted(T *v)
 *  \brief This method evaluate the vector v in the gaussian mixture
 *  \param v the vector to evaluate
 *  \return a TEV corresponding to the likelihood of the vector
 */
double GaussianMixture::EvalWeighted(double *v)
{
  double prob=0.0;

  /* Calcul des probabilites pour chaque gaussienne */
  for(int i=0;i<curnb;i++)
    prob+=coeff[i]*glist[i]->Eval(v);

  return prob;
}

/*! \fn void Draw(T *vout)
 *  \brief This function draw a vector from the gaussian mixture
 *  \param v the vector to evaluate
 */
void GaussianMixture::Draw(double *v)
{  
  /* selection de la gaussienne */
  int selected = Select();

  /* tir du vecteur selon la gaussienne selectionnee */
  glist[selected]->Draw(v);
}

/*! \fn void Draw(T* mm, T *vout)
 *  \brief This function draw a vector from the gaussian mixture
 *  \param v the vector to evaluate
 */
void GaussianMixture::Draw(double* mm, double *v)
{  
  /* selection de la gaussienne */
  int selected = Select();

  /* tir du vecteur selon la gaussienne selectionnee */
  glist[selected]->Draw(mm,v);
}

/*! \fn int Select()
 *  \brief This select un number corresponding to a gaussian index
 *  according to the gaussian weights
 *  \return an integer that is a gaussian index
 */
int GaussianMixture::Select()
{
  if(curnb==1) return 0;

  /* si les gaussiennes sont equiprobables alors on retourne un entier 
     tire aleatoirement entre 0 et le nombre de gaussiennes */
  if(areEquiprob) return cvRandInt(&rng_state) % curnb;
  else
    {
      /* Choix de l'index en fonction des poids de chaque gaussiennes */
      int id;
      double RandVal;
      
      /* Tir aleatoire uniforme d'un reel entre 0 et 1 */
      RandVal = cvRandReal(&rng_state);
      
      /* Choix de l'etat en fonction des probas de transition */
      id=0;
      
      /* Choix en fonction des poids */
      while((id < curnb) && (RandVal > coeffCumul[id]) )	
	id++;

      return id;
    }
}

/*! \fn void Disp()
 *  \brief Display the dim, mean and cov of each gaussian in the mixture
 */
void GaussianMixture::Disp()
{
  cout << dispdec << "Mixture of " << curnb << " gaussian" << endl;
  for(int i=0;i<curnb;i++)
    {
      cout << dispdec << "G" << i << " : " << endl;
      glist[i]->Disp();
    }
}

/*! \fn void ViewGM(char *nm, int w=320, int h=240)
 *  \brief this function display the 2D gaussian mixture on an image
 */
void GaussianMixture::ViewGM(char *nm, int w, int h)
{
  if(this->dim != 2) cout << "|ERROR|--> GaussianMixture (ViewGM) \t:\t dim not match to display on 2D image" << endl;
  IplImage *dsp = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,3);
  double v[2];
  for(int i=0;i<h;i++)
    for(int j=0;j<w;j++)
      {
	v[0] = j;
	v[1] = i;
	double prob = this->Eval(v);
	cvSet2D(dsp,i,j,SCalcTeint((int)(prob/glist[0]->coeff*255)));
      }
  cvNamedWindow(nm,0);
  cvMoveWindow(nm,0,0);
  cvShowImage(nm,dsp);
  cvWaitKey(5);
}


/*! \fn void ViewFromDraw(int nbpt, char *nm, int w=320, int h=240)
 *  \brief this function display a 2D gaussian mixture on an image with weighted points
 */
void GaussianMixture::ViewFromDraw(int nbpt, char *nm, int w, int h)
{
  if(this->dim != 2) cout << "|ERROR|--> GaussianMixture (ViewFromDraw) \t:\t dim not match to display on 2D image" << endl;
  IplImage *dsp = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,3);
  double v[2];
  for(int i=0;i<nbpt;i++)
    {
      //cout << "1 : " << v[0] << "," << v[1] << "\n";
      this->Draw(v);
      //cout << "2 : " << v[0] << "," << v[1] << "\n";     
      double prob = this->Eval(v);
      if((v[1]>=0)&&(v[1]<h)&&(v[0]>=0)&&(v[0]<w))
	cvSet2D(dsp,(int)v[1],(int)v[0],SCalcTeint((int)(prob/glist[0]->coeff*255)));
    }

  cvNamedWindow(nm,0);
  cvMoveWindow(nm,w+10,0);
  cvShowImage(nm,dsp);
  cvWaitKey(5);
  cvReleaseImage(&dsp);
}

int GaussianMixture::updateMixtureCoeff(double* coefflist, double coeffsum)
{
  //printf("curnb = %d\n",curnb);

  if(!coeffsum) {
    curnb = 0;
    return curnb;
  }

  /* Recopie des coeff */
  memcpy(coeff,coefflist,sizeof(double)*curnb);

  /* Normalisation et calcul du vecteur de coeff cumule */
  coeff[0] /= coeffsum;
  coeffCumul[0] = coeff[0];
  //printf("0 : coeff = %f coeffCumul = %f\n",coeff[0],coeffCumul[0]);
  for(int i=1;i<curnb;i++) {
    coeff[i] /= coeffsum;
    coeffCumul[i] = coeffCumul[i-1] + coeff[i];
    //printf("%d : coeff = %f coeffCumul = %f\n",i,coeff[i],coeffCumul[i]);
  }

  /* Mixture plus equiprobable */
  areEquiprob = 0;    

  return curnb;
}
