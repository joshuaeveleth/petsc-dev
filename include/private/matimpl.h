
#ifndef __MATIMPL_H
#define __MATIMPL_H

#include <petscmat.h>

/*
  This file defines the parts of the matrix data structure that are 
  shared by all matrix types.
*/

/*
    If you add entries here also add them to the MATOP enum
    in include/petscmat.h and include/finclude/petscmat.h
*/
typedef struct _MatOps *MatOps;
struct _MatOps {
  /* 0*/
  PetscErrorCode (*setvalues)(Mat,PetscInt,const PetscInt[],PetscInt,const PetscInt[],const PetscScalar[],InsertMode);
  PetscErrorCode (*getrow)(Mat,PetscInt,PetscInt *,PetscInt*[],PetscScalar*[]);
  PetscErrorCode (*restorerow)(Mat,PetscInt,PetscInt *,PetscInt *[],PetscScalar *[]);
  PetscErrorCode (*mult)(Mat,Vec,Vec);
  PetscErrorCode (*multadd)(Mat,Vec,Vec,Vec);
  /* 5*/
  PetscErrorCode (*multtranspose)(Mat,Vec,Vec);
  PetscErrorCode (*multtransposeadd)(Mat,Vec,Vec,Vec);
  PetscErrorCode (*solve)(Mat,Vec,Vec);
  PetscErrorCode (*solveadd)(Mat,Vec,Vec,Vec);
  PetscErrorCode (*solvetranspose)(Mat,Vec,Vec);
  /*10*/
  PetscErrorCode (*solvetransposeadd)(Mat,Vec,Vec,Vec);
  PetscErrorCode (*lufactor)(Mat,IS,IS,const MatFactorInfo*);
  PetscErrorCode (*choleskyfactor)(Mat,IS,const MatFactorInfo*);
  PetscErrorCode (*sor)(Mat,Vec,PetscReal,MatSORType,PetscReal,PetscInt,PetscInt,Vec);
  PetscErrorCode (*transpose)(Mat,MatReuse,Mat *);
  /*15*/
  PetscErrorCode (*getinfo)(Mat,MatInfoType,MatInfo*);
  PetscErrorCode (*equal)(Mat,Mat,PetscBool  *);
  PetscErrorCode (*getdiagonal)(Mat,Vec);
  PetscErrorCode (*diagonalscale)(Mat,Vec,Vec);
  PetscErrorCode (*norm)(Mat,NormType,PetscReal*);
  /*20*/
  PetscErrorCode (*assemblybegin)(Mat,MatAssemblyType);
  PetscErrorCode (*assemblyend)(Mat,MatAssemblyType);
  PetscErrorCode (*setoption)(Mat,MatOption,PetscBool );
  PetscErrorCode (*zeroentries)(Mat);
  /*24*/
  PetscErrorCode (*zerorows)(Mat,PetscInt,const PetscInt[],PetscScalar,Vec,Vec);
  PetscErrorCode (*lufactorsymbolic)(Mat,Mat,IS,IS,const MatFactorInfo*);
  PetscErrorCode (*lufactornumeric)(Mat,Mat,const MatFactorInfo*);
  PetscErrorCode (*choleskyfactorsymbolic)(Mat,Mat,IS,const MatFactorInfo*);
  PetscErrorCode (*choleskyfactornumeric)(Mat,Mat,const MatFactorInfo*);
  /*29*/
  PetscErrorCode (*setuppreallocation)(Mat);
  PetscErrorCode (*ilufactorsymbolic)(Mat,Mat,IS,IS,const MatFactorInfo*);
  PetscErrorCode (*iccfactorsymbolic)(Mat,Mat,IS,const MatFactorInfo*);
  PetscErrorCode (*getarray)(Mat,PetscScalar**);
  PetscErrorCode (*restorearray)(Mat,PetscScalar**);
  /*34*/
  PetscErrorCode (*duplicate)(Mat,MatDuplicateOption,Mat*);
  PetscErrorCode (*forwardsolve)(Mat,Vec,Vec);
  PetscErrorCode (*backwardsolve)(Mat,Vec,Vec);
  PetscErrorCode (*ilufactor)(Mat,IS,IS,const MatFactorInfo*);
  PetscErrorCode (*iccfactor)(Mat,IS,const MatFactorInfo*);
  /*39*/
  PetscErrorCode (*axpy)(Mat,PetscScalar,Mat,MatStructure);
  PetscErrorCode (*getsubmatrices)(Mat,PetscInt,const IS[],const IS[],MatReuse,Mat *[]);
  PetscErrorCode (*increaseoverlap)(Mat,PetscInt,IS[],PetscInt);
  PetscErrorCode (*getvalues)(Mat,PetscInt,const PetscInt[],PetscInt,const PetscInt[],PetscScalar []);
  PetscErrorCode (*copy)(Mat,Mat,MatStructure);
  /*44*/
  PetscErrorCode (*getrowmax)(Mat,Vec,PetscInt[]);
  PetscErrorCode (*scale)(Mat,PetscScalar);
  PetscErrorCode (*shift)(Mat,PetscScalar);
  PetscErrorCode (*diagonalset)(Mat,Vec,InsertMode);
  PetscErrorCode (*zerorowscolumns)(Mat,PetscInt,const PetscInt[],PetscScalar,Vec,Vec);
  /*49*/
  PetscErrorCode (*setblocksize)(Mat,PetscInt);
  PetscErrorCode (*getrowij)(Mat,PetscInt,PetscBool ,PetscBool ,PetscInt*,PetscInt *[],PetscInt *[],PetscBool  *);
  PetscErrorCode (*restorerowij)(Mat,PetscInt,PetscBool ,PetscBool ,PetscInt *,PetscInt *[],PetscInt *[],PetscBool  *);
  PetscErrorCode (*getcolumnij)(Mat,PetscInt,PetscBool ,PetscBool ,PetscInt*,PetscInt *[],PetscInt *[],PetscBool  *);
  PetscErrorCode (*restorecolumnij)(Mat,PetscInt,PetscBool ,PetscBool ,PetscInt*,PetscInt *[],PetscInt *[],PetscBool  *);
  /*54*/
  PetscErrorCode (*fdcoloringcreate)(Mat,ISColoring,MatFDColoring);
  PetscErrorCode (*coloringpatch)(Mat,PetscInt,PetscInt,ISColoringValue[],ISColoring*);
  PetscErrorCode (*setunfactored)(Mat);
  PetscErrorCode (*permute)(Mat,IS,IS,Mat*);
  PetscErrorCode (*setvaluesblocked)(Mat,PetscInt,const PetscInt[],PetscInt,const PetscInt[],const PetscScalar[],InsertMode);
  /*59*/
  PetscErrorCode (*getsubmatrix)(Mat,IS,IS,MatReuse,Mat*);
  PetscErrorCode (*destroy)(Mat);
  PetscErrorCode (*view)(Mat,PetscViewer);
  PetscErrorCode (*convertfrom)(Mat, const MatType,MatReuse,Mat*);
  PetscErrorCode (*usescaledform)(Mat,PetscBool );
  /*64*/
  PetscErrorCode (*scalesystem)(Mat,Vec,Vec);
  PetscErrorCode (*unscalesystem)(Mat,Vec,Vec);
  PetscErrorCode (*setlocaltoglobalmapping)(Mat,ISLocalToGlobalMapping,ISLocalToGlobalMapping);
  PetscErrorCode (*setvalueslocal)(Mat,PetscInt,const PetscInt[],PetscInt,const PetscInt[],const PetscScalar[],InsertMode);
  PetscErrorCode (*zerorowslocal)(Mat,PetscInt,const PetscInt[],PetscScalar,Vec,Vec);
  /*69*/
  PetscErrorCode (*getrowmaxabs)(Mat,Vec,PetscInt[]);
  PetscErrorCode (*getrowminabs)(Mat,Vec,PetscInt[]);
  PetscErrorCode (*convert)(Mat, const MatType,MatReuse,Mat*);
  PetscErrorCode (*setcoloring)(Mat,ISColoring);
  PetscErrorCode (*setvaluesadic)(Mat,void*);
  /*74*/
  PetscErrorCode (*setvaluesadifor)(Mat,PetscInt,void*);
  PetscErrorCode (*fdcoloringapply)(Mat,MatFDColoring,Vec,MatStructure*,void*);
  PetscErrorCode (*setfromoptions)(Mat);
  PetscErrorCode (*multconstrained)(Mat,Vec,Vec);
  PetscErrorCode (*multtransposeconstrained)(Mat,Vec,Vec);
  /*79*/
  PetscErrorCode (*findzerodiagonals)(Mat,IS*);
  PetscErrorCode (*mults)(Mat, Vecs, Vecs);
  PetscErrorCode (*solves)(Mat, Vecs, Vecs);
  PetscErrorCode (*getinertia)(Mat,PetscInt*,PetscInt*,PetscInt*);
  PetscErrorCode (*load)(Mat, PetscViewer);
  /*84*/
  PetscErrorCode (*issymmetric)(Mat,PetscReal,PetscBool *);
  PetscErrorCode (*ishermitian)(Mat,PetscReal,PetscBool *);
  PetscErrorCode (*isstructurallysymmetric)(Mat,PetscBool *);
  PetscErrorCode (*setvaluesblockedlocal)(Mat,PetscInt,const PetscInt[],PetscInt,const PetscInt[],const PetscScalar[],InsertMode);
  PetscErrorCode (*getvecs)(Mat,Vec*,Vec*);
  /*89*/
  PetscErrorCode (*matmult)(Mat,Mat,MatReuse,PetscReal,Mat*);
  PetscErrorCode (*matmultsymbolic)(Mat,Mat,PetscReal,Mat*);
  PetscErrorCode (*matmultnumeric)(Mat,Mat,Mat);
  PetscErrorCode (*ptap)(Mat,Mat,MatReuse,PetscReal,Mat*);
  PetscErrorCode (*ptapsymbolic)(Mat,Mat,PetscReal,Mat*); /* double dispatch wrapper routine */
  /*94*/
  PetscErrorCode (*ptapnumeric)(Mat,Mat,Mat);             /* double dispatch wrapper routine */
  PetscErrorCode (*mattransposemult)(Mat,Mat,MatReuse,PetscReal,Mat*);
  PetscErrorCode (*mattransposemultsymbolic)(Mat,Mat,PetscReal,Mat*);
  PetscErrorCode (*mattransposemultnumeric)(Mat,Mat,Mat);
  PetscErrorCode (*ptapsymbolic_seqaij)(Mat,Mat,PetscReal,Mat*); /* actual implememtation, A=seqaij */
  /*99*/
  PetscErrorCode (*ptapnumeric_seqaij)(Mat,Mat,Mat);             /* actual implememtation, A=seqaij */
  PetscErrorCode (*ptapsymbolic_mpiaij)(Mat,Mat,PetscReal,Mat*); /* actual implememtation, A=mpiaij */
  PetscErrorCode (*ptapnumeric_mpiaij)(Mat,Mat,Mat);             /* actual implememtation, A=mpiaij */
  PetscErrorCode (*conjugate)(Mat);                              /* complex conjugate */
  PetscErrorCode (*setsizes)(Mat,PetscInt,PetscInt,PetscInt,PetscInt);
  /*104*/
  PetscErrorCode (*setvaluesrow)(Mat,PetscInt,const PetscScalar[]);
  PetscErrorCode (*realpart)(Mat);
  PetscErrorCode (*imaginarypart)(Mat);
  PetscErrorCode (*getrowuppertriangular)(Mat);
  PetscErrorCode (*restorerowuppertriangular)(Mat);
  /*109*/
  PetscErrorCode (*matsolve)(Mat,Mat,Mat);
  PetscErrorCode (*getredundantmatrix)(Mat,PetscInt,MPI_Comm,PetscInt,MatReuse,Mat*);
  PetscErrorCode (*getrowmin)(Mat,Vec,PetscInt[]);
  PetscErrorCode (*getcolumnvector)(Mat,Vec,PetscInt);
  PetscErrorCode (*missingdiagonal)(Mat,PetscBool *,PetscInt*);
  /*114*/
  PetscErrorCode (*getseqnonzerostructure)(Mat,Mat *);
  PetscErrorCode (*create)(Mat);  
  PetscErrorCode (*getghosts)(Mat,PetscInt*,const PetscInt *[]);
  PetscErrorCode (*getlocalsubmatrix)(Mat,IS,IS,Mat*);
  PetscErrorCode (*restorelocalsubmatrix)(Mat,IS,IS,Mat*);
  /*119*/
  PetscErrorCode (*multdiagonalblock)(Mat,Vec,Vec);
  PetscErrorCode (*hermitiantranspose)(Mat,MatReuse,Mat*);
  PetscErrorCode (*multhermitiantranspose)(Mat,Vec,Vec);
  PetscErrorCode (*multhermitiantransposeadd)(Mat,Vec,Vec,Vec);
  PetscErrorCode (*getmultiprocblock)(Mat,MPI_Comm,Mat*);
  /*124*/
  PetscErrorCode (*findnonzerorows)(Mat,IS*);
  PetscErrorCode (*getcolumnnorms)(Mat,NormType,PetscReal*);
  PetscErrorCode (*invertblockdiagonal)(Mat,PetscScalar**);
  PetscErrorCode (*dummy4)(Mat,Vec,Vec,Vec);
  PetscErrorCode (*getsubmatricesparallel)(Mat,PetscInt,const IS[], const IS[], MatReuse, Mat**);
  /*129*/
  PetscErrorCode (*setvaluesbatch)(Mat,PetscInt,PetscInt,PetscInt*,const PetscScalar*);
  PetscErrorCode (*transposematmult)(Mat,Mat,MatReuse,PetscReal,Mat*);
  PetscErrorCode (*transposematmultsymbolic)(Mat,Mat,PetscReal,Mat*);
  PetscErrorCode (*transposematmultnumeric)(Mat,Mat,Mat);
  PetscErrorCode (*transposecoloringcreate)(Mat,ISColoring,MatTransposeColoring);
  /*134*/
  PetscErrorCode (*transcoloringapplysptoden)(MatTransposeColoring,Mat,Mat);
  PetscErrorCode (*transcoloringapplydentosp)(MatTransposeColoring,Mat,Mat);
  PetscErrorCode (*rart)(Mat,Mat,MatReuse,PetscReal,Mat*);
  PetscErrorCode (*rartsymbolic)(Mat,Mat,PetscReal,Mat*); /* double dispatch wrapper routine */
  PetscErrorCode (*rartnumeric)(Mat,Mat,Mat);             /* double dispatch wrapper routine */
};
/*
    If you add MatOps entries above also add them to the MATOP enum
    in include/petscmat.h and include/finclude/petscmat.h
*/

