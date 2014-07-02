#include "ContDynamicModel.h"

ContDynamicModel::ContDynamicModel()
{
  //cout << "Allocation de ContDynamicModel" << endl;

  /* Nombre de parametres a estimer pour la dynamique (par defaut) */
  this->nDynCP = 0;
  this->nDynDP = 0;

  this->dynamicID = -1;
}

ContDynamicModel::~ContDynamicModel()
{
  cout << "Destruction de ContDynamicModel" << endl;
}

double ContDynamicModel::ContEval(Sample* s1, Sample*s2)
{
  cout << "ERROR !!!! ContEval Must be DEFINED for your application !!!! " << endl;
}
