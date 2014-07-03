/******************************************************************************
 * Programme d'exemple permettant de :
 * - Lire la fréquence du processeur
 * - Accéder à l'instruction RDTSC
 * - Chronométrer très précisément une durée.
 *
 * Ce code est prévu pour fonctionner sous Linux ou sous Windows.
 *
 * ATTENTION: Ce programme ne fonctionne que sur des processeurs compatibles
 * Intel Pentium ou supérieur (à cause de l'instruction RDTSC).
 *
 * CE CODE EST SOUS LICENCE GPL : http://www.fsf.org/licenses/gpl.html
 *
 * Historique :
 * - 8 septembre 2003 : Correction pour Visual C++, ce naze ne sait pas 
 *   convertir des uint64 en double, j'ai fait un ptit hack tout naze ...
 * - 5 septembre 2003 : Portage pour Visual C++
 * - 29 mars 2003     : Création, code pour Linux sous GCC, 
 *                      et Borland C++ Builder sous Windows
 *
 * Par Haypo (victor.stinner@haypocalc.com) - http://www.haypocalc.com/
 *****************************************************************************/

/**************************************************************************************/
/* Adapte par Ludovic Brethes simplement pour l'utiliser comme librairie de fonctions */
/**************************************************************************************/


#include "LibPF/timeutils/temps_cpu.h"

static double cpu_avant = 0;
static double cpu_apres = 0;
static double cpu_duree = 0;
static double cpu_frequence = 0;


//---------------------------------------------------------------------------

// Instruction RDTSC du processeur Pentium
double RDTSC(void)
{
#ifdef linux
  unsigned long long x;
  __asm__ volatile  (".byte 0x0f, 0x31" : "=A"(x));
  return (double)x;
#else
  unsigned long a, b;
  double x;

#ifdef _MSC_VER
  // Code pour Visual C++
  __asm
  {
    RDTSC
#else
  // Code pour Borland et autres
  asm
  {
    db 0x0F,0x31 // instruction RDTSC
#endif
    mov [a],eax
    mov [b],edx
  }
  x  = b;
  x *= 4294967296;
  x += a;
  return x;
#endif
}

//---------------------------------------------------------------------------

// Affichage d'une fréquence en utilisant le suffixe adapté (GHz, MHz, KHz, Hz)
void AfficheFrequence (double frequence)
{
  if (1e9<frequence)
    printf ("%.1f GHz\n", frequence/1e9);
  else if (1e6<frequence)
    printf ("%.1f MHz\n", frequence/1e6);
  else if (1e3<frequence)
    printf ("%.1f KHz\n", frequence/1e3);
  else
    printf ("%.1f Hz\n", frequence);
}

//---------------------------------------------------------------------------

// Lit la fréquence du processeur
// Renvoie la fréquence en Hz dans 'frequence' si le code de retour est
// différent de 1. Renvoie 0 en cas d'erreur.
int LitFrequenceCpu (double* frequence)
{
#ifdef linux
  const char* prefixe_cpu_mhz = "cpu MHz";
  FILE* F;
  char ligne[300+1];
  char *pos;
  int ok=0;

  // Ouvre le fichier
  F = fopen(NOMFICH_CPUINFO, "r");
  if (!F) return 0;

  // Lit une ligne apres l'autre
  while (!feof(F))
  {
    // Lit une ligne de texte
    fgets (ligne, sizeof(ligne), F);

    // C'est la ligne contenant la frequence?
    if (!strncmp(ligne, prefixe_cpu_mhz, strlen(prefixe_cpu_mhz)))
    {
      // Oui, alors lit la frequence
      pos = strrchr (ligne, ':') +2;
      if (!pos) break;
      if (pos[strlen(pos)-1] == '\n') pos[strlen(pos)-1] = '\0';
      strcpy (ligne, pos);
      strcat (ligne,"e6");
      *frequence = atof (ligne);
      ok = 1;
      break;
    }
  }
  fclose (F);
  return ok;
#else
  uint64 Fwin;
  uint64 Twin_avant, Twin_apres;
  double Tcpu_avant, Tcpu_apres;
  double Fcpu;

  // Lit la frequence du chronomêtre Windows
  if (!QueryPerformanceFrequency((LARGE_INTEGER*)&Fwin)) return 0;
  printf ("Frequence du compteur Windows = ");
  AfficheFrequence (uint64_to_double(Fwin));

  // Avant
  Tcpu_avant = RDTSC();
  QueryPerformanceCounter((LARGE_INTEGER*)&Twin_avant);

  // Attend quelques itérations (10 000) du chronomètre Windows
  do
  {
    QueryPerformanceCounter((LARGE_INTEGER*)&Twin_apres);
  } while (Twin_apres-Twin_avant < 10000);

  // Apres
  Tcpu_apres = RDTSC();
  QueryPerformanceCounter((LARGE_INTEGER*)&Twin_apres);

  // Calcule la fréquence en MHz
  Fcpu  = (Tcpu_apres - Tcpu_avant);
  Fcpu *= uint64_to_double(Fwin);
  Fcpu /= uint64_to_double(Twin_apres - Twin_avant);
  *frequence = Fcpu;
  return 1;
#endif
}

//---------------------------------------------------------------------------

//Fonction d'init de prise de temps
void get_time_start()
{
//   if (LitFrequenceCpu(&cpu_frequence)) {
//     printf ("Frequence du processeur = ");
//     AfficheFrequence(cpu_frequence);
//   }
//   else
//     printf ("ERREUR : Impossible de lire la frequence du processeur !\n");

  LitFrequenceCpu(&cpu_frequence);
  cpu_avant = RDTSC();
}

//---------------------------------------------------------------------------

//Fonction de mesure du temps
double get_time_stop(int disp)
{
  cpu_apres = RDTSC();
  cpu_duree = (cpu_apres-cpu_avant)/cpu_frequence;
  if(disp) printf ("Duree du calcul : %.1f ms\n",cpu_duree*1000);
  return cpu_duree*1000;
}

//---------------------------------------------------------------------------

//Fonction qui fait une pause
void pause(int nbms)
{
  if(!cpu_frequence) LitFrequenceCpu(&cpu_frequence);
  cpu_duree = 0;
  cpu_avant = RDTSC();
  while(nbms-cpu_duree)
    {
      cpu_apres = RDTSC();
      cpu_duree = 1000*(cpu_apres-cpu_avant)/cpu_frequence;
    }
}
