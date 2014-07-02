#include "ParticleFilter.h"

/*!
 *  \brief Class constructor : the particle filter is created using context parameters
 *  here, some context tools such as face detector or blob detector are constructed
 */
ParticleFilter::ParticleFilter(string filename, ImgProducer* imgproducer, twoDDetector* detector, Prior* prior, 
			       ContDynamicModel* ContDynamic, DiscDynamicModel** DiscDynamic)
{
  string tmp;
  ifstream fichier;

  Message("ParticleFilter","(CO)","Loading from "+filename+" ...");

  /* ouverture du fichier */
  fichier.open(filename.c_str(), ios::in);
  if(!fichier)
    {
      ErrorMessage("ParticleFilter","(CO)","Error loading "+filename);
      return;
    }

  /* Lecture du nombre de particules pour le filtre */
  FindStr2(fichier,"<nbSamples>");
  if(fichier.eof()) 
    {
      ErrorMessage("ParticleFilter","(LO)","<nbSamples> not found"); 
      return;
    }
  fichier >> nbSamples;
  cout << "--> ParticleFilter (CO) \t:\t Number of samples : " << nbSamples << endl;

  /* Lecture du fichier de config de la cible */
  FindStr2(fichier,"<TargetFile>");
  if(fichier.eof()) 
    {
      ErrorMessage("ParticleFilter","(LO)","<TGFile> not found");
      return;
    }
  fichier >> this->TGFile;
  Message("ParticleFilter","(LO)","Target filename ["+TGFile+"]");

  /* Allocation des parametres a partir des donnees chargees */
  Message("ParticleFilter","(LO)","ParticleFilter Allocation....");
 

  /**************************************************************/
  /*                     ALLOCATIONS                            */
  /**************************************************************/

  /* Allocation de la cible  */
  Message("ParticleFilter","(CO)","Target Allocation .... ");
  string nomcomplet;
  FileReplace(filename,TGFile,nomcomplet);
  target = TargetAlloc(nomcomplet,imgproducer,detector,prior, 
		       ContDynamic,DiscDynamic);
  if(!this->target)
    {
      ErrorMessage("ParticleFilter","(CO)","Target Allocation FAILED !");
      return;
    }
  Message("ParticleFilter","(CO)","Target Allocation DONE");

  if(!target->ContDynamic)
    {
      ErrorMessage("ParticleFilter","(CO)","Target have not Continuous dynamic !!!");
      return;
    }

  /* Par defaut pas de cible supplementaire */
  this->TargetFirstPass = 0;
  this->target_first_pass = NULL;

  /* recuperation du nombre de parametres a estimer */
  nCP = target->nCP + target->ContDynamic->nDynCP;
  nDP = target->nDP + target->ContDynamic->nDynDP;
  //nCPnCP = nCP + nCP;

  cout << "--> ParticleFilter (Alloc) \t:\t Estimation of " << nCP << " continuous parameters" << endl;
  cout << "--> ParticleFilter (Alloc) \t:\t Estimation of " << nDP << " discrets parameters" << endl;
    
  /* Allocation des vecteurs de poids et des samples */
  /* creation d'un vecteur de poids courant */
  curw = new WeightsVector(nbSamples);
  curw->InitVector();
  
  /* creation d'un vecteur de poids precedents */
  prevw = new WeightsVector(nbSamples);
  prevw->InitVector();
  
  /* Allocation of the samples */
  sps = new Sample*[nbSamples];
  spsnew = new Sample*[nbSamples];

  /* creation des samples et init de m et P dans le cas de JMSUKF */
  for(int i=0;i<nbSamples;i++)
    {
      sps[i] = new Sample(nCP,nDP);
      spsnew[i] = new Sample(nCP,nDP);
    }

  /* Allocation du sample moyen */
  pfState = new Sample(nCP,nDP);

  /* Init du N efficace */
  this->Neff = 1.0;
  this->sumNeff = 0;

  /* Init du seuil de resampling */
  this->Ns = 0.5;

  /* covariance des particules */
  Pkk = (double*) AllocMatrix<double>(nCP,nCP);

  /* si il y a des parametres discrets a estimer alors on alloue ce qu'il faut pour la selection de la particule moyenne */
  if(nDP)
    {
      /* Allocation du vecteur de proba cumule par etat possible de chaque parametre discret */
      VectProbByDState = AllocVector<double*>(nDP);

      for(int i=0;i<nDP;i++)
	VectProbByDState[i] = AllocVector<double>(target->DiscDynamic[i]->nbStates);
    }
 
  /* Generateur aleatoire en fonction de l'horloge */
  //rng_state = cvRNG(0xffffffff);
  rng_state = cvRNG((uint64)RDTSC());
  
  /* Init du compteur de pas */
  this->numStep = 0;

#ifdef SAUVEIMGPARTICULES
  this->imgParticles = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,3);
