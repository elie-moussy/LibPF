#include "distribution/ColorDistribution.h"

// conversion tab used to speed up conversion from R or G or B to bin value 
static int bins_tab[256];

// this is a conversion cub to improve speed in (R,G,B)-->bin conversion for
static int bins_cub[256][256][256];

// flag indiquant que l'init est faite
static bool binsOK=false;

// Initialisation
static void InitBinsTbl() {
  int ir,ig,ib,r,g,b;
  
  if(binsOK) return;

  /* Init table de conversion d'une composante (0..255) vers une valeur de bins (0..7) */
  for (int i = 0; i < 256; i++) {
    bins_tab[i] = (int)(((float)i/256.0)*8.0);
  }

  /* Init du cube de conversion d'un triplet RGB (0..255 , 0..255 , 0..255) vers une valeur de bins (0..512) avec bin = rbin + 8*gbin + 64*bbin  */
  for (r = 0; r < 256; r++) 
    {
      for (g = 0; g < 256; g++) 
	{
	  for (b = 0; b < 256; b++) 
	    {
	      ir = bins_tab[r];
	      ig = bins_tab[g];
	      ib = bins_tab[b];
	      bins_cub[r][g][b] = (int)(ir + 8*ig + 64*ib);
	    }
	}
    }
  binsOK=true;
  printf("--> ColorDistribution (InitBinsTbl) \t:\t bins_cub initialised\n");
}

int ColorDistribution::pixRGB2bin(int r, int g, int b) { 
  return bins_cub[r][g][b]; 
}


/*! \fn ColorDistribution()
 *  \brief class constructor
 *  \param w is the width of the treated images
 *  \param h is the height of the treated images
 *  \param nbPix is the number of pixels (in case of fixed size)
 *  to be considered in each distribution computation
 */
ColorDistribution::ColorDistribution(int nbH, int w, int h, int* nbPix)
{
  this->Alloc(nbH,w,h);

  if(nbPix)
    {
      for(int i=0;i<this->nbHist;i++)
	this->incr[i]=1.0/(double)(nbPix[i]);
      
      memcpy(this->nbPix,nbPix,nbH*sizeof(int));
    }
  else
    {
      for(int i=0;i<this->nbHist;i++)
	this->incr[i]=0;
    }
}


/*! \fn ColorDistribution()
 *  \brief class destructor
 */
ColorDistribution::~ColorDistribution()
{  
  delete [] this->nbPix;
  delete [] this->incr;
  delete [] this->bins_mask;
  delete [] this->bins_mask_ref;

  for(int i=0;i<this->nbHist;i++)
    delete [] this->data[i];

  delete [] this->data;
}

/*! \fn void AllocMask(int w, int h)
 *  \brief this function allocate and initialize the bins mask and bins mask ref
 *  \param w is the width of the treated images
 *  \param h is the height of the treated images 
 */
void ColorDistribution::Alloc(int nbH, int w, int h)
{
  this->nbHist = nbH;

  this->nbPix = new int[this->nbHist];
  memset(this->nbPix,0,sizeof(int)*this->nbHist);

  this->incr = new double[this->nbHist];
  memset(this->incr,0,sizeof(double)*this->nbHist);

  /* Alloc des histogrammes */
  this->data = new double*[this->nbHist];

  /* taille du vecteur (histogramme) */
  this->vectSize = 512*sizeof(double);

  for(int i=0;i<this->nbHist;i++)
    {
      this->data[i] = new double[512];
      memset(this->data[i],0,this->vectSize);
    }

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
  InitBinsTbl();

}

/*! \fn void CalcFromRect(int histnum, IplImage* img, CvPoint& ptUL, CvPoint& ptLR, double incr)
 *  \brief this method compute color distribution histogram in a rectangle
 *  defined by its upper left and lower right corners
 *  \param histnum is the histogram to compute with the ptUL ptLR rectangle
 *  \param img the input RGB image
 *  \param ptUL the upper left corner of the bounding rectangle
 *  \param ptLR the lower right corner of the bounding rectangle
 *  \param incr = 1.0/(number of pixels)
 */
void ColorDistribution::CalcFromRect(int histnum, IplImage* img, CvPoint& ptUL, CvPoint& ptLR, double incr)
{
  register unsigned char * ptr;
  int offsetx = ptUL.x*3;
  int pos,posbin;

  //RAZ de l'histogramme
  memset(this->data[histnum],0,this->vectSize);

  for(register int i=ptUL.y;i<ptLR.y;i++) 
    {
      ptr = (unsigned char*)img->imageData+i*img->widthStep+offsetx;
      pos = i*img->width;
      for(register int j=ptUL.x;j<ptLR.x;j++) 
	{
	  posbin=pos+j;	  
	  if(this->bins_mask[posbin]==-1) 
	    /* Calcul des bins pour chaque couleur */
	    this->bins_mask[posbin] = bins_cub[*(ptr)][*(ptr+1)][*(ptr+2)];
	    
	  this->data[histnum][this->bins_mask[posbin]]+=incr;
	  ptr+=3;
	}
    }
}

