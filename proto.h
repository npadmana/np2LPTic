#include <gsl/gsl_rng.h>

void   print_spec(void);
void   displacement_fields(void);
void   set_units(void);
void   assemble_particles(void);
double fnl(double x);

void   assemble_grid(void);
double periodic_wrap(double x);

void   combine_particle_data(void);

void  write_particle_data(void);
void  read_parameterfile(char *fname);
void  init_particles();

size_t my_fread(void *ptr, size_t size, size_t nmemb, FILE * stream);
size_t my_fwrite(void *ptr, size_t size, size_t nmemb, FILE * stream);

void save_local_data(void);
void add_WDM_thermal_speeds(float *vel);

int compare_type(const void *a, const void *b);