#include <petscsys.h>
extern PetscErrorCode  MatRegisterOp(MPI_Comm, const char[], PetscVoidFunction, const char[], PetscInt, ...);
extern PetscErrorCode  MatQueryOp(MPI_Comm, PetscVoidFunction*, const char[], PetscInt, ...);

typedef struct _p_MatBaseName* MatBaseName;
struct _p_MatBaseName {
  char        *bname,*sname,*mname;
  MatBaseName next;
};

extern MatBaseName MatBaseNameList;

/*
   Utility private matrix routines
*/
extern PetscErrorCode MatConvert_Basic(Mat, const MatType,MatReuse,Mat*);
extern PetscErrorCode MatCopy_Basic(Mat,Mat,MatStructure);
extern PetscErrorCode MatView_Private(Mat);

extern PetscErrorCode MatHeaderMerge(Mat,Mat);
extern PetscErrorCode MatHeaderReplace(Mat,Mat);
extern PetscErrorCode MatAXPYGetxtoy_Private(PetscInt,PetscInt*,PetscInt*,PetscInt*, PetscInt*,PetscInt*,PetscInt*, PetscInt**);
extern PetscErrorCode MatPtAP_Basic(Mat,Mat,MatReuse,PetscReal,Mat*);
extern PetscErrorCode MatDiagonalSet_Default(Mat,Vec,InsertMode);

/* 
  The stash is used to temporarily store inserted matrix values that 
  belong to another processor. During the assembly phase the stashed 
  values are moved to the correct processor and 
*/

typedef struct _MatStashSpace *PetscMatStashSpace;

struct _MatStashSpace {
  PetscMatStashSpace next;
  PetscScalar        *space_head,*val;
  PetscInt           *idx,*idy;
  PetscInt           total_space_size;
  PetscInt           local_used;
  PetscInt           local_remaining;
};

extern PetscErrorCode PetscMatStashSpaceGet(PetscInt,PetscInt,PetscMatStashSpace *);
extern PetscErrorCode PetscMatStashSpaceContiguous(PetscInt,PetscMatStashSpace *,PetscScalar *,PetscInt *,PetscInt *);
extern PetscErrorCode PetscMatStashSpaceDestroy(PetscMatStashSpace*);

