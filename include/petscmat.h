/* $Id: petscmat.h,v 1.228 2001/09/07 20:09:08 bsmith Exp $ */
/*
     Include file for the matrix component of PETSc
*/
#ifndef __PETSCMAT_H
#define __PETSCMAT_H
#include "petscvec.h"

#define MAT_COOKIE         PETSC_COOKIE+5

/*S
     Mat - Abstract PETSc matrix object

   Level: beginner

  Concepts: matrix; linear operator

.seealso:  MatCreate(), MatType, MatSetType()
S*/
typedef struct _p_Mat*           Mat;

/*E
    MatType - String with the name of a PETSc matrix or the creation function
       with an optional dynamic library name, for example
       http://www.mcs.anl.gov/petsc/lib.a:mymatcreate()

   Level: beginner

.seealso: MatSetType(), Mat
E*/
#define MATSAME     "same"
#define MATSEQMAIJ  "seqmaij"
#define MATMPIMAIJ  "mpimaij"
#define MATIS       "is"
#define MATMPIROWBS "mpirowbs"
#define MATSEQDENSE "seqdense"
#define MATSEQAIJ   "seqaij"
#define MATMPIAIJ   "mpiaij"
#define MATSHELL    "shell"
#define MATSEQBDIAG "seqbdiag"
#define MATMPIBDIAG "mpibdiag"
#define MATMPIDENSE "mpidense"
#define MATSEQBAIJ  "seqbaij"
#define MATMPIBAIJ  "mpibaij"
#define MATMPIADJ   "mpiadj"
#define MATSEQSBAIJ "seqsbaij"
#define MATMPISBAIJ "mpisbaij"
#define MATDAAD     "daad"
#define MATMFFD     "mffd"
typedef char* MatType;

EXTERN int MatCreate(MPI_Comm,int,int,int,int,Mat*);
EXTERN int MatSetType(Mat,MatType);
EXTERN int MatSetFromOptions(Mat);
EXTERN int MatSetUpPreallocation(Mat);
EXTERN int MatRegisterAll(char*);
EXTERN int MatRegister(char*,char*,char*,int(*)(Mat));
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define MatRegisterDynamic(a,b,c,d) MatRegister(a,b,c,0)
#else
#define MatRegisterDynamic(a,b,c,d) MatRegister(a,b,c,d)
#endif
extern PetscTruth MatRegisterAllCalled;
extern PetscFList MatList;

EXTERN int MatCreate(MPI_Comm,int,int,int,int,Mat*);
EXTERN int MatCreateSeqDense(MPI_Comm,int,int,PetscScalar*,Mat*);
EXTERN int MatCreateMPIDense(MPI_Comm,int,int,int,int,PetscScalar*,Mat*); 
EXTERN int MatCreateSeqAIJ(MPI_Comm,int,int,int,int*,Mat*);
EXTERN int MatCreateMPIAIJ(MPI_Comm,int,int,int,int,int,int*,int,int*,Mat*); 
EXTERN int MatCreateMPIRowbs(MPI_Comm,int,int,int,int*,Mat*); 
EXTERN int MatCreateSeqBDiag(MPI_Comm,int,int,int,int,int*,PetscScalar**,Mat*); 
EXTERN int MatCreateMPIBDiag(MPI_Comm,int,int,int,int,int,int*,PetscScalar**,Mat*); 
EXTERN int MatCreateSeqBAIJ(MPI_Comm,int,int,int,int,int*,Mat*); 
EXTERN int MatCreateMPIBAIJ(MPI_Comm,int,int,int,int,int,int,int*,int,int*,Mat*);
EXTERN int MatCreateMPIAdj(MPI_Comm,int,int,int*,int*,int *,Mat*);
EXTERN int MatCreateSeqSBAIJ(MPI_Comm,int,int,int,int,int*,Mat*); 
EXTERN int MatCreateMPISBAIJ(MPI_Comm,int,int,int,int,int,int,int*,int,int*,Mat*);
EXTERN int MatCreateShell(MPI_Comm,int,int,int,int,void *,Mat*);
EXTERN int MatCreateAdic(MPI_Comm,int,int,int,int,int,void (*)(void),Mat*);
EXTERN int MatDestroy(Mat);

EXTERN int MatPrintHelp(Mat);
EXTERN int MatGetPetscMaps(Mat,PetscMap*,PetscMap*);

/* ------------------------------------------------------------*/
EXTERN int MatSetValues(Mat,int,int*,int,int*,PetscScalar*,InsertMode);
EXTERN int MatSetValuesBlocked(Mat,int,int*,int,int*,PetscScalar*,InsertMode);

/*S
     MatStencil - Data structure (C struct) for storing information about a single row or
        column of a matrix as index on an associated grid.

   Level: beginner

  Concepts: matrix; linear operator

.seealso:  MatSetValuesStencil(), MatSetStencil()
S*/
typedef struct {
  int k,j,i,c;
} MatStencil;

EXTERN int MatSetValuesStencil(Mat,int,MatStencil*,int,MatStencil*,PetscScalar*,InsertMode);
EXTERN int MatSetValuesBlockedStencil(Mat,int,MatStencil*,int,MatStencil*,PetscScalar*,InsertMode);
EXTERN int MatSetStencil(Mat,int,int*,int*,int);

