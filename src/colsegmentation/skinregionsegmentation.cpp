#include "skinregionsegmentation.h"


SkinRegionSegmentation::SkinRegionSegmentation(string basenm)
{
  /* Charge la liste de base de couleur et on selectionne la base a utiliser */
  this->base = ParamsList(icuConfigPath+"./objectslist/ColorBaseList.txt");
  this->base = basenm;

  /* Chargement de la base de couleur de destination */
  basenm+=".txt";
  this->colBase.load(basenm);

  /* Allocation du convertisseur couleur pour passer vers une base Luminance / Chrominance */
  int baseSrc = this->base.getIndex("BGR");
  this->colorconverter = CvtColorAlloc(baseSrc,this->base.i(),img_width,img_height);

  /********** Allocation des images ***********/

  this->imgBase = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,3);
  this->imgProbaSeuillee = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,1);
  this->imgP1 = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,1);
  this->imgP2 = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,1);
  this->imgP3 = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,1);
  this->imgTMP = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,1);
  this->imgRegions = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,1);
  this->imgRegionsMask = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,1);
  this->imgRegionsFiltrees = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,1);
  this->imgMask = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,1); 
  this->imgGRAY = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,1);
  this->imgMEDIUM = cvCreateImage(cvSize(img_width/2,img_height/2),IPL_DEPTH_8U,1);
  this->imgEDGE = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,1);

  /* Connection des images correspondant au plan chromatique pour faire le calcul de l'histogramme */
  this->planChrom[0] = this->imgP2;
  this->planChrom[1] = this->imgP3;

  /* Alloc de l'espace de memoire */
  this->storage = cvCreateMemStorage(0);

  /* Nombre total de pixels dans les images */
  this->nbPixels = img_width*img_height;

  /* Init de la taille de certains vecteurs */
  this->statsnbRegPixVectSize = NBMAXREG * sizeof(int);
  this->statsSumVectSize = NBMAXREG * sizeof(float);

  /* Init de la liste des voisins */
  this->voisList[0] = -img_width-1;
  this->voisList[1] = -img_width;
  this->voisList[2] = -img_width+1;
  this->voisList[3] = -1;
  this->voisList[4] = 1;
  this->voisList[5] = img_width-1;
  this->voisList[6] = img_width;
  this->voisList[7] = img_width+1;

  /* Alloc d'un element structurant pour le calcul du masque final */
  this->eltStructurant = cvCreateStructuringElementEx(3,3,1,1,CV_SHAPE_RECT,NULL);

  /********************************************/

  /* Allocation watershed pour traiter les histogrammes */
  this->ws.alloc(256,256);
  this->wsLum.alloc(256);
  
  /* Allocation des histogrammes */
  this->alloc_histos();

  /**** INIT DES PARAMETRES ****/
  this->init_params();

}

SkinRegionSegmentation::~SkinRegionSegmentation()
{

}

/* Classe qui initialise les differents parametres */
void SkinRegionSegmentation::init_params()
{  
  /* Init du seuil de segmentation proba couleur peau (on ne conserve que les pixels de proba importante) */
  //this->skinprob.setThreshold(150);

  /* On remplace le seuil precedent par un seuillage apres calcul de la carte de probabilite */
  this->seuilSkinProb = 150;

  /* Seuil pour clusteriser les regions de l'histogramme de chrominance, seul les pics de niveau > 1/2 du max sont consideres */
  this->seuilPic = 127;
  this->seuilPicLum = 127;

  /* Definition du seuil pour la selection des pics par calcul du contraste normalise (on veut une separation > a la moitie du pic) */
  this->seuilContrasteNorm = 0.6;
  this->seuilContrasteNormLum = 0.5;

  /* Seuil sur la taille des regions */
  this->seuilRegSize = 50;

  /* Init seuil de distance colorimetrique */
  this->seuilColdist=50;

  /* Init seuil de propagation des regions dans le watershed couleur */
  this->seuilProbVois = 15;
}

