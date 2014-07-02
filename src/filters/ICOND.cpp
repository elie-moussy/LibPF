#include "ICOND.h"

/**
   Consruction du filtre APF
*/
ICONDFilter::ICONDFilter(string nomfic, ImgProducer* imgproducer, 
		     twoDDetector* detector, Prior* prior, 
		     ContDynamicModel* ContDynamic, DiscDynamicModel** DiscDynamic):ParticleFilter(nomfic,imgproducer,
												   detector,prior,
												   ContDynamic,DiscDynamic)
{
  cout << "--> ICONDFilter (Constructor) \t:\t ICondensation construction ....." << endl;
  
  string tmp;
  string filename;
  ifstream fichier;

  cout << "************************************" << endl;
  cout << "*    LOADING NEW ICOND FILTER    *" << endl;
  cout << "************************************" << endl;

  /* On colle le nom du filtre */
  this->nmfiltre = "ICOND";

  /* Generateur aleatoire en fonction de l'horloge */
  rng_state = cvRNG((uint64)RDTSC());
  
  /* creation du vecteur de selection de methode de sampling */
  alfa = cvCreateMat(nbSamples, 1, CV_32FC1);

  /* Init des seuils de selection du mode de sampling */
  this->q = 0.15;                        //Proportion de sampling selon prior
  this->r = 0.25;                        //Proportion de sampling hybride
  this->qr = this->q+this->r;            // 1-qr --> Proportion de sampling condensation

  /* Vecteur d'indices pour le resampling */
  this->idvector = new int[this->nbSamples];

  /* Alloc de la gaussienne contenant les bruits de dynamique sur la position (deux premiers parametres) */
  double* dynNoiseMatr = this->target->ContDynamic->getDynamicNoiseMatr();
  double* posdynNoiseMatr;
  if(target->useTk) posdynNoiseMatr = getSubMatr<double>(dynNoiseMatr,4,2,0,1);
  else posdynNoiseMatr = getSubMatr<double>(dynNoiseMatr,3,2,0,1);

  this->posNoise = GaussianAlloc(2,NULL,posdynNoiseMatr);
  delete [] posdynNoiseMatr;
  
  /* Vecteur de particules temporaire */
  this->spsTmp = new Sample*[this->nbSamples];
  for(int i=0;i<this->nbSamples;i++) this->spsTmp[i] = new Sample(nCP,nDP);

  cout << "--> ICONDFilter (Constructor) \t:\t ICondensation.....OK construction done" << endl;
  cout << "*********************************" << endl;
  cout << "*      ICOND FILTER LOADED      *" << endl;
  cout << "*********************************" << endl;
}

/**
   Destructeur du filtre APF
*/
ICONDFilter::~ICONDFilter()
{
  cvReleaseMat(&this->alfa);
  delete [] this->idvector;
}

/*! \fn int step()
 *  \brief This is the main processing step. This method correspond to a step of the
 *  algorithm. Calling step for each image achieve a APF filtering. To run rigth the image measure
 *  building method must have been called before
 */
int ICONDFilter::step(IplImage* obs)
{
  int isdetected=0;
  int method;
  double lambda;
  double gt_st,ft_st;
 
  /**** Pres traitement ****/  

  /* Resampling des index */
  this->ResampleIndex(this->curw,this->idvector);
  
  /* Prediction des particules sans bruit */
  for(int i=0 ; i < this->nbSamples ; i++ )
    { 
      /* Remise des poids a 1/N */
      this->curw->normalizedw[i] = 1.0/this->nbSamples;

      /* Prediction sans bruit des particules resamplees */
      this->target->ContDynamic->ContDrawFromDynWithOutNoise(this->sps[this->idvector[i]],this->spsnew[i]); 
    }

  /**** Fin Pres traitement ****/  

  /* Commutation curw / prevw */
  this->SwitchW();

  /* Init des poids courants */
  this->curw->ResetVector();

  /* Tirage uniforme d'un vecteur de alfa pour selectionner la methode de sampling pour chaque particule */
  cvRandArr( &this->rng_state, this->alfa, CV_RAND_UNI, cvScalar(0), cvScalar(1) );

  /* Connection des images de traitement dans la cible pour limiter les appels de fonctions dans la boucle */
  this->target->LinkImages();

  /* Filtrage */  
  for(int i=0 ; i < this->nbSamples ; i++ )
    {
      /* choix de la methode de sampling en fonction de alfa si le detecteur a marche */
      method = (target->detector->nbdetected ? ( this->alfa->data.fl[i] < this->q ? 1 : ( this->alfa->data.fl[i] < this->qr ? 2 : 0  )  ) : 0 );

      //cout << "Sampling method " << method << endl;

      /*************************************************************************************************************/
      /*                                    Sampling partie continue                                               */
      /*************************************************************************************************************/
      switch(method)
	{
	  /* Sampling selon la dynamique (Condensation) */
	case 0:
	  {
	    /* Ajout du bruit a la particule selon index resample */
	    this->target->ContDynamic->ContAddDynNoise(this->spsnew[i],this->sps[i]);

	    /* lambda = 1 */
	    lambda = 1;
	  }
	  break;
	  
	  /* Sampling selon le prior */
	case 1:
	  {
	    /* Sampling from detection + prior */
	    this->InitOneSampleFromDetection(i);

	    /* lambda = 1 */
	    lambda = 1;
	  }
	  break;

	  /* Sampling selon la fonction d'importance hybride */
	case 2:
	  {
	    /* Ajout du bruit a la particule selon index resample */
	    this->target->ContDynamic->ContAddDynNoise(this->spsnew[i],this->sps[i]);

	    /* On tire la position selon le detecteur */
        //Sample spl = *(this->sps[i]);
        //double sk = spl.ContinusP[2];
	    this->target->detector->gm->Draw(this->sps[i]->ContinusP);
	    
	    /* Evaluation du tir */
	    gt_st = this->target->detector->gm->Eval(this->sps[i]->ContinusP);

	    /* Evaluation de la dynamique */
 	    ft_st = 0.0;
 	    for(int j=0;j<this->nbSamples;j++)
 	      {
 			ft_st+=prevw->getNormalizedW(idvector[j])*posNoise->Eval(sps[i]->ContinusP,spsnew[j]->ContinusP);
 	      }
	    //ft_st=this->prevw->normalizedw[this->idvector[i]]*this->posNoise->Eval(this->sps[i]->ContinusP,this->spsnew[i]->ContinusP);
	    
	    /* Calcul de lambda */
	    lambda = ft_st/gt_st;
	  }
	  break;
	}

      /*****************************************/
      /*    Calcul du poids de la particule    */
      /*****************************************/
      curw->setW(i,this->target->CalcLikelihood(this->sps[i])*lambda);
    }

  /* Normalisation des poids */
  if(!curw->NormVector()) {
      cout << "Weights NULL" << endl;
      return 0;
  }

  /* Selection de la particule moyenne */
  Select();

  /* Calcul de N efficace */
  CalcNeff();
  
  return 1;
}

