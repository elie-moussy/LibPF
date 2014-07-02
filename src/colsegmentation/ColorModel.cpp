#include "colsegmentation/ColorModel.h"

ColorModel::ColorModel(int base)
{
  this->hobj = NULL;  
  this->htotal = NULL;  
  this->histo = NULL;  

  this->valid = 1;

  string nomfic = ColorBaseType.getStr(base);
  nomfic += ".txt";

  this->cb.load(nomfic);

  /* creation des histogrammes */
  this->AllocHistos();

  /* alloc matrice de covariance */
  this->cov = cvCreateMat(2,2,CV_32FC1);

  /* RAZ des parametres */
  this->raz();
  
  cout << "--> ColorModel (Constructor) \t:\t " << this->cb.nm << " empty color model initialized" << endl;
}

ColorModel::ColorModel(string nomfic)
{
  this->valid = 1;

  /* alloc matrice de covariance */
  this->cov = cvCreateMat(2,2,CV_64FC1);

  /* charge le modele de couleur contenue dans nomfic */
  if(this->loadBasic(nomfic)) cout << "--> ColorModel (Constructor) \t:\t color model loaded from " << nomfic << endl;
  else  cout << "|ERROR|--> ColorModel (Constructor) \t:\t error loading color model ! " << endl;
}

ColorModel::~ColorModel()
{

}

void ColorModel::raz()
{
  this->mC1 = 0;
  this->mC2 = 0;
  this->mC3 = 0;

  this->sigC1 = 0;
  this->sigC2 = 0;
  this->sigC3 = 0;

  this->minC1 = 0;
  this->maxC1 = 0;
  this->minC2 = 0;
  this->maxC2 = 0;
  this->minC3 = 0;
  this->maxC3 = 0;

  cvZero(this->cov);

  if(this->htotal) cvClearHist(this->htotal);
  if(this->hobj) cvClearHist(this->hobj);
  if(this->histo) cvClearHist(this->histo);

}

int ColorModel::load(string nomfic)
{
  string chemin = icuConfigPath + "/color/" + nomfic;

  CvFileStorage* storage = cvOpenFileStorage(chemin.c_str(), 0, CV_STORAGE_READ );
  if(!storage) { cout << "|ERROR|--> ColorModel (load) \t:\t error loading " << chemin << endl; return 0; }

  /* Lecture de la base a charger */
  string filename = cvReadStringByName(storage, NULL, "nm");

  /* chargement de la base */
  filename += ".txt";
  this->cb.load(filename);

  
  /* lecture des moyennes */
  cout << "--> ColorModel (load) \t:\t loading means" << endl;
  this->mC1 = cvReadIntByName( storage, NULL, "mC1" );
  this->mC2 = cvReadIntByName( storage, NULL, "mC2" );
  this->mC3 = cvReadIntByName( storage, NULL, "mC3" );


  /* lecture des ecarts types */
  cout << "--> ColorModel (load) \t:\t loading sig" << endl;
  this->sigC1 = cvReadRealByName( storage, NULL, "sigC1" );
  this->sigC2 = cvReadRealByName( storage, NULL, "sigC2" );
  this->sigC3 = cvReadRealByName( storage, NULL, "sigC3" );

  /* lecture des min et max */
  cout << "--> ColorModel (load) \t:\t loading min and max" << endl;
  this->minC1 = cvReadIntByName( storage, NULL, "minC1" );
  this->minC2 = cvReadIntByName( storage, NULL, "minC2" );
  this->minC3 = cvReadIntByName( storage, NULL, "minC3" );

  this->maxC1 = cvReadIntByName( storage, NULL, "maxC1" );
  this->maxC2 = cvReadIntByName( storage, NULL, "maxC2" );
  this->maxC3 = cvReadIntByName( storage, NULL, "maxC3" );

  /* lecture de la covariance */
  cout << "--> ColorModel (load) \t:\t loading covariance" << endl;
  this->cov->data.fl[0] = cvReadRealByName( storage, NULL, "cov0" );
  this->cov->data.fl[1] = cvReadRealByName( storage, NULL, "cov1" );
  this->cov->data.fl[2] = cvReadRealByName( storage, NULL, "cov2" );
  this->cov->data.fl[3] = cvReadRealByName( storage, NULL, "cov3" );

  /* creation des histogrammes */
  cout << "--> ColorModel (load) \t:\t histogram allocation" << endl;
  if(this->AllocHistos())
    {
      cout << "--> ColorModel (load) \t:\t loading histograms" << endl;
      /* lecture des histogrammes */
      this->hobj->bins = cvReadByName(storage,NULL,"hobj");
      this->htotal->bins = cvReadByName(storage,NULL,"htotal");
      this->histo->bins = cvReadByName(storage,NULL,"histo");
    }

  cout << "--> ColorModel (load) \t:\t load OK" << endl;
  cvReleaseFileStorage(&storage);

  return 1;
}

