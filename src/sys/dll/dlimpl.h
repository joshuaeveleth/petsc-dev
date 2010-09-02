#if !defined(_DLIMPL_H)
#define _DLIMPL_H

#include "petscsys.h"

typedef void* PetscDLHandle;

#define PETSC_DL_DECIDE   0
#define PETSC_DL_NOW      1
#define PETSC_DL_LOCAL    2

EXTERN PetscErrorCode PETSCSYS_DLLEXPORT PetscDLOpen(const char[],int,PetscDLHandle *);
EXTERN PetscErrorCode PETSCSYS_DLLEXPORT PetscDLClose(PetscDLHandle *);
EXTERN PetscErrorCode PETSCSYS_DLLEXPORT PetscDLSym(PetscDLHandle,const char[],void **);

#endif