/*! \fn void CalcFromRect(int histnum, IplImage* img, CvRect r, double incr)
 *  \brief this method compute color distribution histogram in a rectangle
 *  defined by CvRect structure
 *  \param histnum is the histogram to compute with the ptUL ptLR rectangle
 *  \param img the input RGB image
 *  \param r the englobant rectangle
 *  \param incr = 1.0/(number of pixels)
 */
void ColorDistribution::CalcFromRect(int histnum, IplImage* img, CvRect& r, double incr)
{
  register unsigned char * ptr;
  CvPoint ptUL,ptLR;
  ptUL.x = r.x;
  ptUL.y = r.y;
  ptLR.x = r.x+r.width;
  ptLR.y = r.y+r.height;

  int offsetx = ptUL.x*3;
  int pos,posbin;

  //RAZ de l'histogramme
  memset(this->data[histnum],0,this->vectSize);

  for(register int i=ptUL.y;i<ptLR.y;i++) 
    {
      ptr = (unsigned char*)img->imageData+i*img->widthStep+offsetx;
      pos = i*img->width;
      for(register int j=ptUL.x;j<ptLR.x;j++) 
	{
	  posbin=pos+j;	  
	  if(this->bins_mask[posbin]==-1) 
	    /* Calcul des bins pour chaque couleur */
	    this->bins_mask[posbin] = bins_cub[*(ptr)][*(ptr+1)][*(ptr+2)];
	    
	  this->data[histnum][this->bins_mask[posbin]]+=incr;
	  ptr+=3;
	}
    }
}

/*! \fn void CalcFromRectList(IplImage* img_mask, CvPoint* ptUL, CvPoint* ptLR, int* nbPix)
 *  \brief this method compute color distribution histogram in a list of rectangle
 *  defined by their upper left and lower right corners. Each rectangle is used for one histogram
 *  \param img the input RGB image
 *  \param ptUL the upper left corner list of the bounding rectangles
 *  \param ptLR the lower right corner list of the bounding rectangles
 *  \param nbPix is the number of pixels of each rectangles
 */
void ColorDistribution::CalcFromRectList(IplImage* img, CvPoint* ptUL, CvPoint* ptLR, int* nbPix)
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


/*! \fn void CalcFromRectList(IplImage* img, CvRect* r, int* nbPix)
 *  \brief this method compute color distribution histogram in a rectangle
 *  defined by CvRect structure
 *  \param img the input RGB image
 *  \param r the englobant rectangle
 *  \param nbPix is the number of pixels of each rectangles
 */
void ColorDistribution::CalcFromRectList(IplImage* img, CvRect* r,  int* nbPix)
{
  for(int i=0;i<this->nbHist;i++)
    CalcFromRect(i,img,r[i],1.0/(float)nbPix[i]);
}

/*! \fn void CalcFromMask(IplImage* img, CvPoint& ptUL, CvPoint& ptLR, IplImage* img_mask, double incr)
 *  \brief this method compute color distribution histogram in the masked region in the rectangle
 *  defined by its upper left and lower right corners
 *  \param img the input RGB image
 *  \param ptUL the upper left corner of the bounding rectangle
 *  \param ptLR the lower right corner of the bounding rectangle
 *  \param img_mask is the mask witch indicate the pixels to be used for computing the histogram
 */
void ColorDistribution::CalcFromMask(IplImage* img, CvPoint& ptUL, CvPoint& ptLR, IplImage* img_mask)
{
  register unsigned char * ptr, *ptrmask;
  int offsetx = ptUL.x*3;
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
      ptr = (unsigned char*)img->imageData+i*img->widthStep+offsetx;
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
		this->bins_mask[posbin] = bins_cub[*(ptr)][*(ptr+1)][*(ptr+2)];
	      
	      this->data[numreg][this->bins_mask[posbin]]+=1;
	      nbpix[numreg]+=1;
	    }
	  ptr+=3;
	  ptrmask++;	  
	}
    }
  /* Normalisation des histogrammes */
  for(int i=0;i<this->nbHist;i++)
    {
      if(nbpix[i])
	for(register int b=0;b<512;b++)
	  this->data[i][b]= this->data[i][b]/nbpix[i] ;
    }
}

/*! \fn void Display()
 *  \brief this function display the histogram
 */