int SkinRegionSegmentation::alloc_histos()
{
  //Allocation des Histogrammes
  int hist_sizeChrom[2];
  int hist_sizeLum[1];
  float rangeC2[2];
  float rangeC3[2];  
  float rangeC1[2];
  float* ranges[] = {rangeC2,rangeC3};
  float* rangesLum[] = {rangeC1};

  /* Definition des ranges et size sur la chrominance pour la base courante */
  hist_sizeChrom[0] = this->colBase.getdimChrom1();
  hist_sizeChrom[1] = this->colBase.getdimChrom2();
  rangeC2[0] = this->colBase.getChrom1Rmin();
  rangeC2[1] = this->colBase.getChrom1Rmax();
  rangeC3[0] = this->colBase.getChrom2Rmin();
  rangeC3[1] = this->colBase.getChrom2Rmax();

  /* Histogramme de luminance */
  hist_sizeLum[0] = 256;
  rangeC1[0] = 0;
  rangeC1[1] = 255;

  /* Creation de l'histogramme de chrominance */
  this->histoChrom = cvCreateHist(2,hist_sizeChrom,CV_HIST_ARRAY,ranges, 1 );
  this->histoChromClust = cvCreateHist(2,hist_sizeChrom,CV_HIST_ARRAY,ranges, 1 );

  /* Creation de l'histogramme de luminance */
  this->histoLum = cvCreateHist(1,hist_sizeLum,CV_HIST_ARRAY,rangesLum,1);
  this->histoLumClust = cvCreateHist(1,hist_sizeLum,CV_HIST_ARRAY,rangesLum,1);

  /* Liaison des bins des histogrammes avec des matrices */
  this->histBins = (CvMat*)this->histoChrom->bins;
  this->histClustBins = (CvMat*)this->histoChromClust->bins;
  this->histLumBins = (CvMat*)this->histoLum->bins;
  this->histLumClustBins = (CvMat*)this->histoLumClust->bins;

  /* Nombre de bins total de l'histogramme de chrominance */
  this->nbbins = (int)(rangeC2[1]*rangeC3[1]);

  /* Allocation des donnees de l'histogramme temporaire */
  this->tmpHist = new float[this->nbbins];

  /* Taille du vecteur de donnees d'un histogramme */
  this->sizeHist = this->nbbins*sizeof(float);

  return 1;
}

/* Augmentation de la dynamique de l'histogramme et filtrage par dilatation */
void SkinRegionSegmentation::hdyn()
{
  float min = 9999;
  float max = 0;
  
  /* Recherche min et max de l'histo */
  for(int i=0;i<this->nbbins;i++)
    {
      if(this->histBins->data.fl[i])
	{
	  if(this->histBins->data.fl[i]<min) min = this->histBins->data.fl[i];
	  if(this->histBins->data.fl[i]>max) max = this->histBins->data.fl[i];
	}
    }
  
  /* Etirement des valeurs de l'histogramme entre 0 et 255 */
  double delta = max - min;
  for(int i=0;i<this->nbbins;i++)    
    if(this->histBins->data.fl[i])
      this->histBins->data.fl[i] = 255.0 * (this->histBins->data.fl[i] - min) / delta;
  
}

/* Dilatation de l'histogramme de chromiance avec un element structurant de taille 5 x 5 
   (Attention ne convient que pour des bases dont les valeurs sont entre 0 et 255 
*/ 
void SkinRegionSegmentation::hdil()
{
  /* Propagation horizontale */
  for(int i=0,j=-1,k=-2,l=1,m=2,cpt=0;i<nbbins;i++,j++,k++,l++,m++,cpt++)
      {
	/* Recopie de la valeur du centre */
	this->tmpHist[i] = this->histBins->data.fl[i];
	
	/* Traitement predecesseurs */ 
	if(cpt>0 && this->tmpHist[i] < this->histBins->data.fl[j]) this->tmpHist[i] = this->histBins->data.fl[j];       
	if(cpt>1 && this->tmpHist[i] < this->histBins->data.fl[k]) this->tmpHist[i] = this->histBins->data.fl[k]; 
      
	/* Traitement des suivants */ 
	if(cpt<255 && this->tmpHist[i] < this->histBins->data.fl[l]) this->tmpHist[i] = this->histBins->data.fl[l]; 
	if(cpt<254 && this->tmpHist[i] < this->histBins->data.fl[m]) this->tmpHist[i] = this->histBins->data.fl[m]; 

	/* remise a 0 de cpt */
	if(cpt>255) cpt = 0;
      }
  /* Propagation verticale */
  for(int i=0,j=-256,k=-512,l=256,m=512;i<this->nbbins;i++,j++,k++,l++,m++)
    {
      /* Recopie de la valeur du centre */
      this->histBins->data.fl[i] = this->tmpHist[i];
      
      /* Traitement predecesseurs */ 
      if(j>=0 && this->histBins->data.fl[i] < this->tmpHist[j]) this->histBins->data.fl[i] = this->tmpHist[j]; 
      if(k>=0 && this->histBins->data.fl[i] < this->tmpHist[k]) this->histBins->data.fl[i] = this->tmpHist[k]; 

      /* Traitement des suivants */ 
      if(l<this->nbbins && this->histBins->data.fl[i] < this->tmpHist[l]) this->histBins->data.fl[i] = this->tmpHist[l];       
      if(m<this->nbbins && this->histBins->data.fl[i] < this->tmpHist[m]) this->histBins->data.fl[i] = this->tmpHist[m];        
    }
}

