#include "GaussianAlloc.h"

/* allocation d'une gaussienne a partir d'un fichier */
Gaussian* GaussianAlloc(string nomfic, string dispdec)
{
  int dim;
  double* mean;
  double* cov;
  ifstream fichier;

  /* ouverture du fichier et chargement de dim, mean et cov avant de faire l'allocation */
  /* ouverture */
  fichier.open(nomfic.c_str(), ios::in);
  if(fichier.bad())
    return NULL;
  
  /* Lecture de la dimension */
  FindStr(fichier,"<dim>");
  if(fichier.eof()) {cout << dispdec << "|ERROR|--> GaussianAlloc \t:\t <dim> not found" << endl; return 0;}
  fichier >> dim;
  
  /* Allocations */
  mean = new double[dim];
  cov = new double[dim*dim];
  
  /* Lecture moyenne */
  FindStr(fichier,"<mean>");
  if(fichier.eof()) {cout << dispdec << "|ERROR|--> GaussianAlloc \t:\t <mean> not found" << endl; return 0;}
  for(int i=0;i<dim;i++) 
    fichier >> mean[i];
       
  /* Lecture covariance */
  FindStr(fichier,"<cov>");
  if(fichier.eof()) {cout << dispdec << "|ERROR|--> GaussianAlloc \t:\t <cov> not found" << endl; return 0;}
  for(int i=0;i<dim*dim;i++) 
    fichier >> cov[i];

  /* allocation de la gaussienne */
  return GaussianAlloc(dim,mean,cov,dispdec);
}

Gaussian* GaussianAlloc(int dim, int diag, string dispdec)
{
  /* appel a la fonction d'allocation */
  return GaussianAlloc(dim, diag, NULL, NULL, dispdec);
}

Gaussian* GaussianAlloc(int dim, double* m, double* c, string dispdec)
{
  int isdiag=0;

  /* teste si covariance diagonale */
  if(c) isdiag = isDiagonal<double>(c,dim);
  //  cout << "Diag ? : " << isdiag << endl;
  
  /* appel a la fonction d'allocation */
  return GaussianAlloc(dim, isdiag, m, c, dispdec);
}

Gaussian* GaussianAlloc(int dim, int diag, double* m, double* c, string dispdec)
{
  switch(dim)
    {
    case 1:
      {
	//cout << dispdec << "--> GaussianAlloc \t:\t Gaussian1D allocation" << endl;
	return new Gaussian1D(dim,m,c, dispdec);
      }
      break;
      
    case 2:
      {
	if(diag)
	  {
	    //cout << dispdec << "--> GaussianAlloc \t:\t GaussianDiag2D allocation" << endl;
	    return new GaussianDiag2D(dim,m,c, dispdec);
	  }
	else
	  {
	    ////cout << dispdec << "--> GaussianAlloc \t:\t Gaussian2D allocation" << endl;
	    return new Gaussian2D(dim,m,c, dispdec);
	  }
      }
      break;

    case 3:
      {
	if(diag)
	  {
	    //cout << dispdec << "--> GaussianAlloc \t:\t GaussianDiag3D allocation" << endl;
	    return new GaussianDiag3D(dim,m,c, dispdec);
	  }
	else
	  {
	    ////cout << dispdec << "--> GaussianAlloc \t:\t Gaussian3D allocation" << endl;
	    return new Gaussian3D(dim,m,c, dispdec);
	  }
      }
      break;

    case 4:
      {
	if(diag)
	  {
	    //cout << dispdec << "--> GaussianAlloc \t:\t GaussianDiag4D allocation" << endl;
	    return new GaussianDiag4D(dim,m,c, dispdec);
	  }
	else
	  {
	    //cout << dispdec << "--> GaussianAlloc \t:\t Gaussian4D allocation" << endl;
	    return new Gaussian4D(dim,m,c, dispdec);
	  }
      }
      break;

    default:
      {
	if(diag)
	  {
	    //cout << dispdec << "--> GaussianAlloc \t:\t GaussianDiagND allocation" << endl;
	    return new GaussianDiagND(dim,m,c, dispdec);
	  }
	else
	  {
	    //cout << dispdec << "--> GaussianAlloc \t:\t GaussianND allocation" << endl;
	    return new GaussianND(dim,m,c, dispdec);
	  }
      }
    }
}
