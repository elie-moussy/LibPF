/***************************************************************************
                          multibspline2d.cpp  -  description
                             -------------------
    begin                :
    copyright            : (C) 2004 Ludovic Brethes
    email                : lbrethes@laas.fr
 ***************************************************************************/


#include "bspline/multibspline2d.h"
#define EPAISSEUR 1

/*
  Constructeur :
  \param ficspline : nom du fichier contenant les points de controles de la forme
  \param ficconfig : nom de fichier contenant la description des differentes parties 
  et configurations de la forme
*/
MultiBSpline2D::MultiBSpline2D(char *ficspline, char *ficconfig)
{
/* par defaut on cherche le rectangle englobant de la forme complete */
  numenglob=0;

  /* Chargement de la forme (points de controles de la spline) */
  LoadSpline(ficspline);

  /* Chargement de la configuration de la spline (parties configurations ...) */
  LoadConfig(ficconfig);
 
}

MultiBSpline2D::~MultiBSpline2D()
{

}



void MultiBSpline2D::LoadSpline(char *fichier)
{
  static double ctlx[100];
  static double ctly[100];
  static double tmp_knots[100];
  char line[80];
  FILE * fp;
  int minx,miny,maxx,maxy,midx,midy;

  fp=fopen(fichier,"r");
  if (fp==NULL)
    cerr << "|ERROR|--> MultiBSpline2D (LoadSpline) \t:\t failed to load " << fichier << "\n";
  else
    cout << "--> MultiBSpline2D (LoadSpline) \t:\t Loaded " << fichier <<"\n";


  /****************************  Chargement a partir du fichier des points de ctrl **************************************/
  knot_nr=1;
  minx=miny=1000;
  maxx=maxy=-1000;
  while(!feof(fp))
    {
      fgets(line,80,fp);
      sscanf(line," %lf %lf ",&ctlx[knot_nr],&ctly[knot_nr]);
      if(ctlx[knot_nr]<minx)
	minx=cvRound(ctlx[knot_nr]);
      if(ctlx[knot_nr]>maxx)
	maxx=cvRound(ctlx[knot_nr]);
      if (ctly[knot_nr]<miny)
	miny=cvRound(ctly[knot_nr]);
      if(ctly[knot_nr]>maxy)
	maxy=cvRound(ctly[knot_nr]);
      
      
      tmp_knots[knot_nr]=knot_nr++;
      
    }
  ctlx[0]=ctlx[1];
  ctly[0]=ctly[1];
  tmp_knots[0]=0;
  midx=(minx+maxx)/2;
  midy=(miny+maxy)/2;

  /******************** Allocations/Init des liste de pts de ctrl et de divers vecteurs (normales, coefficients...) *************************/
  intnum=knot_nr-2;
  density=5;
  degree = 2;
  /* liste des points originaux */
  controlpts_init=new Point2D[knot_nr];
  /* liste des points apres transformation */
  controlpts_trans = new CvPoint[knot_nr];
  /* liste des points pour trace  */
  controlpts_draw = new CvPoint[knot_nr+4];
  /* coefficients x */
  coeffx= new double[knot_nr];
  /* coefficients y */
  coeffy= new double[knot_nr]; 
  /* Liste des points generes correspondant a la forme */
  points = new Point2D[intnum*(density+1)];
  /* Liste des normales en chaque point de la forme generee */
  normals = new Vector2D[intnum*(density+1)];  
  /* nombre de points pour le trace de la forme */
  numpoints=intnum*(density+1);
 
  /* Chargement des pts de ctrl et positionnemet a l'origine (pts init) */
  for (int k=0;k<knot_nr;k++)
    controlpts_init[k]=Point2D(ctlx[k]-midx,ctly[k]-midy);
  
  /* Allocation du bon nombre de noeuds puis recopie */
  knots = new double[knot_nr];
  for (int i=0;i<knot_nr;i++)
    knots[i]=tmp_knots[i];


  /* Liste de points pour trace de la forme avec cvPolly */
  pts = (CvPoint*)calloc(NBMAXPTS,sizeof(CvPoint));

}