EXTERN int MatSetColoring(Mat,ISColoring);
EXTERN int MatSetValuesAdic(Mat,void*);
EXTERN int MatSetValuesAdifor(Mat,int,void*);

/*E
    MatAssemblyType - Indicates if the matrix is now to be used, or if you plan 
     to continue to add values to it

    Level: beginner

.seealso: MatAssemblyBegin(), MatAssemblyEnd()
E*/
typedef enum {MAT_FLUSH_ASSEMBLY=1,MAT_FINAL_ASSEMBLY=0} MatAssemblyType;
EXTERN int MatAssemblyBegin(Mat,MatAssemblyType);
EXTERN int MatAssemblyEnd(Mat,MatAssemblyType);
EXTERN int MatAssembled(Mat,PetscTruth*);

#define MatSetValue(v,i,j,va,mode) \
0; {int _ierr,_row = i,_col = j; PetscScalar _va = va; \
  _ierr = MatSetValues(v,1,&_row,1,&_col,&_va,mode);CHKERRQ(_ierr); \
}
#define MatGetValue(v,i,j,va) \
0; {int _ierr,_row = i,_col = j; \
  _ierr = MatGetValues(v,1,&_row,1,&_col,&va);CHKERRQ(_ierr); \
}
#define MatSetValueLocal(v,i,j,va,mode) \
0; {int _ierr,_row = i,_col = j; PetscScalar _va = va; \
  _ierr = MatSetValuesLocal(v,1,&_row,1,&_col,&_va,mode);CHKERRQ(_ierr); \
}
/*E
    MatOption - Options that may be set for a matrix and its behavior or storage

    Level: beginner

   Any additions/changes here MUST also be made in include/finclude/petscmat.h

.seealso: MatSetOption()
E*/
typedef enum {MAT_ROW_ORIENTED=1,MAT_COLUMN_ORIENTED=2,MAT_ROWS_SORTED=4,
              MAT_COLUMNS_SORTED=8,MAT_NO_NEW_NONZERO_LOCATIONS=16,
              MAT_YES_NEW_NONZERO_LOCATIONS=32,MAT_SYMMETRIC=64,
              MAT_STRUCTURALLY_SYMMETRIC=65,MAT_NO_NEW_DIAGONALS=66,
              MAT_YES_NEW_DIAGONALS=67,MAT_INODE_LIMIT_1=68,MAT_INODE_LIMIT_2=69,
              MAT_INODE_LIMIT_3=70,MAT_INODE_LIMIT_4=71,MAT_INODE_LIMIT_5=72,
              MAT_IGNORE_OFF_PROC_ENTRIES=73,MAT_ROWS_UNSORTED=74,
              MAT_COLUMNS_UNSORTED=75,MAT_NEW_NONZERO_LOCATION_ERR=76,
              MAT_NEW_NONZERO_ALLOCATION_ERR=77,MAT_USE_HASH_TABLE=78,
              MAT_KEEP_ZEROED_ROWS=79,MAT_IGNORE_ZERO_ENTRIES=80,MAT_USE_INODES=81,
              MAT_DO_NOT_USE_INODES=82,MAT_USE_SINGLE_PRECISION_SOLVES=83} MatOption;
EXTERN int MatSetOption(Mat,MatOption);
EXTERN int MatGetType(Mat,MatType*);

EXTERN int MatGetValues(Mat,int,int*,int,int*,PetscScalar*);
EXTERN int MatGetRow(Mat,int,int *,int **,PetscScalar**);
EXTERN int MatRestoreRow(Mat,int,int *,int **,PetscScalar**);
EXTERN int MatGetColumn(Mat,int,int *,int **,PetscScalar**);
EXTERN int MatRestoreColumn(Mat,int,int *,int **,PetscScalar**);
EXTERN int MatGetColumnVector(Mat,Vec,int);
EXTERN int MatGetArray(Mat,PetscScalar **);
EXTERN int MatRestoreArray(Mat,PetscScalar **);
EXTERN int MatGetBlockSize(Mat,int *);

EXTERN int MatMult(Mat,Vec,Vec);
EXTERN int MatMultAdd(Mat,Vec,Vec,Vec);
EXTERN int MatMultTranspose(Mat,Vec,Vec);
EXTERN int MatMultTransposeAdd(Mat,Vec,Vec,Vec);

/*E
    MatDuplicateOption - Indicates if a duplicated sparse matrix should have
  its numerical values copied over or just its nonzero structure.

    Level: beginner

   Any additions/changes here MUST also be made in include/finclude/petscmat.h

.seealso: MatDuplicate()
E*/
typedef enum {MAT_DO_NOT_COPY_VALUES,MAT_COPY_VALUES} MatDuplicateOption;

