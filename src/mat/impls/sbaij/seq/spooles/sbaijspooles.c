/*$Id: sbaijspooles.c,v 1.10 2001/08/15 15:56:50 bsmith Exp $*/
/* 
   Provides an interface to the Spooles serial sparse solver
*/

#include "src/mat/impls/aij/seq/spooles/spooles.h"

#undef __FUNCT__  
#define __FUNCT__ "MatDestroy_SeqSBAIJ_Spooles"
int MatDestroy_SeqSBAIJ_Spooles(Mat A) {
  int         ierr;
  
  PetscFunctionBegin;
  /* SeqSBAIJ_Spooles isn't really the matrix that USES spooles, */
  /* rather it is a factory class for creating a symmetric matrix that can */
  /* invoke Spooles' sequential cholesky solver. */
  /* As a result, we don't have to clean up the stuff set by spooles */
  /* as in MatDestroy_SeqAIJ_Spooles. */
  ierr = MatConvert_Spooles_Base(A,MATSEQSBAIJ,&A);CHKERRQ(ierr);
  ierr = (*A->ops->destroy)(A);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatAssemblyEnd_SeqSBAIJ_Spooles"
int MatAssemblyEnd_SeqSBAIJ_Spooles(Mat A,MatAssemblyType mode) {
  int         ierr,bs;
  Mat_Spooles *lu=(Mat_Spooles *)(A->spptr);

  PetscFunctionBegin;
  ierr = (*lu->MatAssemblyEnd)(A,mode);CHKERRQ(ierr);
  ierr = MatGetBlockSize(A,&bs);CHKERRQ(ierr);
  if (bs > 1) SETERRQ1(1,"Block size %d not supported by Spooles",bs);
  lu->MatCholeskyFactorSymbolic  = A->ops->choleskyfactorsymbolic;
  A->ops->choleskyfactorsymbolic = MatCholeskyFactorSymbolic_SeqSBAIJ_Spooles;  
  PetscFunctionReturn(0);
}

/* 
  input:
   F:                 numeric factor
  output:
   nneg, nzero, npos: matrix inertia 
*/

#undef __FUNCT__  
#define __FUNCT__ "MatGetInertia_SeqSBAIJ_Spooles"
int MatGetInertia_SeqSBAIJ_Spooles(Mat F,int *nneg,int *nzero,int *npos)
{ 
  Mat_Spooles          *lu = (Mat_Spooles*)F->spptr; 
  int                  neg,zero,pos;

  PetscFunctionBegin;
  FrontMtx_inertia(lu->frontmtx, &neg, &zero, &pos) ;
  if(nneg)  *nneg  = neg;
  if(nzero) *nzero = zero;
  if(npos)  *npos  = pos;
  PetscFunctionReturn(0);
}

/* Note the Petsc r permutation is ignored */
#undef __FUNCT__  
#define __FUNCT__ "MatCholeskyFactorSymbolic_SeqSBAIJ_Spooles"
int MatCholeskyFactorSymbolic_SeqSBAIJ_Spooles(Mat A,IS r,MatFactorInfo *info,Mat *F)
{ 
  Mat         B;
  Mat_Spooles *lu;   
  int         ierr,m=A->m,n=A->n;

  PetscFunctionBegin;	
  /* Create the factorization matrix */  
  ierr = MatCreate(A->comm,m,n,m,n,&B);
  ierr = MatSetType(B,MATSEQAIJSPOOLES);CHKERRQ(ierr);
  ierr = MatSeqAIJSetPreallocation(B,PETSC_NULL,PETSC_NULL);CHKERRQ(ierr);

  B->ops->choleskyfactornumeric  = MatFactorNumeric_SeqAIJ_Spooles;
  B->ops->getinertia             = MatGetInertia_SeqSBAIJ_Spooles;
  B->factor                      = FACTOR_CHOLESKY;  

  lu                        = (Mat_Spooles *)(B->spptr);
  lu->options.pivotingflag  = SPOOLES_NO_PIVOTING;
  lu->options.symflag       = SPOOLES_SYMMETRIC;   /* default */
  lu->flg                   = DIFFERENT_NONZERO_PATTERN;
  lu->options.useQR         = PETSC_FALSE;

  *F = B;
  PetscFunctionReturn(0); 
}

EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "MatConvert_SeqSBAIJ_Spooles"
int MatConvert_SeqSBAIJ_Spooles(Mat A,MatType type,Mat *newmat) {
  /* This routine is only called to convert a MATSEQSBAIJ matrix */
  /* to a MATSEQSBAIJSPOOLES matrix, so we will ignore 'MatType type'. */
  int         ierr;
  Mat         B=*newmat;
  Mat_Spooles *lu;

  PetscFunctionBegin;
  if (B != A) {
    /* This routine is inherited, so we know the type is correct. */
    ierr = MatDuplicate(A,MAT_COPY_VALUES,&B);CHKERRQ(ierr);
  }

  ierr = PetscNew(Mat_Spooles,&lu);CHKERRQ(ierr); 
  B->spptr                       = (void*)lu;

  lu->basetype                   = MATSEQSBAIJ;
  lu->CleanUpSpooles             = PETSC_FALSE;
  lu->MatCholeskyFactorSymbolic  = A->ops->choleskyfactorsymbolic;
  lu->MatLUFactorSymbolic        = A->ops->lufactorsymbolic; 
  lu->MatView                    = A->ops->view;
  lu->MatAssemblyEnd             = A->ops->assemblyend;
  lu->MatDestroy                 = A->ops->destroy;
  B->ops->choleskyfactorsymbolic = MatCholeskyFactorSymbolic_SeqSBAIJ_Spooles;
  B->ops->assemblyend            = MatAssemblyEnd_SeqSBAIJ_Spooles;
  B->ops->destroy                = MatDestroy_SeqSBAIJ_Spooles;
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatConvert_spooles_seqsbaij_C",
                                           "MatConvert_Spooles_Base",MatConvert_Spooles_Base);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatConvert_seqsbaij_spooles_C",
                                           "MatConvert_SeqSBAIJ_Spooles",MatConvert_SeqSBAIJ_Spooles);CHKERRQ(ierr);
  ierr = PetscObjectChangeTypeName((PetscObject)B,MATSEQSBAIJSPOOLES);CHKERRQ(ierr);
  *newmat = B;
  PetscFunctionReturn(0);
}
EXTERN_C_END

EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "MatCreate_SeqSBAIJ_Spooles"
int MatCreate_SeqSBAIJ_Spooles(Mat A) {
  int ierr;

  PetscFunctionBegin;
  ierr = MatSetType(A,MATSEQSBAIJ);CHKERRQ(ierr);
  ierr = MatConvert_SeqSBAIJ_Spooles(A,MATSEQSBAIJSPOOLES,&A);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END