#endif

  cout << "--> ParticleFilter (Alloc) \t:\t Allocation .... OK" << endl;

}

/**
   Destructeur du Filtre : Libere toutes les ressources allouées pour le filtrage.
*/

ParticleFilter::~ParticleFilter()
{
  cout << "Destruction de ParticleFilter" << endl;
  delete target;

  for(int i=0;i<nbSamples;i++)
    {
      delete sps[i];
      delete spsnew[i];
    }

  delete [] sps;
  delete [] spsnew;

  delete curw;
  delete prevw;
  delete pfState;

  //cvFree((void**)&Pkk);

  if(nDP)
    {
      for(int i=0;i<nDP;i++)
	cvFree((void**)VectProbByDState+i);
      
      cvFree((void**)VectProbByDState);
    }  
}


/*! \fn double ParticleFilter::CalcNeff()
 *  \brief Calcul of the filter efficacity :
 *  \f$N_{eff} \propto \frac{1}{\sum_{i=1}^{N} (w_k^i)^2}\f$
 *  \return Neff the filter efficacity
   */

double ParticleFilter:: CalcNeff(WeightsVector* weights)
{
  int i;
  double* normW = weights->getNormalizedWptr();
  this->Neff=0.0;
  for(i=0;i<this->nbSamples;i++) this->Neff+=(pow(normW[i],2));
  this->Neff = (double)100.0/(double)(this->Neff*this->nbSamples);
  this->sumNeff+=this->Neff;
  return this->Neff;
}

/*! \fn void ParticleFilter::CalcPkk()
 *  \brief Processing of the SampleSet covariance at time k given k
 *  \f$P_{k|k} = \sum_{i=1}^{N} (w_k^i * x_k^i * (x_k^i)^T) - \sum_{i=1}^{N} (w_k^i * x_k^i) *  \sum_{i=1}^{N} (w_k^i * x_k^i)^T\f$
 */
void ParticleFilter::CalcPkk(Sample** Spl, WeightsVector* weights)
{
  int i,j,k;
  double *Sum1,*Sum2,*Sum3;
  double *ptSum1,*ptSum2,*ptSum3,*ptPkk;

  /* recup du vecteur de poids normalises */
  double* normW = weights->getNormalizedWptr();

  Sum1 = (double*) calloc(nCP*nCP, sizeof(double) );
  Sum2 = (double*) calloc(nCP, sizeof(double) );
  Sum3 = (double*) calloc(nCP*nCP, sizeof(double) );
 
  SetZero(Sum2,nCP);

  //SUM_i(w_k_i * x_k_i)
  for(i=0;i<nbSamples;i++)
    {
      for(j=0,ptSum2 = Sum2;j<nCP;j++)
	{
	  //parcours de tous les parametres continus
	  ptSum2[j] += normW[i] * Spl[i]->ContinusP[j];
	  
	}
    }
  // printf("Somme 2\n");
  //   DispVector<double>(Sum2,nCP);
  //   getchar();

  SetZero(Sum1,nCP*nCP);
  SetZero(Sum3,nCP*nCP);

  //SUM_i(w_k_i * x_k_i * x_k_i')
  for(i=0;i<nbSamples;i++)
    {
      ptSum1 = Sum1;
      for(j=0;j<nCP;j++,ptSum1+=nCP)
	{
	  for(k=0;k<nCP;k++)
	    {	      
	      ptSum1[k] +=  normW[i] * Spl[i]->ContinusP[j]* Spl[i]->ContinusP[k];
	    
	    }
	}
    }

  //SUM_i(w_k_i * x_k_i) * SUM_i(w_k_i * x_k_i)'
  ptSum2 = Sum2;
  ptSum3 = Sum3;
  for(j=0;j<nCP;j++,ptSum3+=nCP)
    {
      for(k=0;k<nCP;k++)
	{	      
	  ptSum3[k] +=  ptSum2[j] * ptSum2[k];
	}
    }
  
  //Calcul de la covariance
  //SUM_i(w_k_i * x_k_i * x_k_i') - SUM_i(w_k_i * x_k_i) * SUM_i(w_k_i * x_k_i)'
  for(j=0,ptSum1=Sum1,ptSum3=Sum3,ptPkk=Pkk;j<nCP*nCP;j++,ptSum1++,ptSum3++,ptPkk++)
    {
      *ptPkk =  (*ptSum1) -  (*ptSum3);
    }

  DispMatrix<double>(Pkk,nCP,nCP);

  free(Sum1);
  free(Sum2);
  free(Sum3);

}

