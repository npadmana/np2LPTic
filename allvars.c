#include "allvars.h"


struct io_header_1 header1, header;

int WhichSpectrum;


int SphereMode;
FILE *FdTmp, *FdTmpInput;

int Nmesh, Nsample;

long long IDStart;



char GlassFile[500];
char FileWithInputSpectrum[500];

int GlassTileFac;

double Box;
int Seed;

long long TotNumPart;

int NumPart;

int NTaskWithN;

struct part_data *P;

int Nglass;

double InitTime;
double Redshift;
double MassTable[6];


char OutputDir[100], FileBase[100];
int NumFilesWrittenInParallel;


int ThisTask, NTask;



double UnitTime_in_s, UnitLength_in_cm, UnitMass_in_g, UnitVelocity_in_cm_per_s;
double InputSpectrum_UnitLength_in_cm;
double G, Hubble;
double RhoCrit;

double Omega, OmegaLambda, OmegaDM_2ndSpecies, Sigma8;
double OmegaBaryon, HubbleParam;
double ShapeGamma;
double PrimordialIndex;
double Dplus;			/* growth factor */

#ifdef DIFFERENT_TRANSFER_FUNC
int Type, MinType, MaxType;
#endif

int WDM_On;
int WDM_Vtherm_On;
double WDM_PartMass_in_kev;