/**
   Fonction de chargement de la description multi partie de la forme
   - on definit ici le nombre de morceaux constituant la forme
   - on decrit les associations de parties --> differentes configurations de la forme
*/
void MultiBSpline2D::LoadConfig(char *fichier)
{
  //int verbose = 1;
  FILE * fp,*fpcur;
  
  fp=fopen(fichier,"r");
  if (fp==NULL)
    {
      cerr << "|ERROR|--> MultiBSpline2D (LoadConfig) \t:\t failed to load config from " << fichier << "\n";
      return;
    }
  else
    cout << "--> MultiBSpline2D (LoadConfig) \t:\t Config " << fichier <<" loaded\n";

  /********************************************************************************/
  /*       Description des differentes parties de la forme (ex : paume, doigts)   */
  /********************************************************************************/

  /* Chargement du nb de parties dans la forme */
  fpcur = FindStr(fp,"<nbparts>");
  if(feof(fpcur)) 
    {
      printf("|ERROR|--> MultiBSpline2D (LoadConfig) \t:\t <nbparts> not found\n");
      return;
    }
  fscanf(fpcur,"%d",&nbparts);

  /* Chargement du nombre de points de controle par partie constituant la forme */
  fpcur = FindStr(fp,"<nbctrlpts_bypart>");
  if(feof(fpcur)) 
    {
      printf("|ERROR|--> MultiBSpline2D (LoadConfig) \t:\t <nbctrlpts_bypart> not found\n");
      return;
    }
  nbctrlpts_bypart = (int*)calloc(nbparts,sizeof(int));
  for(int i=0;i<nbparts;i++) 
    fscanf(fpcur,"%d",nbctrlpts_bypart+i);
    
  /* Chargement des numeros de pts de ctrl de chaque partie de la forme */
  numctrlpts_bypart = (int **)calloc(nbparts,sizeof(int*)); 
  fpcur = FindStr(fp,"<numctrlpts_bypart>");
  if(feof(fpcur))
    {
      printf("|ERROR|--> MultiBSpline2D (LoadConfig) \t:\t <numctrlpts_bypart> not found\n");
      return;
    }
  for(int i=0;i<nbparts;i++)
    {
      //Alloc
      numctrlpts_bypart[i] = (int*)calloc(nbctrlpts_bypart[i],sizeof(int));

      //Lecture
      for(int j=0;j<nbctrlpts_bypart[i];j++)
	fscanf(fpcur,"%d",numctrlpts_bypart[i]+j);
    }

  /*************************************************************************************/
  /*       Description des masques correspondant aux differentes parties de la forme   */
  /*************************************************************************************/
  /* Chargement du nombre de points de controle par masque constituant la forme */
  fpcur = FindStr(fp,"<nbctrlpts_bymask>");
  if(feof(fpcur)) 
    {
      printf("|ERROR|--> MultiBSpline2D (LoadConfig) \t:\t <nbctrlpts_bymask> not found\n");
      return;
    }
  nbctrlpts_bymask = (int*)calloc(nbparts,sizeof(int));
  for(int i=0;i<nbparts;i++) 
    fscanf(fpcur,"%d",nbctrlpts_bymask+i);
    
  /* Chargement des numeros de pts de ctrl de chaque masque de la forme */
  numctrlpts_bymask = (int **)calloc(nbparts,sizeof(int*)); 
  fpcur = FindStr(fp,"<numctrlpts_bymask>");
  if(feof(fpcur))
    {
      printf("|ERROR|--> MultiBSpline2D (LoadConfig) \t:\t <numctrlpts_bymask> not found\n");
      return;
    }
  for(int i=0;i<nbparts;i++)
    {
      //Alloc
      numctrlpts_bymask[i] = (int*)calloc(nbctrlpts_bymask[i],sizeof(int));

      //Lecture
      for(int j=0;j<nbctrlpts_bymask[i];j++)
	fscanf(fpcur,"%d",numctrlpts_bymask[i]+j);
    }

  /********************************************************************************/
  /*       Description des differentes configurations (associations de parties)   */
  /********************************************************************************/

  /* Chargement du nb de configurations de la forme */
  fpcur = FindStr(fp,"<nbconfigs>");
  if(feof(fpcur)) 
    {
      printf("|ERROR|--> MultiBSpline2D (LoadConfig) \t:\t <nbconfigs> not found\n");
      return;
    }
  fscanf(fpcur,"%d",&nbconfigs);

  /* Chargement du nombre de parties par configuration */
  fpcur = FindStr(fp,"<nbparts_byconfig>");
  if(feof(fpcur)) 
    {
      printf("|ERROR|--> MultiBSpline2D (LoadConfig) \t:\t <nbparts_byconfig> not found\n");
      return;
    }
  nbparts_byconfig = (int*)calloc(nbconfigs,sizeof(int));
  for(int i=0;i<nbconfigs;i++) 
    fscanf(fpcur,"%d",nbparts_byconfig+i);

  /* Chargement des configurations = suite de numero de partie a utiliser/combiner */
  numparts_byconfig = (int **)calloc(nbconfigs,sizeof(int*)); 
  ispart_byconfig = (int**)calloc(nbconfigs,sizeof(int*));
  fpcur = FindStr(fp,"<numparts_byconfig>");
  if(feof(fpcur))
    {
      printf("|ERROR|--> MultiBSpline2D (LoadConfig) \t:\t <numparts_byconfig> not found\n");
      return;
    }
  for(int i=0;i<nbconfigs;i++)
    {
      //Alloc
      numparts_byconfig[i] = (int*)calloc(nbparts_byconfig[i],sizeof(int));
      ispart_byconfig[i] = (int*)calloc(nbparts,sizeof(int));

      //Lecture
      for(int j=0;j<nbparts_byconfig[i];j++)
	{
	  fscanf(fpcur,"%d",numparts_byconfig[i]+j);
	  ispart_byconfig[i][numparts_byconfig[i][j]]=1;
	}
    }


  // for(int i=0;i<nbconfigs;i++)
//     {
//       printf("Config %d :",i);
//       //Lecture
//       for(int j=0;j<nbparts_byconfig[i];j++)
// 	printf("%d ",ispart_byconfig[i][j]);
//       printf("\n");
//     }

  //numctrlpts_bypart[0] = {0,7,8,14,15,20,21,27,28,32,33,34,35};
  //numctrlpts_bypart[1] = {1,2,3,4,5,6};
  //numctrlpts_bypart[2] = {9,10,11,12,13};
  //numctrlpts_bypart[3] = {16,17,18,19};
  //numctrlpts_bypart[4] = {22,23,24,25,26};
  //numctrlpts_bypart[5] = {29,30,31};
  
  /* Generation des configurations */
  genconfigs();
}