EXTERN int MatConvertRegister(char*,char*,char*,int (*)(Mat,MatType,Mat*));
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define MatConvertRegisterDynamic(a,b,c,d) MatConvertRegister(a,b,c,0)
#else
#define MatConvertRegisterDynamic(a,b,c,d) MatConvertRegister(a,b,c,d)
#endif
EXTERN int        MatConvertRegisterAll(char*);
EXTERN int        MatConvertRegisterDestroy(void);
extern PetscTruth MatConvertRegisterAllCalled;
extern PetscFList MatConvertList;
EXTERN int        MatConvert(Mat,MatType,Mat*);
EXTERN int        MatDuplicate(Mat,MatDuplicateOption,Mat*);

/*E
    MatStructure - Indicates if the matrix has the same nonzero structure

    Level: beginner

   Any additions/changes here MUST also be made in include/finclude/petscmat.h

.seealso: MatCopy(), SLESSetOperators(), PCSetOperators()
E*/
typedef enum {SAME_NONZERO_PATTERN,DIFFERENT_NONZERO_PATTERN,SAME_PRECONDITIONER} MatStructure;

EXTERN int MatCopy(Mat,Mat,MatStructure);
EXTERN int MatView(Mat,PetscViewer);

EXTERN int MatLoadRegister(char*,char*,char*,int (*)(PetscViewer,MatType,Mat*));
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define MatLoadRegisterDynamic(a,b,c,d) MatLoadRegister(a,b,c,0)
#else
#define MatLoadRegisterDynamic(a,b,c,d) MatLoadRegister(a,b,c,d)
#endif
EXTERN int        MatLoadRegisterAll(char*);
EXTERN int        MatLoadRegisterDestroy(void);
extern PetscTruth MatLoadRegisterAllCalled;
extern PetscFList MatLoadList;
EXTERN int        MatLoad(PetscViewer,MatType,Mat*);

EXTERN int MatGetRowIJ(Mat,int,PetscTruth,int*,int **,int **,PetscTruth *);
EXTERN int MatRestoreRowIJ(Mat,int,PetscTruth,int *,int **,int **,PetscTruth *);
EXTERN int MatGetColumnIJ(Mat,int,PetscTruth,int*,int **,int **,PetscTruth *);
EXTERN int MatRestoreColumnIJ(Mat,int,PetscTruth,int *,int **,int **,PetscTruth *);

/*S
     MatInfo - Context of matrix information, used with MatGetInfo()

   In Fortran this is simply a double precision array of dimension MAT_INFO_SIZE

   Level: intermediate

  Concepts: matrix^nonzero information

.seealso:  MatGetInfo(), MatInfoType
S*/
typedef struct {
  PetscLogDouble rows_global,columns_global;         /* number of global rows and columns */
  PetscLogDouble rows_local,columns_local;           /* number of local rows and columns */
  PetscLogDouble block_size;                         /* block size */
  PetscLogDouble nz_allocated,nz_used,nz_unneeded;   /* number of nonzeros */
  PetscLogDouble memory;                             /* memory allocated */
  PetscLogDouble assemblies;                         /* number of matrix assemblies called */
  PetscLogDouble mallocs;                            /* number of mallocs during MatSetValues() */
  PetscLogDouble fill_ratio_given,fill_ratio_needed; /* fill ratio for LU/ILU */
  PetscLogDouble factor_mallocs;                     /* number of mallocs during factorization */
} MatInfo;

/*E
    MatInfoType - Indicates if you want information about the local part of the matrix,
     the entire parallel matrix or the maximum over all the local parts.

    Level: beginner

   Any additions/changes here MUST also be made in include/finclude/petscmat.h

.seealso: MatGetInfo(), MatInfo
E*/
typedef enum {MAT_LOCAL=1,MAT_GLOBAL_MAX=2,MAT_GLOBAL_SUM=3} MatInfoType;
EXTERN int MatGetInfo(Mat,MatInfoType,MatInfo*);
EXTERN int MatValid(Mat,PetscTruth*);
EXTERN int MatGetDiagonal(Mat,Vec);
EXTERN int MatGetRowMax(Mat,Vec);
EXTERN int MatTranspose(Mat,Mat*);
EXTERN int MatPermute(Mat,IS,IS,Mat *);
EXTERN int MatDiagonalScale(Mat,Vec,Vec);
EXTERN int MatDiagonalSet(Mat,Vec,InsertMode);
EXTERN int MatEqual(Mat,Mat,PetscTruth*);

EXTERN int MatNorm(Mat,NormType,PetscReal *);
EXTERN int MatZeroEntries(Mat);
EXTERN int MatZeroRows(Mat,IS,PetscScalar*);
EXTERN int MatZeroColumns(Mat,IS,PetscScalar*);

EXTERN int MatUseScaledForm(Mat,PetscTruth);
EXTERN int MatScaleSystem(Mat,Vec,Vec);
EXTERN int MatUnScaleSystem(Mat,Vec,Vec);

EXTERN int MatGetSize(Mat,int*,int*);
EXTERN int MatGetLocalSize(Mat,int*,int*);
EXTERN int MatGetOwnershipRange(Mat,int*,int*);

