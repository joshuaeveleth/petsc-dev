/*$Id: ex4.c,v 1.14 2001/04/10 19:34:49 bsmith Exp $*/

static char help[] = "Tests ISStrideToGeneral().\n\n";

#include "petscis.h"

#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc,char **argv)
{
  int        ierr,step = 2;
  IS         is;

  ierr = PetscInitialize(&argc,&argv,(char*)0,help);CHKERRQ(ierr); 

  ierr = PetscOptionsGetInt(PETSC_NULL,"-step",&step,PETSC_NULL);CHKERRQ(ierr);
  ierr = ISCreateStride(PETSC_COMM_SELF,10,0,step,&is);CHKERRQ(ierr);

  ierr = ISStrideToGeneral(is);CHKERRQ(ierr);

  ierr = ISDestroy(is);CHKERRQ(ierr);

  ierr = PetscFinalize();CHKERRQ(ierr);
  return 0;
}
 