/* 
   Generation des configurations :
   D'apres la description chargee on genere une liste de vecteur de num de pts de ctrl qui correspond
   a la liste des configurations disponibles dans cette description
*/
void MultiBSpline2D::genconfigs()
{
  int index;

  nbctrlpts_byconfig = (int*)calloc(nbconfigs,sizeof(int));
  numctrlpts_byconfig = (int**)calloc(nbconfigs,sizeof(int*));

  /* Pour chaque configuration */
  for(int i=0;i<nbconfigs;i++)
    {  
      /* Allouer le vecteur de num de controle correspondant */
      //Compter de nombre de pts de controle = somme des numctrlpts_bypart de chaque partie de la config
      nbctrlpts_byconfig[i]=0;
      for(int j=0;j<nbparts_byconfig[i];j++)
	nbctrlpts_byconfig[i]+=nbctrlpts_bypart[numparts_byconfig[i][j]];

      //printf("Config %d ---> nb ctrlpts %d\n",i,nbctrlpts_byconfig[i]);
      numctrlpts_byconfig[i]=(int*)calloc(nbctrlpts_byconfig[i],sizeof(int));

      /* Charger (par concatenation) les pts de ctrl des differentes parties */
      index = 0;
      //printf("Pour les parties de 0 a %d",nbparts_byconfig[i]);
      for(int j=0;j<nbparts_byconfig[i];j++)
	{
	  //printf("partie %d --> copie de %d points",i,nbctrlpts_bypart[numparts_byconfig[i][j]]);
	  for(int k=0;k<nbctrlpts_bypart[numparts_byconfig[i][j]];k++)
	    {
	      numctrlpts_byconfig[i][index]=numctrlpts_bypart[numparts_byconfig[i][j]][k];
	      index++;
	    }
	}

      //  printf("Config %d\n",i);
      //       for(int k=0;k<nbctrlpts_byconfig[i];k++)
      // 	printf("%d ",numctrlpts_byconfig[i][k]);
      //       printf("\n");

      /* Classer les num de pts de ctrl pour permettre l'affichage */
      //qsort(numctrlpts_byconfig[i],nbctrlpts_byconfig[i], sizeof(int), compare_int);
      qsort(numctrlpts_byconfig[i],nbctrlpts_byconfig[i], sizeof(int), cmp);

      //      for(int k=0;k<nbctrlpts_byconfig[i];k++)
      // 	printf("%d ",numctrlpts_byconfig[i][k]);
      //       printf("\n\n");

      /* Visualisation des configurations */ 
    }

}

