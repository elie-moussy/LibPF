#include "distribution/MotionDistribution.h"

// conversion tab used to speed conversion from motion level (0 .. 255) to bin value
static int bins_tab[256];

// flag indiquant que l'init est faite
static bool binsOK=false;

static void InitBinsTbl(int nbbins) {
  if(binsOK) return;
  /* Init table de conversion d'une composante (0..255) vers une valeur de bins (0..7) */
  for (int i = 0; i < 256; i++) {
    bins_tab[i] = (int)(((float)i/256.0)*(float)nbbins);
  }
  binsOK=true;
  printf("--> MotionDistribution (InitBinsTbl) \t:\t bins_tab initialised\n");
}

MotionDistribution::MotionDistribution(int nbb, int nbH, int w, int h)
{
  this->Alloc(nbb, nbH, w, h);
  
  for(int i=0;i<this->nbHist;i++)
    this->incr[i]=0;
}


MotionDistribution::MotionDistribution(int nbb, int nbH, int w, int h, int* nbPix)
{
  this->Alloc(nbb,nbH,w,h);

  for(int i=0;i<this->nbHist;i++)
    this->incr[i]=1.0/(double)(nbPix[i]);

  memcpy(this->nbPix,nbPix,nbH*sizeof(int));
}


MotionDistribution::~MotionDistribution()
{  
  delete [] this->nbPix;
  delete [] this->incr;
  delete [] this->bins_mask;
  delete [] this->bins_mask_ref;

  for(int i=0;i<this->nbHist;i++)
    delete [] this->data[i];

  delete [] this->data;
  delete [] this->unifHist;
}


void MotionDistribution::Alloc(int nbb, int nbH, int w, int h)
{
  this->nbHist = nbH;

  this->nbbins = nbb;

  this->nbPix = new int[this->nbHist];
  memset(this->nbPix,0,sizeof(int)*this->nbHist);

  this->incr = new double[this->nbHist];
  memset(this->incr,0,sizeof(double)*this->nbHist);

  /* Alloc des histogrammes */
  this->data = new double*[this->nbHist];

  /* taille du vecteur (histogramme) */
  this->vectSize = this->nbbins*sizeof(double);

  for(int i=0;i<this->nbHist;i++)
    {
      this->data[i] = new double[this->nbbins];
      memset(this->data[i],0,this->vectSize);
    }

  /* Alloc histo uniforme */
  this->unifHist = new double[this->nbbins];
  double val = 1.0/(double)this->nbbins;
  for(int i=0 ; i<this->nbbins ; i++) this->unifHist[i] = val;

  /* alloc des masques */
  this->bins_mask = new int[w*h];
  this->bins_mask_ref = new int[w*h];
  
  /* taille des masques */
  this->maskSize = w*h*sizeof(int);

  /* init des masques */
  for(int i=0;i<w*h;i++)
    {
      this->bins_mask[i] = -1;
      this->bins_mask_ref[i] = -1;
    }
  
  /* init des tables de conversion */
  InitBinsTbl(this->nbbins);

  moncpt=0;
}

void MotionDistribution::CalcFromRect(int histnum, IplImage* img, CvPoint& ptUL, CvPoint& ptLR, double incr)
{
  register unsigned char * ptr;
  int pos,posbin;

  //RAZ de l'histogramme
  memset(this->data[histnum],0,this->vectSize);

  for(register int i=ptUL.y;i<ptLR.y;i++) 
    {
      ptr = (unsigned char*)img->imageData+i*img->widthStep+ ptUL.x;
      pos = i*img->width;
      for(register int j=ptUL.x;j<ptLR.x;j++) 
	{
	  posbin=pos+j;	  
	  if(this->bins_mask[posbin]==-1) 
	    /* Calcul des bins pour chaque couleur */
	    this->bins_mask[posbin] = bins_tab[*(ptr)];
	    
	  this->data[histnum][this->bins_mask[posbin]]+=incr;
	  ptr++;
	}
    }
}


