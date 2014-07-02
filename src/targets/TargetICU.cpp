#include "TargetICU.h"

TargetICU::TargetICU(string nomfic, ImgProducer* imgBank, 
			 twoDDetector* detector, Prior* prior, 
			 ContDynamicModel* ContDynamic, 
			 DiscDynamicModel** DiscDynamic):Target(nomfic,imgBank,
								detector,prior,
								ContDynamic,DiscDynamic)
{
  if(!Load(nomfic)) 
    cout << "|ERROR|--> TargetICU (Constructor) \t:\t error loading parameters from file" << endl;
  
  cout << "*************************" << endl;
  cout << "*   TARGET ICU LOADED   *" << endl;
  cout << "*************************" << endl;
  cout << endl;
}


TargetICU::~TargetICU()
{

}

/*
 * Chargement des paramatres communs aux cibles (type de dynamique, type de prior ...)
 */

int TargetICU::Load(string file)
{
  string filename;
  string tmp;
  string patchfic;
  ifstream fichier;
  string ficshape = icuConfigPath +"shapes/";
  string ficshape_config = icuConfigPath +"shapes/";

  cout << "****************************************" << endl;
  cout << "*   LOADING NEW FACE TRACKING TARGET   *" << endl;
  cout << "****************************************" << endl;
  
  /* charge les parametres communs de la cible */
  if(!Target::Load(file)) 
    {
      cout << "|ERROR|--> TargetICU (LO) \t:\t Loading error" << endl;
      return 0;
    } 

  /* ouverture */
  fichier.open(file.c_str(), ios::in);
  if(!fichier)
    {
      cout << "|ERROR|--> TargetICU (LO) \t:\t file not found [" << file << "]" << endl;  
      return 0;
    }   

  /* Charge les parametres de forme a utiliser */

  /*******************************************************************************************************/
  /* Chargement des parametres de mesure de la forme                                                     */
  /*******************************************************************************************************/

  /* Fichier contenant la forme */
  cout << "--> TargetICU (Load) \t:\t Loading shape" << endl;
  FindStr2(fichier,"<ShapeModel>");
  if(fichier.eof()) 
    { 
      cout << "--> TargetICU (Load) \t:\t No <ShapeModel> defined" << endl;
      this->spline=NULL;
      shapeCue=false;
      fichier.clear();
    }
  else
    {
      fichier >> tmp;
      ficshape+=tmp;
      ficshape_config+=tmp + "_config";
      cout << "--> TargetICU (Load) \t:\t Loading shape from [" << ficshape  << ".dat]" << endl;
      this->ShM = new ShapeModels(ficshape.c_str(),ficshape_config.c_str(),"dat");
      cout << "--> TargetICU (Load) \t:\t ShapeModel loaded" << endl;
      
      /* Fichier contenant la forme */
      FindStr2(fichier,"<UseFlowMask>");
      if(fichier.eof())
	{
	  cout << "--> TargetICU (LO) \t:\t Don't use flowmask" << endl;
	  useflowmask=false;
	  fichier.clear();
	}
      else
	{    
	  fichier >> tmp;
	  if(tmp=="yes") 
	    {
	      useflowmask=true;
	      cout << "--> TargetICU (LO) \t:\t Use flowmask" << endl;
	    }
	  else
	    {
	      useflowmask=false;
	      cout << "--> TargetICU (LO) \t:\t Don't use flowmask" << endl;
	    }
	}

      /* Pointe sur la forme */
      this->spline = this->ShM->GetShapeModels();
      shapeCue=true;
    }


  /*******************************************************************************************************/
  /* Chargement des parametres de mesure de la couleur                                                   */
  /*******************************************************************************************************/

  /* On charge la Structure des rectangles qui constituent la cible */
  cout << "--> TargetICU (LO) \t:\t Loading rect structure" << endl;
  FindStr2(fichier,"<ColorRectStructFile>");
  if(fichier.eof()) 
    { 
      cout << "--> TargetICU (LO) \t:\t Don't use color distribution cue" << endl;
      colorCue=false;
      fichier.clear();
    }
  else
    {
      /* Lecture du nom de fichier a charger */
      fichier >> tmp;
      
      /* Allocation de la cible multi rectangles */
      cout << "--> TargetICU (LO) \t:\t Using color distribution cue" << endl;
      colorCue=true;

      /* Alloc des patchs */
      FileReplace(file,tmp,patchfic);
      colorPatch = new RoiManager(width,height,patchfic);

      /* Allocation des histogrammes */
      cout << "--> TargetICU (LO) \t:\t Histograms allocations....(" << colorPatch->nbp << ")" << endl;
      coldistref = new ColorDistribution(colorPatch->nbp,width,height);
      coldist = new ColorDistribution(colorPatch->nbp,width,height);
      cout << "--> TargetICU (LO) \t:\t Histograms allocations....OK" << endl;

      cout << "--> TargetICU (LO) \t:\t Loading " << colorPatch->nbp << " coeff in the list" << endl;
      coefflist = new double[colorPatch->nbp];
      
      FindStr2(fichier,"<ColorUpdateCoeff>");
      if(fichier.eof()) 
	{
	  cout << "|ERROR|--> TargetICU (LO) \t:\t <ColorUpdateCoeff> not found" << endl;       
	  return 0;
	}
      for(int i=0;i<colorPatch->nbp;i++)
	{
	  fichier >> this->coefflist[i];
	  cout << "--> TargetICU (LO) \t:\t Histogram " << i << " coeff = " << this->coefflist[i]  << endl;
	}
      
      /* Controle la validite de la lecture */
      fichier >> tmp;
      if(tmp!="</ColorUpdateCoeff>") 
	{
	  cout << "|ERROR|--> TargetICU (LO) \t:\t not enought coeffs, must be = " << colorPatch->nbp << endl;       
	  return 0;
	}
    }

  /*******************************************************************************************************/
  /* Chargement des parametres de mesure de mouvement                                                    */
  /*******************************************************************************************************/

  /* On charge la Structure des rectangles qui constituent la cible */
  cout << "--> TargetICU (LO) \t:\t Loading motion rect structure" << endl;
  FindStr2(fichier,"<MotionRectStructFile>");
  if(fichier.eof()) 
    { 
      cout << "--> TargetICU (LO) \t:\t Don't use motion distribution cue" << endl;
      motionCue=false;
      fichier.clear();
    }
  else
    {
      /* Lecture du nom de fichier a charger */
      fichier >> tmp;
      
      /* Allocation de la cible multi rectangles */
      cout << "--> TargetICU (LO) \t:\t Using motion distribution cue" << endl;
      motionCue=true;

      /* Alloc des patchs */
      FileReplace(file,tmp,patchfic);
      motionPatch = new RoiManager(width,height,patchfic);

      /* Allocation des histogrammes */
      cout << "--> TargetICU (LO) \t:\t Histogram allocation...." << endl;
      md = new MotionDistribution(8,motionPatch->nbp,width,height);
      cout << "--> TargetICU (LO) \t:\t Histogram allocation....OK" << endl;
    }

  fichier.close(); 

  /* En fonction du type de generateur d'image de mesure on associe une methode de calcul de vraissemblance */
  
  return 1;
}

