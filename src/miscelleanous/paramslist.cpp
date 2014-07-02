#include "miscelleanous/paramslist.h"


ParamsList::ParamsList(int offset)
{
  //cout << "Construction ParamsList par defaut" << endl;
  /* nb de parametres par defaut */
  this->nbparams = NBMAXPARAMS;

  this->strlist = new string[NBMAXPARAMS];

  /* init de l'indice selectionne */
  this->curselected = 0;

  /* init du nombre de parametres charges */
  this->nb=0;

  this->offset = offset;
}

ParamsList::ParamsList(string nomfic)
{
  //cout << "Construction ParamsList a partir de " << nomfic << endl;
  /* nb de parametres par defaut */
  this->nbparams = NBMAXPARAMS;

  this->strlist = new string[NBMAXPARAMS];

  /* init de l'indice selectionne */
  this->curselected = 0;  
  
  /* init du nombre de parametres charges */
  this->nb=0;  

  /* charge les parametres a partir d'un fichier */
  Load(nomfic);

  /* Affiche pour verifier ce qui a ete charge */
  //DispStrList();
}

ParamsList::ParamsList(ParamsList const& pl)
{
  /*cout << "Construction ParamsList par recopie" << endl;*/
  *this=pl; 
}

ParamsList::~ParamsList()
{
  //cout << "@ = " << this << endl;
  //getchar();

  //delete [] this->strlist;

  //this->DispStrList();
  //cout << "Destruction de ParamsList" << endl;
}

/* return the equivalent index for a given string */
int ParamsList::getIndex(string str)
{
  for(int i=0;i<this->nb;i++)
    if(str == this->strlist[i]) return i;

  return -1;
}

/* return the equivalent index for a given string */
int ParamsList::getIndexWithOffset(string str)
{
  for(int i=0;i<this->nb;i++)
    if(str == this->strlist[i]) return (this->offset+i);

  return -1;
}

/* add new couple of index and string */
int ParamsList::add(string str)
{
  if(this->nb<this->nbparams)
    {
      /* On teste si ce parametre est deja charge ou non */
      if(this->getIndex(str)!=-1) return -1;
      this->strlist[this->nb] = str;
      this->nb++;
    }
  else
    {
      cout << "add ERROR" << endl;
      return -2;
    }

  return this->nb-1;
}

/* add new couple of index and string */
int ParamsList::add(char* chaine)
{
  string tmp;
  tmp = chaine;
  return this->add(tmp);
}

/* overload operators */
ParamsList &ParamsList::operator=(const int id)
{
  this->curselected = id;
  return (*this);
}

ParamsList &ParamsList::operator=(const string str)
{
  this->curselected = this->getIndex(str);
  return (*this);
}

ParamsList &ParamsList::operator=(const char* chaine)
{
  string tmp=chaine;
  this->curselected = this->getIndex(tmp);
  return (*this);
}

ParamsList &ParamsList::operator=(ParamsList const& pl)
{
  if(this==&pl) return *this;

  this->nbparams = pl.nbparams;
  this->nb = pl.nb;

  for(int i=0;i<this->nb;i++)
    this->strlist[i] = pl.strlist[i];

  this->curselected = pl.curselected;

  return (*this);
}

bool ParamsList::operator==(int n) const
{
  if(this->curselected==n) return true;
  else return false;
}

bool ParamsList::operator==(string str) const
{
  if(this->strlist[this->curselected]==str) return true;
  else return false;
}

bool ParamsList::operator==(char* chaine) const
{
  string tmp=chaine;
  if(this->strlist[this->curselected]==tmp) return true;
  else return false;
}

void ParamsList::Disp()
{
  //cout << "ParamsList of " << this->nb << " parameters" <<  endl;
  cout << "Current selected = " << this->curselected << " and Str = " << this->strlist[this->curselected] << endl;
}

void ParamsList::DispStrList()
{
  if(this->nb>=0 && this->nb<NBMAXPARAMS)
    {
      cout << "ParamsList of " << this->nb << " parameters" <<  endl;
      cout << "StrList :" << endl;
      for(int i=0;i<this->nb;i++)
	cout << this->strlist[i] << " ; ";
      
      cout << endl;
    }
  else
    {
      cout << "ERROR ParamsList parametres non valides (ou pas assez de place)" << endl;
    }
}

void ParamsList::Load(string nomfic)
{
  ifstream fichier;
  string tmp;

  //init
  this->reset();

  fichier.open(nomfic.c_str(),ios::in);
  if(fichier)
    {
      while(!fichier.eof())
	{	
	  fichier >> tmp;
	  this->add(tmp);
	}
    }
  fichier.close();
}

ostream & operator << (ostream & os, ParamsList &pl)
{
  //os << "Current selected = " << pl.curselected << " and Str = " << pl.strlist[pl.curselected] << endl;
  pl.Disp();
  return os ;
}

istream & operator >> (istream & is, ParamsList &pl) 
{
  string tmp;
  is >> tmp;
  pl.add(tmp);
  return is ;
}
