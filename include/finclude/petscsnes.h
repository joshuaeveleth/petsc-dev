!
!  Include file for Fortran use of the SNES package in PETSc
!
#if !defined (__PETSCSNES_H)
#define __PETSCSNES_H

#define SNES PetscFortranAddr
#define SNESType character*(80)
#define SNESConvergedReason integer
#define MatSNESMFCtx PetscFortranAddr
#define MatSNESMFType PetscFortranAddr
!
!  SNESType
!
#define SNESLS 'ls'
#define SNESTR 'tr'
#define SNESTEST 'test'
!
! MatSNESMFCtx
! 
#define MATSNESMF_DEFAULT 'ds'
#define MATSNESMF_WP 'wp'

#endif

#if !defined (PETSC_AVOID_DECLARATIONS)
!
!  Convergence flags
!
      PetscEnum SNES_CONVERGED_FNORM_ABS
      PetscEnum SNES_CONVERGED_FNORM_RELATIVE
      PetscEnum SNES_CONVERGED_PNORM_RELATIVE
      PetscEnum SNES_CONVERGED_TR_DELTA

      PetscEnum SNES_DIVERGED_FUNCTION_COUNT
      PetscEnum SNES_DIVERGED_FNORM_NAN
      PetscEnum SNES_DIVERGED_MAX_IT
      PetscEnum SNES_DIVERGED_LS_FAILURE
      PetscEnum SNES_DIVERGED_LOCAL_MIN
      PetscEnum SNES_CONVERGED_ITERATING
   
      parameter (SNES_CONVERGED_FNORM_ABS         =  2)
      parameter (SNES_CONVERGED_FNORM_RELATIVE    =  3)
      parameter (SNES_CONVERGED_PNORM_RELATIVE    =  4)
      parameter (SNES_CONVERGED_TR_DELTA          =  7)

      parameter (SNES_DIVERGED_FUNCTION_COUNT     = -2)  
      parameter (SNES_DIVERGED_FNORM_NAN          = -4) 
      parameter (SNES_DIVERGED_MAX_IT             = -5)
      parameter (SNES_DIVERGED_LS_FAILURE         = -6)
      parameter (SNES_DIVERGED_LOCAL_MIN          = -8)
      parameter (SNES_CONVERGED_ITERATING         =  0)
     
!
!  Some PETSc fortran functions that the user might pass as arguments
!
      external SNESDEFAULTCOMPUTEJACOBIAN
      external SNESDEFAULTCOMPUTEJACOBIANCOLOR
      external SNESDEFAULTMONITOR
      external SNESLGMONITOR
      external SNESVECVIEWMONITOR
      external SNESVECVIEWUPDATEMONITOR

!PETSC_DEC_ATTRIBUTES(SNESDEFAULTCOMPUTEJACOBIAN,'_SNESDEFAULTCOMPUTEJACOBIAN')
!PETSC_DEC_ATTRIBUTES(SNESDEFAULTCOMPUTEJACOBIANCOLOR,'_SNESDEFAULTCOMPUTEJACOBIANCOLOR')
!PETSC_DEC_ATTRIBUTES(SNESDEFAULTMONITOR,'_SNESDEFAULTMONITOR')
!PETSC_DEC_ATTRIBUTES(SNESLGMONITOR,'_SNESLGMONITOR')
!PETSC_DEC_ATTRIBUTES(SNESVECVIEWMONITOR,'_SNESVECVIEWMONITOR')
!PETSC_DEC_ATTRIBUTES(SNESVECVIEWUPDATEMONITOR,'_SNESVECVIEWUPDATEMONITOR')

      external SNESCONVERGED_LS
      external SNESCONVERGED_TR

!PETSC_DEC_ATTRIBUTES(SNESCONVERGED_LS,'_SNESCONVERGED_LS')
!PETSC_DEC_ATTRIBUTES(SNESCONVERGED_TR,'_SNESCONVERGED_TR')

      external SNESLINESEARCHCUBIC
      external SNESLINESEARCHQUADRATIC
      external SNESLINESEARCHNO
      external SNESLINESEARCHNONORMS

!PETSC_DEC_ATTRIBUTES(SNESLINESEARCHCUBIC,'_SNESLINESEARCHCUBIC')
!PETSC_DEC_ATTRIBUTES(SNESLINESEARCHQUADRATIC,'_SNESLINESEARCHQUADRATIC')
!PETSC_DEC_ATTRIBUTES(SNESLINESEARCHNO,'_SNESLINESEARCHNO')
!PETSC_DEC_ATTRIBUTES(SNESLINESEARCHNONORMS,'_SNESLINESEARCHNONORMS')

      external SNESDAFORMFUNCTION
      external SNESDACOMPUTEJACOBIANWITHADIFOR
      external SNESDACOMPUTEJACOBIAN

!PETSC_DEC_ATTRIBUTES(SNESDAFORMFUNCTION,'_SNESDAFORMFUNCTION')
!PETSC_DEC_ATTRIBUTES(SNESDACOMPUTEJACOBIANWITHADIFOR,'_SNESDACOMPUTEJACOBIANWITHADIFOR')
!PETSC_DEC_ATTRIBUTES(SNESDACOMPUTEJACOBIAN,'_SNESDACOMPUTEJACOBIAN')
!
!  End of Fortran include file for the SNES package in PETSc

#endif