int ColorModel::loadBasic(string nomfic)
{
  string chemin = icuConfigPath + "/color/" + nomfic;
  ifstream ficin;
  string tmp;
  string filename;

  cout << "--> ColorModel (loadBasic) \t:\t LOAD BASIC : " << chemin << endl;

  ficin.open(chemin.c_str(),ios::in);
  if(!ficin.good()) return 0;

  /* Lecture de la base a charger */
  ficin >> tmp;
  ficin >> filename;
  ficin >> tmp;

  /* chargement de la base */
  filename += ".txt";
  this->cb.load(filename);
  
  /* Lecture des moyennes */
  ficin >> tmp;
  ficin >> this->mC1;
  ficin >> tmp;
  ficin >> tmp;
  ficin >> this->mC2;
  ficin >> tmp;
  ficin >> tmp;
  ficin >> this->mC3;
  ficin >> tmp;

  cout << "--> ColorModel (loadBasic) \t:\t mean loaded : " << this->mC1 << " ; " << this->mC2 << " ; " << this->mC3 << "\n";

  /* Lecture des ecarts types */
  ficin >> tmp;
  ficin >> this->sigC1;
  ficin >> tmp;
  ficin >> tmp;
  ficin >> this->sigC2;
  ficin >> tmp;
  ficin >> tmp;
  ficin >> this->sigC3;
  ficin >> tmp;

  /* Lecture des min et max */
  ficin >> tmp;
  ficin >> this->minC1;
  ficin >> tmp;
  ficin >> tmp;
  ficin >> this->minC2;
  ficin >> tmp;
  ficin >> tmp;
  ficin >> this->minC3;
  ficin >> tmp;

  ficin >> tmp;
  ficin >> this->maxC1;
  ficin >> tmp;
  ficin >> tmp;
  ficin >> this->maxC2;
  ficin >> tmp;
  ficin >> tmp;
  ficin >> this->maxC3;
  ficin >> tmp;

  /* Lecture de la covariance */
  cout << "--> ColorModel (loadBasic) \t:\t loading covariance" << endl;
  ficin >> tmp;
  ficin >> this->cov->data.fl[0];
  ficin >> tmp;
  ficin >> tmp;
  ficin >> this->cov->data.fl[1];
  ficin >> tmp;
  ficin >> tmp;
  ficin >> this->cov->data.fl[2];
  ficin >> tmp;
  ficin >> tmp;
  ficin >> this->cov->data.fl[3];
  ficin >> tmp;

  /* Lecture des histogrammes */
  cout << "--> ColorModel (loadBasic) \t:\t histogram allocation" << endl;
  if(this->AllocHistos())
    {
      float valbin;
      int size[CV_MAX_DIM];

      ficin >> tmp;
      ficin >> tmp;  
      ficin >> size[0];
      ficin >> size[1];
      ficin >> tmp;
      ficin >> tmp;
      for(int i=0;i<size[0];i++)
	for(int j=0;j<size[1];j++)
	  {
	    ficin >> valbin;
	    cvSetReal2D(this->hobj->bins,i,j,valbin);
	  }
      ficin >> tmp;

      ficin >> tmp;
      ficin >> tmp;  
      ficin >> size[0];
      ficin >> size[1];
      ficin >> tmp;
      ficin >> tmp;
      for(int i=0;i<size[0];i++)
	for(int j=0;j<size[1];j++)
	  {
	    ficin >> valbin;
	    cvSetReal2D(this->htotal->bins,i,j,valbin);
	  }
      ficin >> tmp;

      ficin >> tmp;
      ficin >> tmp;  
      ficin >> size[0];
      ficin >> size[1];
      ficin >> tmp;
      ficin >> tmp;
      for(int i=0;i<size[0];i++)
	for(int j=0;j<size[1];j++)
	   {
	    ficin >> valbin;
	    cvSetReal2D(this->histo->bins,i,j,valbin);
	   }
      ficin >> tmp;

      Hist2DDisplay(this->hobj,"CHARGE hobj");
      Hist2DDisplay(this->htotal,"CHARGE htotal");
      Hist2DDisplay(this->histo,"CHARGE histo");
      cvWaitKey(0);

    }
  else
    cout << "|ERROR|--> ColorModel (loadBasic) \t:\t histogram allocation FAILED" << endl;


  ficin.close();

  return 1;
}