/** No descriptions */
void MultiBSpline2D::genpoints()
{
  int i,ii,l2;
  float u;
  
  l2=intnum+degree-1;
  numpoints=0;

  for (i=degree-1; i<l2; i++)
    {
      if(knots[i+1]>knots[i])
	for(ii=0; ii<=density; ii++)
	  {
	    u=knots[i]+ii*(knots[i+1]-knots[i])/density;
	    points[numpoints]=deboor2D(u,i);
	    numpoints+=1;
	  }
    }

  //Generation des normales aux segments
  gennormals();
}


/** For each of the generated points it creates a vector that is
    normal to the spline   */ 
void MultiBSpline2D::gennormals()
{
  Vector2D v;
  for (int i=1;i<numpoints-1;i++)
    {
      v = points[i+1] - points[i-1];
      normals[i] = v.normal();
      //printf("Point %d  vecteur = %f %f  normale = %f %f\n",i,v.myx,v.myy,normals[i].myx,normals[i].myy);
    }
  v=points[1]-points[0];
  normals[0]=v.normal();
  v=points[numpoints-1]-points[numpoints-2];
  normals[numpoints-1]=v.normal();
}

/** this method generates new control points translating, rotating and
    scaling the base ones  */ 
void MultiBSpline2D::transform(double x, double y, double theta, double scale, int model)
{
  Point2D tmp;

  //Teste la validite du modele
  if(model>=nbconfigs)
    {
      printf("|ERROR|--> MultiBSpline2D (transform) \t:\t The model %d does not exist there are only %d models loaded (0..nbconfigs-1)\n",model,nbconfigs);
      return;
    }

  //Translation  
  Point2D transl(x,y);
  
  Posmin.x = 1000;
  Posmin.y = 1000;
  Posmax.x = -1;
  Posmax.y = -1;
  PosminGlobal.x = 1000;
  PosminGlobal.y = 1000;
  PosmaxGlobal.x = -1;
  PosmaxGlobal.y = -1;

  /* indice qui va parcourir les differents points de ctrl de la forme a englober */
  int id=0;

  //int j=0;
  for (int i=0;i<knot_nr;i++)
    {
      //if( (i<22)||(i>26) )//&&(i<29))||(i>31) )
	//{
	  tmp=controlpts_init[i];
	  //mise a l'echelle
	  tmp=scale*tmp;
	  //rotation
	  if (theta!=0.0)
	    tmp.rotate(theta);
	  
	  //controlpts_trans[i]=tmp+transl;
	  controlpts_trans[i].x=((int)(tmp.myx+transl.myx));
	  controlpts_trans[i].y=((int)(tmp.myy+transl.myy));
	  
	  //Recup des min et max
	  if(numenglob>=0)
	    {
	      if(numctrlpts_bypart[numenglob][id]==i)
		{
		  id++;
		  /* recherche englober partie */
		  if(controlpts_trans[i].x<Posmin.x) Posmin.x=controlpts_trans[i].x;
		  else
		    if(controlpts_trans[i].x>Posmax.x) Posmax.x=controlpts_trans[i].x;
		  if(controlpts_trans[i].y<Posmin.y) Posmin.y=controlpts_trans[i].y;
		  else
		    if(controlpts_trans[i].y>Posmax.y) Posmax.y=controlpts_trans[i].y;
		}
	    }
	  // else
// 	    {
	      /* on englobe la forme complete */
	      if(controlpts_trans[i].x<PosminGlobal.x) PosminGlobal.x=controlpts_trans[i].x;
	      else
		if(controlpts_trans[i].x>PosmaxGlobal.x) PosmaxGlobal.x=controlpts_trans[i].x;
	      if(controlpts_trans[i].y<PosminGlobal.y) PosminGlobal.y=controlpts_trans[i].y;
	      else
		if(controlpts_trans[i].y>PosmaxGlobal.y) PosmaxGlobal.y=controlpts_trans[i].y;

	      //}

	  //  controlpts2[i]=controlpts[i];       // virer 
	  //j++;
	  //	}
    }

  //Generation des points avec la bonne config
  for(int i=0;i<nbctrlpts_byconfig[model];i++)
    controlpts_draw[i] = controlpts_trans[numctrlpts_byconfig[model][i]];

  intnum = nbctrlpts_byconfig[model]-2;

  //Generation des points a partir des points de controle de la config selectionnee
  genpoints();
}