/* Augmentation de la dynamique de l'histogramme et filtrage par dilatation */
void SkinRegionSegmentation::lumdyn()
{
  float min = 9999;
  float max = 0;
  
  /* Recherche min et max de l'histo */
  for(int i=0;i<256;i++)
    {
      if(this->histLumBins->data.fl[i])
	{
	  if(this->histLumBins->data.fl[i]<min) min = this->histLumBins->data.fl[i];
	  if(this->histLumBins->data.fl[i]>max) max = this->histLumBins->data.fl[i];
	}
    }
  
  /* Etirement des valeurs de l'histogramme entre 0 et 255 */
  double delta = max - min;
  for(int i=0;i<256;i++)    
    if(this->histLumBins->data.fl[i])
      this->histLumBins->data.fl[i] = 255.0 * (this->histLumBins->data.fl[i] - min) / delta;
  
}

/* Dilatation de l'histogramme de chromiance avec un element structurant de taille 5 x 5 
   (Attention ne convient que pour des bases dont les valeurs sont entre 0 et 255 
*/ 
void SkinRegionSegmentation::lumdil()
{
  int pos;
  float tmp[3];

  /* Propagation horizontale */
  for(int i=0,j=-1,k=-2,l=1,m=2;i<256;i++,j++,k++,l++,m++)
      {
	pos = i % 3;

	/* Recopie des elements stockes en temporaire */
	if(i>2) this->histLumBins->data.fl[k-1]=tmp[pos];

	/* Recopie de la valeur du centre */
	tmp[pos] = this->histLumBins->data.fl[i];
	
	/* Traitement predecesseurs */ 
	if(i>0 && tmp[pos] < this->histLumBins->data.fl[j]) tmp[pos] = this->histLumBins->data.fl[j];       
	if(i>1 && tmp[pos] < this->histLumBins->data.fl[k]) tmp[pos] = this->histLumBins->data.fl[k]; 
      
	/* Traitement des suivants */ 
	if(i<255 && tmp[pos] < this->histLumBins->data.fl[l]) tmp[pos] = this->histLumBins->data.fl[l]; 
	if(i<254 && tmp[pos] < this->histLumBins->data.fl[m]) tmp[pos] = this->histLumBins->data.fl[m]; 
      }
  
  /* Recopie des dernieres valeurs */
  this->histLumBins->data.fl[253]=tmp[0];
  this->histLumBins->data.fl[254]=tmp[1];
  this->histLumBins->data.fl[255]=tmp[2];
}


/* Clusterisation des regions de l'histogramme par watershed */
void SkinRegionSegmentation::hclusterise()
{

  /* Init de l'histogramme clusterise */
  memset(this->histClustBins->data.fl,0,this->sizeHist);

  /* Extraction de pics significatifs dans l'histogramme --> nb de regions finale */  
  this->nbreg = this->ws.ExtractMax(this->histBins->data.fl,this->histClustBins->data.fl, this->picList,this->seuilContrasteNorm,this->seuilPic);
  //cout << this->nbreg << " regions\n";

  /* Calcul du partage des eaux pour obtenir les differentes regions */
  this->ws.Process(this->histBins->data.fl,this->histClustBins->data.fl,this->picList,1);  
}

