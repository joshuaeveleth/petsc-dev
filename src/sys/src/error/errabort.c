/*$Id: errabort.c,v 1.12 2001/03/23 23:20:26 balay Exp $*/
/*
       The default error handlers and code that allows one to change
   error handlers.
*/
#include "petsc.h"           /*I "petsc.h" I*/
#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif
#include "petscfix.h"

#undef __FUNCT__  
#define __FUNCT__ "PetscAbortErrorHandler" 
/*@C
   PetscAbortErrorHandler - Error handler that calls abort on error. 
   This routine is very useful when running in the debugger, because the 
   user can look directly at the stack frames and the variables.

   Not Collective

   Input Parameters:
+  line - the line number of the error (indicated by __LINE__)
.  func - function where error occured (indicated by __FUNCT__)
.  file - the file in which the error was detected (indicated by __FILE__)
.  dir - the directory of the file (indicated by __SDIR__)
.  mess - an error text string, usually just printed to the screen
.  n - the generic error number
.  p - specific error number
-  ctx - error handler context

   Options Database Keys:
+  -on_error_abort - Activates aborting when an error is encountered
-  -start_in_debugger [noxterm,dbx,xxgdb]  [-display name] - Starts all
    processes in the debugger and uses PetscAbortErrorHandler().  By default the 
    debugger is gdb; alternatives are dbx and xxgdb.

   Level: developer

   Notes:
   Most users need not directly employ this routine and the other error 
   handlers, but can instead use the simplified interface SETERRQ, which
   has the calling sequence
$     SETERRQ(number,p,mess)

   Notes for experienced users:
   Use PetscPushErrorHandler() to set the desired error handler.  The
   currently available PETSc error handlers include PetscTraceBackErrorHandler(),
   PetscAttachDebuggerErrorHandler(), and PetscAbortErrorHandler().

   Concepts: error handler^aborting
   Concepts: aborting on error

.seealso: PetscPushErrorHandler(), PetscTraceBackErrorHandler(), 
          PetscAttachDebuggerErrorHandler()
@*/
int PetscAbortErrorHandler(int line,char *func,char *file,char* dir,int n,int p,char *mess,void *ctx)
{
  PetscFunctionBegin;

  abort(); 
  PetscFunctionReturn(0);
}

