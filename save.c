#include <math.h>
#include <stdlib.h>

#include "allvars.h"
#include "proto.h"
#include "utils.h"
#include "mpi.h"



void write_particle_data(void)
{
  int nprocgroup, groupTask, masterTask;

  if(ThisTask == 0)
    printf("\nwriting initial conditions... \n");


  if((NTask < NumFilesWrittenInParallel))
    {
      printf
	("Fatal error.\nNumber of processors must be a smaller or equal than `NumFilesWrittenInParallel'.\n");
      FatalError(24131);
    }


  nprocgroup = NTask / NumFilesWrittenInParallel;

  if((NTask % NumFilesWrittenInParallel))
    nprocgroup++;

  masterTask = (ThisTask / nprocgroup) * nprocgroup;


  for(groupTask = 0; groupTask < nprocgroup; groupTask++)
    {
      if(ThisTask == (masterTask + groupTask))	/* ok, it's this processor's turn */
	save_local_data();

      /* wait inside the group */
      MPI_Barrier(MPI_COMM_WORLD);
    }

  if(ThisTask == 0)
    printf("done with writing initial conditions.\n");
}




void save_local_data(void)
{
#define BUFFER 10
  size_t bytes;
  float *block;
  int *blockid;
  long long *blocklongid;
  int blockmaxlen, maxidlen, maxlongidlen;
  int4byte dummy;
  FILE *fd;
  char buf[300];
  int i, k, pc;
  double meanspacing, shift_gas, shift_dm;


  if(NumPart == 0)
    return;

  if(NTaskWithN > 1)
    sprintf(buf, "%s/%s.%d", OutputDir, FileBase, ThisTask);
  else
    sprintf(buf, "%s/%s", OutputDir, FileBase);

  if(!(fd = fopen(buf, "w")))
    {
      printf("Error. Can't write in file '%s'\n", buf);
      FatalError(10);
    }

  for(i = 0; i < 6; i++)
    {
      header.npart[i] = 0;
      header.npartTotal[i] = 0;
      header.mass[i] = 0;
    }

  header.npart[1] = NumPart;
  header.npartTotal[1] = TotNumPart;
  header.npartTotal[2] = (TotNumPart >> 32);
  header.mass[1] = (Omega) * 3 * Hubble * Hubble / (8 * PI * G) * pow(Box, 3) / TotNumPart;

  header.time = InitTime;
  header.redshift = 1.0 / InitTime - 1;

  header.flag_sfr = 0;
  header.flag_feedback = 0;
  header.flag_cooling = 0;
  header.flag_stellarage = 0;
  header.flag_metals = 0;

  header.num_files = NTaskWithN;

  header.BoxSize = Box;
  header.Omega0 = Omega;
  header.OmegaLambda = OmegaLambda;
  header.HubbleParam = HubbleParam;

  header.flag_stellarage = 0;
  header.flag_metals = 0;
  header.hashtabsize = 0;

  dummy = sizeof(header);
  my_fwrite(&dummy, sizeof(dummy), 1, fd);
  my_fwrite(&header, sizeof(header), 1, fd);
  my_fwrite(&dummy, sizeof(dummy), 1, fd);


  meanspacing = Box / pow(TotNumPart, 1.0 / 3);
  shift_gas = -0.5 * (Omega - OmegaBaryon) / (Omega) * meanspacing;
  shift_dm = +0.5 * OmegaBaryon / (Omega) * meanspacing;


  if(!(block = malloc(bytes = BUFFER * 1024 * 1024)))
    {
      printf("failed to allocate memory for `block' (%g bytes).\n", (double)bytes);
      FatalError(24);
    }

  blockmaxlen = bytes / (3 * sizeof(float));

  blockid = (int *) block;
  blocklongid = (long long *) block;
  maxidlen = bytes / (sizeof(int));
  maxlongidlen = bytes / (sizeof(long long));

  /* write coordinates */
  dummy = sizeof(float) * 3 * NumPart;
  my_fwrite(&dummy, sizeof(dummy), 1, fd);
  for(i = 0, pc = 0; i < NumPart; i++)
    {
      for(k = 0; k < 3; k++)
	{
	  block[3 * pc + k] = P[i].Pos[k];
	}

      pc++;

      if(pc == blockmaxlen)
	{
	  my_fwrite(block, sizeof(float), 3 * pc, fd);
	  pc = 0;
	}
    }
  if(pc > 0)
    my_fwrite(block, sizeof(float), 3 * pc, fd);
  my_fwrite(&dummy, sizeof(dummy), 1, fd);



  /* write velocities */
  dummy = sizeof(float) * 3 * NumPart;
  my_fwrite(&dummy, sizeof(dummy), 1, fd);
  for(i = 0, pc = 0; i < NumPart; i++)
    {
      for(k = 0; k < 3; k++)
	block[3 * pc + k] = P[i].Vel[k];

      pc++;

      if(pc == blockmaxlen)
	{
	  my_fwrite(block, sizeof(float), 3 * pc, fd);
	  pc = 0;
	}
    }
  if(pc > 0)
    my_fwrite(block, sizeof(float), 3 * pc, fd);
  my_fwrite(&dummy, sizeof(dummy), 1, fd);


  /* write particle ID */
#ifdef NO64BITID
  dummy = sizeof(int) * NumPart;
#else
  dummy = sizeof(long long) * NumPart;
#endif
  my_fwrite(&dummy, sizeof(dummy), 1, fd);
  for(i = 0, pc = 0; i < NumPart; i++)
    {
#ifdef NO64BITID
      blockid[pc] = P[i].ID;
#else
      blocklongid[pc] = P[i].ID;
#endif

      pc++;

      if(pc == maxlongidlen)
	{
#ifdef NO64BITID
	  my_fwrite(blockid, sizeof(int), pc, fd);
#else
	  my_fwrite(blocklongid, sizeof(long long), pc, fd);
#endif
	  pc = 0;
	}
    }
  if(pc > 0)
    {
#ifdef NO64BITID
      my_fwrite(blockid, sizeof(int), pc, fd);
#else
      my_fwrite(blocklongid, sizeof(long long), pc, fd);
#endif
    }

  my_fwrite(&dummy, sizeof(dummy), 1, fd);

  free(block);

  fclose(fd);
}


/* This catches I/O errors occuring for my_fwrite(). In this case we better stop.
 */
size_t my_fwrite(void *ptr, size_t size, size_t nmemb, FILE * stream)
{
  size_t nwritten;

  if((nwritten = fwrite(ptr, size, nmemb, stream)) != nmemb)
    {
      printf("I/O error (fwrite) on task=%d has occured.\n", ThisTask);
      fflush(stdout);
      FatalError(777);
    }
  return nwritten;
}


/* This catches I/O errors occuring for fread(). In this case we better stop.
 */
size_t my_fread(void *ptr, size_t size, size_t nmemb, FILE * stream)
{
  size_t nread;

  if((nread = fread(ptr, size, nmemb, stream)) != nmemb)
    {
      printf("I/O error (fread) on task=%d has occured.\n", ThisTask);
      fflush(stdout);
      FatalError(778);
    }
  return nread;
}