/*E
    MatReuse - Indicates if matrices obtained from a previous call to MatGetSubMatrices()
     or MatGetSubMatrix() are to be reused to store the new matrix values.

    Level: beginner

   Any additions/changes here MUST also be made in include/finclude/petscmat.h

.seealso: MatGetSubMatrices(), MatGetSubMatrix(), MatDestroyMatrices()
E*/
typedef enum {MAT_INITIAL_MATRIX,MAT_REUSE_MATRIX} MatReuse;
EXTERN int MatGetSubMatrices(Mat,int,IS *,IS *,MatReuse,Mat **);
EXTERN int MatDestroyMatrices(int,Mat **);
EXTERN int MatGetSubMatrix(Mat,IS,IS,int,MatReuse,Mat *);

EXTERN int MatIncreaseOverlap(Mat,int,IS *,int);

EXTERN int MatAXPY(PetscScalar *,Mat,Mat);
EXTERN int MatAYPX(PetscScalar *,Mat,Mat);
EXTERN int MatCompress(Mat);

EXTERN int MatScale(PetscScalar *,Mat);
EXTERN int MatShift(PetscScalar *,Mat);

EXTERN int MatSetLocalToGlobalMapping(Mat,ISLocalToGlobalMapping);
EXTERN int MatSetLocalToGlobalMappingBlock(Mat,ISLocalToGlobalMapping);
EXTERN int MatZeroRowsLocal(Mat,IS,PetscScalar*);
EXTERN int MatSetValuesLocal(Mat,int,int*,int,int*,PetscScalar*,InsertMode);
EXTERN int MatSetValuesBlockedLocal(Mat,int,int*,int,int*,PetscScalar*,InsertMode);

EXTERN int MatSetStashInitialSize(Mat,int,int);

EXTERN int MatInterpolateAdd(Mat,Vec,Vec,Vec);
EXTERN int MatInterpolate(Mat,Vec,Vec);
EXTERN int MatRestrict(Mat,Vec,Vec);

