#include "skincolormodel.h"


 /* Constructeur par defaut, il ne charge pas de modele (on l'utilise classiquement avant de charger un modele) */
SkinColorModel::SkinColorModel()
{
  /* Charge la liste de base de couleur mais la base du modele n'est pas encore selectionne */
  this->base = ParamsList(icuConfigPath+"/objectslist/ColorBaseList.txt");
   
  /* Flag indiquant que la classe ne contient pas de modele de couleur charge */
  this->modelLoaded = false;

  /* Pas pret pour l'apprentissage */
  this->readyForLearn = false;
}

/* Constructeur qui ne fait que definir la base dans laquelle on travaille (on l'utilise avant de faire l'apprentissage) */
SkinColorModel::SkinColorModel(string base)
{
  cout << "--> SkinColorModel (Constructor) \t:\t Selection de la base de couleur\n";

  /* Charge la liste de base de couleur */
  this->base = ParamsList(icuConfigPath+"/objectslist/ColorBaseList.txt");

  int baseSrc = this->base.getIndex("RGB");

  /* Selection de la base de couleur */
  this->base = base;

  cout << base << endl;

  /* Alloc du convertisseur de couleur utilise pour convertir les combinaisons RGB dans l'apprentissage */
  this->colorconverter = CvtColorAlloc(baseSrc,this->base.i(),img_width,img_height);

  cout << "--> SkinColorModel (Constructor) \t:\t Chargement des parametres de la base de couleur\n";

  /* Chargement des parametres de la base */
  base += ".txt";
  this->cb.load(base);

  cout << "--> SkinColorModel (Constructor) \t:\t Allocation des histogrammes\n";

  /* Fait l'allocation des histogrammes en fonction des caracteristiques de la base choisie */
  this->alloc_histos();

  /* Flag indiquant que la classe ne contient pas de modele de couleur charge */
  this->modelLoaded = false;

  /* Pret pour l'apprentissage */
  this->readyForLearn = true;

  /* Remplissage de la Lut de conversion */
  int cpt=0;
  for(int i=0;i<32;i++)
    for(int j=0;j<8;j++)
      binLut[cpt++] = i;

  cout << "--> SkinColorModel (Constructor) \t:\t Allocation de la classe SkinColorModel OK\n";

}

/* Destructeur */
SkinColorModel::~SkinColorModel()
{

}

/* Allocation des histogrammes en fonction de la base utilisee */
int SkinColorModel::alloc_histos()
{
  int hist_size[] = {32,32};
  float rangeP1[2];
  float rangeP2[2];
  float *range[] = {rangeP1,rangeP2};
  this->dim=2;
  
  /* Selection en fonction de la position du plan luminance */
  switch(this->cb.pLum)
    {
    case 0:
      {
	int rgb_hist_size[] = {32,32,32};
	float rgb_rangeP1[2];
	float rgb_rangeP2[2];
	float rgb_rangeP3[2];
	float *rgb_range[] = {rgb_rangeP1,rgb_rangeP2,rgb_rangeP3};
	
	/* Cas ou on utilise les trois plans */
	this->dim = 3;

	/* Nombre de bins par composante */
	//hist_size[0] = this->cb.rC1[1]+1;
	//hist_size[1] = this->cb.rC2[1]+1;
	//hist_size[2] = this->cb.rC3[1]+1;

	rgb_rangeP1[0] = this->cb.rC1[0];
	rgb_rangeP1[1] = this->cb.rC1[1];
	
	rgb_rangeP2[0] = this->cb.rC2[0];
	rgb_rangeP2[1] = this->cb.rC2[1];

	rgb_rangeP3[0] = this->cb.rC3[0];
	rgb_rangeP3[1] = this->cb.rC3[1];	

	cout << "--> SkinColorModel (alloc_histos) \t:\t 3D histogram allocation\n";

	this->hist = cvCreateHist(this->dim,rgb_hist_size,CV_HIST_ARRAY,rgb_range,1); 
	this->hist_thresh = cvCreateHist(this->dim,rgb_hist_size,CV_HIST_ARRAY,rgb_range,1);
	
	return 1;
      }
      break;

    case 1:
      {
	/* Nombre de bins par composante */
	//hist_size[0] = this->cb.rC2[1]+1;
	//hist_size[1] = this->cb.rC3[1]+1;

	rangeP1[0] = this->cb.rC2[0];
	rangeP1[1] = this->cb.rC2[1];
	
	rangeP2[0] = this->cb.rC3[0];
	rangeP2[1] = this->cb.rC3[1];
       
	cout << "RangeP1 : " << rangeP1[0] << " -- " << rangeP1[1] << endl;
	cout << "RangeP2 : " << rangeP2[0] << " -- " << rangeP2[1] << endl;

	cout << "--> SkinColorModel (alloc_histos) \t:\t 2D histogram allocation and pLum = 1\n";
      }
      break;

    case 2:
      {
	/* Nombre de bins par composante */
	//hist_size[0] = this->cb.rC1[1]+1;
	//hist_size[1] = this->cb.rC3[1]+1;

	rangeP1[0] = this->cb.rC1[0];
	rangeP1[1] = this->cb.rC1[1];
	
	rangeP2[0] = this->cb.rC3[0];
	rangeP2[1] = this->cb.rC3[1];

	cout << "--> SkinColorModel (alloc_histos) \t:\t 2D histogram allocation and pLum = 2\n";
      }
      break;

    case 3:
      {
	/* Nombre de bins par composante */
	//hist_size[0] = this->cb.rC1[1]+1;
	//hist_size[1] = this->cb.rC2[1]+1;

	rangeP1[0] = this->cb.rC1[0];
	rangeP1[1] = this->cb.rC1[1];
	
	rangeP2[0] = this->cb.rC2[0];
	rangeP2[1] = this->cb.rC2[1];

	cout << "--> SkinColorModel (alloc_histos) \t:\t 2D histogram allocation and pLum = 3\n";
      }
      break;
    }
  this->hist = cvCreateHist(dim,hist_size,CV_HIST_ARRAY,range,1); 
  this->hist_thresh = cvCreateHist(dim,hist_size,CV_HIST_ARRAY,range,1);

  cout << "--> SkinColorModel (alloc_histos) \t:\t Histogram allocation OK\n";

  return 1;
}