/*! \fn void ParticleFilter::Select()
 *  \brief Selection of the best sample by posterior mean computation
 *  from samples and WeightsVector in parameters
 */

void ParticleFilter:: Select(Sample** spl, WeightsVector* wv)
{
  //Il faut d'abbord choisir les parametres discrets les plus probables
  if(nDP) this->SelectDiscretsP(spl,wv);

  //Ensuite on selectionne les parametres continus conditionnellement aux parametres discrets
  this->SelectContinusP(spl,wv);
  
  //L'etat du filtre etant calcule je peux mettre a jour le modele de la cible 
  this->target->UpdateModel(pfState);
  if(this->TargetFirstPass) this->target_first_pass->UpdateModel(pfState);
}

/*! \fn void ParticleFilter::SelectContinusP()
 *  \brief Computation of the posterior mean on the continuous parameters 
 *  from samples and WeightsVector in parameters
 */

void ParticleFilter:: SelectContinusP(Sample** spl, WeightsVector* wv)
{
  int i,j;
  double confidencesum=0;
  double w;

  /* recup du vecteur de poids normalises */
  double* normW = wv->getNormalizedWptr();

  //RAZ du sample moyen
  pfState->ResetContinusP();


#warning ici on peut passer en parametre la somme des poids dans l etat le plus probable afin de ne pas recalculer confidencesum

  if(nDP)
    {
      /* Si il a des parametres discretss */
      /* Calcul de la MAP */
      for( i=0 ; i < nbSamples ; i++ )
	{
	  //w = this->SplSet->getCurW(i);
	  if( (normW[i]>0) && this->CheckDiscretsP(spl[i]) )
	    {
	      confidencesum+=normW[i];
	      /** MAJ du sample moyen */
	      for(j=0;j < nCP;j++)
		pfState->ContinusP[j] += spl[i]->ContinusP[j] * normW[i];
	    }
	} 

      /* Normalisation */
      for(j=0;j < nCP;j++)
	pfState->ContinusP[j]/=confidencesum;
    }
  else
    {
      /* Moyenne a posteriori classique */
      for( i=0 ; i < nbSamples ; i++ )
	{
	  if(normW[i]>0)
	    {
	      /** MAJ du sample moyen */
	      for(j=0;j < nCP;j++)
		{  
		  pfState->ContinusP[j] += spl[i]->ContinusP[j] * normW[i] ;
		}
	    }
	}
    }
}

/*! \fn void ParticleFilter::SelectDiscretsP()
 *  \brief Selection of the best state of each discrets parameters
 *  from samples and WeightsVector in parameters
 */

void ParticleFilter:: SelectDiscretsP(Sample** spl, WeightsVector* wv)
{
  double pmax=0;
  int i,j;
  int etat=0;

  /* recup du vecteur de poids normalises */
  double* normW = wv->getNormalizedWptr();
  
  /* Init du vecteur de proba par etat */
  for(j=0;j<nDP;j++)
    memset(VectProbByDState[j],0,sizeof(double)*target->DiscDynamic[j]->nbStates);
  
  /* Parcours des particules pour determiner l'etat le plus probable */
  for(i=0;i<nbSamples;i++)
    for(j=0;j<nDP;j++)
      VectProbByDState[j][(int)spl[i]->DiscretsP[j]]+=normW[i];
  
  /* Affichage des probas de chaque etat */
  for(j=0;j<nDP;j++)
    {
      pmax=0.0;
      etat=0;
      for(i=0;i<target->DiscDynamic[j]->nbStates;i++)
 	{
 	  if(VectProbByDState[j][i]>pmax)
 	    {
 	      pmax = VectProbByDState[j][i];
	      etat = i;
	    }
 	}
      pfState->DiscretsP[j]=etat;
    }
}

