#include "stringutils.h"


void Message(string cla, string func, string msg)
{
  string disp = cla + " " + func;
  /* formatage des differents elements */
  FormatStr(disp,disp,MESSAGENBCHAR);
  cout << "--> " << disp << " : " << msg << endl;
}

void ErrorMessage(string cla, string func, string msg)
{
  string disp = cla + " " + func;
  /* formatage des differents elements */
  FormatStr(disp,disp,MESSAGENBCHAR);
  cout << "|ERROR|--> " << disp << " : " << msg << endl;

}

void Message(string msg)
{
  string disp=" ";
  FormatStr(disp,disp,MESSAGENBCHAR+4);
  cout << disp << " : " << msg << endl;

}

void ErrorMessage(string msg)
{
  string disp=" ";
  FormatStr(disp,disp,MESSAGENBCHAR+11);
  cout << disp << " : " << msg << endl;
}

/* Fonction qui enleve le nom de fichier d'un chemin relatif et colle un autre nom de fichier */
void FileReplace(string & ficcomplet, string fic)
{
  int pos = ficcomplet.find_last_of("/");
  if(pos)
    ficcomplet.replace(pos+1,ficcomplet.length()-pos,fic);
}

void FileReplace(string ficcomplet, string fic, string & ficout)
{
  ficout = ficcomplet;
  FileReplace(ficout,fic);
}


void FormatStr(string strIn, string & strOut, int size)
{
  if(size>strIn.length())
    {
      int bourrage=size-strIn.length();
      string bourrageStr(bourrage,' ');
      strOut = strIn+bourrageStr;
    }
  else
    strOut=strIn;
}

/* fonction cpp qui recherche un pattern dans un fichier */
void FindStr2(istream & ptfic, const char* pattern, const char* commentaire)
{
  string buff;

  /* recherche du pattern */
  buff.erase();
  /* remise au debut du fichier */
  ptfic.seekg(0, ios::beg);

  while ( std::getline( ptfic, buff ) )
    {
      /* Pattern sans commentaire trouve */
      if(!buff.find(pattern,0) && buff.find(commentaire,0))
	break;
    }
}

/* fonction cpp qui recherche un pattern dans un fichier */
void FindStr(istream & ptfic, const char* pattern)
{
  string buff;
  /* recherche du pattern */
  buff.erase();
  /* remise au debut du fichier */
  ptfic.seekg(0, ios::beg);

  while ( std::getline( ptfic, buff ) )
    {
      /* saut de commentaire */
      if(!buff.find("/#",0))
	{
	  while(std::getline( ptfic, buff ))
	    if(!buff.find("#/",0)) break;
	}
      else
	if(!buff.find(pattern,0)) break;
    }
}

/* Fonction qui recherche un pattern dans un fichier et retourne le pointeur de fichier courant */
FILE * FindStr(FILE * fichier, const char* pattern)
{
  char buff[256];
  strcpy(buff,"");
  while(!feof(fichier))
    {
      /* Test si debut de commentaire */
      if(strcmp(buff,"/#")==0)
	{
	  /* Lecture jusqu'a la fin du commentaire */
	  while(strcmp(buff,"#/")!=0) fscanf(fichier,"%s",buff);
	}
      else
	if (strcmp(buff,pattern)==0) return fichier;
      fscanf(fichier,"%s",buff);      
    }
  return fichier;
}

/*
  Convertit une chaine de caractere passe en parametre en un indice correpondant
  a la position de la chaine dans un tableau passe aussi en parametre
*/
int StrToIndex(char strtab[][50], int nb, char *pattern)
{
  for(int i=0;i<nb;i++)
    if(strcmp(strtab[i],pattern)==0) return i;

  printf("|ERROR|--> StrToIndex (outil.cpp) \t:\t %s not found in possible values\n",pattern);
  return -1;
}
