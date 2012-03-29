#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


// Clean exit code
int FatalError(int errnum)
{
  printf("FatalError called with number=%d\n", errnum);
  fflush(stdout);
  MPI_Abort(MPI_COMM_WORLD, errnum);
  exit(0);
}



