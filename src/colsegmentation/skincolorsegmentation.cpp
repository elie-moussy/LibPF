#include "skincolorsegmentation.h"

/* Constructeur de la segmentation */
SkinColorSegmentation::SkinColorSegmentation(string base, int w, int h)
{
  this->width = w;
  this->height = h;

  string colormodelfilename = "skincolormodel_" + base;

  /* Charge le modele de couleur */
  this->cm.load(colormodelfilename.c_str());

  /* Allocation des images */
  this->imgConverted = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,3);
  this->imgProba = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,1);
  this->imgP1 = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,1);
  this->imgP2 = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,1);
  this->imgP3 = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,1);

  /* Teste si on est en dimension 3 (RGB ou BGR) */
  if(this->cm.dim==3)
    {
      this->images = new IplImage*[3];
      this->images[0] = this->imgP1;
      this->images[1] = this->imgP2;
      this->images[2] = this->imgP3;
    }
  else
    {
      this->images = new IplImage*[2];      
      switch(this->cm.cb.pLum)
	{
	case 1 :
	  {
	    this->images[0] = this->imgP2;
	    this->images[1] = this->imgP3;
	  }
	  break;
	case 2 :
	  {
	    this->images[0] = this->imgP1;
	    this->images[1] = this->imgP3;
	  }
	  break;
	case 3 :
	  {
	    this->images[0] = this->imgP1;
	    this->images[1] = this->imgP2;
	  }
	  break;
	}	 
    }
  
  /* Alloc du convertisseur de couleur */
  int fromBase = this->cm.base.getIndex("BGR");
  int toBase =  this->cm.base.i();
  this->colorconverter = CvtColorAlloc(fromBase,toBase,w,h);
}

/* Destructeur */
SkinColorSegmentation::~SkinColorSegmentation()
{

}
 
/* Methode de calcul de la carte de probabilite avec conversion couleur ou non */
IplImage* SkinColorSegmentation::process(IplImage* imgIn)
{
  /* Separation en trois plans */
  cvSplit(imgIn,imgP3,imgP2,imgP1,NULL);

  /* Segmentation par projection de l'histogramme */
  cvCalcBackProject(this->images,this->imgProba,this->cm.hist);

  return this->imgProba;
}
 
/* Methode de calcul de la carte de probabilite avec conversion couleur ou non */
IplImage* SkinColorSegmentation::process(IplImage* imgIn, int cvtcolor)
{
  /* Conversion dans la base */
  colorconverter->convert(imgIn,this->imgConverted);
  
  /* Separation */
  cvSplit(this->imgConverted,imgP1,imgP2,imgP3,NULL);
  
  /* Segmentation par projection de l'histogramme */
  cvCalcBackProject(this->images,this->imgProba,this->cm.hist);

  return this->imgProba;
}


/* Methode de calcul de la carte de probabilite avec conversion couleur ou non */
IplImage* SkinColorSegmentation::processThresh(IplImage* imgIn)
{
  /* Separation en trois plans */
  cvSplit(imgIn,imgP3,imgP2,imgP1,NULL);

  /* Segmentation par projection de l'histogramme */
  cvCalcBackProject(this->images,this->imgProba,this->cm.hist_thresh);

  return this->imgProba;
}
 
/* Methode de calcul de la carte de probabilite avec conversion couleur ou non */
IplImage* SkinColorSegmentation::processThresh(IplImage* imgIn, int cvtcolor)
{
  /* Conversion dans la base */
  colorconverter->convert(imgIn,this->imgConverted);
  
  /* Separation */
  cvSplit(this->imgConverted,imgP1,imgP2,imgP3,NULL);
  
  /* Segmentation par projection de l'histogramme */
  cvCalcBackProject(this->images,this->imgProba,this->cm.hist_thresh);

  return this->imgProba;
}