void MotionDistribution::CalcFromRect(int histnum, IplImage* img, CvRect& r, double incr)
{
  register unsigned char * ptr;
  CvPoint ptUL,ptLR;
  ptUL.x = r.x;
  ptUL.y = r.y;
  ptLR.x = r.x+r.width;
  ptLR.y = r.y+r.height;
  int pos,posbin;

  //RAZ de l'histogramme
  memset(this->data[histnum],0,this->vectSize);

  for(register int i=ptUL.y;i<ptLR.y;i++) 
    {
      ptr = (unsigned char*)img->imageData+i*img->widthStep+ptUL.x;
      pos = i*img->width;
      for(register int j=ptUL.x;j<ptLR.x;j++) 
	{
	  posbin=pos+j;	  
	  if(this->bins_mask[posbin]==-1) 
	    /* Calcul des bins pour chaque couleur */
	    this->bins_mask[posbin] = bins_tab[*(ptr)];
	    
	  this->data[histnum][this->bins_mask[posbin]]+=incr;
	  ptr++;
	}
    }
}


void MotionDistribution::CalcFromRectList(IplImage* img, CvPoint* ptUL, CvPoint* ptLR, int* nbPix)
{
  //IplImage* imgtmp = cvCloneImage(img);

  for(int i=0;i<this->nbHist;i++)
    {
      // cout << "nb de pixels (dans " << i << ") = " << nbPix[i] << endl;
      // cvRectangle(imgtmp,ptUL[i],ptLR[i],CV_RGB(0,255,0));
      // Show("CalcFromRectList",imgtmp,0);
      //printf("Rect %d : (%d,%d)--(%d,%d)\n",i,ptUL[i].x,ptUL[i].y,ptLR[i].x,ptLR[i].y);
      CalcFromRect(i,img,ptUL[i],ptLR[i],1.0/(float)nbPix[i]);
    }
  //this->Disp("histos",1);
  //cvReleaseImage(&imgtmp);
}


void MotionDistribution::CalcFromRectList(IplImage* img, CvRect* r,  int* nbPix)
{
  for(int i=0;i<this->nbHist;i++)
    CalcFromRect(i,img,r[i],1.0/(float)nbPix[i]);
}


void MotionDistribution::CalcFromMask(IplImage* img, CvPoint& ptUL, CvPoint& ptLR, IplImage* img_mask)
{
  register unsigned char * ptr, *ptrmask;
  int pos,posbin,ii;
  float nbpix[20];
  int numreg;

  /* RAZ des histogrammes */
  for(int i=0;i<this->nbHist;i++)
    {
      /* init de l'histo */
      memset(this->data[i],0,this->vectSize);
      /* Init du nombre de pixels de la region */
      nbpix[i] = 0;
    }

  for(register int i=ptUL.y;i<ptLR.y;i++) 
    {
      ptr = (unsigned char*)img->imageData+i*img->widthStep+ptUL.x;
      ptrmask = (unsigned char*)img_mask->imageData+i*img_mask->widthStep+ptUL.x;
      pos = i*img->width;
      for(register int j=ptUL.x;j<ptLR.x;j++) 
	{
	  /* Teste si le pixel appartient a une partie de la forme */
	  if(*ptrmask)
	    {
	      numreg = *(ptrmask)-1;
	      posbin=pos+j;	  
	      if(this->bins_mask[posbin]==-1) 
		/* Calcul des bins pour chaque couleur */
		this->bins_mask[posbin] = bins_tab[*(ptr)];
	      
	      this->data[numreg][this->bins_mask[posbin]]+=1;
	      nbpix[numreg]+=1;
	    }
	  ptr++;
	  ptrmask++;	  
	}
    }

  /* Normalisation des histogrammes */
  for(int i=0;i<this->nbHist;i++)
    {
      if(nbpix[i])
	for(register int b=0;b<this->nbbins;b++)
	  this->data[i][b]= this->data[i][b]/nbpix[i] ;
    }
}