int ColorModel::AllocHistos()
{
  int hist_size[2];
  float rangeP1[2];
  float rangeP2[2];
  float *range[] = {rangeP1,rangeP2};
  
  switch(this->cb.pLum)
    {
    case 1:
      {
	hist_size[0] = this->cb.rC2[1]+1;
	hist_size[1] = this->cb.rC3[1]+1;

	rangeP1[0] = this->cb.rC2[0];
	rangeP1[1] = this->cb.rC2[1];
	
	rangeP2[0] = this->cb.rC3[0];
	rangeP2[1] = this->cb.rC3[1];

	this->mLum = &this->mC1;
	this->mChrom1 = &this->mC2;
	this->mChrom2 = &this->mC3;
	this->sigLum = &this->sigC1;
	this->sigChrom1 = &this->sigC2;
	this->sigChrom2 = &this->sigC3;
	this->minLum = &this->minC1;
	this->maxLum = &this->maxC1;
	this->minChrom1 = &this->minC2;
	this->maxChrom1 = &this->maxC2;
	this->minChrom2 = &this->minC3;
	this->maxChrom2 = &this->maxC3;

      }
      break;

    case 2:
      {
	hist_size[0] = this->cb.rC1[1]+1;
	hist_size[1] = this->cb.rC3[1]+1;

	rangeP1[0] = this->cb.rC1[0];
	rangeP1[1] = this->cb.rC1[1];
	
	rangeP2[0] = this->cb.rC3[0];
	rangeP2[1] = this->cb.rC3[1];

	this->mLum = &this->mC2;
	this->mChrom1 = &this->mC1;
	this->mChrom2 = &this->mC3;
	this->sigLum = &this->sigC2;
	this->sigChrom1 = &this->sigC1;
	this->sigChrom2 = &this->sigC3;
	this->minLum = &this->minC2;
	this->maxLum = &this->maxC2;
	this->minChrom1 = &this->minC1;
	this->maxChrom1 = &this->maxC1;
	this->minChrom2 = &this->minC3;
	this->maxChrom2 = &this->maxC3;

      }
      break;

    case 3:
      {
	hist_size[0] = this->cb.rC1[1]+1;
	hist_size[1] = this->cb.rC2[1]+1;

	rangeP1[0] = this->cb.rC1[0];
	rangeP1[1] = this->cb.rC1[1];
	
	rangeP2[0] = this->cb.rC2[0];
	rangeP2[1] = this->cb.rC2[1];

	this->mLum = &this->mC3;
	this->mChrom1 = &this->mC1;
	this->mChrom2 = &this->mC2;
	this->sigLum = &this->sigC3;
	this->sigChrom1 = &this->sigC1;
	this->sigChrom2 = &this->sigC2;
	this->minLum = &this->minC3;
	this->maxLum = &this->maxC3;
	this->minChrom1 = &this->minC1;
	this->maxChrom1 = &this->maxC1;
	this->minChrom2 = &this->minC2;
	this->maxChrom2 = &this->maxC2;
      }
      break;
    default:
      {
	cout << "|WARNING|--> ColorModel (load) \t:\t " << this->cb.nm <<" cannot be used for color segmentation using Luminance and chrominance approach" << endl;
	this->valid = 0;
	return 0;
      }
    }

  this->hobj = cvCreateHist(2,hist_size,CV_HIST_ARRAY,range,1);
  this->htotal = cvCreateHist(2,hist_size,CV_HIST_ARRAY,range,1);
  this->histo = cvCreateHist(2,hist_size,CV_HIST_ARRAY,range,1);

  return 1;
}