void ColorDistribution::Disp(string fen, int pause)
{
  IplImage *dspimg = cvCreateImage(cvSize(512,256),IPL_DEPTH_8U,3);
  CvPoint p1;
  CvPoint p2;
  string nmfen;
  char num[5];
  int coul=1;

  for(int i=0;i<this->nbHist;i++)
    {
      cvZero(dspimg);
      for(p1.x=0;p1.x<511;p1.x++)
	{
	  p2.x = p1.x+1;
	  p1.y = (int)(255*(1-(this->data[i][p1.x])));
	  p2.y = 255;
	  cvRectangle(dspimg,p1,p2,CV_RGB(255,0,0),-1,8,0);
	}
      sprintf(num,"%d",i);
      nmfen = fen + num;
      cvNamedWindow(nmfen.c_str(),0);
      cvShowImage(nmfen.c_str(),dspimg);
    }
  if(pause) cvWaitKey(0);
  cvReleaseImage(&dspimg);
}


/*! \fn double BhattaDistance(int histnum, double* histo)
 *  \brief this method calculate a bhattacharrya distance between the histogram in this color distribution and
 *  an other color distribution histogram
 *  \param histnum is the histogram number to be compared with histo
 *  \param histo is the histogram data (vector 512)
 */
double ColorDistribution::BhattaDistance(int histnum, double* histo)
{
  double final=0;
  
  for(register int i=0;i<512;i++)
    final+=sqrt(this->data[histnum][i]*histo[i]);
  
  this->dist = 1.0-final;

  return this->dist;
}

/*! \fn double BhattaDistance(int histnum, double* histo, float deuxsigcarre)
 *  \brief this method calculate a bhattacharrya distance between the histogram in this color distribution and
 *  an other color distribution histogram.
 *  \param histnum is the histogram number to be compared with histo
 *  \param histo is the histogram data (vector 512)
 *  \param deuxsigcarre is the coefficient used to compute the exponential distance
 */
double ColorDistribution::BhattaDistance(int histnum, double* histo, float deuxsigcarre)
{
  double final=0;
  
  for(register int i=0;i<512;i++)
    final+=sqrt(this->data[histnum][i]*histo[i]);
  
  this->dist = 1.0-final;

  return exp(-(this->dist/deuxsigcarre));
}

/*! \fn void BhattaDistance(ColorDistribution* histref, double* dist)
 *  \brief this function compute the multihistogram distance
 */
void ColorDistribution::BhattaDistance(ColorDistribution* histref, double* dist)
{
  for(int i=0;i<this->nbHist;i++)
    dist[i] = BhattaDistance(i,histref->data[i]);    
}

/*! \fn void BhattaDistance(ColorDistribution* histref, float deuxsigcarre, double* dist)
 *  \brief this function compute the multihistogram distance
 *  \param deuxsigcarre is the coefficient used to compute the exponential distance
 */
void ColorDistribution::BhattaDistance(ColorDistribution* histref, float deuxsigcarre, double* dist)
{
  for(int i=0;i<this->nbHist;i++)
    dist[i] = BhattaDistance(i,histref->data[i],deuxsigcarre);    
}

/*! \fn void BhattaDistance(ColorDistribution* histref, double* dist)
 *  \brief this function compute the multihistogram distance
 */
double ColorDistribution::BhattaDistance(ColorDistribution* histref)
{
  double dist=1;
  for(int i=0;i<this->nbHist;i++)
    dist *= BhattaDistance(i,histref->data[i]);    

  return dist;
}

/*! \fn void BhattaDistance(ColorDistribution* histref, float deuxsigcarre, double* dist)
 *  \brief this function compute the multihistogram distance
 *  \param deuxsigcarre is the coefficient used to compute the exponential distance
 */
double ColorDistribution::BhattaDistance(ColorDistribution* histref, float deuxsigcarre)
{
  double dist=1;
  for(int i=0;i<this->nbHist;i++)
    dist *= BhattaDistance(i,histref->data[i],deuxsigcarre);    
    
  return dist;
}

/*! \fn void Update(int histnum, double* hist, double alfa)
 *  \brief this method update the color distribution by taking account of the histogram in parameter
 */
void ColorDistribution::Update(int histnum, double* hist, double alpha)
{
  double unmoinsalpha = 1-alpha;

  for(int i=0;i<512;i++) 
    this->data[histnum][i] = unmoinsalpha*this->data[histnum][i]+alpha*hist[i];
}

/*! \fn void Update(int histnum, double* hist, double* alfa)
 *  \brief this method update the color distribution by taking account of the histogram in parameter
 *  \param coldist is the color distribution to use for update
 *  \param alfa is a vector of coefficients (one by color histogram) to use for update
 */
void ColorDistribution::Update(ColorDistribution* coldist, double* alfa)
{
  //cout << "UPDATE HISTOS" << endl;
  //cout << "alfa = ";
  //DispVector<T>(alfa,this->nbHist);
  //this->Disp("REF",0);
  //coldist->Disp("UPDATE",1);
  
  for(int i=0;i<this->nbHist;i++)
    Update(i,coldist->data[i],alfa[i]);

  //cout << "Apres update" << endl;
  //this->Disp("APRES UPDATE",1);
}

