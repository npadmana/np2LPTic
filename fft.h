#ifndef FFT_H_
#define FFT_H_ 


#include <drfftw_mpi.h>
#include "utils.h"

// Header file for FFT definitions
// This should allow us to factor out FFT changes relatively easily


// Globals -- sorry.
extern rfftwnd_mpi_plan Inverse_plan;
extern rfftwnd_mpi_plan Forward_plan;
extern fftw_real        *Workspace;
extern int      Local_nx, Local_x_start;
extern int      *Slab_to_task;
extern unsigned int TotalSizePlusAdditional;
extern int      *Local_nx_table;



// FFT initialization and clean up
void   initialize_ffts(int ThisTask, int NTask, int Nmesh);
void   free_ffts(void);


#endif /* FFT_H_ */