/* Methode d'apprentissage du modele de couleur en fonction d'un fichier contenant les occurences en RGB de
   la couleur peau et non peau */
int SkinColorModel::learn(string skin_filename,string nonskin_filename)
{
  /* Fichiers */
  ifstream skin;
  ifstream nonskin;

  /* Nombre total d'occurences */
  ulint nbTotalSkin;
  ulint nbTotalNonSkin;

  /* Nombre de cellules (32*32*32) */
  //int totalNumberOfCells = 32768;

  /* Cube de donnees RGB correspondant a l'apprentissage */
  ulint skinCub[32][32][32];
  ulint nonskinCub[32][32][32];

  string enddata;

  /* Probabilite des pixels d'etre de l'objet (a priori 0.5 sur l'ensemble de l'apprentissage) */
  float objProb = 0.5;
  float nonobjProb = 1.0 - objProb; 

  if(!this->readyForLearn)
    {
      cout << "La base n'est pas connue on ne peut donc pas faire d'apprentissage !!!\n";
      return 0;
    }

  cout << "Apprentissage du modele de couleur peau a partir des fichiers " << skin_filename << " et " << nonskin_filename << endl;

  /* Ouverture des fichiers */
  skin.open(skin_filename.c_str(),ios::in);
  nonskin.open(nonskin_filename.c_str(),ios::in);


  /* Chargement des donnees d'apprentissage */
  FindStr(skin,"<numberOfEntries>");
  if(skin.eof()) { cout << "|ERROR|--> SkinColorModel (Load) \t:\t Error skin <numberOfEntries> not found !" << endl; return 0; }
  skin >> nbTotalSkin;

  cout << "nbTotalSkin = " << nbTotalSkin << endl;

  FindStr(nonskin,"<numberOfEntries>");
  if(nonskin.eof()) { cout << "|ERROR|--> SkinColorModel (Load) \t:\t Error nonskin <numberOfEntries> not found !" << endl; return 0; }
  nonskin >> nbTotalNonSkin;

  cout << "nbTotalNonSkin = " << nbTotalNonSkin << endl;

  FindStr(skin,"<data>");
  if(skin.eof()) { cout << "|ERROR|--> SkinColorModel (Load) \t:\t Error skin <data> not found !" << endl; return 0; }

  FindStr(nonskin,"<data>");
  if(nonskin.eof()) { cout << "|ERROR|--> SkinColorModel (Load) \t:\t Error nonskin <data> not found !" << endl; return 0; }
  
  cout << "Lecture des donnees RGB source\n";

  for(int b=0;b<32;b++)
    for(int g=0;g<32;g++)
      for(int r=0;r<32;r++)
	{
	  /* Lecture peau */
	  skin >> skinCub[r][g][b];

	  /* Lecture nonpeau */
	  nonskin >> nonskinCub[r][g][b];
	}

  cout << "Verification de fin skin : ";
  skin >> enddata;
  cout << enddata << endl;
  cout << " et de fin nonskin : ";
  nonskin >> enddata;
  cout << enddata << endl;

  cout << "Apprentissage pour la base " << this->base.s() << endl;
  
  /* Si on utilise la base RGB alors pas de conversion, on calcule directement l'histogramme */
  if(this->base=="RGB" || this->base=="BGR")
    {
      cout << "Calcul du rapport d'histogrammes pour la base RGB\n";
      
      float skinProb, nonskinProb;
      float ratio[32][32][32];
      float maxval=0;

      /* Calcul du rapport d'histogrammes */      
      for(int b=0;b<32;b++)
	for(int g=0;g<32;g++)
	  for(int r=0;r<32;r++)
	    {
	      /* Normalisation */
	      skinProb = objProb * (float)(skinCub[r][g][b]) / (float)(nbTotalSkin);
	      nonskinProb = nonobjProb * (float)(nonskinCub[r][g][b]) / (float)(nbTotalNonSkin);
	      
	      if(nonskinProb==0 && skinProb==0)
		ratio[r][g][b] = 0;
	      else
		{
		  /* Calcul du rapport */
		  ratio[r][g][b] = skinProb / (nonskinProb + skinProb);
		  if(ratio[r][g][b]>maxval) maxval = ratio[r][g][b];
		}	    
	    }
      
      cout << "Scale du resultat et ecriture dans l'histogramme\n";
      
      /* RAZ de l'histo */
      cvClearHist(this->hist);
      
      /* On etire le ratio pour obtenir des valeurs entre 0 et 255 puis on met dans l'histo */
      float scale = 255.0*maxval;
      for(int b=0;b<32;b++)
	for(int g=0;g<32;g++)
	  for(int r=0;r<32;r++)
	    if(this->base=="RGB") cvSetReal3D(this->hist->bins,r,g,b,ratio[r][g][b]*scale);  
	    else cvSetReal3D(this->hist->bins,b,g,r,ratio[r][g][b]*scale);  
    }
  else
    {
      cout << "Calcul du rapport d'histogrammes pour le plan chromatique de la base selectionnee\n";

      //#warning La base HSV ne peut pas etre utilisee tel quel (pb de size et de position de la luminance)

      /* Matrice de chrominance de la nouvelle base */
      ulint skinMat[32][32];
      ulint nonskinMat[32][32];
      float ratio[32][32];
      float maxval=0;
      uchar c1,c2,c3;
      uchar rbin,gbin,bbin;
      uchar cbin2,cbin3;

      float skinProb, nonskinProb;

      cout << "Init\n";

      /* Mise a 0 des matrices */
      for(int j=0;j<32;j++)
	for(int i=0;i<32;i++)
	  {
	    skinMat[i][j] = 0;
	    nonskinMat[i][j] = 0;
	  }

      cout << "Debut conversion\n";

      /* Conversion vers la base souhaitee en parcourant toutes les combinaisons RGB possibles */
      for(int b=0;b<256;b++)
	for(int g=0;g<256;g++)
	  for(int r=0;r<256;r++)
	    {
	      //cout << "RGB : (" << r << "," << g << "," << b << ") --> (";

	      /* Decouper les valeurs RGB en bins */
	      rbin = binLut[r];
	      gbin = binLut[g];
	      bbin = binLut[b];

	      //cout << (int)(rbin) << "," << (int)(gbin) << "," << (int)(bbin) << ")\n";
	     
	      /* Convertir dans la base souhaitee */ 
	      this->colorconverter->convert((uchar)r,(uchar)g,(uchar)b,&c1,&c2,&c3);

	      //cout << this->base.s() << " : (" << (int)(c1) << "," << (int)(c2) << "," << (int)(c3) << ") --> (";	      

	      /* Decouper les valeurs de la nouvelle base en bins */
	      cbin2 = binLut[(int)(c2)];
	      cbin3 = binLut[(int)(c3)];

	      //cout << (int)(cbin2) << "," << (int)(cbin3) << ")\n";	      

	      /* Ajouter les occurences dans la nouvelle base */
	      skinMat[(int)(cbin2)][(int)(cbin3)]+=skinCub[(int)(rbin)][(int)(gbin)][(int)(bbin)];
	      nonskinMat[(int)(cbin2)][(int)(cbin3)]+=nonskinCub[(int)(rbin)][(int)(gbin)][(int)(bbin)];	      
	    }

      cout << "Calcul du rapport\n";

      /* Calcul du rapport d'histogrammes */
      for(int j=0;j<32;j++)
	for(int i=0;i<32;i++)
	  {
	    /* Normalisation */
	    skinProb = objProb * (float)(skinMat[i][j]) / (float)(nbTotalSkin);
	    nonskinProb = nonobjProb * (float)(nonskinMat[i][j]) / (float)(nbTotalNonSkin);
	    
	    if(nonskinProb==0 && skinProb==0)
	      ratio[i][j] = 0;
	    else
	      {
		/* Calcul du rapport */
		ratio[i][j] = skinProb / (nonskinProb + skinProb);
		if(ratio[i][j]>maxval) maxval = ratio[i][j];
	         
		//cout << "ratio[" << i << "][" << j << "] = " << ratio[i][j] << endl;
	      }	    
	  }
       
       cout << "Scale du resultat et ecriture dans l'histogramme\n";

       /* RAZ de l'histo */
       cvClearHist(this->hist);

       /* On etire le ratio pour obtenir des valeurs entre 0 et 255 puis on met dans l'histo */
       float scale = 255.0*maxval;
       for(int j=0;j<32;j++)
	 for(int i=0;i<32;i++)
	   cvSetReal2D(this->hist->bins,i,j,ratio[i][j]*scale);

       Hist2DDisplay(this->hist,"HISTO",10);
       cvWaitKey(0);	     
    }
 
  cout << "Apprentissage termine\n";

  this->modelLoaded = true;

  return 1;
}

