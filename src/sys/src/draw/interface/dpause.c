/*$Id: dpause.c,v 1.28 2001/03/23 23:20:08 balay Exp $*/
/*
       Provides the calling sequences for all the basic PetscDraw routines.
*/
#include "src/sys/src/draw/drawimpl.h"  /*I "petscdraw.h" I*/

#undef __FUNCT__  
#define __FUNCT__ "PetscDrawPause" 
/*@
   PetscDrawPause - Waits n seconds or until user input, depending on input 
               to PetscDrawSetPause().

   Collective operation on PetscDraw object.

   Input Parameter:
.  draw - the drawing context

   Level: beginner

   Concepts: waiting^for user input
   Concepts: drawing^waiting
   Concepts: graphics^waiting

.seealso: PetscDrawSetPause(), PetscDrawGetPause()
@*/
int PetscDrawPause(PetscDraw draw)
{
  int ierr;
  PetscFunctionBegin;
  PetscValidHeaderSpecific(draw,PETSC_DRAW_COOKIE);
  if (draw->ops->pause) {
    ierr = (*draw->ops->pause)(draw);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}
