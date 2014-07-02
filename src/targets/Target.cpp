#include "Target.h"


Target::Target(string nomfic, ImgProducer* imgBank, twoDDetector* detector, Prior* prior, 
	       ContDynamicModel* ContDynamic, DiscDynamicModel** DiscDynamic)
{
  this->color = CV_RGB(0,255,0);

  /* Recopie des parametres (NULL par defaut) */
  this->imgBank = imgBank;  
  if(imgBank) this->localImgBank=false;
  else this->localImgBank=true;
  
  this->detector = detector;
  if(detector) this->localDetector=false;
  else this->localDetector=true;

  this->prior = prior;
  if(prior) this->localPrior=false;
  else this->localPrior=true;

  this->ContDynamic = ContDynamic;
  if(ContDynamic) this->localContDynamic=false;
  else this->localContDynamic=true;
  
  this->DiscDynamic = DiscDynamic;
  if(DiscDynamic) this->localDiscDynamic=false;
  else this->localDiscDynamic=true;

  /* Chargement des parametres manquant a partir du fichier */
  if(!Load(nomfic)) cout << "|ERROR|--> Target (Constructor) \t:\t Fail !\n";
  else cout << "--> Target (Constructor) \t:\t Done\n";
}


/* Destructeur */
Target::~Target()
{
  if(localImgBank) delete imgBank;
  if(localDetector) delete detector;
  if(localPrior) delete prior;
  if(localContDynamic) delete ContDynamic;
  if(localDiscDynamic) delete DiscDynamic;

  delete [] priormean;
}

/*
 * Chargement des paramatres communs aux cibles (type de dynamique, type de prior ...)
 */