/* Fonction de trace de la cible avec en parametre la particule et un parametre si besoin (genre l'image ou on veut tracer) */

void TargetICU::Trace(Sample* spl, IplImage* img, CvScalar couleur, int ep)
{
  double sample_x = spl->ContinusP[this->posXk];
  double sample_y = spl->ContinusP[this->posYk];
  double sample_theta=0;
  if(useTk) sample_theta = spl->ContinusP[this->posTk];
  double sample_scale = spl->ContinusP[this->posSk];
 
  /* Trace de la forme */ 
  if(0 && shapeCue)
    {
      /* Projection de la spline */
      this->spline->transform((int)sample_x,(int)sample_y,sample_theta,sample_scale,0);
      
      /* Trace */
      this->spline->draw(img,couleur,0,ep);
      //this->spline->draw_rect(img,couleur,ep);
    }

  /* Trace patchs couleur */
  if(colorCue)
    {
      /* Projection des patchs */
      colorPatch->transform((int)sample_x,(int)sample_y,sample_scale);
      
      /* Affichage */
      colorPatch->draw(img,couleur,ep);
    }

  /* Trace patchs mvt */
  if(motionCue)
    {
      /* Projection des patchs */
      motionPatch->transform((int)sample_x,(int)sample_y,sample_scale);
      
      /* Affichage */
      motionPatch->draw(img,couleur,ep);
    }
}

