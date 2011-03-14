
#include "private/fortranimpl.h"
#include "petscdmda.h"

#if defined(PETSC_HAVE_FORTRAN_CAPS)
#define dmdacreate1d_                  DMDACREATE1D
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE)
#define dmdacreate1d_                  dmdacreate1d
#endif

EXTERN_C_BEGIN

void PETSC_STDCALL dmdacreate1d_(MPI_Comm *comm,DMDABoundaryType *bx,PetscInt *M,PetscInt *w,PetscInt *s,
                 PetscInt *lc,DM *inra,PetscErrorCode *ierr)
{
 CHKFORTRANNULLINTEGER(lc);
  *ierr = DMDACreate1d(MPI_Comm_f2c(*(MPI_Fint *)&*comm),*bx,*M,*w,*s,lc,inra);
}
EXTERN_C_END