/** Draws the spline with the current transformation on the image */ 
void MultiBSpline2D::draw(IplImage * img,CvScalar color, int fill, int epaisseur)
{

  if(fill)
    {
      //Trace rempli
      for (int i=0;i<numpoints;i++)
	{
	  pts[i].x = (int)points[i].myx;
	  pts[i].y = (int)points[i].myy;
	}      
      cvFillPoly(img,&pts,&numpoints,1,color,8,0 );
    }
  else
    {
      //Trace ligne
      for (int i=1;i<numpoints;i++)
	{
    //cvLine(img,cvPoint(cvRound(points[i-1].myx),cvRound(points[i-1].myy)),cvPoint(cvRound(points[i].myx),cvRound(points[i].myy)),color,2);
	  cvLine(img,cvPoint((int)points[i-1].myx,(int)points[i-1].myy),cvPoint((int)points[i].myx,(int)points[i].myy),color,epaisseur);
	}
    } 
}

void MultiBSpline2D::draw_rect(IplImage *img,CvScalar color,int epaisseur)
{
  cvRectangle(img,Posmin,Posmax,color,epaisseur,8,0);
}

void MultiBSpline2D::draw_rect_mask(IplImage *img)
{
  cvRectangle(img,Posmin,Posmax,cvScalar(1),-1,8,0);
}

void MultiBSpline2D::draw_global_rect(IplImage *img,CvScalar color)
{
  cvRectangle(img,PosminGlobal,PosmaxGlobal,color,-1,8,0);
}

void MultiBSpline2D::draw_global_rect_mask(IplImage *img)
{
  cvRectangle(img,PosminGlobal,PosmaxGlobal,cvScalar(7),-1,8,0);
}

/** Draw normals */
void MultiBSpline2D::draw_normals(IplImage * img,int size,CvScalar color, int epaisseur)
{
  Point2D p1,p2;

  for(int i=0;i<numpoints;i++)
    {
      p1 = points[i]+(normals[i]*size);
      p2 = points[i]-(normals[i]*size);

      cvLine(img,cvPoint((int)p1.myx,(int)p1.myy),cvPoint((int)p2.myx,(int)p2.myy),color,epaisseur,8,0);
      //cvLine(img,cvPoint((int)points[i].myx,(int)points[i].myy),cvPoint((int)p1.myx,(int)p1.myy),CV_RGB(0,0,255),2,8,0);
      //cvLine(img,cvPoint((int)points[i].myx,(int)points[i].myy),cvPoint((int)p2.myx,(int)p2.myy),CV_RGB(255,0,0),2,8,0);
    }



}

/* Trace les points de controle */
void MultiBSpline2D::draw_ctrlpoints(IplImage * img,CvScalar color)
{
  /* Trace des Points de Controle */
  for (int i=0;i<knot_nr;i++)
    {
      cvCircle(img,controlpts_trans[i],2,color,-1,8,0);
    }
}

/* Trace les zones de la main */
void MultiBSpline2D::draw_mask(IplImage * img)
{ 
  for(int i=0;i<nbparts;i++)
    draw_one_mask(img,i);
}

