/* Factor out the FFT code, so that one can switch over to FFTW 3.3 at some point in time

-- Nikhil Padmanabhan, Yale
*/

#include "fft.h"

// Globals -- sorry
int Local_nx, Local_x_start;
rfftwnd_mpi_plan Inverse_plan;
rfftwnd_mpi_plan Forward_plan;
unsigned int TotalSizePlusAdditional;
fftw_real *Workspace;
int *Slab_to_task;
int *Local_nx_table;



// Initialization code
void initialize_ffts(int ThisTask, int NTask, int Nmesh)
{
  int total_size, i, additional;
  int local_ny_after_transpose, local_y_start_after_transpose;
  int *slab_to_task_local;
  size_t bytes;


  Inverse_plan = rfftw3d_mpi_create_plan(MPI_COMM_WORLD,
					 Nmesh, Nmesh, Nmesh, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE);

  Forward_plan = rfftw3d_mpi_create_plan(MPI_COMM_WORLD,
					 Nmesh, Nmesh, Nmesh, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);

  rfftwnd_mpi_local_sizes(Forward_plan, &Local_nx, &Local_x_start,
			  &local_ny_after_transpose, &local_y_start_after_transpose, &total_size);

  Local_nx_table = malloc(sizeof(int) * NTask);
  MPI_Allgather(&Local_nx, 1, MPI_INT, Local_nx_table, 1, MPI_INT, MPI_COMM_WORLD);

  if(ThisTask == 0)
    {
      for(i = 0; i < NTask; i++)
	printf("Task=%d Local_nx=%d\n", i, Local_nx_table[i]);
      fflush(stdout);
    }


  Slab_to_task = malloc(sizeof(int) * Nmesh);
  slab_to_task_local = malloc(sizeof(int) * Nmesh);

  for(i = 0; i < Nmesh; i++)
    slab_to_task_local[i] = 0;

  for(i = 0; i < Local_nx; i++)
    slab_to_task_local[Local_x_start + i] = ThisTask;

  MPI_Allreduce(slab_to_task_local, Slab_to_task, Nmesh, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  free(slab_to_task_local);



  additional = (Nmesh) * (2 * (Nmesh / 2 + 1));	/* additional plane on the right side */

  TotalSizePlusAdditional = total_size + additional;

  Workspace = (fftw_real *) malloc(bytes = sizeof(fftw_real) * total_size);

  ASSERT_ALLOC(Workspace)
}



void free_ffts(void)
{
  free(Workspace);
  free(Slab_to_task);
  rfftwnd_mpi_destroy_plan(Inverse_plan);
  rfftwnd_mpi_destroy_plan(Forward_plan);
}