/* Filtrage des petites regions et bouchage des trous */
void SkinRegionSegmentation::filtrage()
{
  CvSeq* contour = 0;
  int cpt=0;

  /* Init de l'image de resultat */
  cvZero(this->imgRegionsFiltrees);

  /* On traite les regions une par une */
  for(int i=1;i<=this->nbreg;i++)
    {
      /* Elimination des regions de numero > i */
      cvThreshold(this->imgRegions,this->imgMask,i,0,CV_THRESH_TOZERO_INV);
            
      /* Elimination des regions < i */
      cvThreshold(this->imgMask,this->imgMask,i-1,255,CV_THRESH_BINARY);

      /**** BOUCHAGE DES TROUS ET FILTRAGE DES PETITES REGIONS ****/
      
      /* Recherche des contours des regions connexes */
      cvFindContours(this->imgMask,this->storage,&contour,sizeof(CvContour),CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);
      
      /* Trace des regions dans l'image resultat --> remplissage des trous */
      for( ; contour != 0; contour = contour->h_next )
	{
	  /* Filtrage des petites regions en meme temps */
	  if(contour->total>this->seuilRegSize)
	    {
	      /* recuperation d'un point pour obtenir le numero de region */
	      //pt = CV_GET_SEQ_ELEM(CvPoint,contour,0);
	      
	      /* recup du label de la région */
	      //color = (int)this->imgRegions->imageData[pt->y*this->imgRegions->widthStep+pt->x];

	      /* Trace de la région */
	      //cvDrawContours(this->imgRegionsFiltrees,contour,cvScalar(color),cvScalar(color),-1,CV_FILLED,8);

	      cpt++;
	      cvDrawContours(this->imgRegionsFiltrees,contour,cvScalar(cpt),cvScalar(cpt),-1,CV_FILLED,8);
	    }	    
	}
    }
  
  /* recopie du nombre de regions obtenues */
  this->nbreg = cpt;
}

/* Watershed couleur qui permet de completer les regions en fonction de la couleur locale et de la probabilite */
void SkinRegionSegmentation::color_watershed(IplImage* img_edge)
{
  int regnum;
  int pos;
  int pt,vois;
  int lbl,lblvois;
  int niv;
  float mR,mG,mB;
  float coldist,dr,dg,db;
 
  /* Mise a 0 des stats des regions */
  memset(this->sumR,0,this->statsSumVectSize);
  memset(this->sumG,0,this->statsSumVectSize);
  memset(this->sumB,0,this->statsSumVectSize);
  memset(this->nbRegpix,0,this->statsnbRegPixVectSize);

  /* Parcours de l'image des regions pour calculer les statistiques couleur des regions et empiler les coordonnees pour le watershed */
  for(int i=0;i<this->nbPixels;i++)
    {
      regnum = (int)imgRegionsFiltrees->imageData[i];

      /* Si le pixel appartient a une region, ses coordonnees sont empilees et sa couleur contribue aux stats couleur de la region */
      if(regnum)
	{
	  this->niveaux[0].push(i);

	  /* Position dans l'image couleur */	  
	  pos = 3*i;

	  /* Mise a jour des stats */
	  this->nbRegpix[regnum]++;
	  this->sumB[regnum]+=this->imgIn->imageData[pos];
	  this->sumG[regnum]+=this->imgIn->imageData[pos+1];
	  this->sumR[regnum]+=this->imgIn->imageData[pos+2];
	}      
    }

  /* Calcul du watershed couleur */
  for(int i=0;i<256;i++)
    {
      //inondation
      while(!this->niveaux[i].empty())
	{
	  //tant que file[i] non vide
	  //Lire un point dans la file
	  pt = this->niveaux[i].front();
	  this->niveaux[i].pop();

	  //recuperation du label du pixel
	  lbl = (int)this->imgRegionsFiltrees->imageData[pt];

	  //on traite ses voisins
	  for(int v=0;v<8;v++)
	    {
	      /* On calcule l'indice du voisin */
	      vois = pt + voisList[v];
	      
	      //si voisin pas traité alors 
	      //      elargir l'image label 
	      //             et
	      //      mettre les coordonnées du voisin dans la file :
	      //         file[max(gradient,i)]
	      /* Teste si on est dans le tableau, si le voisin n'est pas deja traite et si ce n'est pas un point de contour */
	      if(vois>=0 && vois<this->nbPixels && !((int)this->imgRegionsFiltrees->imageData[vois]) && !((int)img_edge->imageData[vois]) )
		{
		  /* Recup de la proba du pixel */
		  lblvois = (int)cvGetReal1D(this->imgProba,vois);
		  
		  /* On calcule la distance colorimetrique avec la region a laquelle il pourrait appartenir */
		  mR = this->sumR[lbl] / this->nbRegpix[lbl];
		  mG = this->sumG[lbl] / this->nbRegpix[lbl];
		  mB = this->sumB[lbl] / this->nbRegpix[lbl];

		  pos = vois*3;

		  dr = mR - (int)(this->imgIn->imageData[pos+2]); 
		  dg = mG - (int)(this->imgIn->imageData[pos+1]); 
		  db = mB - (int)(this->imgIn->imageData[pos]); 

		  coldist = sqrt(dr*dr+dg*dg+db*db);

		  /* Si la couleur est proche alors on empile le voisin selon sa probabilite couleur peau */
		  if(coldist<this->seuilColdist && lblvois>this->seuilProbVois)
		  {
		      /* Mise a jour stat couleur de la region */
		      this->sumR[lbl] += (int)(this->imgIn->imageData[pos+2]);
		      this->sumG[lbl] += (int)(this->imgIn->imageData[pos+1]);
		      this->sumB[lbl] += (int)(this->imgIn->imageData[pos]);
		      this->nbRegpix[lbl]++;
		      
		      /* Labelisation du voisin */
		      this->imgRegionsFiltrees->imageData[vois] = (uchar)lbl;

		      /* On place le voisin dans la liste selon sa probabilite de couleur peau */
		      /* On peut aussi ajouter la valeur du contour mais pas dans le cas d'un contour binaire */		     
		      niv = 255 - lblvois;

		      /* Si la proba est inferieure a au niveau actuel alors on injecte le pixel dans la file courante pour le traiter a la suite */
		      if(niv<i)
			this->niveaux[i].push(vois);
		      else
			this->niveaux[niv].push(vois);		
		    }
		}
	    }
	}
    }
}