int ColorModel::save(string nomfic)
{
  string chemin = icuConfigPath + "/color/" + nomfic + ".xml";

  CvFileStorage* storage = cvOpenFileStorage(chemin.c_str(), 0, CV_STORAGE_WRITE);
  if(!storage) return 0;

  /* Ecriture de la base a charger */
  cvWriteString(storage, "nm", cb.nm.c_str());
  
  /* lecture des moyennes */
  cvWriteInt( storage, "mC1", this->mC1);
  cvWriteInt( storage, "mC2", this->mC2);
  cvWriteInt( storage, "mC3", this->mC3);

  /* lecture des ecarts types */
  cvWriteReal( storage, "sigC1", this->sigC1);
  cvWriteReal( storage, "sigC2", this->sigC2);
  cvWriteReal( storage, "sigC3", this->sigC3);

  /* lecture des min et max */
  cvWriteInt( storage, "minC1", this->minC1);
  cvWriteInt( storage, "minC2", this->minC2);
  cvWriteInt( storage, "minC3", this->minC3);

  cvWriteInt( storage, "maxC1", this->maxC1);
  cvWriteInt( storage, "maxC2", this->maxC2);
  cvWriteInt( storage, "maxC3", this->maxC3);

  /* lecture de la covariance */
  cvWriteReal( storage, "cov0", (double)(this->cov->data.fl[0]) );
  cvWriteReal( storage, "cov1", (double)(this->cov->data.fl[1]) );
  cvWriteReal( storage, "cov2", (double)(this->cov->data.fl[2]) );
  cvWriteReal( storage, "cov3", (double)(this->cov->data.fl[3]) );

  /* lecture des histogrammes */
  cvWrite(storage, "hobj", this->hobj->bins);
  cvWrite(storage, "htotal", this->htotal->bins);
  cvWrite(storage, "histo", this->histo->bins);

  cvReleaseFileStorage(&storage);

  return 1;
}