/*! \fn int ParticleFilter::CheckDiscretsP()
 *  \brief Checking of the discret parameter of the sample snum, this method look if its state are the same of those 
 *  computed in SelectDiscretsP()
 *  \param spl : the particle to check
 *  \return an int that indicate if the particle snum is ok 
 *  @arg 1 --> ok 
 *  @arg 0 --> not ok
 */

int ParticleFilter::CheckDiscretsP(Sample* spl)
{
  int i;
  for(i=0;i<nDP;i++)
    if((int)pfState->DiscretsP[i] != (int)spl->DiscretsP[i]) return 0;
  return 1;
}

/*! \fn int Process()
 *  \brief This method perform the filtering loop with filter step and target model update 
 */

int ParticleFilter::Process()
{
  /* Filtrage */

  /* Mise a jour des */

  cout << "Je ne suis pas sur d'avoir besoin de cette fonction, il faudra voir si elle devient necessaire ???? " << endl;

}

/*! \fn int InitSamplesFromPrior(Sample** spl)
 *  \brief This method initialize the filter by drawing the particles around a prior mean vector
 *  with a prior covariance.
 */
int ParticleFilter::InitSamplesFromPrior(Sample** spl)
{
  /* Teste si on utilise le mode xxdot */
  switch(target->ContDynamic->dynamicID)
    {
    case RW:
      {
	/* Tir selon la detection et init des vitesses selon prior */
	for(int i=0;i<this->nbSamples;i++)
	  {
	    /* tir de tous les parametres selon le prior */
	    this->target->prior->Draw(spl[i]);
		
	    /* Init equiprobable des etats discrets */
	    InitDiscFromUniform(spl[i]);
	  }
      }
      break;
    }

  /* Init du modele de la cible */
  target->InitModelFromPrior();

  /* Init des vecteurs de poids */
  this->curw->InitVector();
  this->prevw->InitVector();

  return 1;
}

/*! \fn int InitSamplesFromPrior(double* m)
 *  \brief This method initialize the filter by drawing the particles around a vector m
 *  with a prior covariance.
 */
int ParticleFilter::InitSamplesFromPrior(Sample** spl,double* m)
{
  /* Teste si on utilise le mode xxdot */
  switch(target->ContDynamic->dynamicID)
    {
    case RW:
      {
	/* Tir selon la detection et init des vitesses selon prior */
	for(int i=0;i<this->nbSamples;i++)
	  {
	    /* tir de tous les parametres selon le prior */
	    this->target->prior->Draw(m,spl[i]);
	    
	    /* Init equiprobable des etats discrets */
	    InitDiscFromUniform(spl[i]);
	  }
      }
      break;
    }
  
  /* Init du modele de la cible */
  target->InitModelFromPrior(m);

  /* Init des vecteurs de poids */
  this->curw->InitVector();
  this->prevw->InitVector();
  
  return 1;
}

/*! \fn int InitSamplesFromDetection()
 *  \brief This method initialize the filter by drawing the particles from the target
 *  detector result. The parameters that can't be initialized from detection are initialized
 *  using the target prior
 */