/* Methode qui fait la segmentation region et retourne un pointeur vers l'image des regions */
void SkinRegionSegmentation::lumSeg()
{
  int new_nbreg=0;

  cout << "Debut lumSeg\n";

  cvZero(this->imgRegions);

  /* On traite chaque region */
  for(int i=1;i<=this->nbreg;i++)
    {
      cout << "region " << i << "\n";
  
      /* Elimination des regions de numero > i */
      cvThreshold(this->imgRegionsFiltrees,this->imgMask,i,0,CV_THRESH_TOZERO_INV);
            
      /* Elimination des regions < i */
      cvThreshold(this->imgMask,this->imgMask,i-1,255,CV_THRESH_BINARY);

      DispFcol(this->imgMask,"REGION");

      /* Calcul de l'histogramme de luminance des pixels de la region i */
      cvCalcHist(&this->imgP1,this->histoLum,0,this->imgMask);
      
      /* Dynamisation de l'histogramme */
      this->lumdyn();

      Hist1DDisplay(this->histoLum,"HistoLum 1",2,CV_RGB(255,0,0));

      /* Dilatation de l'histogramme */
      this->lumdil();

      Hist1DDisplay(this->histoLum,"HistoLum",2,CV_RGB(255,0,0));
      cvWaitKey(0);
      
      /* Extraction des Max Locaux pour la luminence de la region i (contraste normalisé) */
      new_nbreg = this->wsLum.ExtractMax(this->histLumBins->data.fl,this->histLumClustBins->data.fl, picList,this->seuilContrasteNormLum,this->seuilPicLum,new_nbreg);

      /* Clusterisation de l'histogramme de luminence de la region i */
      this->wsLum.Process(this->histLumBins->data.fl,this->histLumClustBins->data.fl, picList);

      /* Fabrication d'une image résultant de la clusterisation par luminance */
      cvCalcBackProject(&this->imgP1,this->imgTMP,this->histoLumClust);
      
      /* Concaténation du resultat de cette région avec les autres résultats des autres régions */
      cvOr(this->imgTMP,this->imgRegions,this->imgRegions,this->imgMask);
    }

  /* Mise a jour du nombre de regions */
  this->nbreg = new_nbreg;
}

