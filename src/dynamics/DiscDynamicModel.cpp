#include "dynamics/DiscDynamicModel.h"

/*! \fn   DiscDynamicModel(string nomfic, int PPos=0)
 *  \brief this is the class constructor
 *  \param nomfic is the filename of the Discret dynamic model to be loaded (in)
 *  \param PPos is the position of the considered parameter in the sample vector
 */
DiscDynamicModel::DiscDynamicModel(string nomfic, int PPos)
{
  this->discdynID = -1;
  this->ParamPos = PPos;

  cout << "--> DiscDynamicModel (Constructor) \t:\t Loading new Discret dynamic model for discret parameter at " << this->ParamPos << endl; 
  
  if(!Load(nomfic))
    cout << "|ERROR|--> DiscDynamicModel (Constructor) \t:\t ERROR Loading new Discret dynamic" << endl;
 
  /* Generateur aleatoire en fonction de l'horloge */
  //rng_state = cvRNG(0xffffffff);
  rng_state = cvRNG((uint64)RDTSC());
}

/*! \fn   ~DiscDynamicModel()
 *  \brief this is the class destructor
 */

DiscDynamicModel::~DiscDynamicModel()
{
  cout << "Destruction de DiscDynamicModel" << endl;
}

/*! \fn int Load(string fichier);
 *  \brief this is a function that load a dynamic model from file
 *  \param nomfic is the model filename (in)
 */

int DiscDynamicModel::Load(string nomfic)
{
  ifstream fichier;
  fichier.open(nomfic.c_str(), ios::in);
  if(!fichier)
    {
      cout << "|ERROR|--> DiscDynamicModel (Load) \t:\t FILE ERROR" << endl;
      return 0;
    }

  FindStr(fichier,"<nbStates>");
  if(fichier.eof()) {cout << "|ERROR|--> DiscDynamicModel (Load) \t:\t <nbStates> not found" << endl; return 0;}
  else
    {
      fichier >> this->nbStates;
      cout << "--> DiscDynamicModel (Load) \t:\t " << this->nbStates << " possible states" << endl;

      /* allocation de la matrice de transition */
      this->TransMatr = new double[this->nbStates*this->nbStates];

      /* allocation des vecteurs de transition cumules */
      this->CumulativeTransVect = new double*[this->nbStates];
      this->TransVect = new double*[this->nbStates];    
      for(int i=0;i<this->nbStates;i++)
	{
	  this->CumulativeTransVect[i] = new double[this->nbStates];   	
	  this->TransVect[i] = new double[this->nbStates];   	
	}

      cout << "--> DiscDynamicModel (Load) \t:\t Loading transition matrix" << endl;
      FindStr(fichier,"<TransMatr>");
      if(fichier.eof()) {cout << "|ERROR|--> DiscDynamicModel (Load) \t:\t <TransMatr> not found" << endl; return 0;}
      else
	{
	  //lecture matrice de transition
	  for(int i=0;i<this->nbStates;i++)
	    {
	      fichier >> this->TransMatr[i*this->nbStates];
	      this->TransVect[i][0] = this->TransMatr[i*this->nbStates+0];
	      this->CumulativeTransVect[i][0] = this->TransMatr[i*this->nbStates];
	      for(int j=1;j<this->nbStates;j++)
		{
		  fichier >> this->TransMatr[i*this->nbStates+j];
		  this->CumulativeTransVect[i][j] =  this->CumulativeTransVect[i][j-1]+this->TransMatr[i*this->nbStates+j];
		  this->TransVect[i][j] = this->TransMatr[i*this->nbStates+j];
		}
	    }
	  cout << "--> DiscDynamicModel (Load) \t:\t Loading transition matrix OK" << endl;	    
	}
    }
}

/*! \fn int PredictState(int stin)
 *  \brief this function return the predicted state from state stin according
 *  to the transition matrix
 *  \param stin is the current state of the discret parameter that must be predicted
 *  \return the function return a predicted state according to the transition matrix
 */
int DiscDynamicModel::PredictState(int curstate)
{
  int i;
  double Uj=0;
  int nbSMoinsUn = this->nbStates-1;

  //Selection d'un point de depart aleatoire dans l'intervalle [0 step]
  Uj = cvRandReal(&this->rng_state);
  
  //Retirage
  i=0;
  
  //Selection du nouvel etat
  while( ( i < nbSMoinsUn ) && (Uj > this->CumulativeTransVect[curstate][i])  ) i++;

  /* Retourne le nouvel etat */
  return i;
}

/*! \fn void Disp()
 *  \brief this method display the loaded parameters
 */
void DiscDynamicModel::Disp()
{
  cout << "--------------------------------" << endl;
  cout << "|     Discret Dynamic Model    |" << endl;
  cout << "--------------------------------" << endl;

  cout << "--> DiscDynamicModel (Disp) \t:\t " << this->nbStates << " states" << endl;
  cout << "Transition matrix" << endl;
  DispMatrix<double>(this->TransMatr,this->nbStates,this->nbStates);

  cout << "Transition vector" << endl;
  for(int i=0;i<this->nbStates;i++)
    {
      DispVector<double>(this->TransVect[i],this->nbStates);
    }
  
  cout << "Cumulative Transition Vector" << endl;
  for(int i=0;i<this->nbStates;i++)
    DispVector<double>(this->CumulativeTransVect[i],this->nbStates);
}