int ParticleFilter::InitSamplesFromDetection(Sample** spl)
{
  /* Execution de la detection */
  if(this->target->detector && this->target->detector->nbdetected)
    {
      /* Teste si on utilise le mode xxdot */
      switch(target->ContDynamic->dynamicID)
	{
	case RW:
	  {
	    /* Tir selon la detection et init des vitesses selon prior */
	    for(int i=0;i<this->nbSamples;i++)
	      {
		/* tir de l'echelle si on utilise un detecteur de visage */
		if(this->target->detector->detectorID==0) {
		  /* selection de la gaussienne */
		  int selected = this->target->detector->gm->Select();
		  
		  /* determiner l'echelle en fonction de la detection et tir */
		  double tab[4];
		  tab[this->target->posXk] = this->target->priormean[this->target->posXk];
		  tab[this->target->posYk] = this->target->priormean[this->target->posYk];
		  if(this->target->useTk) 
		    tab[this->target->posTk] = this->target->priormean[this->target->posTk];
		/************************************************************************************
							A REVOIR
		************************************************************************************/
		  tab[this->target->posSk] = 0.03*this->target->detector->detectedROI[selected].width+0.46;

		  this->target->prior->Draw(tab,spl[i]);

		  /* tir du vecteur selon la gaussienne selectionnee */
		  this->target->detector->gm->glist[selected]->Draw(spl[i]->ContinusP);

		  logout << "InitSamplesFromDetection : [" << spl[i]->ContinusP[0] << "," << spl[i]->ContinusP[1] << ","<< spl[i]->ContinusP[2];
		  if(this->target->useTk) 
		    logout << ","<< spl[i]->ContinusP[3] << "]\n";
		  else
		    logout << "]\n";

		  
		  //printf("Init echelle : %f\n",tab[this->target->posSk]);
		}
		else {
		  //printf("Init normal\n");
		  /* tir de tous les parametres selon le prior */
		  this->target->prior->Draw(spl[i]);
		  
		  
		  /* tir selon la detection pour initialiser les parametres de position detectes */
		  this->target->detector->gm->Draw(spl[i]->ContinusP);
		  }
		
		/* Init equiprobable des etats discrets */
		InitDiscFromUniform(spl[i]);
	      }
	  }
	  break;
	}
    }
  else
    {
      /* tir a partir du prior uniquement */
      // for(int i=0;i<this->nbSamples;i++)
      // 	{
      // 	  /* tir uniquement selon prior */
      // 	  this->target->prior->Draw(spl[i]);
      // 	}
      return 0;
    }

  /* Init du modele de la cible */
  //cout << "Init du modele de la cible" << endl;
  this->target->InitModelFromDetection();

  /* Init des vecteurs de poids */
  this->curw->InitVector();
  this->prevw->InitVector();
  
  return 1;
}

/*! \fn int InitOneSampleFromDetection(Sample** spl, int i)
 *  \brief This method initialize one sample from the target detector result. The parameters 
 *  that can't be initialized from detection are initialized using the target prior.
 *  Detection must have been achieved before
 */
int ParticleFilter::InitOneSampleFromDetection(Sample** spl, int i)
{
  /* Teste si on utilise le mode xxdot */
  switch(target->ContDynamic->dynamicID)
    {
    case RW:
      {
	/* tir de tous les parametres selon le prior */
	this->target->prior->Draw(spl[i]);
	
	/* tir selon la detection pour initialiser les parametres detectes */
	this->target->detector->gm->Draw(spl[i]->ContinusP);
	
	/* Init equiprobable des etats discrets */
	InitDiscFromUniform(spl[i]);
      }
      break;
    }
 
  return 1;
}

/*! \fn void  InitDiscFromUniform(Sample<TYCP,TYDP> *Splin)
 *  \brief This method initialize the discret parameters of the particle
 *  with a uniform random generation of the state
 */
void ParticleFilter::InitDiscFromUniform(Sample* spl)
{
  /* Tirage equiprobable des parametres discrets */
  for(int j = 0; j < this->nDP; j++ )
    spl->DiscretsP[j] = cvRandInt(&rng_state) % target->DiscDynamic[j]->nbStates;
    
}


/*! \fn void ParticleFilter::DispSamples()
 *  \brief Print values of all the particles on the screen 
 */

void ParticleFilter::DispSamples()
{
  cout << "DispSamples" << endl;
  for(int i=0;i<nbSamples;i++)
    sps[i]->Disp();
}

/*! \fn void ParticleFilter::DispOneSample()
 *  \brief Print values of one particle on the screen
 *  \param num the particle to be printed 
 */

void ParticleFilter::DispOneSample(int num)
{
  cout << "DispSamples " << num << endl;
}
  