/* Methode d'apprentissage du modele de couleur en fonction d'un fichier contenant les occurences en RGB de
   la couleur peau et non peau */
int SkinColorModel::learn_distribution(string skin_filename, double* hist)
{
  /* Fichiers */
  ifstream skin;

  /* Nombre total d'occurences */
  ulint nbTotalSkin;

  /* Nombre de cellules (32*32*32) */
  //int totalNumberOfCells = 32768;

  //ulint skinCub[32][32][32];

  string enddata;

  cout << "Apprentissage d'une distribution de couleur a partir du fichier " << skin_filename << endl;

  /* Ouverture des fichiers */
  skin.open(skin_filename.c_str(),ios::in);

  /* Chargement des donnees d'apprentissage */
  FindStr(skin,"<numberOfEntries>");
  if(skin.eof()) { cout << "|ERROR|--> SkinColorModel (Load) \t:\t Error skin <numberOfEntries> not found !" << endl; return 0; }
  skin >> nbTotalSkin;

  cout << "nbTotalSkin = " << nbTotalSkin << endl;

  FindStr(skin,"<data>");
  if(skin.eof()) { cout << "|ERROR|--> SkinColorModel (Load) \t:\t Error skin <data> not found !" << endl; return 0; }

  cout << "Lecture des donnees RGB source\n";

  /* Raz de l'histogramme */
  for(int i=0;i<512;i++) hist[i] = 0;

  /* Calcul de la distribution d'apprentissage */
  float binval;  
  int hr,hg,hb;
  for(int b=0;b<32;b++)
    for(int g=0;g<32;g++)
      for(int r=0;r<32;r++)
	{
	  /* Lecture nb d'occurences */
	  //skin >> skinCub[r][g][b];	 
	  skin >> binval;

	  hr = (int)(((float)r)/32.0*8);
	  hg = (int)(((float)g)/32.0*8);
	  hb = (int)(((float)b)/32.0*8);

	  //cout << "Cub (" << r << "," << g << "," << b << ") --> (" << hr << "," << hg << "," << hb << ")\n";

	  hist[(int)(hr+8*hg+64*hb)] += binval/(float)(nbTotalSkin);
	}

  cout << "Verification de fin skin : ";
  skin >> enddata;
  cout << enddata << endl;

  return 1;
}

  
/* Enregistrement du modele de couleur dans un fichier */
int SkinColorModel::save(string model_filename)
{
  string chemin = icuConfigPath + "/color/" + model_filename;
  ofstream ficout;
  
  cout << "--> SkinColorModel (save) \t:\t saving color model" << endl;

  ficout.open(chemin.c_str(),ios::out|ios::trunc);
  if(!ficout.good()) return 0;

  /* Ecriture de la base a charger */
  ficout << "<basename>\n";
  ficout << this->base.s() << endl;
  ficout << "</basename>\n";

  /* Ecriture du modele (histogramme) */
  int dims, size[CV_MAX_DIM];

  dims = cvGetDims( this->hist->bins, size );
  ficout << "<dim>" <<endl;
  ficout << dims << endl;
  ficout << "</dim>" <<endl;

  ficout << "<size>\n";  
  for(int i=0;i<dims;i++)
    ficout << size[i] << "\t";
  ficout << endl;
  ficout << "</size>\n";

  ficout << "<data>\n";
  switch(dims)
    {
    case 1 :
      {
	for(int i=0;i<size[0];i++)
	  ficout << cvGetReal1D(this->hist->bins,i) << " ";
      }
      break;
    case 2 :
      {
	for(int i=0;i<size[0];i++)
	  for(int j=0;j<size[1];j++)
	    ficout << cvGetReal2D(this->hist->bins,i,j) << " ";
      }
      break;
    case 3 :
      {
	for(int i=0;i<size[0];i++)
	  for(int j=0;j<size[1];j++)
	    for(int k=0;k<size[2];k++)
	      ficout << cvGetReal3D(this->hist->bins,i,j,k) << " ";
      }
      break;
    }
  ficout << endl;
  ficout << "</data>\n";

  ficout.close();

  return 1;

}