void MotionDistribution::Disp(string fen, int pause)
{
  IplImage *dspimg = cvCreateImage(cvSize(256,256),IPL_DEPTH_8U,3);
  CvPoint p1;
  CvPoint p2;
  string nmfen;
  char num[25];
  int pas = 256/this->nbbins;
  int cpt=0; 
  char nm[50];

  for(int i=0;i<this->nbHist;i++)
    {
      cvZero(dspimg);
      
      for(p1.x=0;p1.x<256;p1.x+=pas)
	{
	  p2.x = p1.x+pas;
	  p1.y = (int)(255*(1-(this->data[i][cpt++])));
	  p2.y = 255;
	  cvRectangle(dspimg,p1,p2,CV_RGB(255,0,0),-1,8,0);
	  cvRectangle(dspimg,p1,p2,CV_RGB(0,0,0),1,8,0);
	}
      sprintf(num,"%d",i);
      nmfen = fen + num;
      cvNamedWindow(nmfen.c_str(),0);
      cvShowImage(nmfen.c_str(),dspimg);

      sprintf(nm,"motion_histo%03d.jpg",moncpt++);
      cvSaveImage(nm,dspimg);
    }

  /* Histo uniforme */
  cvZero(dspimg);     
  cpt=0;
  for(p1.x=0;p1.x<256;p1.x+=pas)
    {
      p2.x = p1.x+pas;
      p1.y = (int)(255*(1-(this->unifHist[cpt++])));
      p2.y = 255;
      cvRectangle(dspimg,p1,p2,CV_RGB(255,0,0),-1,8,0);
      cvRectangle(dspimg,p1,p2,CV_RGB(0,0,0),1,8,0);
    }
  sprintf(num," Uniforme");
  nmfen = fen + num;
  cvNamedWindow(nmfen.c_str(),0);
  cvShowImage(nmfen.c_str(),dspimg);
  cvSaveImage("motion_histo_uniforme.jpg",dspimg);

  if(pause) cvWaitKey(0);
  cvReleaseImage(&dspimg);
}

/* Distance */

double MotionDistribution::BhattaDistance(int histnum, double* histo)
{
  double final=0;
  
  for(register int i=0;i<this->nbbins;i++)
    final+=sqrt(this->data[histnum][i]*histo[i]);
  
  this->dist = 1.0-final;

  return this->dist;
}

/* Vraissemblance */

double MotionDistribution::BhattaDistance(int histnum, double* histo, float deuxsigcarre)
{
  double final=0;
  
  for(register int i=0;i<this->nbbins;i++)
    final+=sqrt(this->data[histnum][i]*histo[i]);

  this->dist = 1.0-final;

  return exp(-(this->dist/deuxsigcarre));
}

/* Distance */

void MotionDistribution::BhattaDistance(MotionDistribution* histref, double* dist)
{
  for(int i=0;i<this->nbHist;i++)
    dist[i] = BhattaDistance(i,histref->data[i]);    
}

/* Vraissemblance*/

void MotionDistribution::BhattaDistance(MotionDistribution* histref, float deuxsigcarre, double* dist)
{
  for(int i=0;i<this->nbHist;i++)
    dist[i] = BhattaDistance(i,histref->data[i],deuxsigcarre);    
}

/* Distance */

double MotionDistribution::BhattaDistance(MotionDistribution* histref)
{
  double dist=0;
  for(int i=0;i<this->nbHist;i++)
    dist += BhattaDistance(i,histref->data[i]);    

  return dist;
}

/* Distance */

double MotionDistribution::BhattaDistance(MotionDistribution* histref, float deuxsigcarre)
{
  double dist=1;
  for(int i=0;i<this->nbHist;i++)
    dist *= BhattaDistance(i,histref->data[i],deuxsigcarre);    
    
  return dist;
}

/* Vraissemblance */

double MotionDistribution::BhattaDistance(float deuxsigcarre)
{
  double dist=1;
  for(int i=0;i<this->nbHist;i++)
    dist *= BhattaDistance(i, this->unifHist, deuxsigcarre);
    
  return dist;
}

/* Distance */

double MotionDistribution::BhattaDistance()
{
  double dist=0;
  for(int i=0;i<this->nbHist;i++)
    dist += BhattaDistance(i, this->unifHist);
    
  return dist;
}


void MotionDistribution::Update(int histnum, double* hist, double alpha)
{
  double unmoinsalpha = 1-alpha;

  for(int i=0;i<this->nbbins;i++) 
    this->data[histnum][i] = unmoinsalpha*this->data[histnum][i]+alpha*hist[i];
}


void MotionDistribution::Update(MotionDistribution* coldist, double* alfa)
{
  //cout << "UPDATE HISTOS" << endl;
  //cout << "alfa = ";
  //DispVector(alfa,this->nbHist);
  //this->Disp("REF",0);
  //coldist->Disp("UPDATE",1);
  
  for(int i=0;i<this->nbHist;i++)
    Update(i,coldist->data[i],alfa[i]);

  //cout << "Apres update" << endl;
  //this->Disp("APRES UPDATE",1);
}
