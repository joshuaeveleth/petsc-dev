#ifdef PETSC_RCS_HEADER
static char vcid[] = "$Id: ex4.c,v 1.1 1997/04/23 22:03:12 bsmith Exp bsmith $";
#endif

static char help[] = "Demonstrates using ADIC to compute a derivative.\n\n";

/*T
   Concepts: Vectors^Using basic vector routines;
   Processors: n
T*/

/* 
  Include "vec.h" so that we can use vectors.  Note that this file
  automatically includes:
     petsc.h  - base PETSc routines
     sys.h    - system routines
*/

#include "vec.h"
#include "petscadic.h"

extern int Function(Vec,Vec);
extern int ad_Function(Vec,Vec);



int main(int argc,char **argv)
{
  Vec               x,y,z,Az;               /* vectors */
  int               n = 20, ierr, flg,indices[3];
  Scalar            one = 1.0,values[3];
  PetscADICFunction adicctx;
  Mat               grad,igrad;

  PetscInitialize(&argc,&argv,(char*)0,help);
  ad_PetscInitialize(&argc,&argv,(char*)0,help);
  OptionsGetInt(PETSC_NULL,"-n",&n,&flg);
  
  ad_AD_Init();

  /* 
     Create a vector, specifying only its global dimension.
     When using VecCreate() and VecSetFromOptions(), the vector format (currently parallel
     or sequential) is determined at runtime.  Also, the parallel
     partitioning of the vector is determined by PETSc at runtime.

     Routines for creating particular vector types directly are:
        VecCreateSeq() - uniprocessor vector
        VecCreateMPI() - distributed vector, where the user can
                         determine the parallel partitioning
  */
  ierr = VecCreate(PETSC_COMM_WORLD,n,&x); CHKERRA(ierr);
  ierr = VecSetFromOptions(x);HKERRA(ierr);
  ierr = VecDuplicate(x,&y); CHKERRA(ierr);
  ierr = VecDuplicate(x,&z); CHKERRA(ierr);
  ierr = VecDuplicate(z,&Az); CHKERRA(ierr);

  ierr = VecSet(&one,x); CHKERRA(ierr);
  indices[0] = 0; indices[1] = 1; indices[2] = 2;
  values[0] = 1.5; values[1] = 23.9; values[2] = 1.5;
  ierr = VecSetValues(x,3,indices,values,ADD_VALUES);

  /*
     Create a dense matrix to store the Jacobian in 
  */
  ierr = MatCreateSeqDense(PETSC_COMM_WORLD,n,n,PETSC_NULL,&grad); CHKERRA(ierr);
  /*
        Register a function that will be differentiated.
  */
  ierr = PetscADICFunctionCreate(x,y,ad_Function,0,&adicctx); CHKERRA(ierr);
  ierr = PetscADICFunctionSetFunction(adicctx,Function,0); CHKERRQ(ierr);

  /*
     First evaluate the function itself 
  */
  /*  ierr = Function(x,y); CHKERRA(ierr); */
  /* VecView(y,0); */


  /*
     Now evalute the function and its gradient
  */
  ierr = PetscADICFunctionEvaluateGradient(adicctx,x,y,grad); CHKERRA(ierr);
  /* VecView(y,0); */ MatView(grad,0);

  /*
     Now apply the gradient to a vector z
  */
  /*
  ierr = PetscADICFunctionApplyGradientInitialize(adicctx,x,&igrad); CHKERRA(ierr);
  ierr = MatMult(igrad,z,Az); CHKERRA(ierr);
  VecView(Az,0); 
  */
  
  /* 
     Free work space.  All PETSc objects should be destroyed when they
     are no longer needed.
  */
  ierr = MatDestroy(grad); CHKERRA(ierr);
  /*  ierr = MatDestroy(igrad); CHKERRA(ierr); */
  ierr = VecDestroy(x); CHKERRA(ierr);
  ierr = VecDestroy(y); CHKERRA(ierr);
  ierr = VecDestroy(z); CHKERRA(ierr);
  ierr = VecDestroy(Az); CHKERRA(ierr);
  ad_AD_Final();
  PetscFinalize();
  ad_PetscFinalize();
  return 0;
}
 