int Target::Load(string file)
{
  /* ouverture */
  ifstream fichier;
  string tmp;
  string filename;
  double bmin;
  double bmax;
  int end;

  fichier.open(file.c_str(), ios::in);
  if(!fichier)
    {
      cout << "|ERROR|--> Target (Load) \t:\t Opening file [" << file << "] ERROR" << endl;
      return 0;
    }
  
  /*************** Taille des images *****************/
  
  /* <img_width> */
  FindStr2(fichier,"<img_width>");
  if(fichier.eof())
    { cout << "|ERROR|--> Target (LO) \t:\t ERROR <img_width> not found !" << endl; return 0; }
  fichier >> this->width;
  
  /* <img_height> */
  FindStr2(fichier,"<img_height>");
  if(fichier.eof())
    { cout << "|ERROR|--> Target (LO) \t:\t ERROR <img_height> not found !" << endl; return 0; }
  fichier >> this->height;


  /*************** Parametres continus ***************/
  
  /* <nCP> */
  FindStr2(fichier,"<nCP>");
  if(fichier.eof())
    { cout << "|ERROR|--> Target (Load) \t:\t ERROR <nCP> not found !" << endl; return 0; }
  fichier >> this->nCP;

  /* <ContPRanges> */
  FindStr2(fichier,"<ContPRanges>");
  if(fichier.eof())
    { cout << "|ERROR|--> Target (Load) \t:\t ERROR <ContPRanges> not found !" << endl; return 0; }
  for(int i=0;i<this->nCP;i++)
    fichier >> this->ContPRanges;

  if(nCP==3)
    {
      useTk=false;
      posTk = -1;
      
      posXk = 0;
      posYk = 1;
      posSk = 2;      
    }
  else
    {
      useTk=true;
      posXk = 0;
      posYk = 1;
      posTk = 2;      
      posSk = 3;      

    }

  /* Chargement de la dynamique continue si locale */
  if(!this->ContDynamic) {
    this->ContDynamic = ContDynamicAlloc(file);
    localContDynamic = true;
  }

  
  /*************** Parametres discrets ***************/
  
  /* <nDP> */
  FindStr2(fichier,"<nDP>");
  if(fichier.eof())
    { cout << "|ERROR|--> Target (Load) \t:\t ERROR <nDP> not found !" << endl; return 0; }
  fichier >> this->nDP;

  /* Si il y a des parametres discrets on charge les bornes et la dynamique */
  if(this->nDP)
    {
      cout << "--> Target (Load) \t:\t Discret Dynamic Model(s) ........ " << endl;

      /* <DiscPRanges> */ 
      FindStr2(fichier,"<DiscPRanges>");
      if(fichier.eof())
	{ cout << "|ERROR|--> Target (Load) \t:\t ERROR <DiscPRanges> not found !" << endl; return 0; }
      for(int i=0;i<this->nDP;i++)
	fichier >> this->DiscPRanges;
    
      /* Chargement de la dynamique discrete (matrices de transition) */
      if(!this->DiscDynamic)
	{
	  for(int i=0;i<this->nDP;i++)
	    this->DiscDynamic[i] = DiscDynamicAlloc(file);
	  localDiscDynamic = true;	
	}
    }
  else  cout << "--> Target (Load) \t:\t no discret parameters" << endl;

  /*************** Parametres ImgProducer ****************/

  /* Chargement du producteur d'images si il n'a pas ete passe en parametre */
  if(!this->imgBank)
    {
      cout << "--> Target (Load) \t:\t local ImgProducer allocation\n";
      this->imgBank = new ImgProducer(img_width,img_height);
      localImgBank = true;
    }
  else  cout << "--> Target (Load) \t:\t using external ImgProducer\n";


  /*************** Parametres prior ****************/

  /* Chargement du prior si il n'a pas ete passe en parametre */
  if(!this->prior)    
    {
      cout << "--> Target (Load) \t:\t local Prior allocation\n";
      this->prior = PriorAlloc(file);
      localPrior = true;
    }
  else  cout << "--> Target (Load) \t:\t using external Prior\n";

  this->priormean = new double[nCP];
  this->prior->getMean(this->priormean);

  /*************** Parametres detector ****************/

  /* Chargement du detecteur si il n'a pas ete passe en parametre */
  if(!this->detector)    
    {
      cout << "--> Target (Load) \t:\t local twoDDetector allocation\n";
      this->detector = twoDDetectorAlloc(file,this->imgBank);
      localDetector = true;
    }
  else  cout << "--> Target (Load) \t:\t using external twoDDetectorAlloc\n";

  fichier.close();

  cout << "--> Target (Load) \t:\t default sigma initialization \n";

  /* Alloc et init des sigmas */
  this->nbSigma = 5;
  this->sigma = new double[this->nbSigma];
  this->deux_sigma_carre = new double[this->nbSigma];
  this->moins_un_sur_deux_sigma_carre = new double[this->nbSigma];

  /* Sigma pour mesure forme */
  this->setSigma(0,28.0);

  /* Sigma pour mesure distribution de couleur */
  this->setSigma(1,0.03);

  /* Sigma pour mesure distribution de mouvement */
  this->setSigma(2,0.2);

  /* Sigma pour mesure forme a partir d'image de distance */
  this->setSigma(3,25.0);

  /* Sigma pour forme avec flot optique */
  this->setSigma(4,36.0);

  /****/

  /* Alloc et init des penalites */
  this->nbPenality = 2;
  this->ofpenality = new double[this->nbPenality];

  /* Penalite pour le masquage dans le calcul de vraisemblance avec par exemple forme plus flot optique */
  this->setPenality(0,12);

  /* Penalite pour le masquage dans le calcul de vraisemblance avec par exemple forme image de distances plus segmentation regions */
  this->setPenality(1,8);

  /* Alloc du vecteur de moyenne a priori */
  this->priormean = new double[this->nCP];

  /* Recuperation du vecteur moyen */
  if(this->prior) this->prior->getMean(this->priormean);

  return 1;
}

/* Fonction d'init des sigma pour les mesures des filtres */

void Target::InitSigma(double* sig)
{
  for(int i=0;i<this->nbSigma;i++)
    this->setSigma(i,sig[i]);
}

/* Fonction d'init des penalites pour les mesures des filtres */

void Target::InitPenality(double* pen)
{
  for(int i=0;i<this->nbPenality;i++)
    this->setPenality(i,pen[i]);
}

/*
 * Affichage des parametres charges de la cible 
 */

  void Target::DispCommon()
{
  //cout << "Target Parameters" << endl;

  if(nCP) cout << "Number of continuous parameters : " << nCP << endl;
  if(nDP) cout << "Number of discrets parameters : " << nDP << endl;
  if(this->ContDynamic)
    {
      cout << "----------------------" << endl;
      cout << "| Continuous Dynamic |" << endl;
      cout << "----------------------" << endl;
      this->ContDynamic->Disp();
      cout << endl;
    }

  if(this->nDP && this->DiscDynamic)
    {
      for(int i=0;i<this->nDP;i++)
	{
	  cout << "----------------------" << endl;
	  cout << "| Discrets Dynamic " << i << " |" << endl;
	  cout << "----------------------" << endl;
	  this->DiscDynamic[i]->Disp();
	  cout << endl;
	}
    }
  if(this->prior)
    {
      cout << "---------" << endl;
      cout << "| Prior |" << endl;
      cout << "---------" << endl;
      this->prior->Disp();
      cout << endl;
    }
}