void ParticleFilter::SaveSamplesImg(IplImage* imgIn, Sample** ss, WeightsVector* weights)
{
  /* On traite les poids pour avoir les indice de lut (recherche du max) */
  double maxpds=0;
  for(int i=0;i<this->nbSamples;i++)
    if(weights->w[i]>maxpds) maxpds = weights->w[i];

  /* Construction du vecteur d'indices de couleur de la lut */
  int* idvect=new int[this->nbSamples];
  for(int i=0;i<this->nbSamples;i++)
    idvect[i]= (int)((weights->w[i]/maxpds)*255.0)/4;

  /* Recopie image observation */
  cvCopy(imgIn,this->imgParticles);

  /* Trace par ordre croissant */
  CvScalar coul;
  for(int id=0;id<64;id++)
    {
      for(int i=0 ; i < this->nbSamples ; i++ )
	{
	  if(idvect[i]==id)
	    {
	      //getSummerRGB(id,coul);
	      getJetRGB(id,coul);
	      this->target->Trace(ss[i],imgParticles,coul,1);
	      if(this->TargetFirstPass) this->target_first_pass->Trace(ss[i],imgParticles,coul,1);
	    }
	}
    }

  delete [] idvect;
  //this->TraceAllSamples(this->imgParticles,ss,CV_RGB(0,0,255));
}

/*! \fn void TraceAllSamples(IplImage* img);
 *  \brief draw the sample set on the image given in parameter
 *  \param img is the image on witch the samples must be drawed
 */

void ParticleFilter::TraceAllSamples(IplImage* img, CvScalar coul)
{
    for(int i=0 ; i < this->nbSamples ; i++ )
      this->target->Trace(this->sps[i],img,coul);
}

/*! \fn void TraceAllSamples(IplImage* img, Sample** s);
 *  \brief draw the sample set in parametr on the image given in parameter
 *  \param img is the image on witch the samples must be drawed
 */

void ParticleFilter::TraceAllSamples(IplImage* img, Sample** ss, CvScalar coul)
{
  for(int i=0 ; i < this->nbSamples ; i++ )
      this->target->Trace(ss[i],img,coul);
}

/* Trace des particules dont les indices sont dans le vecteur ivector */

void ParticleFilter::TraceAllSamples(IplImage* img, Sample** ss, int* ivector, CvScalar coul)
{
  for(int i=0 ; i < this->nbSamples ; i++ )
      this->target->Trace(ss[ivector[i]],img,coul);
}


/*########## Methods from old SampleSet class ########*/

/*! \fn void SwitchW()
 *  \brief Method that make a switch between curw and prevw
 */

void ParticleFilter:: SwitchW()
{
  WeightsVector* tmp;
  tmp = curw;
  curw = prevw;
  prevw = tmp;
}

/*! \fn void SampleSet::SwitchSpl()
 *  \brief Method that make a switch between sps and spsnew
 */

void ParticleFilter:: SwitchSpl()
{
  Sample** tmp;
  tmp = sps;
  sps = spsnew;
  spsnew = tmp;
}

/*########## END Methods from old SampleSet class ########*/




/*########### Resampling methods from old class DrawSample ###########*/


/*! \fn   void Resample(Sample** Splin, Sample** Splout, WeightsVector* wv, int nbS, int nbR);
 *  \brief this method resample a sample vector to an other sample vector
 *  using the weights in wv
 *  \param Splin is the sample vector to resample
 *  \param Splout is the resampled vector
 *  \param wv is the weights vector
 *  \param nbS is the number of samples in Splin vector
 *  \param nbR is the number of sample to resample from the weights vector
 */

void ParticleFilter:: Resample(Sample** Splin, Sample** Splout, WeightsVector* wv, int nbS, int nbR)
{
  int i, j;
  double Uj=0,val;
  double step = 1.0/nbR;   
  int nbSMoinsUn = nbS-1;
  double* wcumul = wv->getCumulativeWptr();

  // cout << endl;
//   wv->Disp();

  //Selection d'un point de départ aléatoire dans l'intervalle [0 step]
  val = step*cvRandReal(&this->rng_state);
  
  //Retirage
  i=0;
  for(j=0, Uj=val ; j < nbR ; j++, Uj+=step )
    {
      //Pour chaque nouvelle particule
      while( ( i < nbSMoinsUn ) && (Uj > wcumul[i])  ) i++;
      
      /* Recopie de la particule selectionnee */
      //cout << j << " <--- " << i << endl;
      //*(Splout[j]) = *(Splin[i]);
      Splout[j]->SplCpy(Splin[i]);
    }
}

   
/*! \fn void ResampleIndex(WEIGHTS *curw, int *idvector, int nbS, int nbR)
 *  \brief this function resample nb indexes from the weights in curw
 *  \param curw is the structure containing the weights vector
 *  \param idvector is the resulting vector containing the resampled indexes
 *  \param nbS is the number of oriiginal samples 
 *  \param nbR is the number of indexes that must be sampled
 */