typedef struct {
  PetscInt      nmax;                   /* maximum stash size */
  PetscInt      umax;                   /* user specified max-size */
  PetscInt      oldnmax;                /* the nmax value used previously */
  PetscInt      n;                      /* stash size */
  PetscInt      bs;                     /* block size of the stash */
  PetscInt      reallocs;               /* preserve the no of mallocs invoked */    
  PetscMatStashSpace space_head,space;  /* linked list to hold stashed global row/column numbers and matrix values */
  /* The following variables are used for communication */
  MPI_Comm      comm;
  PetscMPIInt   size,rank;
  PetscMPIInt   tag1,tag2;
  MPI_Request   *send_waits;            /* array of send requests */
  MPI_Request   *recv_waits;            /* array of receive requests */
  MPI_Status    *send_status;           /* array of send status */
  PetscInt      nsends,nrecvs;          /* numbers of sends and receives */
  PetscScalar   *svalues;               /* sending data */
  PetscInt      *sindices;    
  PetscScalar   **rvalues;              /* receiving data (values) */
  PetscInt      **rindices;             /* receiving data (indices) */
  PetscInt      nprocessed;             /* number of messages already processed */
  PetscMPIInt   *flg_v;                 /* indicates what messages have arrived so far and from whom */
  PetscBool     reproduce;
  PetscInt      reproduce_count;
} MatStash;

extern PetscErrorCode MatStashCreate_Private(MPI_Comm,PetscInt,MatStash*);
extern PetscErrorCode MatStashDestroy_Private(MatStash*);
extern PetscErrorCode MatStashScatterEnd_Private(MatStash*);
extern PetscErrorCode MatStashSetInitialSize_Private(MatStash*,PetscInt);
extern PetscErrorCode MatStashGetInfo_Private(MatStash*,PetscInt*,PetscInt*);
extern PetscErrorCode MatStashValuesRow_Private(MatStash*,PetscInt,PetscInt,const PetscInt[],const PetscScalar[],PetscBool );
extern PetscErrorCode MatStashValuesCol_Private(MatStash*,PetscInt,PetscInt,const PetscInt[],const PetscScalar[],PetscInt,PetscBool );
extern PetscErrorCode MatStashValuesRowBlocked_Private(MatStash*,PetscInt,PetscInt,const PetscInt[],const PetscScalar[],PetscInt,PetscInt,PetscInt);
extern PetscErrorCode MatStashValuesColBlocked_Private(MatStash*,PetscInt,PetscInt,const PetscInt[],const PetscScalar[],PetscInt,PetscInt,PetscInt);
extern PetscErrorCode MatStashScatterBegin_Private(Mat,MatStash*,PetscInt*);
extern PetscErrorCode MatStashScatterGetMesg_Private(MatStash*,PetscMPIInt*,PetscInt**,PetscInt**,PetscScalar**,PetscInt*);

typedef struct {
  PetscInt   dim;
  PetscInt   dims[4];
  PetscInt   starts[4];
  PetscBool  noc;        /* this is a single component problem, hence user will not set MatStencil.c */
} MatStencilInfo;

/* Info about using compressed row format */
typedef struct {
  PetscBool  check;                         /* indicates that at MatAssembly() it should check if compressed rows will be efficient */
  PetscBool  use;                           /* indicates compressed rows have been checked and will be used */
  PetscInt   nrows;                         /* number of non-zero rows */
  PetscInt   *i;                            /* compressed row pointer  */
  PetscInt   *rindex;                       /* compressed row index               */
} Mat_CompressedRow;
extern PetscErrorCode MatCheckCompressedRow(Mat,Mat_CompressedRow*,PetscInt*,PetscInt,PetscReal);

struct _p_Mat {
  PETSCHEADER(struct _MatOps);
  PetscLayout            rmap,cmap;
  void                   *data;            /* implementation-specific data */
  MatFactorType          factortype;       /* MAT_FACTOR_LU, ILU, CHOLESKY or ICC */
  PetscBool              assembled;        /* is the matrix assembled? */
  PetscBool              was_assembled;    /* new values inserted into assembled mat */
  PetscInt               num_ass;          /* number of times matrix has been assembled */
  PetscBool              same_nonzero;     /* matrix has same nonzero pattern as previous */
  MatInfo                info;             /* matrix information */
  InsertMode             insertmode;       /* have values been inserted in matrix or added? */
  MatStash               stash,bstash;     /* used for assembling off-proc mat emements */
  MatNullSpace           nullsp;           /* null space (operator is singular) */
  MatNullSpace           nearnullsp;       /* near null space to be used by multigrid methods */
  PetscBool              preallocated;
  MatStencilInfo         stencil;          /* information for structured grid */
  PetscBool              symmetric,hermitian,structurally_symmetric,spd;
  PetscBool              symmetric_set,hermitian_set,structurally_symmetric_set,spd_set; /* if true, then corresponding flag is correct*/
  PetscBool              symmetric_eternal;
  PetscBool              nooffprocentries,nooffproczerorows;
#if defined(PETSC_HAVE_CUSP)
  PetscCUSPFlag          valid_GPU_matrix; /* flag pointing to the matrix on the gpu*/
#endif
  void                   *spptr;          /* pointer for special library like SuperLU */
  MatSolverPackage       solvertype;
  };

#define MatPreallocated(A)  ((!(A)->preallocated) ? MatSetUpPreallocation(A) : 0)
extern PetscErrorCode MatAXPY_Basic(Mat,PetscScalar,Mat,MatStructure);
extern PetscErrorCode MatAXPY_BasicWithPreallocation(Mat,Mat,PetscScalar,Mat,MatStructure);
/*
    Object for partitioning graphs
*/

typedef struct _MatPartitioningOps *MatPartitioningOps;
struct _MatPartitioningOps {
  PetscErrorCode (*apply)(MatPartitioning,IS*);
  PetscErrorCode (*setfromoptions)(MatPartitioning);
  PetscErrorCode (*destroy)(MatPartitioning);
  PetscErrorCode (*view)(MatPartitioning,PetscViewer);
};

struct _p_MatPartitioning {
  PETSCHEADER(struct _MatPartitioningOps);
  Mat         adj;
  PetscInt    *vertex_weights;
  PetscReal   *part_weights;
  PetscInt    n;                                 /* number of partitions */
  void        *data;
  PetscInt    setupcalled;
};

/*
    MatFDColoring is used to compute Jacobian matrices efficiently
  via coloring. The data structure is explained below in an example.

   Color =   0    1     0    2   |   2      3       0 
   ---------------------------------------------------
            00   01              |          05
            10   11              |   14     15               Processor  0
                       22    23  |          25
                       32    33  | 
   ===================================================
                                 |   44     45     46
            50                   |          55               Processor 1
                                 |   64            66
   ---------------------------------------------------

    ncolors = 4;

    ncolumns      = {2,1,1,0}
    columns       = {{0,2},{1},{3},{}}
    nrows         = {4,2,3,3}
    rows          = {{0,1,2,3},{0,1},{1,2,3},{0,1,2}}
    columnsforrow = {{0,0,2,2},{1,1},{4,3,3},{5,5,5}}
    vscaleforrow  = {{,,,},{,},{,,},{,,}}
    vwscale       = {dx(0),dx(1),dx(2),dx(3)}               MPI Vec
    vscale        = {dx(0),dx(1),dx(2),dx(3),dx(4),dx(5)}   Seq Vec

    ncolumns      = {1,0,1,1}
    columns       = {{6},{},{4},{5}}
    nrows         = {3,0,2,2}
    rows          = {{0,1,2},{},{1,2},{1,2}}
    columnsforrow = {{6,0,6},{},{4,4},{5,5}}
    vscaleforrow =  {{,,},{},{,},{,}}
    vwscale       = {dx(4),dx(5),dx(6)}              MPI Vec
    vscale        = {dx(0),dx(4),dx(5),dx(6)}        Seq Vec

    See the routine MatFDColoringApply() for how this data is used
    to compute the Jacobian.

*/

struct  _p_MatFDColoring{
  PETSCHEADER(int);
  PetscInt       M,N,m;            /* total rows, columns; local rows */
  PetscInt       rstart;           /* first row owned by local processor */
  PetscInt       ncolors;          /* number of colors */
  PetscInt       *ncolumns;        /* number of local columns for a color */ 
  PetscInt       **columns;        /* lists the local columns of each color (using global column numbering) */
  PetscInt       *nrows;           /* number of local rows for each color */
  PetscInt       **rows;           /* lists the local rows for each color (using the local row numbering) */
  PetscInt       **columnsforrow;  /* lists the corresponding columns for those rows (using the global column) */ 
  PetscReal      error_rel;        /* square root of relative error in computing function */
  PetscReal      umin;             /* minimum allowable u'dx value */
  Vec            w1,w2,w3;         /* work vectors used in computing Jacobian */
  PetscErrorCode (*f)(void);       /* function that defines Jacobian */
  void           *fctx;            /* optional user-defined context for use by the function f */
  PetscInt       **vscaleforrow;   /* location in vscale for each columnsforrow[] entry */
  Vec            vscale;           /* holds FD scaling, i.e. 1/dx for each perturbed column */
  Vec            F;                /* current value of user provided function; can set with MatFDColoringSetF() */
  PetscInt       currentcolor;     /* color for which function evaluation is being done now */
  const char     *htype;            /* "wp" or "ds" */
  ISColoringType ctype;            /* IS_COLORING_GLOBAL or IS_COLORING_GHOSTED */

