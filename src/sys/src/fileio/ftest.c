/*$Id: ftest.c,v 1.39 2001/04/04 21:18:39 bsmith Exp $*/

#include "petsc.h"
#include "petscsys.h"
#if defined(HAVE_PWD_H)
#include <pwd.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif
#if !defined(PARCH_win32)
#include <sys/utsname.h>
#endif
#if defined(PARCH_win32)
#include <windows.h>
#include <io.h>
#include <direct.h>
#endif
#if defined (PARCH_win32_gnu)
#include <windows.h>
#endif
#if defined(HAVE_SYS_SYSTEMINFO_H)
#include <sys/systeminfo.h>
#endif
#include "petscfix.h"

#if defined (HAVE__ACCESS) || defined(HAVE_ACCESS)
#if !defined(R_OK)
#define R_OK 04
#endif
#if !defined(W_OK)
#define W_OK 02
#endif
#if !defined(X_OK)
#define X_OK 01
#endif

#undef __FUNCT__  
#define __FUNCT__ "PetscTestFile"
/*@C
  PetscTestFile - Test for a file existing with a specified mode.

  Input Parameters:
+ fname - name of file
- mode  - mode.  One of r, w, or x

  Output Parameter:
.  flg - PETSC_TRUE if file exists with given mode, PETSC_FALSE otherwise.

  Level: intermediate

@*/
int PetscTestFile(const char fname[],char mode,PetscTruth *flg)
{
  int m;
  
  PetscFunctionBegin;
  *flg = PETSC_FALSE;
  if (!fname) PetscFunctionReturn(0);
  
  if (mode == 'r') m = R_OK;
  else if (mode == 'w') m = W_OK;
  else if (mode == 'x') m = X_OK;
  else SETERRQ(1,"Mode must be one of r, w, or x");
#if defined(HAVE__ACCESS)
  if (m == X_OK) SETERRQ1(PETSC_ERR_SUP,"Unable to check execute permission for file %s",fname);
  if(!_access(fname,m)) *flg = PETSC_TRUE;
#else
  if(!access(fname,m))  *flg = PETSC_TRUE;
#endif
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "PetscTestDirectory"
int PetscTestDirectory(const char fname[],char mode,PetscTruth *flg)
{
  SETERRQ(PETSC_ERR_SUP, "");
}

#else  /* HAVE_ACCESS */

#undef __FUNCT__  
#define __FUNCT__ "PetscTestOwnership"
int PetscTestOwnership(const char fname[], char mode, PetscTruth *flg) {
  uid_t  uid;
  gid_t *gid;
  int    numGroups;
  int    rbit, wbit, ebit;
  int    ierr;

  PetscFunctionBegin;
  /* Get the number of supplementary group IDs */
  numGroups = getgroups(0, gid); if (numGroups < 0) {SETERRQ(numGroups, "Unable to count supplementary group IDs");}
  ierr = PetscMalloc((numGroups+1) * sizeof(gid_t), &gid);                                                CHKERRQ(ierr);

  /* Get the (effective) user and group of the caller */
  uid    = geteuid();
  gid[0] = getegid();

  /* Get supplementary group IDs */
  ierr = getgroups(numGroups, gid+1); if (ierr < 0) {SETERRQ(ierr, "Unable to obtain supplementary group IDs");}

  /* Test for accessibility */
  if (statbuf.st_uid == uid) {
    rbit = S_IRUSR;
    wbit = S_IWUSR;
    ebit = S_IXUSR;
  } else {
    int g;

    for(g = 0; g <= numGroups; g++) {
      if (statbuf.st_gid == gid[g]) {
        rbit = S_IRGRP;
        wbit = S_IWGRP;
        ebit = S_IXGRP;
        break;
      }
    }
    if (g >= numGroups) {
      rbit = S_IROTH;
      wbit = S_IWOTH;
      ebit = S_IXOTH;
    }
  }
  ierr = PetscFree(gid);                                                                                  CHKERRQ(ierr);

  if (mode == 'r') {
    if (stmode & rbit) *flg = PETSC_TRUE;
  } else if (mode == 'w') {
    if (stmode & wbit) *flg = PETSC_TRUE;
  } else if (mode == 'x') {
    if (stmode & ebit) *flg = PETSC_TRUE;
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "PetscTestFile"
int PetscGetFileStatMode(const char fname[], int *statMode) {
  struct stat statbuf;
  int         ierr;

  PetscFunctionBegin;
#if defined(PETSC_HAVE_STAT_NO_CONST)
  ierr = stat((char*) fname, &statbuf);                                                                   CHKERRQ(ierr);
#else
  ierr = stat(fname, &statbuf);                                                                           CHKERRQ(ierr);
#endif
  *statMode = statbuf.st_mode;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "PetscTestFile"
int PetscTestFile(const char fname[], char mode, PetscTruth *flg)
{
  int stmode;
  int err;

  PetscFunctionBegin;
  *flg = PETSC_FALSE;
  if (!fname) PetscFunctionReturn(0);

  ierr = PetscGetFileMode(fname, &stmode);                                                                CHKERRQ(ierr);
  /* Except for systems that have this broken stat macros (rare), this
     is the correct way to check for a regular file */
  if (!S_ISREG(stmode)) PetscFunctionReturn(0);

  ierr = PetscTestOwnership(fname, mode, flg);                                                            CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "PetscTestDirectory"
int PetscTestDirectory(const char fname[],char mode,PetscTruth *flg)
{
  int stmode;
  int ierr;

  PetscFunctionBegin;
  *flg = PETSC_FALSE;
  if (!fname) PetscFunctionReturn(0);

  /* Except for systems that have this broken stat macros (rare), this
     is the correct way to check for a directory */
  if (!S_ISDIR(stmode)) PetscFunctionReturn(0);

  ierr = PetscTestOwnership(fname, mode, flg);                                                            CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#endif /* HAVE_ACCESS */

#undef __FUNCT__  
#define __FUNCT__ "PetscLs"
int PetscLs(MPI_Comm comm,const char libname[],char *found,int tlen,PetscTruth *flg)
{
  int   ierr,len;
  char  *f,program[1024];
  FILE  *fp;

  PetscFunctionBegin;
  ierr   = PetscStrcpy(program,"ls ");CHKERRQ(ierr);
  ierr   = PetscStrcat(program,libname);CHKERRQ(ierr); 
  ierr   = PetscPOpen(comm,PETSC_NULL,program,"r",&fp);CHKERRQ(ierr);
  f      = fgets(found,tlen,fp);
  if (f) *flg = PETSC_TRUE; else *flg = PETSC_FALSE;
  while (f) {
    ierr  = PetscStrlen(found,&len);CHKERRQ(ierr);
    f     = fgets(found+len,tlen-len,fp);
  }
  if (*flg) PetscLogInfo(0,"ls on %s gives \n%s\n",libname,found);
  PetscFunctionReturn(0);
}
