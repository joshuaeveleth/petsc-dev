#ifdef PETSC_RCS_HEADER
"$Id: petscconf.h,v 1.3 2000/11/28 17:26:31 bsmith Exp $"
"Defines the configuration for this machine"
#endif

#if !defined(INCLUDED_PETSCCONF_H)
#define INCLUDED_PETSCCONF_H

#define PARCH_solaris 
#define PETSC_ARCH_NAME "solaris"

#define HAVE_POPEN
#define HAVE_LIMITS_H
#define HAVE_STROPTS_H 
#define HAVE_SEARCH_H 
#define HAVE_PWD_H 
#define HAVE_STRING_H 
#define HAVE_MALLOC_H
#define HAVE_STDLIB_H
#define HAVE_UNISTD_H 
#define HAVE_DRAND48 
#define HAVE_GETCWD
#define HAVE_SLEEP
#define HAVE_SYS_PARAM_H
#define HAVE_SYS_STAT_H

#define HAVE_SYS_TIME_H
#define HAVE_SYS_SYSTEMINFO_H
#define HAVE_SYSINFO_3ARG
#define HAVE_SUNMATH_H
#define PETSC_HAVE_RESTRICT
#define PETSC_HAVE_SUNMATHPRO

#define BITS_PER_BYTE 8

#define PETSC_HAVE_FORTRAN_UNDERSCORE

#define HAVE_READLINK
#define HAVE_MEMMOVE

#define HAVE_DOUBLE_ALIGN_MALLOC

#define HAVE_SYS_PROCFS_H
#define PETSC_USE_PROCFS_FOR_SIZE
#define HAVE_MEMALIGN
#define PETSC_USE_DBX_DEBUGGER

#define PETSC_USE_DYNAMIC_LIBRARIES 1
#define PETSC_HAVE_RTLD_GLOBAL 1
#define HAVE_SYS_TIMES_H
#define HAVE_UCBPS

#define PETSC_HAVE_SOLARIS_STYLE_FPTRAP
#endif