/* Chargement d'un modele de couleur a partir d'un fichier */
int SkinColorModel::load(string model_filename)
{
  /* Lire dans le fichier le nom de la base du modele */
  string chemin = icuConfigPath + "/color/" + model_filename;
  ifstream ficin;
  string tmp;
  string filename;
  string basename;

  cout << "--> SkinColorModel (load) \t:\t load color model : " << chemin << endl;

  ficin.open(chemin.c_str(),ios::in);
  if(!ficin.good()) return 0;

  /* Lecture de la base */
  FindStr(ficin,"<basename>");
  if(ficin.eof()) { cout << "|ERROR|--> SkinColorModel (Load) \t:\t Error skin <basename> not found !" << endl; return 0; }
  ficin >> basename;
  
  /* Selection de la base de couleur */
  this->base = basename;

  /* Chargement des parametres de la base */
  basename += ".txt";
  this->cb.load(basename);

  /* Fait l'allocation des histogrammes en fonction des caracteristiques de la base choisie */
  this->alloc_histos();

  /**** Chargement du modele de couleur dans l'histogramme ****/
  
  /* Lecture de la dimension */
  int dims;
  FindStr(ficin,"<dim>"); 
  if(ficin.eof()) { cout << "|ERROR|--> SkinColorModel (Load) \t:\t Error skin model <dim> not found !" << endl; return 0; }
  ficin >> dims;  
  
  /* Lecture des sizes */
  int size[CV_MAX_DIM];
  FindStr(ficin,"<size>");
  if(ficin.eof()) { cout << "|ERROR|--> SkinColorModel (Load) \t:\t Error skin model <size> not found !" << endl; return 0; }
  for(int i=0;i<dims;i++)
    ficin >> size[i];
  
  /* Lecture des donnees */
  float valbin;
  FindStr(ficin,"<data>");
  if(ficin.eof()) { cout << "|ERROR|--> SkinColorModel (Load) \t:\t Error skin model <data> not found !" << endl; return 0; }
  switch(dims)
    {
    case 1 :
      {
	for(int i=0;i<size[0];i++)
	  {
	    ficin >> valbin;
	    cvSetReal1D(this->hist->bins,i,valbin);
	  }
      }
      break;
    case 2 :
      {
	for(int i=0;i<size[0];i++)
	  for(int j=0;j<size[1];j++)
	    {
	      ficin >> valbin;
	      cvSetReal2D(this->hist->bins,i,j,valbin);
	    }
      }
      break;
    case 3 :
      {
	for(int i=0;i<size[0];i++)
	  for(int j=0;j<size[1];j++)
	    for(int k=0;k<size[2];k++)
	      {
		ficin >> valbin;
		cvSetReal3D(this->hist->bins,i,j,k,valbin);
	      }
      }
      break;
    }

  /* Flag indiquant que le modele de couleur est charge */
  this->modelLoaded = true;

  cout << "--> SkinColorModel (load) \t:\t " << this->base.s() << "skin color model loaded" << endl;

  return 1;
}