/* Trace les zones de la main */
void MultiBSpline2D::draw_mask_fcol(IplImage * img)
{ 
  for(int i=0;i<nbparts;i++)
    draw_one_mask_fcol(img,i);
}

/* Trace les zones de la main */
void MultiBSpline2D::draw_one_mask_fcol(IplImage * img, int masknum)
{
  int nb;
  
  for(int j=0;j<nbctrlpts_bymask[masknum];j++)
    {
      controlpts_draw[j] = controlpts_trans[numctrlpts_bymask[masknum][j]];
    }
  nb = nbctrlpts_bymask[masknum];
  cvFillPoly(img,&controlpts_draw,&nb,1,SCalcFcol(masknum+1),8,0 );

}

void MultiBSpline2D::draw_one_mask(IplImage * img, int masknum)
{
  int nb;
  
  for(int j=0;j<nbctrlpts_bymask[masknum];j++)
    {
      controlpts_draw[j] = controlpts_trans[numctrlpts_bymask[masknum][j]];
    }
  nb = nbctrlpts_bymask[masknum];
  cvFillPoly(img,&controlpts_draw,&nb,1,cvScalar(masknum+1),8,0 );

  //DispFcol(img,"tt",1);
  //cvWaitKey(0);

}

/* trace les zones dans le rectangle englobant qui ne sont pas dans la main (non main) */
void MultiBSpline2D::draw_all_regions_fcol(IplImage * img)
{
  //Trace du carre englobant
  cvRectangle(img,PosminGlobal,PosmaxGlobal,cvScalar(255),-1,8,0);

  //Trace des regions de la main
  this->draw_mask_fcol(img);
}

/* trace les zones dans le rectangle englobant qui ne sont pas dans la main (non main) */
void MultiBSpline2D::draw_all_regions(IplImage * img)
{
  //Trace du carre englobant
  cvRectangle(img,PosminGlobal,PosmaxGlobal,cvScalar(nbparts+1),-1,8,0);

  //Trace des regions de la main
  this->draw_mask(img);
}

/** Draws a mask corresponding to the curent transformation state */
void MultiBSpline2D::DrawMask(IplImage * img)
{

  //Trace rempli
  for (int i=0;i<numpoints;i++)
    {
      pts[i].x = (int)points[i].myx;
      pts[i].y = (int)points[i].myy;
    }      
  cvFillPoly(img,&pts,&numpoints,1,cvScalar(255),8,0 );
}

/** ONLY FOR DEGREE 2 */
inline Point2D MultiBSpline2D::deboor2D(double u, int interval)
{
  register int j;
  double t1,t2;
  
  for (j=interval-1; j<=interval+1; j++)
    {
      //coeffx[j]=controlpts2[j].myx;
      //coeffy[j]=controlpts2[j].myy;
      //if( (j<22)||(j>26) )
      //{
      
      coeffx[j]=controlpts_draw[j].x;
      coeffy[j]=controlpts_draw[j].y;
      // 	}
      //       else printf("j = %d\n",j);
    }
  
  //  for (k=1; k<= 2; k++)
  // for ( j=interval+1 ;j>=interval; j--)
  j=interval+1;
  {
    t1=  (knots[j+1] - u )/(knots[j+1]-knots[j-1]);
    t2=  1.0-t1;
    
    coeffx[j]=t1* coeffx[j-1]+t2* coeffx[j];
    coeffy[j]=t1* coeffy[j-1]+t2* coeffy[j];
  }
  j--;
  {
    t1=  (knots[j+1] - u )/(knots[j+1]-knots[j-1]);
    t2=  1.0-t1;
    
    coeffx[j]=t1* coeffx[j-1]+t2* coeffx[j];
    coeffy[j]=t1* coeffy[j-1]+t2* coeffy[j];
  }   
  j=interval+1;
  {
    t1=  (knots[j] - u )/(knots[j]-knots[j-1]);
    t2=  1.0-t1;
    
    coeffx[j]=t1* coeffx[j-1]+t2* coeffx[j];
    coeffy[j]=t1* coeffy[j-1]+t2* coeffy[j];
  }
  
  return Point2D(coeffx[interval+1],coeffy[interval+1]);
}