/* Fonction de trace de la cible avec en parametre la particule et un parametre si besoin (genre l'image ou on veut tracer) */

void TargetICU::Trace(Sample* spl)
{
  IplImage* imgview = cvCreateImage(cvSize(320,240),IPL_DEPTH_8U,3);
  this->Trace(spl,imgview,this->color);
  cvNamedWindow("TRACE",0);
  cvShowImage("TRACE",imgview);
  cvWaitKey(0);
  cvDestroyWindow("TRACE");
  cvReleaseImage(&imgview);
}


/* Cette fonction connecte les images utilisees pour les traitements */
void TargetICU::LinkImages() {
  imgRGB = this->imgBank->imgRGB();
  imgEDGE = this->imgBank->imgEDGE();
  if(useflowmask)
    imgFLOWMASK = this->imgBank->imgFLOWMASK();
}

  
/* Fonction pour le calcul de la vraissemblance */
double TargetICU::CalcLikelihood(Sample* spl)
{
  int* nbPix;

  /* Controle de la validite des parametres */
  double sample_x = spl->ContinusP[this->posXk];
  double sample_y = spl->ContinusP[this->posYk];
  double sample_theta =0;
  if(useTk) sample_theta = spl->ContinusP[this->posTk];
  double sample_scale = 0.0;
  sample_scale = spl->ContinusP[this->posSk];
  if(!sample_scale)
      sample_scale = 1.1;

  if(!ContPRanges.check(this->posXk,sample_x) || !ContPRanges.check(this->posYk,sample_y) ||
     (useTk && !ContPRanges.check(this->posTk,sample_theta)) || !ContPRanges.check(this->posSk,sample_scale)) return 0;

  double p=1;

  /*************************/
  /* Mesure attribut forme */
  /*************************/
  if(shapeCue)
    {
      /* Projection de la spline */
      this->spline->transform((int)sample_x,(int)sample_y,sample_theta,sample_scale,0);

      /* Calcule une vraissemblance a partir des parametres de spl */
      if(useflowmask) {
	p*=CalcEdgeMesuredst(spline,imgEDGE,imgFLOWMASK,ofpenality[0],
			     moins_un_sur_deux_sigma_carre[4],(float)(sample_scale));      
      }
      else {
	p*=CalcEdgeMesuredst(spline,imgEDGE,this->moins_un_sur_deux_sigma_carre[0],(float)(sample_scale));
      }
    }
  
  /***************************/
  /* Mesure attribut couleur */
  /***************************/
  if(colorCue)
    {
      /* On utilise les parametres de la particule pour calculer l'histo moyen et mettre a jour l'histo de ref */
      nbPix = colorPatch->transform((int)sample_x,(int)sample_y,sample_scale);
 
      /* Calcul de l'histo moyen */
      coldist->CalcFromRectList(imgRGB,colorPatch->getUL(),colorPatch->getLR(),nbPix);
 
      /* Calcul de la distance entre les differents histogrammes */
      p*=coldist->BhattaDistance(coldistref,deux_sigma_carre[1]);
    }

  /*****************************/
  /* Mesure attribut mouvement */
  /*****************************/
  if(motionCue)
    {  
      /* On utilise les parametres de la particule pour calculer l'histo moyen et mettre a jour l'histo de ref */
      nbPix = motionPatch->transform((int)sample_x,(int)sample_y,sample_scale);
 
      /* Calcul de l'histo moyen */
      md->CalcFromRectList(imgRGB,motionPatch->getUL(),motionPatch->getLR(),nbPix);
 
      /* Calcul de la distance entre les differents histogrammes */
      p*=md->BhattaDistance(deux_sigma_carre[2]);
    }

  return p;
}

/* Fonction pour le calcul de la vraissemblance */