  void           *ftn_func_pointer,*ftn_func_cntx; /* serve the same purpose as *fortran_func_pointers in PETSc objects */
};

struct  _p_MatTransposeColoring{
  PETSCHEADER(int);
  PetscInt       M,N,m;            /* total rows, columns; local rows */
  PetscInt       rstart;           /* first row owned by local processor */
  PetscInt       ncolors;          /* number of colors */
  PetscInt       *ncolumns;        /* number of local columns for a color */ 
  PetscInt       *nrows;           /* number of local rows for each color */
  PetscInt       currentcolor;     /* color for which function evaluation is being done now */
  ISColoringType ctype;            /* IS_COLORING_GLOBAL or IS_COLORING_GHOSTED */
 
  PetscInt       *colorforrow,*colorforcol;  /* pointer to rows and columns */
  PetscInt       *rows;                  /* lists the local rows for each color (using the local row numbering) */
  PetscInt       *columnsforspidx;       /* maps (row,color) in the dense matrix to index of sparse matrix arrays a->j and a->a */
  PetscInt       *columns;               /* lists the local columns of each color (using global column numbering) */
};

/*
   Null space context for preconditioner/operators
*/
struct _p_MatNullSpace {
  PETSCHEADER(int);
  PetscBool      has_cnst;
  PetscInt       n;
  Vec*           vecs;
  PetscScalar*   alpha;                 /* for projections */
  Vec            vec;                   /* for out of place removals */
  PetscErrorCode (*remove)(MatNullSpace,Vec,void*);  /* for user provided removal function */
  void*          rmctx;                 /* context for remove() function */
};

/* 
   Checking zero pivot for LU, ILU preconditioners.
*/
typedef struct {
  PetscInt       nshift,nshift_max;
  PetscReal      shift_amount,shift_lo,shift_hi,shift_top,shift_fraction;
  PetscBool      newshift;
  PetscReal      rs;  /* active row sum of abs(offdiagonals) */
  PetscScalar    pv;  /* pivot of the active row */
} FactorShiftCtx;

extern PetscErrorCode MatFactorDumpMatrix(Mat);

