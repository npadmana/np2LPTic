#include <math.h>
#include <stdlib.h>
#include "allvars.h"
//#include "proto.h"
#include "fft.h"



void init_particles()
{
  int i, j, k, slab, count, type;
  float x, y, z;
  size_t bytes;
  int *npart_Task;

  npart_Task = malloc(sizeof(int) * NTask);

  for(i = 0; i < NTask; i++)
    npart_Task[i] = 0;

#if defined(MULTICOMPONENTGLASSFILE) && defined(DIFFERENT_TRANSFER_FUNC)
  MinType = 7;
  MaxType = -2;
  for(type = 0; type < 6; type++)
    if(header1.npartTotal[type])
      {
	if(MinType > type - 1)
	  MinType = type - 1;

	if(MaxType < type - 1)
	  MaxType = type - 1;
      }
#endif

  // Figure out the number of particles per slab
  for(i = 0; i < GlassTileFac; i++)
    {
	x = i * (Box / GlassTileFac);
	slab = (int) (x / Box * Nmesh);
        // To prevent round-off errors
	if(slab >= Nmesh) slab = Nmesh - 1;
	npart_Task[Slab_to_task[slab]] += GlassTileFac*GlassTileFac;
    }

  TotNumPart = 0;		/* note: This is a 64 bit integer */
  NTaskWithN = 0;

  NumPart = npart_Task[ThisTask];

  for(i = 0; i < NTask; i++)
    {
      TotNumPart += npart_Task[i];
      if(npart_Task[i] > 0)
	NTaskWithN++;
    }


  if(ThisTask == 0)
    {
      for(i = 0; i < NTask; i++)
	printf("%d particles on task=%d  (slabs=%d)\n", npart_Task[i], i, Local_nx_table[i]);

      printf("\nTotal number of particles  = %d%09d\n\n",
	     (int) (TotNumPart / 1000000000), (int) (TotNumPart % 1000000000));

      fflush(stdout);
    }


  free(npart_Task);


  if(NumPart)
    {
      P = (struct part_data *) malloc(bytes = sizeof(struct part_data) * NumPart);

      if(!(P))
	{
	  printf("failed to allocate %g Mbyte (%d particles) on Task %d\n", bytes / (1024.0 * 1024.0),
		 NumPart, ThisTask);
	  FatalError(9891);
	}
    }


  count = 0;


  IDStart = 1;

  for(i = 0; i < GlassTileFac; i++)
    for(j = 0; j < GlassTileFac; j++)
      for(k = 0; k < GlassTileFac; k++)
	{
	    x = i * (Box / GlassTileFac);
	    slab = (int) (x / Box * Nmesh);
	    if(slab >= Nmesh) slab = Nmesh - 1;
	    if(Slab_to_task[slab] == ThisTask)
	    {
		y = j * (Box / GlassTileFac);
		z = k * (Box / GlassTileFac);
		P[count].Pos[0] = x;
		P[count].Pos[1] = y;
		P[count].Pos[2] = z;
#ifdef  MULTICOMPONENTGLASSFILE
		      P[count].Type = type - 1;
#endif
		P[count].ID = IDStart;

		count++;
	  }
	  IDStart++;
	}

  if(count != NumPart)
    {
      printf("fatal mismatch (%d %d) on Task %d\n", count, NumPart, ThisTask);
      FatalError(1);
    }

}


