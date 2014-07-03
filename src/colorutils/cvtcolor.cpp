#include "LibPF/colorutils/cvtcolor.h"

/*
  Methodes de conversion couleur qui ne sont pas dans opencv
*/

/* Classe de base */ 
CvtColor::CvtColor(int w, int h, int fromRGB)
{ 
  this->width = w; 
  this->height = h; 

  this->pixImg = cvCreateImage(cvSize(1,1),IPL_DEPTH_8U,3);
  this->pixImg2 = cvCreateImage(cvSize(1,1),IPL_DEPTH_8U,3);

  this->fromRGB = fromRGB;
}


CvtColor::~CvtColor()
{
  cvReleaseImage(&this->pixImg);
  cvReleaseImage(&this->pixImg2);
}

/* Conversion simple des valeurs RGB vers une autre base */
void CvtColor::convert(uchar r, uchar g, uchar b, uchar* C1, uchar* C2, uchar* C3)
{
  /* Recopie des valeurs dans l'image */
  if(fromRGB)
    {
      this->pixImg->imageData[0] = r;
      this->pixImg->imageData[1] = g;
      this->pixImg->imageData[2] = b;
    }
  else
    {
      this->pixImg->imageData[0] = b;
      this->pixImg->imageData[1] = g;
      this->pixImg->imageData[2] = r;
    }

  /* Conversion */
  this->convert(this->pixImg,this->pixImg2);

  /* Recuperation du resultat */
  *C1 = this->pixImg2->imageData[0];
  *C2 = this->pixImg2->imageData[1];
  *C3 = this->pixImg2->imageData[2];
}




/**********************************************************/
/*                 CONVERSION VERS rgbN                   */
/**********************************************************/

/* constructor */
CvtColorRGB2rgbN::CvtColorRGB2rgbN(int w, int h):CvtColor(w,h,1)
{

}


CvtColorRGB2rgbN::~CvtColorRGB2rgbN()
{

}

void CvtColorRGB2rgbN::convert(IplImage* imgin, IplImage* imgout)
{
  uchar* ptIn = (uchar*)imgin->imageData;
  uchar* ptOut = (uchar*)imgout->imageData;
  int taille = imgin->width*imgin->height;
  float sum;

  for(int i=0;i<taille;i++,ptIn+=3,ptOut+=3)
    {
      sum = ptIn[0] + ptIn[1] + ptIn[2];
      if(sum)
	{
	  ptOut[0] = (uchar)( (float)(ptIn[0])/sum * 255.0 );
	  ptOut[1] = (uchar)( (float)(ptIn[1])/sum * 255.0 );
	  ptOut[2] = (uchar)( (float)(ptIn[2])/sum * 255.0 );
	}
      else
	{
	  ptOut[0] = 0;
	  ptOut[1] = 0;
	  ptOut[2] = 0;
	}
    }

}

void CvtColorRGB2rgbN::convert(IplImage* imgin, IplImage* s1out, IplImage* s2out, IplImage* s3out)
{
  uchar* ptIn = (uchar*)imgin->imageData;
  uchar* pts1Out = (uchar*)s1out->imageData;
  uchar* pts2Out = (uchar*)s2out->imageData;
  uchar* pts3Out = (uchar*)s3out->imageData;
  int taille = imgin->width*imgin->height;
  float sum;

  for(int i=0;i<taille;i++,ptIn+=3,pts1Out++,pts2Out++,pts3Out++)
    {
      sum = ptIn[0] + ptIn[1] + ptIn[2];
      if(sum)
	{
	  *pts1Out = (uchar)( (float)(ptIn[0])/sum * 255.0 );
	  *pts2Out = (uchar)( (float)(ptIn[1])/sum * 255.0 );
	  *pts3Out = (uchar)( (float)(ptIn[2])/sum * 255.0 );
	}
      else
	{
	  *pts1Out = 0;
	  *pts2Out = 0;
	  *pts3Out = 0;
	}
    }
}

/**********************************************************/
/*                 CONVERSION VERS Irg                   */
/**********************************************************/

/* constructor */
CvtColorRGB2Irg::CvtColorRGB2Irg(int w, int h):CvtColor(w,h,1)
{
 
}


