/*$Id: snesj.c,v 1.75 2001/09/11 18:06:40 bsmith Exp $*/

#include "src/snes/snesimpl.h"    /*I  "petscsnes.h"  I*/

#undef __FUNCT__  
#define __FUNCT__ "SNESDefaultComputeJacobian"
/*@C
   SNESDefaultComputeJacobian - Computes the Jacobian using finite differences. 

   Collective on SNES

   Input Parameters:
+  x1 - compute Jacobian at this point
-  ctx - application's function context, as set with SNESSetFunction()

   Output Parameters:
+  J - Jacobian matrix (not altered in this routine)
.  B - newly computed Jacobian matrix to use with preconditioner (generally the same as J)
-  flag - flag indicating whether the matrix sparsity structure has changed

   Options Database Key:
+  -snes_fd - Activates SNESDefaultComputeJacobian()
-  -snes_test_err - Square root of function error tolerance, default square root of machine
                    epsilon (1.e-8 in double, 3.e-4 in single)

   Notes:
   This routine is slow and expensive, and is not currently optimized
   to take advantage of sparsity in the problem.  Although
   SNESDefaultComputeJacobian() is not recommended for general use
   in large-scale applications, It can be useful in checking the
   correctness of a user-provided Jacobian.

   An alternative routine that uses coloring to explot matrix sparsity is
   SNESDefaultComputeJacobianColor().

   Level: intermediate

.keywords: SNES, finite differences, Jacobian

.seealso: SNESSetJacobian(), SNESDefaultComputeJacobianColor()
@*/
int SNESDefaultComputeJacobian(SNES snes,Vec x1,Mat *J,Mat *B,MatStructure *flag,void *ctx)
{
  Vec         j1a,j2a,x2;
  int         i,ierr,N,start,end,j;
  PetscScalar dx,mone = -1.0,*y,scale,*xx,wscale;
  PetscReal   amax,epsilon = PETSC_SQRT_MACHINE_EPSILON;
  PetscReal   dx_min = 1.e-16,dx_par = 1.e-1;
  MPI_Comm    comm;
  int         (*eval_fct)(SNES,Vec,Vec)=0;

  PetscFunctionBegin;
  ierr = PetscOptionsGetReal(snes->prefix,"-snes_test_err",&epsilon,0);CHKERRQ(ierr);
  if (snes->method_class == SNES_NONLINEAR_EQUATIONS) eval_fct = SNESComputeFunction;
  else if (snes->method_class == SNES_UNCONSTRAINED_MINIMIZATION) eval_fct = SNESComputeGradient;
  else SETERRQ(PETSC_ERR_ARG_OUTOFRANGE,"Invalid method class");

  ierr = PetscObjectGetComm((PetscObject)x1,&comm);CHKERRQ(ierr);
  ierr = MatZeroEntries(*B);CHKERRQ(ierr);
  if (!snes->nvwork) {
    ierr = VecDuplicateVecs(x1,3,&snes->vwork);CHKERRQ(ierr);
    snes->nvwork = 3;
    PetscLogObjectParents(snes,3,snes->vwork);
  }
  j1a = snes->vwork[0]; j2a = snes->vwork[1]; x2 = snes->vwork[2];

  ierr = VecGetSize(x1,&N);CHKERRQ(ierr);
  ierr = VecGetOwnershipRange(x1,&start,&end);CHKERRQ(ierr);
  ierr = (*eval_fct)(snes,x1,j1a);CHKERRQ(ierr);

  /* Compute Jacobian approximation, 1 column at a time. 
      x1 = current iterate, j1a = F(x1)
      x2 = perturbed iterate, j2a = F(x2)
   */
  for (i=0; i<N; i++) {
    ierr = VecCopy(x1,x2);CHKERRQ(ierr);
    if (i>= start && i<end) {
      ierr = VecGetArray(x1,&xx);CHKERRQ(ierr);
      dx = xx[i-start];
      ierr = VecRestoreArray(x1,&xx);CHKERRQ(ierr);
#if !defined(PETSC_USE_COMPLEX)
      if (dx < dx_min && dx >= 0.0) dx = dx_par;
      else if (dx < 0.0 && dx > -dx_min) dx = -dx_par;
#else
      if (PetscAbsScalar(dx) < dx_min && PetscRealPart(dx) >= 0.0) dx = dx_par;
      else if (PetscRealPart(dx) < 0.0 && PetscAbsScalar(dx) < dx_min) dx = -dx_par;
#endif
      dx *= epsilon;
      wscale = 1.0/dx;
      ierr = VecSetValues(x2,1,&i,&dx,ADD_VALUES);CHKERRQ(ierr);
    } else {
      wscale = 0.0;
    }
    ierr = (*eval_fct)(snes,x2,j2a);CHKERRQ(ierr);
    ierr = VecAXPY(&mone,j1a,j2a);CHKERRQ(ierr);
    /* Communicate scale to all processors */
    ierr = MPI_Allreduce(&wscale,&scale,1,MPIU_SCALAR,PetscSum_Op,comm);CHKERRQ(ierr);
    ierr = VecScale(&scale,j2a);CHKERRQ(ierr);
    ierr = VecNorm(j2a,NORM_INFINITY,&amax);CHKERRQ(ierr); amax *= 1.e-14;
    ierr = VecGetArray(j2a,&y);CHKERRQ(ierr);
    for (j=start; j<end; j++) {
      if (PetscAbsScalar(y[j-start]) > amax) {
        ierr = MatSetValues(*B,1,&j,1,&i,y+j-start,INSERT_VALUES);CHKERRQ(ierr);
      }
    }
    ierr = VecRestoreArray(j2a,&y);CHKERRQ(ierr);
  }
  ierr  = MatAssemblyBegin(*B,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr  = MatAssemblyEnd(*B,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  *flag =  DIFFERENT_NONZERO_PATTERN;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "SNESDefaultComputeHessian"
/*@C
   SNESDefaultComputeHessian - Computes the Hessian using finite differences. 

   Collective on SNES

   Input Parameters:
+  x1 - compute Hessian at this point
-  ctx - application's gradient context, as set with SNESSetGradient()

   Output Parameters:
+  J - Hessian matrix (not altered in this routine)
.  B - newly computed Hessian matrix to use with preconditioner (generally the same as J)
-  flag - flag indicating whether the matrix sparsity structure has changed

   Options Database Key:
$  -snes_fd - Activates SNESDefaultComputeHessian()


   Level: intermediate

   Notes:
   This routine is slow and expensive, and is not currently optimized
   to take advantage of sparsity in the problem.  Although
   SNESDefaultComputeHessian() is not recommended for general use
   in large-scale applications, It can be useful in checking the
   correctness of a user-provided Hessian.

.keywords: SNES, finite differences, Hessian

.seealso: SNESSetHessian()
@*/
int SNESDefaultComputeHessian(SNES snes,Vec x1,Mat *J,Mat *B,MatStructure *flag,void *ctx)
{
  int ierr;

  PetscFunctionBegin;
  ierr = SNESDefaultComputeJacobian(snes,x1,J,B,flag,ctx);CHKERRQ(ierr);
  PetscFunctionReturn(0); 
}

#undef __FUNCT__  
#define __FUNCT__ "SNESDefaultComputeHessianColor"
/*@C
   SNESDefaultComputeHessianColor - Computes the Hessian using colored finite differences. 

   Collective on SNES

   Input Parameters:
+  x1 - compute Hessian at this point
-  ctx - application's gradient context, as set with SNESSetGradient()

   Output Parameters:
+  J - Hessian matrix (not altered in this routine)
.  B - newly computed Hessian matrix to use with preconditioner (generally the same as J)
-  flag - flag indicating whether the matrix sparsity structure has changed

    Options Database Keys:
.  -mat_fd_coloring_freq <freq> - Activates SNESDefaultComputeJacobianColor()

   Level: intermediate

 .keywords: SNES, finite differences, Hessian, coloring, sparse

.seealso: SNESSetHessian()
@*/
int SNESDefaultComputeHessianColor(SNES snes,Vec x1,Mat *J,Mat *B,MatStructure *flag,void *ctx)
{
  int ierr;

  PetscFunctionBegin;
  ierr = SNESDefaultComputeJacobianColor(snes,x1,J,B,flag,ctx);CHKERRQ(ierr);
  PetscFunctionReturn(0); 
}

