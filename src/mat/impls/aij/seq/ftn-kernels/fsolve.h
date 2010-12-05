
#if !defined(__FSOLVE_H)
#include "petscsys.h"
#if defined(PETSC_USE_FORTRAN_KERNEL_SOLVEAIJ)
#if defined(PETSC_HAVE_FORTRAN_CAPS)
#define fortransolveaij_   FORTRANSOLVEAIJ
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE)
#define fortransolveaij_   fortransolveaij
#endif
EXTERN_C_BEGIN
extern void fortransolveaij_(const PetscInt*,void*,const PetscInt*,const PetscInt*,const PetscInt*,const void*,const void*);
EXTERN_C_END
#endif
#endif