int ColorModel::saveBasic(string nomfic)
{
  string chemin = icuConfigPath + "/color/" + nomfic + ".xml";
  ofstream ficout;

  cout << "--> ColorModel (saveBasic) \t:\t SAVE BASIC" << endl;

  ficout.open(chemin.c_str(),ios::out|ios::trunc);
  if(!ficout.good()) return 0;

  /* Ecriture de la base a charger */
  ficout << "<nm>\n";
  ficout << cb.nm.c_str() << endl;
  ficout << "</nm>\n";
  
  /* Ecriture des moyennes */
  ficout << "<mC1>\n";
  ficout << this->mC1 << endl;
  ficout << "</mC1>\n";
  ficout << "<mC2>\n";
  ficout << this->mC2 << endl;
  ficout << "</mC2>\n";
  ficout << "<mC3>\n";
  ficout << this->mC3 << endl;
  ficout << "</mC3>\n";

  /* Ecriture des ecarts types */
  ficout << "<sigC1>\n";
  ficout << this->sigC1 << endl;
  ficout << "</sigC1>\n";
  ficout << "<sigC2>\n";
  ficout << this->sigC2 << endl;
  ficout << "</sigC2>\n";
  ficout << "<sigC3>\n";
  ficout << this->sigC3 << endl;
  ficout << "</sigC3>\n";

  /* Ecriture des min et max */
  ficout << "<minC1>\n";
  ficout << this->minC1 << endl;
  ficout << "</minC1>\n";
  ficout << "<minC2>\n";
  ficout << this->minC2 << endl;
  ficout << "</minC2>\n";
  ficout << "<minC3>\n";
  ficout << this->minC3 << endl;
  ficout << "</minC3>\n";

  ficout << "<maxC1>\n";
  ficout << this->maxC1 << endl;
  ficout << "</maxC1>\n";
  ficout << "<maxC2>\n";
  ficout << this->maxC2 << endl;
  ficout << "</maxC2>\n";
  ficout << "<maxC3>\n";
  ficout << this->maxC3 << endl;
  ficout << "</maxC3>\n";

  /* Ecriture de la covariance */
  ficout << "<cov0>\n";
  ficout << (double)(this->cov->data.fl[0]) << endl;
  ficout << "</cov0>\n";
  ficout << "<cov1>\n";
  ficout << (double)(this->cov->data.fl[1]) << endl;
  ficout << "</cov1>\n";
  ficout << "<cov2>\n";
  ficout << (double)(this->cov->data.fl[2]) << endl;
  ficout << "</cov2>\n";
  ficout << "<cov3>\n";
  ficout << (double)(this->cov->data.fl[3]) << endl;
  ficout << "</cov3>\n";

  /* Ecriture des histogrammes */
  int dims, size[CV_MAX_DIM];

  dims = cvGetDims( this->hobj->bins, size );
  ficout << "<htotal>\n";
  ficout << "\t<size>\n";  
  ficout << size[0] << "\t" << size[1] << endl;
  ficout << "\t</size>\n";
  ficout << "<bins>\n";
  for(int i=0;i<size[0];i++)
    for(int j=0;j<size[1];j++)
      ficout << cvGetReal2D(this->hobj->bins,i,j) << " ";
  ficout << endl;
  ficout << "</bins>\n";

  dims = cvGetDims( this->htotal->bins, size );
  ficout << "<htotal>\n";
  ficout << "\t<size>\n";  
  ficout << size[0] << "\t" << size[1] << endl;
  ficout << "\t</size>\n";
  ficout << "<bins>\n";
  for(int i=0;i<size[0];i++)
    for(int j=0;j<size[1];j++)
      ficout << cvGetReal2D(this->htotal->bins,i,j) << " ";
  ficout << endl;
  ficout << "</bins>\n";

  dims = cvGetDims( this->histo->bins, size );
  ficout << "<htotal>\n";
  ficout << "\t<size>\n";  
  ficout << size[0] << "\t" << size[1] << endl;
  ficout << "\t</size>\n";
  ficout << "<bins>\n";
  for(int i=0;i<size[0];i++)
    for(int j=0;j<size[1];j++)
      ficout << cvGetReal2D(this->histo->bins,i,j) << " ";
  ficout << endl;
  ficout << "</bins>\n";

  ficout.close();

  return 1;
}


void ColorModel::disp()
{
  cout << "--> ColorModel (disp) \t:\t Color base infos :" << endl;
  this->cb.disp();
  cout << "--> ColorModel (disp) \t:\t Statistics of " << this->cb.nm << " color base" << endl;
  cout << "\tChanel\tmean\tsig\tmin\tmax" << endl;
  cout << "\t1\t" << this->mC1 << "\t" << this->sigC1 << "\t" << this->minC1 << "\t" << this->maxC1 << "\t" << endl;
  cout << "\t2\t" << this->mC2 << "\t" << this->sigC2 << "\t" << this->minC2 << "\t" << this->maxC2 << "\t" << endl;
  cout << "\t3\t" << this->mC3 << "\t" << this->sigC3 << "\t" << this->minC3 << "\t" << this->maxC3 << "\t" << endl << endl;
  cout << "\tcov =\t";
  DispVector<float>(this->cov->data.fl,4);
  cout << endl;

}