/*
      These three (or four) macros MUST be used together. The third one closes the open { of the first one
*/
#define MatPreallocateInitialize(comm,nrows,ncols,dnz,onz) 0; \
{ \
  int _4_ierr,__tmp = (nrows),__ctmp = (ncols),__rstart,__start,__end; \
  _4_ierr = PetscMalloc(2*__tmp*sizeof(int),&dnz);CHKERRQ(_4_ierr);onz = dnz + __tmp;\
  _4_ierr = PetscMemzero(dnz,2*__tmp*sizeof(int));CHKERRQ(_4_ierr);\
  _4_ierr = MPI_Scan(&__ctmp,&__end,1,MPI_INT,MPI_SUM,comm);CHKERRQ(_4_ierr); __start = __end - __ctmp;\
  _4_ierr = MPI_Scan(&__tmp,&__rstart,1,MPI_INT,MPI_SUM,comm);CHKERRQ(_4_ierr); __rstart = __rstart - __tmp;

#define MatPreallocateSetLocal(map,nrows,rows,ncols,cols,dnz,onz) 0;\
{\
  int __l;\
  _4_ierr = ISLocalToGlobalMappingApply(map,nrows,rows,rows);CHKERRQ(_4_ierr);\
  _4_ierr = ISLocalToGlobalMappingApply(map,ncols,cols,cols);CHKERRQ(_4_ierr);\
  for (__l=0;__l<nrows;__l++) {\
    _4_ierr = MatPreallocateSet((rows)[__l],ncols,cols,dnz,onz);CHKERRQ(_4_ierr);\
  }\
}
    
#define MatPreallocateSet(row,nc,cols,dnz,onz) 0;\
{ int __i; \
  for (__i=0; __i<nc; __i++) {\
    if (cols[__i] < __start || cols[__i] >= __end) onz[row - __rstart]++; \
  }\
  dnz[row - __rstart] = nc - onz[row - __rstart];\
}

#define MatPreallocateFinalize(dnz,onz) 0;_4_ierr = PetscFree(dnz);CHKERRQ(_4_ierr);}

/* Routines unique to particular data structures */
EXTERN int MatShellGetContext(Mat,void **);

EXTERN int MatBDiagGetData(Mat,int*,int*,int**,int**,PetscScalar***);
EXTERN int MatSeqAIJSetColumnIndices(Mat,int *);
EXTERN int MatSeqBAIJSetColumnIndices(Mat,int *);
EXTERN int MatCreateSeqAIJWithArrays(MPI_Comm,int,int,int*,int*,PetscScalar *,Mat*);

EXTERN int MatSeqBAIJSetPreallocation(Mat,int,int,int*);
EXTERN int MatSeqSBAIJSetPreallocation(Mat,int,int,int*);
EXTERN int MatSeqAIJSetPreallocation(Mat,int,int*);
EXTERN int MatSeqDensePreallocation(Mat,PetscScalar*);
EXTERN int MatSeqBDiagSetPreallocation(Mat,int,int,int*,PetscScalar**);
EXTERN int MatSeqDenseSetPreallocation(Mat,PetscScalar*);

EXTERN int MatMPIBAIJSetPreallocation(Mat,int,int,int*,int,int*);
EXTERN int MatMPISBAIJSetPreallocation(Mat,int,int,int*,int,int*);
EXTERN int MatMPIAIJSetPreallocation(Mat,int,int*,int,int*);
EXTERN int MatMPIDensePreallocation(Mat,PetscScalar*);
EXTERN int MatMPIBDiagSetPreallocation(Mat,int,int,int*,PetscScalar**);
EXTERN int MatMPIAdjSetPreallocation(Mat,int*,int*,int*);
EXTERN int MatMPIDenseSetPreallocation(Mat,PetscScalar*);
EXTERN int MatMPIRowbsSetPreallocation(Mat,int,int*);
EXTERN int MatMPIAIJGetSeqAIJ(Mat,Mat*,Mat*,int**);
EXTERN int MatMPIBAIJGetSeqBAIJ(Mat,Mat*,Mat*,int**);
EXTERN int MatAdicSetLocalFunction(Mat,void (*)(void));

EXTERN int MatStoreValues(Mat);
EXTERN int MatRetrieveValues(Mat);

EXTERN int MatDAADSetCtx(Mat,void*);

/* 
  These routines are not usually accessed directly, rather solving is 
  done through the SLES, KSP and PC interfaces.
*/

/*E
    MatOrderingType - String with the name of a PETSc matrix ordering or the creation function
       with an optional dynamic library name, for example 
       http://www.mcs.anl.gov/petsc/lib.a:orderingcreate()

   Level: beginner

.seealso: MatGetOrdering()
E*/
typedef char* MatOrderingType;
#define MATORDERING_NATURAL   "natural"
#define MATORDERING_ND        "nd"
#define MATORDERING_1WD       "1wd"
#define MATORDERING_RCM       "rcm"
#define MATORDERING_QMD       "qmd"
#define MATORDERING_ROWLENGTH "rowlength"
#define MATORDERING_DSC_ND    "dsc_nd"
#define MATORDERING_DSC_MMD   "dsc_mmd"
#define MATORDERING_DSC_MDF   "dsc_mdf"

EXTERN int MatGetOrdering(Mat,MatOrderingType,IS*,IS*);
EXTERN int MatOrderingRegister(char*,char*,char*,int(*)(Mat,MatOrderingType,IS*,IS*));
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define MatOrderingRegisterDynamic(a,b,c,d) MatOrderingRegister(a,b,c,0)
#else
#define MatOrderingRegisterDynamic(a,b,c,d) MatOrderingRegister(a,b,c,d)
#endif
EXTERN int        MatOrderingRegisterDestroy(void);
EXTERN int        MatOrderingRegisterAll(char*);
extern PetscTruth MatOrderingRegisterAllCalled;
extern PetscFList      MatOrderingList;

EXTERN int MatReorderForNonzeroDiagonal(Mat,PetscReal,IS,IS);

EXTERN int MatCholeskyFactor(Mat,IS,PetscReal);
EXTERN int MatCholeskyFactorSymbolic(Mat,IS,PetscReal,Mat*);
EXTERN int MatCholeskyFactorNumeric(Mat,Mat*);

/*S 
   MatILUInfo - Data based into the matrix ILU factorization routines

   In Fortran these are simply double precision arrays of size MAT_ILUINFO_SIZE

   Notes: These are not usually directly used by users, instead use the PC type of ILU
          All entries are double precision.

   Level: developer

.seealso: MatILUFactorSymbolic(), MatILUFactor(), MatLUInfo, MatCholeskyInfo

S*/
typedef struct {
  PetscReal     levels;         /* ILU(levels) */ 
  PetscReal     fill;           /* expected fill; nonzeros in factored matrix/nonzeros in original matrix*/
  PetscReal     diagonal_fill;  /* force diagonal to fill in if initially not filled */
  PetscReal     dt;             /* drop tolerance */
  PetscReal     dtcol;          /* tolerance for pivoting */
  PetscReal     dtcount;        /* maximum nonzeros to be allowed per row */
  PetscReal     damping;        /* scaling of identity added to matrix to prevent zero pivots */
  PetscReal     damp;           /* if is 1.0 and factorization fails, damp until successful */
  PetscReal     zeropivot; /* pivot is called zero if less than this */
} MatILUInfo;

/*S 
   MatLUInfo - Data based into the matrix LU factorization routines

   In Fortran these are simply double precision arrays of size MAT_LUINFO_SIZE

   Notes: These are not usually directly used by users, instead use the PC type of LU
          All entries are double precision.

   Level: developer

.seealso: MatLUFactorSymbolic(), MatILUInfo, MatCholeskyInfo

S*/
typedef struct {
  PetscReal     fill;    /* expected fill; nonzeros in factored matrix/nonzeros in original matrix */
  PetscReal     dtcol;   /* tolerance for pivoting; pivot if off_diagonal*dtcol > diagonal */
  PetscReal     damping; /* scaling of identity added to matrix to prevent zero pivots */
  PetscReal     damp;    /* if this is 1.0 and factorization fails, damp until successful */
  PetscReal     zeropivot; /* pivot is called zero if less than this */
} MatLUInfo;

/*S 
   MatCholeskyInfo - Data based into the matrix Cholesky factorization routines

   In Fortran these are simply double precision arrays of size MAT_CHOLESKYINFO_SIZE

   Notes: These are not usually directly used by users, instead use the PC type of Cholesky
          All entries are double precision.

   Level: developer

.seealso: MatCholeskyFactorSymbolic(), MatLUInfo, MatILUInfo

S*/
typedef struct {
  PetscReal     fill;    /* expected fill; nonzeros in factored matrix/nonzeros in original matrix */
  PetscReal     damping; /* scaling of identity added to matrix to prevent zero pivots */
  PetscReal     damp;    /* if this is 1.0 and factorization fails, damp until successful */
} MatCholeskyInfo;

EXTERN int MatLUFactor(Mat,IS,IS,MatLUInfo*);
EXTERN int MatILUFactor(Mat,IS,IS,MatILUInfo*);
EXTERN int MatLUFactorSymbolic(Mat,IS,IS,MatLUInfo*,Mat*);
EXTERN int MatILUFactorSymbolic(Mat,IS,IS,MatILUInfo*,Mat*);
EXTERN int MatICCFactorSymbolic(Mat,IS,PetscReal,int,Mat*);
EXTERN int MatICCFactor(Mat,IS,PetscReal,int);
EXTERN int MatLUFactorNumeric(Mat,Mat*);
EXTERN int MatILUDTFactor(Mat,MatILUInfo*,IS,IS,Mat *);

EXTERN int MatSolve(Mat,Vec,Vec);
EXTERN int MatForwardSolve(Mat,Vec,Vec);
EXTERN int MatBackwardSolve(Mat,Vec,Vec);
EXTERN int MatSolveAdd(Mat,Vec,Vec,Vec);
EXTERN int MatSolveTranspose(Mat,Vec,Vec);
EXTERN int MatSolveTransposeAdd(Mat,Vec,Vec,Vec);

EXTERN int MatSetUnfactored(Mat);

/*  MatSORType may be bitwise ORd together, so do not change the numbers */
/*E
    MatSORType - What type of (S)SOR to perform

    Level: beginner

   May be bitwise ORd together

   Any additions/changes here MUST also be made in include/finclude/petscmat.h

.seealso: MatRelax()
E*/
typedef enum {SOR_FORWARD_SWEEP=1,SOR_BACKWARD_SWEEP=2,SOR_SYMMETRIC_SWEEP=3,
              SOR_LOCAL_FORWARD_SWEEP=4,SOR_LOCAL_BACKWARD_SWEEP=8,
              SOR_LOCAL_SYMMETRIC_SWEEP=12,SOR_ZERO_INITIAL_GUESS=16,
              SOR_EISENSTAT=32,SOR_APPLY_UPPER=64,SOR_APPLY_LOWER=128} MatSORType;
EXTERN int MatRelax(Mat,Vec,PetscReal,MatSORType,PetscReal,int,Vec);

/* 
    These routines are for efficiently computing Jacobians via finite differences.
*/

/*E
    MatColoringType - String with the name of a PETSc matrix coloring or the creation function
       with an optional dynamic library name, for example 
       http://www.mcs.anl.gov/petsc/lib.a:coloringcreate()

   Level: beginner

.seealso: MatGetColoring()
E*/
typedef char* MatColoringType;
#define MATCOLORING_NATURAL "natural"
#define MATCOLORING_SL      "sl"
#define MATCOLORING_LF      "lf"
#define MATCOLORING_ID      "id"

EXTERN int MatGetColoring(Mat,MatColoringType,ISColoring*);
EXTERN int MatColoringRegister(char*,char*,char*,int(*)(Mat,MatColoringType,ISColoring *));
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define MatColoringRegisterDynamic(a,b,c,d) MatColoringRegister(a,b,c,0)
#else
#define MatColoringRegisterDynamic(a,b,c,d) MatColoringRegister(a,b,c,d)
#endif
EXTERN int        MatColoringRegisterAll(char *);
extern PetscTruth MatColoringRegisterAllCalled;
EXTERN int        MatColoringRegisterDestroy(void);
EXTERN int        MatColoringPatch(Mat,int,int,int *,ISColoring*);

#define MAT_FDCOLORING_COOKIE PETSC_COOKIE + 23
/*S
     MatFDColoring - Object for computing a sparse Jacobian via finite differences
        and coloring

   Level: beginner

  Concepts: coloring, sparse Jacobian, finite differences

.seealso:  MatFDColoringCreate()
S*/
typedef struct _p_MatFDColoring *MatFDColoring;

EXTERN int MatFDColoringCreate(Mat,ISColoring,MatFDColoring *);
EXTERN int MatFDColoringDestroy(MatFDColoring);
EXTERN int MatFDColoringView(MatFDColoring,PetscViewer);
EXTERN int MatFDColoringSetFunction(MatFDColoring,int (*)(void),void*);
EXTERN int MatFDColoringSetParameters(MatFDColoring,PetscReal,PetscReal);
EXTERN int MatFDColoringSetFrequency(MatFDColoring,int);
EXTERN int MatFDColoringGetFrequency(MatFDColoring,int*);
EXTERN int MatFDColoringSetFromOptions(MatFDColoring);
EXTERN int MatFDColoringApply(Mat,MatFDColoring,Vec,MatStructure*,void *);
EXTERN int MatFDColoringApplyTS(Mat,MatFDColoring,PetscReal,Vec,MatStructure*,void *);
EXTERN int MatFDColoringSetRecompute(MatFDColoring);
EXTERN int MatFDColoringSetF(MatFDColoring,Vec);

/* 
    These routines are for partitioning matrices: currently used only 
  for adjacency matrix, MatCreateMPIAdj().
*/
#define MATPARTITIONING_COOKIE PETSC_COOKIE + 25

/*S
     MatPartitioning - Object for managing the partitioning of a matrix or graph

   Level: beginner

  Concepts: partitioning

.seealso:  MatParitioningCreate(), MatPartitioningType
S*/
typedef struct _p_MatPartitioning *MatPartitioning;

/*E
    MatPartitioningType - String with the name of a PETSc matrix partitioing or the creation function
       with an optional dynamic library name, for example 
       http://www.mcs.anl.gov/petsc/lib.a:partitioningcreate()

   Level: beginner

.seealso: MatPartitioingCreate(), MatPartitioning
E*/
typedef char* MatPartitioningType;
#define MATPARTITIONING_CURRENT  "current"
#define MATPARTITIONING_PARMETIS "parmetis"

EXTERN int MatPartitioningCreate(MPI_Comm,MatPartitioning*);
EXTERN int MatPartitioningSetType(MatPartitioning,MatPartitioningType);
EXTERN int MatPartitioningSetAdjacency(MatPartitioning,Mat);
EXTERN int MatPartitioningSetVertexWeights(MatPartitioning,int*);
EXTERN int MatPartitioningApply(MatPartitioning,IS*);
EXTERN int MatPartitioningDestroy(MatPartitioning);

EXTERN int MatPartitioningRegister(char*,char*,char*,int(*)(MatPartitioning));
#if defined(PETSC_USE_DYNAMIC_LIBRARIES)
#define MatPartitioningRegisterDynamic(a,b,c,d) MatPartitioningRegister(a,b,c,0)
#else
#define MatPartitioningRegisterDynamic(a,b,c,d) MatPartitioningRegister(a,b,c,d)
#endif

EXTERN int        MatPartitioningRegisterAll(char *);
extern PetscTruth MatPartitioningRegisterAllCalled;
EXTERN int        MatPartitioningRegisterDestroy(void);

EXTERN int MatPartitioningView(MatPartitioning,PetscViewer);
EXTERN int MatPartitioningSetFromOptions(MatPartitioning);
EXTERN int MatPartitioningGetType(MatPartitioning,MatPartitioningType*);

EXTERN int MatPartitioningParmetisSetCoarseSequential(MatPartitioning);

/*
    If you add entries here you must also add them to finclude/petscmat.h
*/
typedef enum { MATOP_SET_VALUES=0,
               MATOP_GET_ROW=1,
               MATOP_RESTORE_ROW=2,
               MATOP_MULT=3,
               MATOP_MULT_ADD=4,
               MATOP_MULT_TRANSPOSE=5,
               MATOP_MULT_TRANSPOSE_ADD=6,
               MATOP_SOLVE=7,
               MATOP_SOLVE_ADD=8,
               MATOP_SOLVE_TRANSPOSE=9,
               MATOP_SOLVE_TRANSPOSE_ADD=10,
               MATOP_LUFACTOR=11,
               MATOP_CHOLESKYFACTOR=12,
               MATOP_RELAX=13,
               MATOP_TRANSPOSE=14,
               MATOP_GETINFO=15,
               MATOP_EQUAL=16,
               MATOP_GET_DIAGONAL=17,
               MATOP_DIAGONAL_SCALE=18,
               MATOP_NORM=19,
               MATOP_ASSEMBLY_BEGIN=20,
               MATOP_ASSEMBLY_END=21,
               MATOP_COMPRESS=22,
               MATOP_SET_OPTION=23,
               MATOP_ZERO_ENTRIES=24,
               MATOP_ZERO_ROWS=25,
               MATOP_LUFACTOR_SYMBOLIC=26,
               MATOP_LUFACTOR_NUMERIC=27,
               MATOP_CHOLESKY_FACTOR_SYMBOLIC=28,
               MATOP_CHOLESKY_FACTOR_NUMERIC=29,
               MATOP_GET_SIZE=30,
               MATOP_GET_LOCAL_SIZE=31,
               MATOP_GET_OWNERSHIP_RANGE=32,
               MATOP_ILUFACTOR_SYMBOLIC=33,
               MATOP_ICCFACTOR_SYMBOLIC=34,
               MATOP_GET_ARRAY=35,
               MATOP_RESTORE_ARRAY=36,

               MATOP_CONVERT_SAME_TYPE=37,
               MATOP_FORWARD_SOLVE=38,
               MATOP_BACKWARD_SOLVE=39,
               MATOP_ILUFACTOR=40,
               MATOP_ICCFACTOR=41,
               MATOP_AXPY=42,
               MATOP_GET_SUBMATRICES=43,
               MATOP_INCREASE_OVERLAP=44,
               MATOP_GET_VALUES=45,
               MATOP_COPY=46,
               MATOP_PRINT_HELP=47,
               MATOP_SCALE=48,
               MATOP_SHIFT=49,
               MATOP_DIAGONAL_SHIFT=50,
               MATOP_ILUDT_FACTOR=51,
               MATOP_GET_BLOCK_SIZE=52,
               MATOP_GET_ROW_IJ=53,
               MATOP_RESTORE_ROW_IJ=54,
               MATOP_GET_COLUMN_IJ=55,
               MATOP_RESTORE_COLUMN_IJ=56,
               MATOP_FDCOLORING_CREATE=57,
               MATOP_COLORING_PATCH=58,
               MATOP_SET_UNFACTORED=59,
               MATOP_PERMUTE=60,
               MATOP_SET_VALUES_BLOCKED=61,
               MATOP_DESTROY=250,
               MATOP_VIEW=251
             } MatOperation;
EXTERN int MatHasOperation(Mat,MatOperation,PetscTruth*);
EXTERN int MatShellSetOperation(Mat,MatOperation,void(*)());
EXTERN int MatShellGetOperation(Mat,MatOperation,void(**)());
EXTERN int MatShellSetContext(Mat,void*);

/*
   Codes for matrices stored on disk. By default they are
 stored in a universal format. By changing the format with 
 PetscViewerSetFormat(viewer,PETSC_VIEWER_BINARY_NATIVE); the matrices will
 be stored in a way natural for the matrix, for example dense matrices
 would be stored as dense. Matrices stored this way may only be
 read into matrices of the same time.
*/
#define MATRIX_BINARY_FORMAT_DENSE -1

/*
     New matrix classes not yet distributed 
*/
/*
    MatAIJIndices is a data structure for storing the nonzero location information
  for sparse matrices. Several matrices with identical nonzero structure can share
  the same MatAIJIndices.
*/ 
typedef struct _p_MatAIJIndices* MatAIJIndices;

EXTERN int MatCreateAIJIndices(int,int,int*,int*,PetscTruth,MatAIJIndices*);
EXTERN int MatCreateAIJIndicesEmpty(int,int,int*,PetscTruth,MatAIJIndices*);
EXTERN int MatAttachAIJIndices(MatAIJIndices,MatAIJIndices*);
EXTERN int MatDestroyAIJIndices(MatAIJIndices);
EXTERN int MatCopyAIJIndices(MatAIJIndices,MatAIJIndices*);
EXTERN int MatValidateAIJIndices(int,MatAIJIndices);
EXTERN int MatShiftAIJIndices(MatAIJIndices);
EXTERN int MatShrinkAIJIndices(MatAIJIndices);
EXTERN int MatTransposeAIJIndices(MatAIJIndices,MatAIJIndices*);

EXTERN int MatCreateSeqCSN(MPI_Comm,int,int,int*,int,Mat*);
EXTERN int MatCreateSeqCSN_Single(MPI_Comm,int,int,int*,int,Mat*);
EXTERN int MatCreateSeqCSNWithPrecision(MPI_Comm,int,int,int*,int,PetscScalarPrecision,Mat*);

EXTERN int MatCreateSeqCSNIndices(MPI_Comm,MatAIJIndices,int,Mat *);
EXTERN int MatCreateSeqCSNIndices_Single(MPI_Comm,MatAIJIndices,int,Mat *);
EXTERN int MatCreateSeqCSNIndicesWithPrecision(MPI_Comm,MatAIJIndices,int,PetscScalarPrecision,Mat *);

EXTERN int MatMPIBAIJSetHashTableFactor(Mat,PetscReal);
EXTERN int MatSeqAIJGetInodeSizes(Mat,int *,int *[],int *);
EXTERN int MatMPIRowbsGetColor(Mat,ISColoring *);

/*S
     MatNullSpace - Object that removes a null space from a vector, i.e.
         orthogonalizes the vector to a subsapce

   Level: beginner

  Concepts: matrix; linear operator, null space

.seealso:  MatNullSpaceCreate()
S*/
typedef struct _p_MatNullSpace* MatNullSpace;

#define MATNULLSPACE_COOKIE    PETSC_COOKIE+17

EXTERN int MatNullSpaceCreate(MPI_Comm,int,int,Vec *,MatNullSpace*);
EXTERN int MatNullSpaceDestroy(MatNullSpace);
EXTERN int MatNullSpaceRemove(MatNullSpace,Vec,Vec*);
EXTERN int MatNullSpaceAttach(Mat,MatNullSpace);
EXTERN int MatNullSpaceTest(MatNullSpace,Mat);

EXTERN int MatReorderingSeqSBAIJ(Mat A,IS isp);
EXTERN int MatMPISBAIJSetHashTableFactor(Mat,PetscReal);
EXTERN int MatSeqSBAIJSetColumnIndices(Mat,int *);


EXTERN int MatCreateMAIJ(Mat,int,Mat*);
EXTERN int MatMAIJRedimension(Mat,int,Mat*);
EXTERN int MatMAIJGetAIJ(Mat,Mat*);

EXTERN int MatMPIAdjSetValues(Mat,int*,int*,int*);

EXTERN int MatComputeExplicitOperator(Mat,Mat*);

#endif



