#include <cstdio>
#include <cstdlib>
#include "xcint.hpp"

#ifndef FORTRAN_INT
typedef int fortran_int_t;
#else
typedef FORTRAN_INT fortran_int_t;
#endif

functional_data xcint_funs[XC_NR_FUNCTIONALS]; 
parameter_data xcint_params[XC_NR_PARAMETERS_AND_FUNCTIONALS];

template<int FUN>
void xcint_functional_setup_helper()
{
  fundat_db<FUN>::d.symbol = fundat_db<FUN>::symbol;
  fundat_db<FUN>::d.id = (enum xc_functional_id)FUN;
  xcint_funs[FUN] = fundat_db<FUN>::d;
  xcint_functional_setup_helper<FUN+1>();
}

template<> void xcint_functional_setup_helper<XC_NR_FUNCTIONALS>() {  }

template<int P>
void xcint_parameter_setup_helper()
{
  pardat_db<P>::d.symbol = pardat_db<P>::symbol;
  xcint_params[P] = pardat_db<P>::d;
  xcint_parameter_setup_helper<P+1>();
}

template<> 
void xcint_parameter_setup_helper<XC_NR_PARAMETERS_AND_FUNCTIONALS>() {}

vars_data xcint_vars[XC_NR_VARS] =
  {
    {"XC_A", 1, XC_DENSITY},
    {"XC_N", 1, XC_DENSITY},
    {"XC_A_B", 2, XC_DENSITY},
    {"XC_N_S", 2, XC_DENSITY},
    {"XC_A_GAA", 2, XC_DENSITY | XC_GRADIENT},
    {"XC_N_GNN", 2, XC_DENSITY | XC_GRADIENT},
    {"XC_A_B_GAA_GAB_GBB", 5, XC_DENSITY | XC_GRADIENT},
    {"XC_N_S_GNN_GNS_GSS", 5, XC_DENSITY | XC_GRADIENT},
    {"XC_A_GAA_LAPA", 3, XC_DENSITY | XC_GRADIENT | XC_LAPLACIAN},
    {"XC_A_GAA_TAUA", 3, XC_DENSITY | XC_GRADIENT | XC_LAPLACIAN},
    {"XC_N_GNN_LAPN", 3, XC_DENSITY | XC_GRADIENT | XC_LAPLACIAN},
    {"XC_N_GNN_TAUN", 3, XC_DENSITY | XC_GRADIENT | XC_LAPLACIAN},
    {"XC_A_B_GAA_GAB_GBB_LAPA_LAPB",7, XC_DENSITY | XC_GRADIENT | XC_LAPLACIAN},
    {"XC_A_B_GAA_GAB_GBB_TAUA_TAUB",7, XC_DENSITY | XC_GRADIENT | XC_KINETIC},
    {"XC_N_S_GNN_GNS_GSS_LAPN_LAPS",7, XC_DENSITY | XC_GRADIENT | XC_LAPLACIAN},
    {"XC_N_S_GNN_GNS_GSS_TAUN_TAUS",7, XC_DENSITY | XC_GRADIENT | XC_KINETIC},
    {"XC_A_AX_AY_AZ",4, XC_DENSITY | XC_GRADIENT},
    {"XC_A_B_AX_AY_AZ_BZ_BY_BZ", 8, XC_DENSITY | XC_GRADIENT},
    {"XC_N_NX_NY_NZ",4, XC_DENSITY | XC_GRADIENT},
    {"XC_N_S_NX_NY_NZ_SZ_SY_SZ",8, XC_DENSITY | XC_GRADIENT},
    {"XC_A_2ND_TAYLOR", 10, XC_DENSITY | XC_GRADIENT | XC_LAPLACIAN},
    {"XC_A_B_2ND_TAYLOR", 20, XC_DENSITY | XC_GRADIENT | XC_LAPLACIAN},
    {"XC_N_2ND_TAYLOR", 10, XC_DENSITY | XC_GRADIENT | XC_LAPLACIAN},
    {"XC_N_S_2ND_TAYLOR", 20, XC_DENSITY | XC_GRADIENT | XC_LAPLACIAN},
  };


void xcint_assure_setup()
{
  static bool is_setup = false;
  if (!is_setup)
    {
      xcint_functional_setup_helper<0>();      
      xcint_parameter_setup_helper<XC_NR_FUNCTIONALS>();
      is_setup = true;
    }
}

void xcint_die(const char *message, int code)
{
  fprintf(stderr,"XCFun fatal error %i: ",code);
  fprintf(stderr,"%s",message);
  fprintf(stderr,"\n");
  exit(-1);
}

int xcint_write_fortran_module()
{
  xcint_assure_setup();
  FILE *dst = fopen("fortran/xcfun_autogen.F90","w");
  int i;
  if (!dst)
    {
      perror("Open fortran/xcfun_autogen.F90 for writing failed");
      return -1;
    }
  fprintf(dst,"! Autogenerated file, do not edit\n"
	  "module xcfun_autogen\n");
  fprintf(dst,"  integer, parameter :: XCFUN_CHECKSUM = %i\n",
	  XCFUN_CHECKSUM);
  fprintf(dst,"  integer, parameter :: XCFUN_FORTRAN_INT_SIZE = %lu\n",
	  sizeof(fortran_int_t));
  fprintf(dst,"  integer, parameter :: XC_MAX_ORDER = %i\n",
	  XC_MAX_ORDER);
  fprintf(dst,"  integer, parameter :: XC_NR_FUNCTIONALS = %i\n",
	  XC_NR_FUNCTIONALS);
  fprintf(dst,"  integer, parameter :: XC_NR_PARAMETERS_AND_FUNCTIONALS = %i\n",
	  XC_NR_PARAMETERS_AND_FUNCTIONALS);
  fprintf(dst,"  integer, parameter :: XC_NR_VARS = %i\n",
	  XC_NR_VARS);
  fprintf(dst,"\n");
  fprintf(dst,"!Functionals\n");
  for (i=0;i<XC_NR_FUNCTIONALS;i++)
    fprintf(dst,"  integer, parameter :: %s = %i\n",
	    xcint_funs[i].symbol,i);
  fprintf(dst,"\n");
  fprintf(dst,"!Parameters\n");
  for (i=XC_NR_FUNCTIONALS;i<XC_NR_PARAMETERS_AND_FUNCTIONALS;i++)
    fprintf(dst,"  integer, parameter :: %s = %i\n",
	    xcint_params[i].symbol,i);
  fprintf(dst,"\n");
  for (i=0;i<XC_NR_VARS;i++)
    fprintf(dst,"  integer, parameter :: %s = %i\n",
	    xcint_vars[i].symbol,i);
  fprintf(dst,"end module\n");
  fclose(dst);
  return 0;
}
	  