#undef __FUNCT__
#define __FUNCT__ "MatPivotCheck_nz"
PETSC_STATIC_INLINE PetscErrorCode MatPivotCheck_nz(Mat mat,const MatFactorInfo *info,FactorShiftCtx *sctx,PetscInt row)
{
  PetscReal _rs   = sctx->rs;
  PetscReal _zero = info->zeropivot*_rs;

  PetscFunctionBegin;
  if (PetscAbsScalar(sctx->pv) <= _zero){
    /* force |diag| > zeropivot*rs */
    if (!sctx->nshift) sctx->shift_amount = info->shiftamount;
    else sctx->shift_amount *= 2.0;
    sctx->newshift = PETSC_TRUE;
    (sctx->nshift)++;
  } else {
    sctx->newshift = PETSC_FALSE;
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatPivotCheck_pd"
PETSC_STATIC_INLINE PetscErrorCode MatPivotCheck_pd(Mat mat,const MatFactorInfo *info,FactorShiftCtx *sctx,PetscInt row)
{
  PetscReal _rs   = sctx->rs;
  PetscReal _zero = info->zeropivot*_rs;

  PetscFunctionBegin;
  if (PetscRealPart(sctx->pv) <= _zero){
    /* force matfactor to be diagonally dominant */
    if (sctx->nshift == sctx->nshift_max) {
      sctx->shift_fraction = sctx->shift_hi;
    } else {
      sctx->shift_lo = sctx->shift_fraction;
      sctx->shift_fraction = (sctx->shift_hi+sctx->shift_lo)/2.;
    }
    sctx->shift_amount = sctx->shift_fraction * sctx->shift_top;
    sctx->nshift++;
    sctx->newshift = PETSC_TRUE;
  } else {
    sctx->newshift = PETSC_FALSE;
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatPivotCheck_inblocks"
PETSC_STATIC_INLINE PetscErrorCode MatPivotCheck_inblocks(Mat mat,const MatFactorInfo *info,FactorShiftCtx *sctx,PetscInt row)
{
  PetscReal _zero = info->zeropivot;

  PetscFunctionBegin;
  if (PetscAbsScalar(sctx->pv) <= _zero){
    sctx->pv          += info->shiftamount;
    sctx->shift_amount = 0.0;
    sctx->nshift++;
  }
  sctx->newshift = PETSC_FALSE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatPivotCheck_none"
PETSC_STATIC_INLINE PetscErrorCode MatPivotCheck_none(Mat mat,const MatFactorInfo *info,FactorShiftCtx *sctx,PetscInt row)
{
  PetscReal _zero = info->zeropivot;

  PetscFunctionBegin;
  sctx->newshift = PETSC_FALSE;
  if (PetscAbsScalar(sctx->pv) <= _zero) {
    PetscErrorCode ierr;
    PetscBool      flg = PETSC_FALSE;
    
    ierr = PetscOptionsGetBool(PETSC_NULL,"-mat_dump",&flg,PETSC_NULL);CHKERRQ(ierr);
    if (flg) {
      ierr = MatView(mat,PETSC_VIEWER_BINARY_(((PetscObject)mat)->comm));CHKERRQ(ierr);
    }
    SETERRQ3(PETSC_COMM_SELF,PETSC_ERR_MAT_LU_ZRPVT,"Zero pivot row %D value %G tolerance %G",row,PetscAbsScalar(sctx->pv),_zero);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatPivotCheck"
PETSC_STATIC_INLINE PetscErrorCode MatPivotCheck(Mat mat,const MatFactorInfo *info,FactorShiftCtx *sctx,PetscInt row)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (info->shifttype == (PetscReal) MAT_SHIFT_NONZERO){
    ierr = MatPivotCheck_nz(mat,info,sctx,row);CHKERRQ(ierr);
  } else if (info->shifttype == (PetscReal) MAT_SHIFT_POSITIVE_DEFINITE){
    ierr = MatPivotCheck_pd(mat,info,sctx,row);CHKERRQ(ierr);
  } else if (info->shifttype == (PetscReal) MAT_SHIFT_INBLOCKS){
    ierr = MatPivotCheck_inblocks(mat,info,sctx,row);CHKERRQ(ierr);
  } else {
    ierr = MatPivotCheck_none(mat,info,sctx,row);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

/* 
  Create and initialize a linked list 
  Input Parameters:
    idx_start - starting index of the list
    lnk_max   - max value of lnk indicating the end of the list
    nlnk      - max length of the list
  Output Parameters:
    lnk       - list initialized
    bt        - PetscBT (bitarray) with all bits set to false
    lnk_empty - flg indicating the list is empty
*/
#define PetscLLCreate(idx_start,lnk_max,nlnk,lnk,bt) \
  (PetscMalloc(nlnk*sizeof(PetscInt),&lnk) || PetscBTCreate(nlnk,bt) || PetscBTMemzero(nlnk,bt) || (lnk[idx_start] = lnk_max,0))

#define PetscLLCreate_new(idx_start,lnk_max,nlnk,lnk,bt,lnk_empty)\
  (PetscMalloc(nlnk*sizeof(PetscInt),&lnk) || PetscBTCreate(nlnk,bt) || PetscBTMemzero(nlnk,bt) || (lnk_empty = PETSC_TRUE,0) ||(lnk[idx_start] = lnk_max,0))

/*
  Add an index set into a sorted linked list
  Input Parameters:
    nidx      - number of input indices
    indices   - interger array
    idx_start - starting index of the list
    lnk       - linked list(an integer array) that is created
    bt        - PetscBT (bitarray), bt[idx]=true marks idx is in lnk
  output Parameters:
    nlnk      - number of newly added indices
    lnk       - the sorted(increasing order) linked list containing new and non-redundate entries from indices
    bt        - updated PetscBT (bitarray) 
*/
#define PetscLLAdd(nidx,indices,idx_start,nlnk,lnk,bt) 0;\
{\
  PetscInt _k,_entry,_location,_lnkdata;\
  nlnk     = 0;\
  _lnkdata = idx_start;\
  for (_k=0; _k<nidx; _k++){\
    _entry = indices[_k];\
    if (!PetscBTLookupSet(bt,_entry)){  /* new entry */\
      /* search for insertion location */\
      /* start from the beginning if _entry < previous _entry */\
      if (_k && _entry < _lnkdata) _lnkdata  = idx_start;\
      do {\
        _location = _lnkdata;\
        _lnkdata  = lnk[_location];\
      } while (_entry > _lnkdata);\
      /* insertion location is found, add entry into lnk */\
      lnk[_location] = _entry;\
      lnk[_entry]    = _lnkdata;\
      nlnk++;\
      _lnkdata = _entry; /* next search starts from here if next_entry > _entry */\
    }\
  }\
}

/*
  Add a permuted index set into a sorted linked list
  Input Parameters:
    nidx      - number of input indices
    indices   - interger array
    perm      - permutation of indices
    idx_start - starting index of the list
    lnk       - linked list(an integer array) that is created
    bt        - PetscBT (bitarray), bt[idx]=true marks idx is in lnk
  output Parameters:
    nlnk      - number of newly added indices
    lnk       - the sorted(increasing order) linked list containing new and non-redundate entries from indices
    bt        - updated PetscBT (bitarray) 
*/
#define PetscLLAddPerm(nidx,indices,perm,idx_start,nlnk,lnk,bt) 0;\
{\
  PetscInt _k,_entry,_location,_lnkdata;\
  nlnk     = 0;\
  _lnkdata = idx_start;\
  for (_k=0; _k<nidx; _k++){\
    _entry = perm[indices[_k]];\
    if (!PetscBTLookupSet(bt,_entry)){  /* new entry */\
      /* search for insertion location */\
      /* start from the beginning if _entry < previous _entry */\
      if (_k && _entry < _lnkdata) _lnkdata  = idx_start;\
      do {\
        _location = _lnkdata;\
        _lnkdata  = lnk[_location];\
      } while (_entry > _lnkdata);\
      /* insertion location is found, add entry into lnk */\
      lnk[_location] = _entry;\
      lnk[_entry]    = _lnkdata;\
      nlnk++;\
      _lnkdata = _entry; /* next search starts from here if next_entry > _entry */\
    }\
  }\
}

/*
  Add a SORTED ascending index set into a sorted linked list - same as PetscLLAdd() bus skip 'if (_k && _entry < _lnkdata) _lnkdata  = idx_start;'
  Input Parameters:
    nidx      - number of input indices
    indices   - sorted interger array 
    idx_start - starting index of the list
    lnk       - linked list(an integer array) that is created
    bt        - PetscBT (bitarray), bt[idx]=true marks idx is in lnk
  output Parameters:
    nlnk      - number of newly added indices
    lnk       - the sorted(increasing order) linked list containing new and non-redundate entries from indices
    bt        - updated PetscBT (bitarray) 
*/
#define PetscLLAddSorted(nidx,indices,idx_start,nlnk,lnk,bt) 0;\
{\
  PetscInt _k,_entry,_location,_lnkdata;\
  nlnk      = 0;\
  _lnkdata  = idx_start;\
  for (_k=0; _k<nidx; _k++){\
    _entry = indices[_k];\
    if (!PetscBTLookupSet(bt,_entry)){  /* new entry */\
      /* search for insertion location */\
      do {\
        _location = _lnkdata;\
        _lnkdata  = lnk[_location];\
      } while (_entry > _lnkdata);\
      /* insertion location is found, add entry into lnk */\
      lnk[_location] = _entry;\
      lnk[_entry]    = _lnkdata;\
      nlnk++;\
      _lnkdata = _entry; /* next search starts from here */\
    }\
  }\
}

#define PetscLLAddSorted_new(nidx,indices,idx_start,lnk_empty,nlnk,lnk,bt) 0; \
{\
  PetscInt _k,_entry,_location,_lnkdata;\
  if (lnk_empty){\
    _lnkdata  = idx_start;                      \
    for (_k=0; _k<nidx; _k++){                  \
      _entry = indices[_k];                             \
      PetscBTSet(bt,_entry);  /* mark the new entry */          \
          _location = _lnkdata;                                 \
          _lnkdata  = lnk[_location];                           \
        /* insertion location is found, add entry into lnk */   \
        lnk[_location] = _entry;                                \
        lnk[_entry]    = _lnkdata;                              \
        _lnkdata = _entry; /* next search starts from here */   \
    }                                                           \
    /*\
    lnk[indices[nidx-1]] = lnk[idx_start];\
    lnk[idx_start]       = indices[0];\
    PetscBTSet(bt,indices[0]);  \
    for (_k=1; _k<nidx; _k++){                  \
      PetscBTSet(bt,indices[_k]);                                          \
      lnk[indices[_k-1]] = indices[_k];                                  \
    }                                                           \
     */\
    nlnk      = nidx;\
    lnk_empty = PETSC_FALSE;\
  } else {\
    nlnk      = 0;                              \
    _lnkdata  = idx_start;                      \
    for (_k=0; _k<nidx; _k++){                  \
      _entry = indices[_k];                             \
      if (!PetscBTLookupSet(bt,_entry)){  /* new entry */       \
        /* search for insertion location */                     \
        do {                                                    \
          _location = _lnkdata;                                 \
          _lnkdata  = lnk[_location];                           \
        } while (_entry > _lnkdata);                            \
        /* insertion location is found, add entry into lnk */   \
        lnk[_location] = _entry;                                \
        lnk[_entry]    = _lnkdata;                              \
        nlnk++;                                                 \
        _lnkdata = _entry; /* next search starts from here */   \
      }                                                         \
    }                                                           \
  }                                                             \
}

/*
  Add a SORTED index set into a sorted linked list used for LUFactorSymbolic()
  Same as PetscLLAddSorted() with an additional operation:
       count the number of input indices that are no larger than 'diag' 
  Input Parameters:
    indices   - sorted interger array 
    idx_start - starting index of the list, index of pivot row
    lnk       - linked list(an integer array) that is created
    bt        - PetscBT (bitarray), bt[idx]=true marks idx is in lnk
    diag      - index of the active row in LUFactorSymbolic
    nzbd      - number of input indices with indices <= idx_start
    im        - im[idx_start] is initialized as num of nonzero entries in row=idx_start
  output Parameters:
    nlnk      - number of newly added indices
    lnk       - the sorted(increasing order) linked list containing new and non-redundate entries from indices
    bt        - updated PetscBT (bitarray) 
    im        - im[idx_start]: unchanged if diag is not an entry 
                             : num of entries with indices <= diag if diag is an entry
*/
#define PetscLLAddSortedLU(indices,idx_start,nlnk,lnk,bt,diag,nzbd,im) 0;\
{\
  PetscInt _k,_entry,_location,_lnkdata,_nidx;\
  nlnk     = 0;\
  _lnkdata = idx_start;\
  _nidx = im[idx_start] - nzbd; /* num of entries with idx_start < index <= diag */\
  for (_k=0; _k<_nidx; _k++){\
    _entry = indices[_k];\
    nzbd++;\
    if ( _entry== diag) im[idx_start] = nzbd;\
    if (!PetscBTLookupSet(bt,_entry)){  /* new entry */\
      /* search for insertion location */\
      do {\
        _location = _lnkdata;\
        _lnkdata  = lnk[_location];\
      } while (_entry > _lnkdata);\
      /* insertion location is found, add entry into lnk */\
      lnk[_location] = _entry;\
      lnk[_entry]    = _lnkdata;\
      nlnk++;\
      _lnkdata = _entry; /* next search starts from here */\
    }\
  }\
}

/*
  Copy data on the list into an array, then initialize the list 
  Input Parameters:
    idx_start - starting index of the list 
    lnk_max   - max value of lnk indicating the end of the list 
    nlnk      - number of data on the list to be copied
    lnk       - linked list
    bt        - PetscBT (bitarray), bt[idx]=true marks idx is in lnk
  output Parameters:
    indices   - array that contains the copied data
    lnk       - linked list that is cleaned and initialize
    bt        - PetscBT (bitarray) with all bits set to false
*/
#define PetscLLClean(idx_start,lnk_max,nlnk,lnk,indices,bt) 0;\
{\
  PetscInt _j,_idx=idx_start;\
  for (_j=0; _j<nlnk; _j++){\
    _idx = lnk[_idx];\
    indices[_j] = _idx;\
    ierr = PetscBTClear(bt,_idx);CHKERRQ(ierr);\
  }\
  lnk[idx_start] = lnk_max;\
}
/*
  Free memories used by the list
*/
#define PetscLLDestroy(lnk,bt) (PetscFree(lnk) || PetscBTDestroy(bt))

/* Routines below are used for incomplete matrix factorization */
/* 
  Create and initialize a linked list and its levels
  Input Parameters:
    idx_start - starting index of the list
    lnk_max   - max value of lnk indicating the end of the list
    nlnk      - max length of the list
  Output Parameters:
    lnk       - list initialized
    lnk_lvl   - array of size nlnk for storing levels of lnk
    bt        - PetscBT (bitarray) with all bits set to false
*/
#define PetscIncompleteLLCreate(idx_start,lnk_max,nlnk,lnk,lnk_lvl,bt)\
  (PetscMalloc(2*nlnk*sizeof(PetscInt),&lnk) || PetscBTCreate(nlnk,bt) || PetscBTMemzero(nlnk,bt) || (lnk[idx_start] = lnk_max,lnk_lvl = lnk + nlnk,0))

/*
  Initialize a sorted linked list used for ILU and ICC
  Input Parameters:
    nidx      - number of input idx
    idx       - interger array used for storing column indices
    idx_start - starting index of the list
    perm      - indices of an IS
    lnk       - linked list(an integer array) that is created
    lnklvl    - levels of lnk
    bt        - PetscBT (bitarray), bt[idx]=true marks idx is in lnk
  output Parameters:
    nlnk     - number of newly added idx
    lnk      - the sorted(increasing order) linked list containing new and non-redundate entries from idx
    lnklvl   - levels of lnk
    bt       - updated PetscBT (bitarray) 
*/
#define PetscIncompleteLLInit(nidx,idx,idx_start,perm,nlnk,lnk,lnklvl,bt) 0;\
{\
  PetscInt _k,_entry,_location,_lnkdata;\
  nlnk     = 0;\
  _lnkdata = idx_start;\
  for (_k=0; _k<nidx; _k++){\
    _entry = perm[idx[_k]];\
    if (!PetscBTLookupSet(bt,_entry)){  /* new entry */\
      /* search for insertion location */\
      if (_k && _entry < _lnkdata) _lnkdata  = idx_start;\
      do {\
        _location = _lnkdata;\
        _lnkdata  = lnk[_location];\
      } while (_entry > _lnkdata);\
      /* insertion location is found, add entry into lnk */\
      lnk[_location]  = _entry;\
      lnk[_entry]     = _lnkdata;\
      lnklvl[_entry] = 0;\
      nlnk++;\
      _lnkdata = _entry; /* next search starts from here if next_entry > _entry */\
    }\
  }\
}

/*
  Add a SORTED index set into a sorted linked list for ILU
  Input Parameters:
    nidx      - number of input indices
    idx       - sorted interger array used for storing column indices
    level     - level of fill, e.g., ICC(level)
    idxlvl    - level of idx 
    idx_start - starting index of the list
    lnk       - linked list(an integer array) that is created
    lnklvl    - levels of lnk
    bt        - PetscBT (bitarray), bt[idx]=true marks idx is in lnk
    prow      - the row number of idx
  output Parameters:
    nlnk     - number of newly added idx
    lnk      - the sorted(increasing order) linked list containing new and non-redundate entries from idx
    lnklvl   - levels of lnk
    bt       - updated PetscBT (bitarray) 

  Note: the level of factor(i,j) is set as lvl(i,j) = min{ lvl(i,j), lvl(i,prow)+lvl(prow,j)+1)
        where idx = non-zero columns of U(prow,prow+1:n-1), prow<i
*/
#define PetscILULLAddSorted(nidx,idx,level,idxlvl,idx_start,nlnk,lnk,lnklvl,bt,lnklvl_prow) 0;\
{\
  PetscInt _k,_entry,_location,_lnkdata,_incrlev,_lnklvl_prow=lnklvl[prow];\
  nlnk     = 0;\
  _lnkdata = idx_start;\
  for (_k=0; _k<nidx; _k++){\
    _incrlev = idxlvl[_k] + _lnklvl_prow + 1;\
    if (_incrlev > level) continue;\
    _entry = idx[_k];\
    if (!PetscBTLookupSet(bt,_entry)){  /* new entry */\
      /* search for insertion location */\
      do {\
        _location = _lnkdata;\
        _lnkdata  = lnk[_location];\
      } while (_entry > _lnkdata);\
      /* insertion location is found, add entry into lnk */\
      lnk[_location]  = _entry;\
      lnk[_entry]     = _lnkdata;\
      lnklvl[_entry] = _incrlev;\
      nlnk++;\
      _lnkdata = _entry; /* next search starts from here if next_entry > _entry */\
    } else { /* existing entry: update lnklvl */\
      if (lnklvl[_entry] > _incrlev) lnklvl[_entry] = _incrlev;\
    }\
  }\
}

/*
  Add a index set into a sorted linked list
  Input Parameters:
    nidx      - number of input idx
    idx   - interger array used for storing column indices
    level     - level of fill, e.g., ICC(level)
    idxlvl - level of idx 
    idx_start - starting index of the list
    lnk       - linked list(an integer array) that is created
    lnklvl   - levels of lnk
    bt        - PetscBT (bitarray), bt[idx]=true marks idx is in lnk
  output Parameters:
    nlnk      - number of newly added idx
    lnk       - the sorted(increasing order) linked list containing new and non-redundate entries from idx
    lnklvl   - levels of lnk
    bt        - updated PetscBT (bitarray) 
*/
#define PetscIncompleteLLAdd(nidx,idx,level,idxlvl,idx_start,nlnk,lnk,lnklvl,bt) 0;\
{\
  PetscInt _k,_entry,_location,_lnkdata,_incrlev;\
  nlnk     = 0;\
  _lnkdata = idx_start;\
  for (_k=0; _k<nidx; _k++){\
    _incrlev = idxlvl[_k] + 1;\
    if (_incrlev > level) continue;\
    _entry = idx[_k];\
    if (!PetscBTLookupSet(bt,_entry)){  /* new entry */\
      /* search for insertion location */\
      if (_k && _entry < _lnkdata) _lnkdata  = idx_start;\
      do {\
        _location = _lnkdata;\
        _lnkdata  = lnk[_location];\
      } while (_entry > _lnkdata);\
      /* insertion location is found, add entry into lnk */\
      lnk[_location]  = _entry;\
      lnk[_entry]     = _lnkdata;\
      lnklvl[_entry] = _incrlev;\
      nlnk++;\
      _lnkdata = _entry; /* next search starts from here if next_entry > _entry */\
    } else { /* existing entry: update lnklvl */\
      if (lnklvl[_entry] > _incrlev) lnklvl[_entry] = _incrlev;\
    }\
  }\
}

/*
  Add a SORTED index set into a sorted linked list
  Input Parameters:
    nidx      - number of input indices
    idx   - sorted interger array used for storing column indices
    level     - level of fill, e.g., ICC(level)
    idxlvl - level of idx 
    idx_start - starting index of the list
    lnk       - linked list(an integer array) that is created
    lnklvl    - levels of lnk
    bt        - PetscBT (bitarray), bt[idx]=true marks idx is in lnk
  output Parameters:
    nlnk      - number of newly added idx
    lnk       - the sorted(increasing order) linked list containing new and non-redundate entries from idx
    lnklvl    - levels of lnk
    bt        - updated PetscBT (bitarray) 
*/
#define PetscIncompleteLLAddSorted(nidx,idx,level,idxlvl,idx_start,nlnk,lnk,lnklvl,bt) 0;\
{\
  PetscInt _k,_entry,_location,_lnkdata,_incrlev;\
  nlnk = 0;\
  _lnkdata = idx_start;\
  for (_k=0; _k<nidx; _k++){\
    _incrlev = idxlvl[_k] + 1;\
    if (_incrlev > level) continue;\
    _entry = idx[_k];\
    if (!PetscBTLookupSet(bt,_entry)){  /* new entry */\
      /* search for insertion location */\
      do {\
        _location = _lnkdata;\
        _lnkdata  = lnk[_location];\
      } while (_entry > _lnkdata);\
      /* insertion location is found, add entry into lnk */\
      lnk[_location] = _entry;\
      lnk[_entry]    = _lnkdata;\
      lnklvl[_entry] = _incrlev;\
      nlnk++;\
      _lnkdata = _entry; /* next search starts from here */\
    } else { /* existing entry: update lnklvl */\
      if (lnklvl[_entry] > _incrlev) lnklvl[_entry] = _incrlev;\
    }\
  }\
}

/*
  Add a SORTED index set into a sorted linked list for ICC
  Input Parameters:
    nidx      - number of input indices
    idx       - sorted interger array used for storing column indices
    level     - level of fill, e.g., ICC(level)
    idxlvl    - level of idx 
    idx_start - starting index of the list
    lnk       - linked list(an integer array) that is created
    lnklvl    - levels of lnk
    bt        - PetscBT (bitarray), bt[idx]=true marks idx is in lnk
    idxlvl_prow - idxlvl[prow], where prow is the row number of the idx
  output Parameters:
    nlnk   - number of newly added indices
    lnk    - the sorted(increasing order) linked list containing new and non-redundate entries from idx
    lnklvl - levels of lnk
    bt     - updated PetscBT (bitarray) 
  Note: the level of U(i,j) is set as lvl(i,j) = min{ lvl(i,j), lvl(prow,i)+lvl(prow,j)+1)
        where idx = non-zero columns of U(prow,prow+1:n-1), prow<i
*/
#define PetscICCLLAddSorted(nidx,idx,level,idxlvl,idx_start,nlnk,lnk,lnklvl,bt,idxlvl_prow) 0;\
{\
  PetscInt _k,_entry,_location,_lnkdata,_incrlev;\
  nlnk = 0;\
  _lnkdata = idx_start;\
  for (_k=0; _k<nidx; _k++){\
    _incrlev = idxlvl[_k] + idxlvl_prow + 1;\
    if (_incrlev > level) continue;\
    _entry = idx[_k];\
    if (!PetscBTLookupSet(bt,_entry)){  /* new entry */\
      /* search for insertion location */\
      do {\
        _location = _lnkdata;\
        _lnkdata  = lnk[_location];\
      } while (_entry > _lnkdata);\
      /* insertion location is found, add entry into lnk */\
      lnk[_location] = _entry;\
      lnk[_entry]    = _lnkdata;\
      lnklvl[_entry] = _incrlev;\
      nlnk++;\
      _lnkdata = _entry; /* next search starts from here */\
    } else { /* existing entry: update lnklvl */\
      if (lnklvl[_entry] > _incrlev) lnklvl[_entry] = _incrlev;\
    }\
  }\
}

/*
  Copy data on the list into an array, then initialize the list 
  Input Parameters:
    idx_start - starting index of the list 
    lnk_max   - max value of lnk indicating the end of the list 
    nlnk      - number of data on the list to be copied
    lnk       - linked list
    lnklvl    - level of lnk
    bt        - PetscBT (bitarray), bt[idx]=true marks idx is in lnk
  output Parameters:
    indices - array that contains the copied data
    lnk     - linked list that is cleaned and initialize
    lnklvl  - level of lnk that is reinitialized 
    bt      - PetscBT (bitarray) with all bits set to false
*/
#define PetscIncompleteLLClean(idx_start,lnk_max,nlnk,lnk,lnklvl,indices,indiceslvl,bt) 0;\
{\
  PetscInt _j,_idx=idx_start;\
  for (_j=0; _j<nlnk; _j++){\
    _idx = lnk[_idx];\
    *(indices+_j) = _idx;\
    *(indiceslvl+_j) = lnklvl[_idx];\
    lnklvl[_idx] = -1;\
    ierr = PetscBTClear(bt,_idx);CHKERRQ(ierr);\
  }\
  lnk[idx_start] = lnk_max;\
}
/*
  Free memories used by the list
*/
#define PetscIncompleteLLDestroy(lnk,bt) (PetscFree(lnk) || PetscBTDestroy(bt))

/* 
  Create and initialize a condensed linked list - 
    same as PetscLLCreate(), but uses a scalable array 'lnk' with size of max number of entries, not O(N).
    Barry suggested this approach (Dec. 6, 2011):
      I've thought of an alternative way of representing a linked list that is efficient but doesn't have the O(N) scaling issue 
      (it may be faster than the O(N) even sequentially due to less crazy memory access).

      Instead of having some like  a  2  -> 4 -> 11 ->  22  list that uses slot 2  4 11 and 22 in a big array use a small array with two slots 
      for each entry for example  [ 2 1 | 4 3 | 22 -1 | 11 2]   so the first number (of the pair) is the value while the second tells you where 
      in the list the next entry is. Inserting a new link means just append another pair at the end. For example say we want to insert 13 into the 
      list it would then become [2 1 | 4 3 | 22 -1 | 11 4 | 13 2 ] you just add a pair at the end and fix the point for the one that points to it. 
      That is 11 use to point to the 2 slot, after the change 11 points to the 4th slot which has the value 13. Note that values are always next 
      to each other so memory access is much better than using the big array.

  Example:
     nlnk_max=5, lnk_max=36:
     Initial list: [0, 0 | 36, 2 | 0, 0 | 0, 0 | 0, 0 | 0, 0 | 0, 0]
     here, head_node has index 2 with value lnk[2]=lnk_max=36,
           _nlnk=0-th entry is used to store the number of entries in the list,
     The initial lnk represents head -> tail(marked by 36) with number of entries = lnk[2*(nlnk_max+1)]=lnk[12]=0.
    
     Now adding a sorted set {2,4}, the list becomes
     [2, 0 | 36, 4 |2, 6 | 4, 2 | 0, 0 | 0, 0 | 0, 0 ]
     represents head -> 2 -> 4 -> tail with number of entries = lnk[0]=2.

     Then adding a sorted set {0,3,35}, the list
     [5, 0 | 36, 8 | 2, 10 | 4, 12 | 0, 4 | 3, 6 | 35, 2 ]
     represents head -> 0 -> 2 -> 3 -> 4 -> 35 -> tail with number of entries = lnk[0]=5.
 
  Input Parameters:
    nlnk_max  - max length of the list
    lnk_max   - max value of the entries
  Output Parameters:
    lnk       - list created and initialized
    nlnk      - number of entries on the list
    bt        - PetscBT (bitarray) with all bits set to false. Note: bt has size lnk_max, not nln_max!
*/
#define PetscLLCondensedCreate_old(nlnk_max,lnk_max,lnk,nlnk,bt) 0; \
{\
  PetscInt _head,_nlnk;\
  PetscMalloc(2*(nlnk_max+2)*sizeof(PetscInt),&lnk);\
  PetscBTCreate(lnk_max,bt);                   \
  PetscBTMemzero(lnk_max,bt);                  \
  _head        = 2*nlnk_max;\
  lnk[_head]   = lnk_max;\
  lnk[_head+1] = _head;\
  _nlnk        = 2*(nlnk_max+1);\
  lnk[_nlnk]   = 0;           /* nlnk: number of entries on the list */\
  nlnk         = lnk + _nlnk;                                          \
  lnk[_nlnk+1] = 0;           /* flag indicating the list is empty -turn off now  */\
}

#define PetscLLCondensedCreate(nlnk_max,lnk_max,lnk,nlnk,bt) 0; \
{\
  PetscInt _head,_nlnk;\
  PetscMalloc(2*(nlnk_max+2)*sizeof(PetscInt),&lnk);\
  PetscBTCreate(lnk_max,bt);                   \
  PetscBTMemzero(lnk_max,bt);                  \
  lnk[0] = 0;   /* nlnk: number of entries on the list */\
  nlnk   = lnk;    \
  lnk[2] = lnk_max;   /* value in the head node */\
  lnk[3] = 2;         /* next for the head node */\
}

/*
  Add a SORTED ascending index set into a sorted linked list. See PetscLLCondensedCreate() for detailed description.
  Input Parameters:
    nlnk_max  - max length of the list
    lnk_max   - max value of the entries
    nidx      - number of input indices
    indices   - sorted interger array   
    lnk       - condensed linked list(an integer array) that is created
    bt        - PetscBT (bitarray), bt[idx]=true marks idx is in lnk
  output Parameters:
    nlnk      - number of indices on the list
    lnk       - the sorted(increasing order) linked list containing previous and newly added non-redundate indices
    bt        - updated PetscBT (bitarray) 
*/
#define PetscLLCondensedAddSorted_old(nlnk_max,lnk_max,nidx,indices,nlnk,lnk,bt) 0; \
{\
  PetscInt _k,_entry,_location,_next,_lnkdata,_nlnk;                    \
  _nlnk     = lnk[2*(nlnk_max+1)]; /* num of entries on the input lnk */\
  _location = 2*nlnk_max; /* head */ \
  if (lnk[2*nlnk_max+3]){/* the list is empty */                        \
    for (_k=0; _k<nidx; _k++){\
      _entry = indices[_k];\
      PetscBTSet(bt,_entry);     /* mark new entry */               \
      _next     = _location + 1; /* link from previous node to next node */\
      _location = lnk[_next];    /* idx of next node */\
      _lnkdata  = lnk[_location];/* value of next node */      \
      /* insertion location is found, add entry into lnk */\
      lnk[_next]   = 2*_nlnk;      /* connect previous node to the new node */\
      _next        = 2*_nlnk;\
      lnk[_next]   = _entry;       /* set value of the new node */       \
      lnk[_next+1] = _location;    /* connect new node to next node */ \
      _location    = _next;        /* next search starts from the new node */\
      _nlnk++;\
    }\
  } else {\
    for (_k=0; _k<nidx; _k++){\
      _entry = indices[_k];\
      if (!PetscBTLookupSet(bt,_entry)){  /* new entry */  \
        /* search for insertion location */\
        do {\
          _next     = _location + 1; /* link from previous node to next node */\
          _location = lnk[_next];    /* idx of next node */\
          _lnkdata  = lnk[_location];/* value of next node */      \
        } while (_entry > _lnkdata);\
        /* insertion location is found, add entry into lnk */\
        lnk[_next]   = 2*_nlnk;      /* connect previous node to the new node */\
        _next        = 2*_nlnk;\
        lnk[_next]   = _entry;       /* set value of the new node */       \
        lnk[_next+1] = _location;    /* connect new node to next node */ \
        _location    = _next;        /* next search starts from the new node */\
        _nlnk++;\
      }   \
    }\
  }\
  _k        = 2*(nlnk_max+1);\
  lnk[_k]   = _nlnk;   /* number of entries in the list */\
  lnk[_k+1] = 0;       /* the list is not empty */\
}

#define PetscLLCondensedAddSorted(nlnk_max,lnk_max,nidx,indices,nlnk,lnk,bt) 0; \
{\
  PetscInt _k,_entry,_location,_next,_lnkdata,_nlnk,_newnode;   \
  _nlnk     = lnk[0]; /* num of entries on the input lnk */\
  _location = 2; /* head */ \
    for (_k=0; _k<nidx; _k++){\
      _entry = indices[_k];\
      if (!PetscBTLookupSet(bt,_entry)){  /* new entry */  \
        /* search for insertion location */\
        do {\
          _next     = _location + 1; /* link from previous node to next node */\
          _location = lnk[_next];    /* idx of next node */\
          _lnkdata  = lnk[_location];/* value of next node */      \
        } while (_entry > _lnkdata);\
        /* insertion location is found, add entry into lnk */\
        _newnode        = 2*(_nlnk+2);   /* index for this new node */\
        lnk[_next]      = _newnode;      /* connect previous node to the new node */ \
        lnk[_newnode]   = _entry;        /* set value of the new node */       \
        lnk[_newnode+1] = _location;     /* connect new node to next node */ \
        _location       = _newnode;      /* next search starts from the new node */\
        _nlnk++;\
      }   \
    }\
  lnk[0]   = _nlnk;   /* number of entries in the list */\
}

#define PetscLLCondensedClean_old(nlnk_max,lnk_max,nidx,indices,nlnk,lnk,bt) 0;\
{\
  PetscInt _k,_next,_nlnk;                            \
  _next  = lnk[2*nlnk_max + 1]; /* head node */\
  _nlnk = *nlnk;                /* =lnk[2*(nlnk_max+1)], num of entries on the list */\
  for (_k=0; _k<_nlnk; _k++){\
    indices[_k] = lnk[_next];\
    _next       = lnk[_next + 1];\
    ierr = PetscBTClear(bt,indices[_k]);CHKERRQ(ierr);             \
  }\
  _k          = 2*nlnk_max; /* head node */\
  lnk[_k]     = lnk_max;    /* initialize head node */\
  lnk[_k + 1] = _k;\
  lnk[_k + 2] = 0;          /* num of entries on the list */\
  lnk[_k + 3] = 1;          /* the list is empty */\
}

#define PetscLLCondensedClean(nlnk_max,lnk_max,nidx,indices,nlnk,lnk,bt) 0;\
{\
  PetscInt _k,_next,_nlnk;\
  _next = lnk[3];       /* head node */\
  _nlnk = *nlnk;        /* =lnk[2*(nlnk_max+1)], num of entries on the list */\
  for (_k=0; _k<_nlnk; _k++){\
    indices[_k] = lnk[_next];\
    _next       = lnk[_next + 1];\
    ierr = PetscBTClear(bt,indices[_k]);CHKERRQ(ierr);      \
  }\
  lnk[0] = 0;          /* num of entries on the list */\
  lnk[2] = lnk_max;    /* initialize head node */\
  lnk[3] = 2;          /* head node */\
}

#define PetscLLCondensedView(nlnk_max,lnk_max,lnk,nlnk) 0;\
{\
  PetscInt _k;\
  ierr = PetscPrintf(PETSC_COMM_SELF,"LLCondensed: size %d\n",*nlnk);       \
  for (_k=0; _k< nlnk_max + 2; _k++){\
    PetscPrintf(PETSC_COMM_SELF," %D: (%D, %D)\n",_k,lnk[2*_k],lnk[2*_k+1]); \
  }\
}

/*
  Free memories used by the list
*/
#define PetscLLCondensedDestroy(lnk,bt) (PetscFree(lnk) || PetscBTDestroy(bt))


extern PetscLogEvent  MAT_Mult, MAT_MultMatrixFree, MAT_Mults, MAT_MultConstrained, MAT_MultAdd, MAT_MultTranspose;
extern PetscLogEvent  MAT_MultTransposeConstrained, MAT_MultTransposeAdd, MAT_Solve, MAT_Solves, MAT_SolveAdd, MAT_SolveTranspose;
extern PetscLogEvent  MAT_SolveTransposeAdd, MAT_SOR, MAT_ForwardSolve, MAT_BackwardSolve, MAT_LUFactor, MAT_LUFactorSymbolic;
extern PetscLogEvent  MAT_LUFactorNumeric, MAT_CholeskyFactor, MAT_CholeskyFactorSymbolic, MAT_CholeskyFactorNumeric, MAT_ILUFactor;
extern PetscLogEvent  MAT_ILUFactorSymbolic, MAT_ICCFactorSymbolic, MAT_Copy, MAT_Convert, MAT_Scale, MAT_AssemblyBegin;
extern PetscLogEvent  MAT_AssemblyEnd, MAT_SetValues, MAT_GetValues, MAT_GetRow, MAT_GetRowIJ, MAT_GetSubMatrices, MAT_GetColoring, MAT_GetOrdering, MAT_GetRedundantMatrix;
extern PetscLogEvent  MAT_IncreaseOverlap, MAT_Partitioning, MAT_ZeroEntries, MAT_Load, MAT_View, MAT_AXPY, MAT_FDColoringCreate, MAT_TransposeColoringCreate;
extern PetscLogEvent  MAT_FDColoringApply, MAT_Transpose, MAT_FDColoringFunction;
extern PetscLogEvent  MAT_MatMult, MAT_MatSolve,MAT_MatMultSymbolic, MAT_MatMultNumeric,MAT_Getlocalmatcondensed,MAT_GetBrowsOfAcols,MAT_GetBrowsOfAocols;
extern PetscLogEvent  MAT_PtAP, MAT_PtAPSymbolic, MAT_PtAPNumeric,MAT_Seqstompinum,MAT_Seqstompisym,MAT_Seqstompi,MAT_Getlocalmat;
extern PetscLogEvent  MAT_RARt, MAT_RARtSymbolic, MAT_RARtNumeric;
extern PetscLogEvent  MAT_MatTransposeMult, MAT_MatTransposeMultSymbolic, MAT_MatTransposeMultNumeric;
extern PetscLogEvent  MAT_TransposeMatMult, MAT_TransposeMatMultSymbolic, MAT_TransposeMatMultNumeric;
extern PetscLogEvent  MAT_Applypapt, MAT_Applypapt_symbolic, MAT_Applypapt_numeric;
extern PetscLogEvent  MAT_Getsymtranspose, MAT_Transpose_SeqAIJ, MAT_Getsymtransreduced,MAT_GetSequentialNonzeroStructure;

extern PetscLogEvent  MATMFFD_Mult;
extern PetscLogEvent  MAT_GetMultiProcBlock;
extern PetscLogEvent  MAT_CUSPCopyToGPU, MAT_SetValuesBatch, MAT_SetValuesBatchI, MAT_SetValuesBatchII, MAT_SetValuesBatchIII, MAT_SetValuesBatchIV;
extern PetscLogEvent  MAT_Merge;

#endif
