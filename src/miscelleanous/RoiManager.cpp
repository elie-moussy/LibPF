#include "LibPF/miscelleanous/RoiManager.h"

RoiManager::RoiManager(int w, int h, string nomfic)
{
  /* recup dimensions des images */
  imgW = w;
  imgH = h;

  cout << "--> RoiManager (CO) \t:\t RoiManager allocation .... " << endl;
  if(!Load(nomfic)) cout << "|ERROR|--> RoiManager (CO) \t:\t error loading parameters from file" << endl;

  /* Au depart on le place en haut a droite */
  this->center.x=this->demiw;
  this->center.y=this->demih;
  this->scale = 1;

  this->transform();

  cout << "--> RoiManager (CO) \t:\t RoiManager allocation .... OK" << endl;
}
int RoiManager::Load(string file)
{
  string filename;
  string tmp;
  ifstream fichier;

  /* ouverture */
  fichier.open(file.c_str(), ios::in);
  if(!fichier)
    {
      cout << "|ERROR|--> RoiManager (LO) \t:\t file not found [" << file << "]" << endl;  
      return 0;
    }   

   /* Lecture du nombre de regions */
  cout << "--> RoiManager (LO) \t:\t Loading the number of patchs.... " << endl;
  FindStr2(fichier,"<nbROI>");
  if(fichier.eof()) 
    { 
      cout << "|ERROR|--> RoiManager (LO) \t:\t Loading shape ERROR <nbROI> not found" << endl;
      return 0;
    }
  fichier >> this->nbp;

  cout << "--> RoiManager (LO) \t:\t " << this->nbp << " rectangles structure" << endl;

  /* Allocations */
  cout << "--> RoiManager (LO) \t:\t Allocations ... " << endl;
  this->offsetX = newZero<int>(this->nbp);
  this->offsetY = newZero<int>(this->nbp);
  this->wlist = newZero<int>(this->nbp);
  this->hlist = newZero<int>(this->nbp);
  this->ptULlist = new CvPoint[this->nbp];
  this->ptLRlist =  new CvPoint[this->nbp];
  this->nbpix = newZero<int>(this->nbp);

  cout << "--> RoiManager (LO) \t:\t Allocations ... OK" << endl;

  /* On charge les parametres des autres patchs (position relative et dimensions */
  char num[50];
  string pattern;
  for(int i=0;i<this->nbp;i++)
    {
      sprintf(num,"_%d>",i);
      
      pattern = "<offsetROI";
      pattern += num;
      FindStr2(fichier,pattern.c_str());
      if(fichier.eof()) 
	{ 
	  cout << "|ERROR|--> RoiManager (LO) \t:\t " << pattern << " not found" << endl; 
	  return 0;
	}
      /* Lecture des offsets */
      int x, y;
      fichier >> x;
      this->offsetX[i] = x;
      fichier >> y;
      this->offsetY[i] = y;

      cout << "--> RoiManager (LO) \t:\t Rectangle " << i << " offsets = [" << this->offsetX[i] << "," << this->offsetY[i] << "]" << endl;

      pattern = "<sizeROI";
      pattern += num;
      FindStr2(fichier,pattern.c_str());
      if(fichier.eof()) 
	{ 
	  cout << "|ERROR|--> RoiManager (LO) \t:\t " << pattern << " not found" << endl; 
	  return 0;
	}

      /* Lecture des dimensions */
      fichier >> this->wlist[i];
      fichier >> this->hlist[i];

      cout << "--> RoiManager (LO) \t:\t Rectangle " << i << " size = [" << this->wlist[i] << "," << this->hlist[i] << "]" << endl;

    }

  fichier.close(); 

  return 1;  
}
 
RoiManager::~RoiManager()
{
  delete [] this->offsetX;
  delete [] this->offsetY;
  delete [] this->wlist;
  delete [] this->hlist;
  delete [] this->ptULlist;
  delete [] this->ptLRlist;
  delete [] this->nbpix;
}

int* RoiManager::transform(int x, int y, float s)
{
  /* On place le centre */
  this->center.x = x;
  this->center.y = y;

  /* on copie l'echelle */
  this->scale = s;

  /* Calcul de la position des rectangles */
  return this->transform();
}

int* RoiManager::transform()
{
  int tmp,tmp2;
  
  /* Calcul des coins des rectangles supplementaires */
  for(int i=0,j=0;i<this->nbp;i++,j++)
    {
      this->ptULlist[i].x = (int)(this->center.x + this->scale*this->offsetX[j]);
      this->ptULlist[i].y = (int)(this->center.y + this->scale*this->offsetY[j]);

      tmp = (int)(this->scale*this->wlist[j]);
      this->ptLRlist[i].x = this->ptULlist[i].x + tmp;

      tmp2 = (int)(this->scale*this->hlist[j]);
      this->ptLRlist[i].y = this->ptULlist[i].y + tmp2;

      //this->nbpix[i]=tmp*tmp2;
       /* Clipping des points */
      ClipPoint(this->ptULlist+i,imgW,imgH);
      ClipPoint(this->ptLRlist+i,imgW,imgH);
      
      /* Init nombre de pixels de la zone principale */
      this->nbpix[i] = (int)((this->ptLRlist[i].x-this->ptULlist[i].x)*(this->ptLRlist[i].y-this->ptULlist[i].y));  
    }

  //cout << "nbpix = " << this->nbpix << endl;

  return this->nbpix;
}