void TargetICU::Disp()
{
  Target::DispCommon();

  /* Display info forme */
  if(shapeCue)
    {
      //cout << "Loaded Shape " << endl;
      this->ShM->ViewModel();
      cvWaitKey(0);
    }

  if(colorCue)
    {
      IplImage* tmp = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,3);
      colorPatch->transform(this->width/2,this->height/2,1);
      colorPatch->draw(tmp);
      Show("Patch",tmp,0,0,0);
      cvReleaseImage(&tmp);
      
      coldistref->Disp("Histo Ref",0);
      coldist->Disp("Histo",1);
    }

  if(motionCue)
    {
      IplImage* tmp = cvCreateImage(cvSize(this->width,this->height),IPL_DEPTH_8U,3);
      motionPatch->transform(this->width/2,this->height/2,1);
      motionPatch->draw(tmp);
      Show("Motion Patch",tmp,0,0,0);
      cvReleaseImage(&tmp);
      
      md->Disp("Histo",0);
    }
}

/* Fonction d'update du modele de la cible (utile pour les cibles qui ont un modele a mettre a jour comme
   par exemple un modele de couleur */

void TargetICU::UpdateModel(Sample* spl)
{

  if(colorCue)
    {

      int* nbPix;
          
      /* On utilise les parametres de la particule pour calculer l'histo moyen et mettre a jour l'histo de ref */
      nbPix = colorPatch->transform((int)(spl->ContinusP[this->posXk]),
				    (int)(spl->ContinusP[this->posYk]),spl->ContinusP[this->posSk]);  

      /* Calcul de l'histo moyen */
      coldist->CalcFromRectList(imgBank->imgRGB(),colorPatch->getUL(),colorPatch->getLR(),nbPix);

      /* Mise a jour de l'histo de reference */
      coldistref->Update(coldist,coefflist);

      /* Le modele de couleur etant mis a jour on re initialise les masques de l'histo pour passer a l'image suivante */
      coldist->InitMask();
    }

  if(motionCue)
    {
      md->InitMask();
    }
}

void TargetICU::InitModelFromPrior(double* mean)
{
  if(colorCue)
    {
      int* nbPix;
      
      /* On utilise les parametres de la particule pour calculer l'histo moyen et mettre a jour l'histo de ref */
      nbPix = colorPatch->transform((int)(mean[this->posXk]),(int)(mean[this->posYk]),mean[this->posSk]);  
      
      cout << "Init from " << mean[posXk] << " " << mean[posYk] << " " << mean[posSk] << " " << endl;

      /* Calcul des histos */  
      coldistref->InitMask();
      coldistref->CalcFromRectList(imgBank->imgRGB(),colorPatch->getUL(),colorPatch->getLR(),nbPix);
    }

  if(motionCue)
    {
      md->InitMask();
    }
}


void TargetICU::InitModelFromDetection()
{
  if(colorCue)
    {
      int* nbPix;
      
      /* On initialise les histogrammes en tenant compte du resultat du detecteur */
      
      /* On recupere la position issue du detecteur */
      //detector->gm->Draw(posDetected);
      
      // /* On utilise les parametres de la particule pour calculer l'histo moyen et mettre a jour l'histo de ref */
      //       nbPix = colorPatch->transform((int)(this->posDetected[this->posXk]),
      // 				    (int)(this->posDetected[this->posYk]),this->priormean[this->posSk]);  
    
      int gaussnum = detector->gm->Select();
      
      /* On utilise les parametres de la particule pour calculer l'histo moyen et mettre a jour l'histo de ref */
      nbPix = colorPatch->transform((int)(detector->gm->glist[gaussnum]->mean[0]),
				    (int)(detector->gm->glist[gaussnum]->mean[1]),this->priormean[this->posSk]);  
      
      logout << "InitModelFromDetection : [" << detector->gm->glist[gaussnum]->mean[0] << "," << detector->gm->glist[gaussnum]->mean[1] << "]\n";

      /* On initialise */
      coldistref->InitMask();
      coldistref->CalcFromRectList(imgBank->imgRGB(),colorPatch->getUL(),colorPatch->getLR(),nbPix);
    }

  if(motionCue)
    {
      md->InitMask();
    }
}