CvtColorRGB2Irg::~CvtColorRGB2Irg()
{

}

void CvtColorRGB2Irg::convert(IplImage* imgin, IplImage* imgout)
{
  uchar* ptIn = (uchar*)imgin->imageData;
  uchar* ptOut = (uchar*)imgout->imageData;
  int taille = imgin->width*imgin->height;
  float sum;

  for(int i=0;i<taille;i++,ptIn+=3,ptOut+=3)
    {
      sum = ptIn[0] + ptIn[1] + ptIn[2];
      if(sum)
	{
	  ptOut[1] = (uchar)( (float)(ptIn[0])/sum * 255.0 );
	  ptOut[2] = (uchar)( (float)(ptIn[1])/sum * 255.0 );
	  ptOut[0] = (uchar)( sum / 3.0 );
	}
      else
	{
	  ptOut[0] = 0;
	  ptOut[1] = 0;
	  ptOut[2] = 0;
	}
    }

}

void CvtColorRGB2Irg::convert(IplImage* imgin, IplImage* s1out, IplImage* s2out, IplImage* s3out)
{
  uchar* ptIn = (uchar*)imgin->imageData;
  uchar* pts1Out = (uchar*)s1out->imageData;
  uchar* pts2Out = (uchar*)s2out->imageData;
  uchar* pts3Out = (uchar*)s3out->imageData;
  int taille = imgin->width*imgin->height;
  float sum;

  for(int i=0;i<taille;i++,ptIn+=3,pts1Out++,pts2Out++,pts3Out++)
    {
      sum = ptIn[0] + ptIn[1] + ptIn[2];
      if(sum)
	{  
	  *pts2Out = (uchar)( (float)(ptIn[0])/sum * 255.0 );
	  *pts3Out = (uchar)( (float)(ptIn[1])/sum * 255.0 );
	  *pts1Out = (uchar)( sum / 3.0 );
	}
      else
	{
	  *pts1Out = 0;
	  *pts2Out = 0;
	  *pts3Out = 0;
	}
    }
}

/* Version BGR2Trg */

/* constructor */
CvtColorBGR2Irg::CvtColorBGR2Irg(int w, int h):CvtColor(w,h,0)
{

}


CvtColorBGR2Irg::~CvtColorBGR2Irg()
{

}

void CvtColorBGR2Irg::convert(IplImage* imgin, IplImage* imgout)
{
  uchar* ptIn = (uchar*)imgin->imageData;
  uchar* ptOut = (uchar*)imgout->imageData;
  int taille = imgin->width*imgin->height;
  float sum;

  for(int i=0;i<taille;i++,ptIn+=3,ptOut+=3)
    {
      sum = ptIn[0] + ptIn[1] + ptIn[2];
      if(sum)
	{
	  ptOut[1] = (uchar)( (float)(ptIn[2])/sum * 255.0 );
	  ptOut[2] = (uchar)( (float)(ptIn[1])/sum * 255.0 );
	  ptOut[0] = (uchar)( sum / 3.0 );
	}
      else
	{
	  ptOut[0] = 0;
	  ptOut[1] = 0;
	  ptOut[2] = 0;
	}
    }

}

void CvtColorBGR2Irg::convert(IplImage* imgin, IplImage* s1out, IplImage* s2out, IplImage* s3out)
{
  uchar* ptIn = (uchar*)imgin->imageData;
  uchar* pts1Out = (uchar*)s1out->imageData;
  uchar* pts2Out = (uchar*)s2out->imageData;
  uchar* pts3Out = (uchar*)s3out->imageData;
  int taille = imgin->width*imgin->height;
  float sum;

  for(int i=0;i<taille;i++,ptIn+=3,pts1Out++,pts2Out++,pts3Out++)
    {
      sum = ptIn[0] + ptIn[1] + ptIn[2];
      if(sum)
	{
	  *pts2Out = (uchar)( (float)(ptIn[2])/sum * 255.0 );
	  *pts3Out = (uchar)( (float)(ptIn[1])/sum * 255.0 );
	  *pts1Out = (uchar)( sum / 3.0 );
	}
      else
	{
	  *pts1Out = 0;
	  *pts2Out = 0;
	  *pts3Out = 0;
	}
    }
}