void ParticleFilter:: ResampleIndex(WeightsVector* curw, int* idvector, int nbS, int nbR)
{
  int i, j;
  double Uj=0,val;
  double step = 1.0/nbR;   
  int nbSMoinsUn = nbS-1;
  double* wcumul = curw->getCumulativeWptr();

  //Selection d'un point de départ aléatoire dans l'intervalle [0 step]
  val = step*cvRandReal(&this->rng_state);;
  
  //Retirage
  i=0;
  for(j=0, Uj=val ; j < nbR ; j++, Uj+=step )
    {
      //Pour chaque nouvelle particule
      while( ( i < nbSMoinsUn ) && (Uj > wcumul[i])  ) i++;

      /* Recopie de l'indice dans le vecteur de sortie */
      idvector[j] = i;
    }
}

/*! \fn void DispState()
 *  \brief this function display the filter state (the target is displayed with pfState as parameter)
 */ 

void ParticleFilter::DispState()
{

  /* Trace du resultat pour visu */
  target->Trace(this->pfState);
  if(this->TargetFirstPass)  this->target_first_pass->Trace(this->pfState);
}

/*! \fn void DispState(void* param)
 *  \brief this function display the filter state (the target is displayed with pfState as parameter)
 */ 

void ParticleFilter::DispState(IplImage* img)
{ 
  /* Trace du resultat pour visu */
  target->Trace(pfState,img);
  if(this->TargetFirstPass)  this->target_first_pass->Trace(this->pfState,img);

  /** Trace des positions des particules **/
  for(int i=0;i<nbSamples;i++) {
    cvCircle(img,cvPoint(this->sps[i]->ContinusP[0],this->sps[i]->ContinusP[1]),2,CV_RGB(255,255,0),-1,8,0);
  }
}


void ParticleFilter::DispState(IplImage* img,CvScalar couleur)
{ 
  /* Trace du resultat pour visu */
  target->Trace(pfState,img,couleur); 
  if(this->TargetFirstPass)  this->target_first_pass->Trace(this->pfState,img,couleur);
}

/* Attention le fichier doit etre ouvert avant */

void ParticleFilter::LogOneSample(Sample* spl, double pds, int num, string stepnm)
{
  this->ficLog.open(this->logFilename.c_str(),ios::out|ios::app);
  this->ficLog << this->nmfiltre << "{" << this->numStep << "}." << stepnm << "(" << num << ",:) = [";

  if(this->target->posTk!=-1)
    this->ficLog << spl->ContinusP[0] << " " << spl->ContinusP[1] << " " << spl->ContinusP[2] << " " << spl->ContinusP[3] << " " << pds << ";";
  else
    this->ficLog << spl->ContinusP[0] << " " << spl->ContinusP[1] << " NaN " << spl->ContinusP[2] << " " << pds << ";";

  this->ficLog << "];";
  this->ficLog.close();
}


/* Log des particules avec un indice d'etape pos (= etape du pas de filtrage) nbs defini le nombre de particules */
void ParticleFilter::LogSamples(Sample** ss, WeightsVector* wv, int nbs, string stepnm) 
{ 
  this->ficLog.open(this->logFilename.c_str(),ios::out|ios::app);
  this->ficLog << this->nmfiltre << "{" << this->numStep << "}."<< stepnm << "(:,:) = [";
  for(int i=0;i<nbs;i++) 
    {
      //this->LogOneSample(ss[i],wv->w[i],i+1,pos); 
      if(this->target->posTk!=-1)
	this->ficLog << ss[i]->ContinusP[0] << " " << ss[i]->ContinusP[1] << " " << ss[i]->ContinusP[2] << " " << ss[i]->ContinusP[3] << " " << wv->w[i] << ";";
      else
	this->ficLog << ss[i]->ContinusP[0] << " " << ss[i]->ContinusP[1] << " NaN " << ss[i]->ContinusP[2] << " " << wv->w[i] << ";";     
    }
  this->ficLog << "];";
  this->ficLog.close();
}