/* Methode qui fait la segmentation region et retourne un pointeur vers l'image des regions */
IplImage* SkinRegionSegmentation::processRegions(IplImage* imgIn)
{ 
  /******** Calcul de l'image de contours **********/
  
  /* conversion BGR vers gris */
  cvCvtColor(imgIn,this->imgGRAY,CV_BGR2GRAY);
  
  /* Lissage de l'image */
  cvPyrDown(this->imgGRAY,this->imgMEDIUM,CV_GAUSSIAN_5x5);
  cvPyrUp(this->imgMEDIUM,this->imgGRAY,CV_GAUSSIAN_5x5);

  /* calcul des contours */
  cvCanny(this->imgGRAY,this->imgEDGE,20,40);

  /******** Traitement ********/

  return this->processRegions(imgIn,this->imgEDGE);
}

/* Methode qui fait la segmentation region et retourne un pointeur vers l'image des regions */
IplImage* SkinRegionSegmentation::processRegions(IplImage* imgIn, IplImage* img_edge)
{
  this->imgIn = imgIn;

  /*************** PRES TRAITEMENT DE L'IMAGE ***************/

  /* Conversion de l'image d'entree dans la base base */
  this->colorconverter->convert(imgIn,this->imgBase);

  /* Separation des plans pour extraire le plan chromatique */
  cvSplit(this->imgBase,this->imgP1,this->imgP2,this->imgP3,NULL);

  /* Presegmentation de l'image en fonction de la probabilite de couleur peau */
  this->imgProba = this->skinprob.process(imgIn);

  /* Seuillage de l'image de probabilite pour ne retenir que les pixels les plus vraisemblables dans la segmentation */
  cvThreshold(this->imgProba,this->imgProbaSeuillee,this->seuilSkinProb,0,CV_THRESH_TOZERO);
  
  /*************** SEGMENTATION SELON LA CHROMINANCE ***************/

  /* Calcul de l'histogramme de chrominance des pixels pre segmentes */
  cvCalcHist(this->planChrom,this->histoChrom,0,this->imgProbaSeuillee);

  /* Amelioration de l'histogramme (etirement des niveaux entre 0 et 255, puis dilatation des bins) */

  //Etirement de l'histogramme 
  this->hdyn();

  //Dilatation  de l'histogramme de chrominance
  this->hdil();

  /* Clusterisation de l'histogramme */
  this->hclusterise(); 

  /* On repasse a l'image en projetant l'image a travers l'histogramme clusterise --> image des regions */
  cvCalcBackProject(this->planChrom,this->imgRegions,this->histoChromClust);

  /* Filtrage de l'image des regions obtenues (on bouche les trous et on enleve les petites regions) */
  this->filtrage();

  /**************** WATERSHED COULEUR POUR REMPLIR LES REGIONS *************/

  /* watershed */
  this->color_watershed(img_edge);

  /* Segmentation avec luminance et elimination des petites regions */
  //this->lumSeg();
  //return this->imgRegions;

  /* Retour de l'image de masques */
  return this->imgRegionsFiltrees;
}

/* Methode qui fait la segmentation region et retourne un pointeur vers l'image des regions */
IplImage* SkinRegionSegmentation::processRegionsMask(IplImage* imgIn)
{
  /* Calcul des regions */
  IplImage* imgRes = this->processRegions(imgIn);

  /* Calcul du masque */
  this->compute_region_mask(imgRes);

  return this->imgRegionsMask;
}

/* Methode qui fait la segmentation region et retourne un pointeur vers l'image des regions */
IplImage* SkinRegionSegmentation::processRegionsMask(IplImage* imgIn, IplImage* img_edge)
{
  /* Calcul des regions */
  IplImage* imgRes = this->processRegions(imgIn,img_edge);

  /* Calcul du masque */
  this->compute_region_mask(imgRes);

  return this->imgRegionsMask;
}

void SkinRegionSegmentation::compute_region_mask(IplImage* imgIn)
{
  /* On construit le masque en utilisant des images deja allouees */
  cvDilate(imgIn,this->imgRegionsMask,this->eltStructurant,7);
  cvErode(imgIn,this->imgTMP,this->eltStructurant,3);
  
  /* Fusion des deux images */
  cvSet(this->imgRegionsMask,cvScalar(0),this->imgTMP);
}


