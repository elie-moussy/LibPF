#include "LibPF/colsegmentation/ColorBase.h"
//#include "libPF/miscelleanous/applipath.h"
//#include "libPF/miscelleanous/def_paramslist.h"


ColorBase::ColorBase()
{
  this->num = -1;
  this->nm = "Not Loaded";
  this->pLum = 0;

  this->rC1[0] = 0;
  this->rC1[1] = 0;

  this->rC2[0] = 0;
  this->rC2[1] = 0;

  this->rC3[0] = 0;
  this->rC3[1] = 0;
}
 
ColorBase::ColorBase(string nomfic)
{
  /* charge la base de couleur contenue dans nomfic */
  if(this->load(nomfic)) cout << "--> ColorBase (Constructor) \t:\t color base " << this->nm << " loaded" << endl;
  else
    {
      cout << "|ERROR|--> ColorBase (Constructor) \t:\t error loading color base ! " << endl;
       this->num = -1;
       this->nm = "Not Loaded";
       this->pLum = 0;
       
       this->rC1[0] = 0;
       this->rC1[1] = 0;
       
       this->rC2[0] = 0;
       this->rC2[1] = 0;
       
       this->rC3[0] = 0;
       this->rC3[1] = 0;
    }

}


ColorBase::~ColorBase()
{

}

int ColorBase::load(string nomfic)
{
  string tmp;
  string filename;
  ifstream fichier;

  /* ouverture du fichier */
  filename = icuConfigPath + "/color/bases/" + nomfic;
  fichier.open(filename.c_str(), ios::in);
  if(!fichier) { cout << "|ERROR|--> ColorBase (Load) \t:\t Error loading " << nomfic << endl; return 0; }

  /* recherche nom de la base */
  FindStr(fichier,"<nm>");
  if(fichier.eof()) { cout << "|ERROR|--> ColorBase (Load) \t:\t Error <nm> not found !" << endl; return 0; }
  else fichier >> this->nm;

  /* recup du numero de la base */
  this->num = ColorBaseType.getIndex(this->nm);

  /* recherche numero du plan contenant la luminance */
  FindStr(fichier,"<pLum>");
  if(fichier.eof()) { cout << "|ERROR|--> ColorBase (Load) \t:\t Error <pLum> not found !" << endl; return 0; }
  else fichier >> this->pLum;
  
  /* recherche nom de la bornes des plans */
  FindStr(fichier,"<ranges>");
  if(fichier.eof()) { cout << "|ERROR|--> ColorBase (Load) \t:\t Error <ranges> not found !" << endl; return 0; }
  else
    {
      fichier >> this->rC1[0];
      fichier >> this->rC1[1];
      fichier >> this->rC2[0];
      fichier >> this->rC2[1];
      fichier >> this->rC3[0];
      fichier >> this->rC3[1];
    }

  this->dimC1 = this->rC1[1]+1;
  this->dimC2 = this->rC2[1]+1;
  this->dimC3 = this->rC3[1]+1;
  
  /* Connexion des plans luminance et chromatiques */
  switch(this->pLum)
    {
    case 1:
      {
	this->LumR = this->rC1;
	this->ChromR1 = this->rC2;
	this->ChromR2 = this->rC3; 

	this->dimLum = &this->dimC1;
	this->dimChrom1 = &this->dimC2;
	this->dimChrom2 = &this->dimC3;
      }
      break;
    case 2:
      {
	this->LumR = this->rC2;
	this->ChromR1 = this->rC1;
	this->ChromR2 = this->rC3;

	this->dimLum = &this->dimC2;
	this->dimChrom1 = &this->dimC1;
	this->dimChrom2 = &this->dimC3;
      }
      break;
    case 3:
      {
	this->LumR = this->rC3;
	this->ChromR1 = this->rC1;
	this->ChromR2 = this->rC2;

	this->dimLum = &this->dimC3;
	this->dimChrom1 = &this->dimC1;
	this->dimChrom2 = &this->dimC2;
      }
      break;

    }


  return 1;
}
 
void ColorBase::disp()
{
  cout << "ColorBase " << this->nm << " [" << this->num << "]" << endl;
  if(this->pLum>0) cout << "\tLuminance is in plane " << this->pLum << endl;
  else cout << "\tThere is not luminance plane" << endl;
  cout << "\trangeC1 : " << this->rC1[0] << " - " << this->rC1[1] << endl;
  cout << "\trangeC2 : " << this->rC2[0] << " - " << this->rC2[1] << endl;
  cout << "\trangeC3 : " << this->rC3[0] << " - " << this->rC3[1] << endl;
  cout << endl;
}
