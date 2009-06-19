#define PETSCMAT_DLL

#include "../src/mat/impls/aij/seq/aij.h"
#include "../src/inline/dot.h"
#define PETSC_USE_WHILE_KERNELS
#include "../src/inline/spops.h"
#include "petscbt.h"
#include "../src/mat/utils/freespace.h"

EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "MatOrdering_Flow_SeqAIJ"
/*
      Computes an ordering to get most of the large numerical values in the lower triangular part of the matrix
*/
PetscErrorCode MatOrdering_Flow_SeqAIJ(Mat mat,const MatOrderingType type,IS *irow,IS *icol)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)mat->data;
  PetscErrorCode    ierr;
  PetscInt          i,j,jj,k, kk,n = mat->rmap->n, current = 0, newcurrent = 0,*order;
  const PetscInt    *ai = a->i, *aj = a->j;
  const PetscScalar *aa = a->a;
  PetscTruth        *done;
  PetscReal         best,past = 0,future;

  PetscFunctionBegin;
  /* pick initial row */
  best = -1;
  for (i=0; i<n; i++) {
    future = 0;
    for (j=ai[i]; j<ai[i+1]; j++) {
      if (aj[j] != i) future  += PetscAbsScalar(aa[j]); else past = PetscAbsScalar(aa[j]);
    }
    if (!future) future = 1.e-10; /* if there is zero in the upper diagonal part want to rank this row high */
    if (past/future > best) {
      best = past/future;
      current = i;
    }
  }

  ierr = PetscMalloc(n*sizeof(PetscTruth),&done);CHKERRQ(ierr);
  ierr = PetscMalloc(n*sizeof(PetscInt),&order);CHKERRQ(ierr);
  ierr = PetscMemzero(done,n*sizeof(PetscTruth));CHKERRQ(ierr);
  order[0] = current;
  for (i=0; i<n-1; i++) {
    done[current] = PETSC_TRUE;
    best          = -1;
    /* loop over all neighbors of current pivot */
    for (j=ai[current]; j<ai[current+1]; j++) {
      jj = aj[j];
      if (done[jj]) continue;
      /* loop over columns of potential next row computing weights for below and above diagonal */
      past = future = 0.0;
      for (k=ai[jj]; k<ai[jj+1]; k++) {
        kk = aj[k];
        if (done[kk]) past += PetscAbsScalar(aa[k]);
        else if (kk != jj) future  += PetscAbsScalar(aa[k]);
      }
      if (!future) future = 1.e-10; /* if there is zero in the upper diagonal part want to rank this row high */
      if (past/future > best) {
        best = past/future;
        newcurrent = jj;
      }
    }
    if (best == -1) { /* no neighbors to select from so select best of all that remain */
      best = -1;
      for (k=0; k<n; k++) {
        if (done[k]) continue;
        future = 0;
        past   = 0;
        for (j=ai[k]; j<ai[k+1]; j++) {
          kk = aj[j];
          if (done[kk]) past += PetscAbsScalar(aa[j]);
          else if (kk != k) future  += PetscAbsScalar(aa[j]);
        }
        if (!future) future = 1.e-10; /* if there is zero in the upper diagonal part want to rank this row high */
        if (past/future > best) {
          best = past/future;
          newcurrent = k;
        }
      }
    }
    if (current == newcurrent) SETERRQ(PETSC_ERR_PLIB,"newcurrent cannot be current");
    current = newcurrent;
    order[i+1] = current;
  }
  ierr = ISCreateGeneral(PETSC_COMM_SELF,n,order,irow);CHKERRQ(ierr);
  *icol = *irow;
  ierr = PetscObjectReference((PetscObject)*irow);CHKERRQ(ierr);
  ierr = PetscFree(done);CHKERRQ(ierr);
  ierr = PetscFree(order);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END

EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "MatGetFactorAvailable_seqaij_petsc"
PetscErrorCode MatGetFactorAvailable_seqaij_petsc(Mat A,MatFactorType ftype,PetscTruth *flg)
{
  PetscFunctionBegin;
  *flg = PETSC_TRUE;
  PetscFunctionReturn(0);
}
EXTERN_C_END

EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "MatGetFactor_seqaij_petsc"
PetscErrorCode MatGetFactor_seqaij_petsc(Mat A,MatFactorType ftype,Mat *B)
{
  PetscInt           n = A->rmap->n;
  PetscErrorCode     ierr;

  PetscFunctionBegin;
  ierr = MatCreate(((PetscObject)A)->comm,B);CHKERRQ(ierr);
  ierr = MatSetSizes(*B,n,n,n,n);CHKERRQ(ierr);
  if (ftype == MAT_FACTOR_LU || ftype == MAT_FACTOR_ILU || ftype == MAT_FACTOR_ILUDT){
    ierr = MatSetType(*B,MATSEQAIJ);CHKERRQ(ierr);
    (*B)->ops->ilufactorsymbolic = MatILUFactorSymbolic_SeqAIJ;
    (*B)->ops->lufactorsymbolic  = MatLUFactorSymbolic_SeqAIJ;
    (*B)->ops->iludtfactor       = MatILUDTFactor_SeqAIJ;
  } else if (ftype == MAT_FACTOR_CHOLESKY || ftype == MAT_FACTOR_ICC) {
    ierr = MatSetType(*B,MATSEQSBAIJ);CHKERRQ(ierr);
    ierr = MatSeqSBAIJSetPreallocation(*B,1,MAT_SKIP_ALLOCATION,PETSC_NULL);CHKERRQ(ierr);
    (*B)->ops->iccfactorsymbolic      = MatICCFactorSymbolic_SeqAIJ;
    (*B)->ops->choleskyfactorsymbolic = MatCholeskyFactorSymbolic_SeqAIJ;
  } else SETERRQ(PETSC_ERR_SUP,"Factor type not supported");
  (*B)->factor = ftype;
  PetscFunctionReturn(0);
}
EXTERN_C_END

#undef __FUNCT__  
#define __FUNCT__ "MatLUFactorSymbolic_SeqAIJ"
PetscErrorCode MatLUFactorSymbolic_SeqAIJ(Mat B,Mat A,IS isrow,IS iscol,const MatFactorInfo *info)
{
  Mat_SeqAIJ         *a = (Mat_SeqAIJ*)A->data,*b;
  IS                 isicol;
  PetscErrorCode     ierr;
  const PetscInt     *r,*ic;
  PetscInt           i,n=A->rmap->n,*ai=a->i,*aj=a->j;
  PetscInt           *bi,*bj,*ajtmp;
  PetscInt           *bdiag,row,nnz,nzi,reallocs=0,nzbd,*im;
  PetscReal          f;
  PetscInt           nlnk,*lnk,k,**bi_ptr;
  PetscFreeSpaceList free_space=PETSC_NULL,current_space=PETSC_NULL;
  PetscBT            lnkbt;

  PetscFunctionBegin;
  if (A->rmap->N != A->cmap->N) SETERRQ(PETSC_ERR_ARG_WRONG,"matrix must be square");
  ierr = ISInvertPermutation(iscol,PETSC_DECIDE,&isicol);CHKERRQ(ierr);
  ierr = ISGetIndices(isrow,&r);CHKERRQ(ierr);
  ierr = ISGetIndices(isicol,&ic);CHKERRQ(ierr);

  /* get new row pointers */
  ierr = PetscMalloc((n+1)*sizeof(PetscInt),&bi);CHKERRQ(ierr);
  bi[0] = 0;

  /* bdiag is location of diagonal in factor */
  ierr = PetscMalloc((n+1)*sizeof(PetscInt),&bdiag);CHKERRQ(ierr);
  bdiag[0] = 0;

  /* linked list for storing column indices of the active row */
  nlnk = n + 1;
  ierr = PetscLLCreate(n,n,nlnk,lnk,lnkbt);CHKERRQ(ierr);

  ierr = PetscMalloc2(n+1,PetscInt**,&bi_ptr,n+1,PetscInt,&im);CHKERRQ(ierr);

  /* initial FreeSpace size is f*(ai[n]+1) */
  f = info->fill;
  ierr = PetscFreeSpaceGet((PetscInt)(f*(ai[n]+1)),&free_space);CHKERRQ(ierr);
  current_space = free_space;

  for (i=0; i<n; i++) {
    /* copy previous fill into linked list */
    nzi = 0;
    nnz = ai[r[i]+1] - ai[r[i]];
    if (!nnz) SETERRQ2(PETSC_ERR_MAT_LU_ZRPVT,"Empty row in matrix: row in original ordering %D in permuted ordering %D",r[i],i);
    ajtmp = aj + ai[r[i]]; 
    ierr = PetscLLAddPerm(nnz,ajtmp,ic,n,nlnk,lnk,lnkbt);CHKERRQ(ierr);
    nzi += nlnk;

    /* add pivot rows into linked list */
    row = lnk[n]; 
    while (row < i) {
      nzbd    = bdiag[row] - bi[row] + 1; /* num of entries in the row with column index <= row */
      ajtmp   = bi_ptr[row] + nzbd; /* points to the entry next to the diagonal */   
      ierr = PetscLLAddSortedLU(ajtmp,row,nlnk,lnk,lnkbt,i,nzbd,im);CHKERRQ(ierr);
      nzi += nlnk;
      row  = lnk[row];
    }
    bi[i+1] = bi[i] + nzi;
    im[i]   = nzi; 

    /* mark bdiag */
    nzbd = 0;  
    nnz  = nzi;
    k    = lnk[n]; 
    while (nnz-- && k < i){
      nzbd++;
      k = lnk[k]; 
    }
    bdiag[i] = bi[i] + nzbd;

    /* if free space is not available, make more free space */
    if (current_space->local_remaining<nzi) {
      nnz = (n - i)*nzi; /* estimated and max additional space needed */
      ierr = PetscFreeSpaceGet(nnz,&current_space);CHKERRQ(ierr);
      reallocs++;
    }

    /* copy data into free space, then initialize lnk */
    ierr = PetscLLClean(n,n,nzi,lnk,current_space->array,lnkbt);CHKERRQ(ierr); 
    bi_ptr[i] = current_space->array;
    current_space->array           += nzi;
    current_space->local_used      += nzi;
    current_space->local_remaining -= nzi;
  }
#if defined(PETSC_USE_INFO)
  if (ai[n] != 0) {
    PetscReal af = ((PetscReal)bi[n])/((PetscReal)ai[n]);
    ierr = PetscInfo3(A,"Reallocs %D Fill ratio:given %G needed %G\n",reallocs,f,af);CHKERRQ(ierr);
    ierr = PetscInfo1(A,"Run with -pc_factor_fill %G or use \n",af);CHKERRQ(ierr);
    ierr = PetscInfo1(A,"PCFactorSetFill(pc,%G);\n",af);CHKERRQ(ierr);
    ierr = PetscInfo(A,"for best performance.\n");CHKERRQ(ierr);
  } else {
    ierr = PetscInfo(A,"Empty matrix\n");CHKERRQ(ierr);
  }
#endif

  ierr = ISRestoreIndices(isrow,&r);CHKERRQ(ierr);
  ierr = ISRestoreIndices(isicol,&ic);CHKERRQ(ierr);

  /* destroy list of free space and other temporary array(s) */
  ierr = PetscMalloc((bi[n]+1)*sizeof(PetscInt),&bj);CHKERRQ(ierr);
  ierr = PetscFreeSpaceContiguous(&free_space,bj);CHKERRQ(ierr); 
  ierr = PetscLLDestroy(lnk,lnkbt);CHKERRQ(ierr);
  ierr = PetscFree2(bi_ptr,im);CHKERRQ(ierr);

  /* put together the new matrix */
  ierr = MatSeqAIJSetPreallocation_SeqAIJ(B,MAT_SKIP_ALLOCATION,PETSC_NULL);CHKERRQ(ierr);
  ierr = PetscLogObjectParent(B,isicol);CHKERRQ(ierr);
  b    = (Mat_SeqAIJ*)(B)->data;
  b->free_a       = PETSC_TRUE;
  b->free_ij      = PETSC_TRUE;
  b->singlemalloc = PETSC_FALSE;
  ierr          = PetscMalloc((bi[n]+1)*sizeof(PetscScalar),&b->a);CHKERRQ(ierr);
  b->j          = bj; 
  b->i          = bi;
  b->diag       = bdiag;
  b->ilen       = 0;
  b->imax       = 0;
  b->row        = isrow;
  b->col        = iscol;
  ierr          = PetscObjectReference((PetscObject)isrow);CHKERRQ(ierr);
  ierr          = PetscObjectReference((PetscObject)iscol);CHKERRQ(ierr);
  b->icol       = isicol;
  ierr          = PetscMalloc((n+1)*sizeof(PetscScalar),&b->solve_work);CHKERRQ(ierr);

  /* In b structure:  Free imax, ilen, old a, old j.  Allocate solve_work, new a, new j */
  ierr = PetscLogObjectMemory(B,(bi[n]-n)*(sizeof(PetscInt)+sizeof(PetscScalar)));CHKERRQ(ierr);
  b->maxnz = b->nz = bi[n] ;

  (B)->factor                = MAT_FACTOR_LU;
  (B)->info.factor_mallocs   = reallocs;
  (B)->info.fill_ratio_given = f;

  if (ai[n]) {
    (B)->info.fill_ratio_needed = ((PetscReal)bi[n])/((PetscReal)ai[n]);
  } else {
    (B)->info.fill_ratio_needed = 0.0;
  }
  (B)->ops->lufactornumeric  = MatLUFactorNumeric_SeqAIJ;
  (B)->ops->solve            = MatSolve_SeqAIJ;
  (B)->ops->solvetranspose   = MatSolveTranspose_SeqAIJ;
  /* switch to inodes if appropriate */
  ierr = MatLUFactorSymbolic_Inode(B,A,isrow,iscol,info);CHKERRQ(ierr); 
  PetscFunctionReturn(0); 
}

/*
    Trouble in factorization, should we dump the original matrix?
*/
#undef __FUNCT__  
#define __FUNCT__ "MatFactorDumpMatrix"
PetscErrorCode MatFactorDumpMatrix(Mat A)
{
  PetscErrorCode ierr;
  PetscTruth     flg = PETSC_FALSE;

  PetscFunctionBegin;
  ierr = PetscOptionsGetTruth(PETSC_NULL,"-mat_factor_dump_on_error",&flg,PETSC_NULL);CHKERRQ(ierr);
  if (flg) {
    PetscViewer viewer;
    char        filename[PETSC_MAX_PATH_LEN];

    ierr = PetscSNPrintf(filename,PETSC_MAX_PATH_LEN,"matrix_factor_error.%d",PetscGlobalRank);CHKERRQ(ierr);
    ierr = PetscViewerBinaryOpen(((PetscObject)A)->comm,filename,FILE_MODE_WRITE,&viewer);CHKERRQ(ierr);
    ierr = MatView(A,viewer);CHKERRQ(ierr);
    ierr = PetscViewerDestroy(viewer);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

extern PetscErrorCode MatSolve_Inode(Mat,Vec,Vec);

/* ----------------------------------------------------------- */
#undef __FUNCT__  
#define __FUNCT__ "MatLUFactorNumeric_SeqAIJ"
PetscErrorCode MatLUFactorNumeric_SeqAIJ(Mat B,Mat A,const MatFactorInfo *info)
{
  Mat            C=B;
  Mat_SeqAIJ     *a=(Mat_SeqAIJ*)A->data,*b=(Mat_SeqAIJ *)C->data;
  IS             isrow = b->row,isicol = b->icol;
  PetscErrorCode ierr;
  const PetscInt  *r,*ic,*ics;
  PetscInt       i,j,n=A->rmap->n,*ai=a->i,*aj=a->j,*bi=b->i,*bj=b->j;
  PetscInt       *ajtmp,*bjtmp,nz,row,*diag_offset = b->diag,diag,*pj;
  MatScalar      *rtmp,*pc,multiplier,*v,*pv,d,*aa=a->a;
  PetscReal      rs=0.0;
  LUShift_Ctx    sctx;
  PetscInt       newshift,*ddiag;

  PetscFunctionBegin;
  ierr = ISGetIndices(isrow,&r);CHKERRQ(ierr);
  ierr = ISGetIndices(isicol,&ic);CHKERRQ(ierr);
  ierr = PetscMalloc((n+1)*sizeof(MatScalar),&rtmp);CHKERRQ(ierr);
  ics  = ic;

  sctx.shift_top      = 0;
  sctx.nshift_max     = 0;
  sctx.shift_lo       = 0;
  sctx.shift_hi       = 0;
  sctx.shift_fraction = 0;

  /* if both shift schemes are chosen by user, only use info->shiftpd */
  if (info->shiftpd) { /* set sctx.shift_top=max{rs} */
    ddiag          = a->diag;
    sctx.shift_top = info->zeropivot;
    for (i=0; i<n; i++) {
      /* calculate sum(|aij|)-RealPart(aii), amt of shift needed for this row */
      d  = (aa)[ddiag[i]];
      rs = -PetscAbsScalar(d) - PetscRealPart(d);
      v  = aa+ai[i];
      nz = ai[i+1] - ai[i];
      for (j=0; j<nz; j++) 
	rs += PetscAbsScalar(v[j]);
      if (rs>sctx.shift_top) sctx.shift_top = rs;
    }
    sctx.shift_top   *= 1.1;
    sctx.nshift_max   = 5;
    sctx.shift_lo     = 0.;
    sctx.shift_hi     = 1.;
  }

  sctx.shift_amount = 0.0;
  sctx.nshift       = 0;
  do {
    sctx.lushift = PETSC_FALSE;
    for (i=0; i<n; i++){
      nz    = bi[i+1] - bi[i];
      bjtmp = bj + bi[i];
      for  (j=0; j<nz; j++) rtmp[bjtmp[j]] = 0.0;

      /* load in initial (unfactored row) */
      nz    = ai[r[i]+1] - ai[r[i]];
      ajtmp = aj + ai[r[i]];
      v     = aa + ai[r[i]];
      for (j=0; j<nz; j++) {
        rtmp[ics[ajtmp[j]]] = v[j];
      }
      rtmp[ics[r[i]]] += sctx.shift_amount; /* shift the diagonal of the matrix */

      row = *bjtmp++;
      while  (row < i) {
        pc = rtmp + row;
        if (*pc != 0.0) {
          pv         = b->a + diag_offset[row];
          pj         = b->j + diag_offset[row] + 1;
          multiplier = *pc / *pv++;
          *pc        = multiplier;
          nz         = bi[row+1] - diag_offset[row] - 1;
          for (j=0; j<nz; j++) rtmp[pj[j]] -= multiplier * pv[j];
          ierr = PetscLogFlops(2.0*nz);CHKERRQ(ierr);
        }
        row = *bjtmp++;
      }
      /* finished row so stick it into b->a */
      pv   = b->a + bi[i] ;
      pj   = b->j + bi[i] ;
      nz   = bi[i+1] - bi[i];
      diag = diag_offset[i] - bi[i];
      rs   = -PetscAbsScalar(pv[diag]);
      for (j=0; j<nz; j++) {
        pv[j] = rtmp[pj[j]];
        rs   += PetscAbsScalar(pv[j]);
      }

      /* 9/13/02 Victor Eijkhout suggested scaling zeropivot by rs for matrices with funny scalings */
      sctx.rs  = rs;
      sctx.pv  = pv[diag];
      ierr = MatLUCheckShift_inline(info,sctx,i,newshift);CHKERRQ(ierr);
      if (newshift == 1) break;
    } 

    if (info->shiftpd && !sctx.lushift && sctx.shift_fraction>0 && sctx.nshift<sctx.nshift_max) {
      /*
       * if no shift in this attempt & shifting & started shifting & can refine,
       * then try lower shift
       */
      sctx.shift_hi       = sctx.shift_fraction;
      sctx.shift_fraction = (sctx.shift_hi+sctx.shift_lo)/2.;
      sctx.shift_amount   = sctx.shift_fraction * sctx.shift_top;
      sctx.lushift        = PETSC_TRUE;
      sctx.nshift++;
    }
  } while (sctx.lushift);

  /* invert diagonal entries for simplier triangular solves */
  for (i=0; i<n; i++) {
    b->a[diag_offset[i]] = 1.0/b->a[diag_offset[i]];
  }
  ierr = PetscFree(rtmp);CHKERRQ(ierr);
  ierr = ISRestoreIndices(isicol,&ic);CHKERRQ(ierr);
  ierr = ISRestoreIndices(isrow,&r);CHKERRQ(ierr);
  if (b->inode.use) {
    C->ops->solve   = MatSolve_Inode;
  } else {
    PetscTruth row_identity, col_identity;
    ierr = ISIdentity(isrow,&row_identity);CHKERRQ(ierr);
    ierr = ISIdentity(isicol,&col_identity);CHKERRQ(ierr);
    if (row_identity && col_identity) {
      C->ops->solve   = MatSolve_SeqAIJ_NaturalOrdering;
    } else {
      C->ops->solve   = MatSolve_SeqAIJ;
    }
  }
  C->ops->solveadd           = MatSolveAdd_SeqAIJ;
  C->ops->solvetranspose     = MatSolveTranspose_SeqAIJ;
  C->ops->solvetransposeadd  = MatSolveTransposeAdd_SeqAIJ;
  C->ops->matsolve           = MatMatSolve_SeqAIJ;
  C->assembled    = PETSC_TRUE;
  C->preallocated = PETSC_TRUE;
  ierr = PetscLogFlops(C->cmap->n);CHKERRQ(ierr);
  if (sctx.nshift){
     if (info->shiftpd) {
      ierr = PetscInfo4(A,"number of shift_pd tries %D, shift_amount %G, diagonal shifted up by %e fraction top_value %e\n",sctx.nshift,sctx.shift_amount,sctx.shift_fraction,sctx.shift_top);CHKERRQ(ierr);
    } else if (info->shiftnz) {
      ierr = PetscInfo2(A,"number of shift_nz tries %D, shift_amount %G\n",sctx.nshift,sctx.shift_amount);CHKERRQ(ierr);
    }
  }
  PetscFunctionReturn(0);
}

/* 
   This routine implements inplace ILU(0) with row or/and column permutations. 
   Input: 
     A - original matrix
   Output;
     A - a->i (rowptr) is same as original rowptr, but factored i-the row is stored in rowperm[i] 
         a->j (col index) is permuted by the inverse of colperm, then sorted
         a->a reordered accordingly with a->j
         a->diag (ptr to diagonal elements) is updated.
*/
#undef __FUNCT__  
#define __FUNCT__ "MatLUFactorNumeric_SeqAIJ_InplaceWithPerm"
PetscErrorCode MatLUFactorNumeric_SeqAIJ_InplaceWithPerm(Mat B,Mat A,const MatFactorInfo *info)
{
  Mat_SeqAIJ     *a=(Mat_SeqAIJ*)A->data; 
  IS             isrow = a->row,isicol = a->icol;
  PetscErrorCode ierr;
  const PetscInt *r,*ic,*ics;
  PetscInt       i,j,n=A->rmap->n,*ai=a->i,*aj=a->j;
  PetscInt       *ajtmp,nz,row;
  PetscInt       *diag = a->diag,nbdiag,*pj;
  PetscScalar    *rtmp,*pc,multiplier,d;
  MatScalar      *v,*pv;
  PetscReal      rs;
  LUShift_Ctx    sctx;
  PetscInt       newshift;

  PetscFunctionBegin;
  if (A != B) SETERRQ(PETSC_ERR_ARG_INCOMP,"input and output matrix must have same address");
  ierr  = ISGetIndices(isrow,&r);CHKERRQ(ierr);
  ierr  = ISGetIndices(isicol,&ic);CHKERRQ(ierr);
  ierr  = PetscMalloc((n+1)*sizeof(PetscScalar),&rtmp);CHKERRQ(ierr);
  ierr  = PetscMemzero(rtmp,(n+1)*sizeof(PetscScalar));CHKERRQ(ierr);
  ics = ic;

  sctx.shift_top      = 0;
  sctx.nshift_max     = 0;
  sctx.shift_lo       = 0;
  sctx.shift_hi       = 0;
  sctx.shift_fraction = 0;

  /* if both shift schemes are chosen by user, only use info->shiftpd */
  if (info->shiftpd) { /* set sctx.shift_top=max{rs} */
    sctx.shift_top = 0;
    for (i=0; i<n; i++) {
      /* calculate sum(|aij|)-RealPart(aii), amt of shift needed for this row */
      d  = (a->a)[diag[i]];
      rs = -PetscAbsScalar(d) - PetscRealPart(d);
      v  = a->a+ai[i];
      nz = ai[i+1] - ai[i];
      for (j=0; j<nz; j++) 
	rs += PetscAbsScalar(v[j]);
      if (rs>sctx.shift_top) sctx.shift_top = rs;
    }
    if (sctx.shift_top < info->zeropivot) sctx.shift_top = info->zeropivot;
    sctx.shift_top    *= 1.1;
    sctx.nshift_max   = 5;
    sctx.shift_lo     = 0.;
    sctx.shift_hi     = 1.;
  }

  sctx.shift_amount = 0;
  sctx.nshift       = 0;
  do {
    sctx.lushift = PETSC_FALSE;
    for (i=0; i<n; i++){
      /* load in initial unfactored row */
      nz    = ai[r[i]+1] - ai[r[i]];
      ajtmp = aj + ai[r[i]];
      v     = a->a + ai[r[i]];
      /* sort permuted ajtmp and values v accordingly */
      for (j=0; j<nz; j++) ajtmp[j] = ics[ajtmp[j]];
      ierr = PetscSortIntWithScalarArray(nz,ajtmp,v);CHKERRQ(ierr);

      diag[r[i]] = ai[r[i]]; 
      for (j=0; j<nz; j++) {
        rtmp[ajtmp[j]] = v[j];
        if (ajtmp[j] < i) diag[r[i]]++; /* update a->diag */
      }
      rtmp[r[i]] += sctx.shift_amount; /* shift the diagonal of the matrix */

      row = *ajtmp++;
      while  (row < i) {
        pc = rtmp + row;
        if (*pc != 0.0) {
          pv         = a->a + diag[r[row]];
          pj         = aj + diag[r[row]] + 1;

          multiplier = *pc / *pv++;
          *pc        = multiplier;
          nz         = ai[r[row]+1] - diag[r[row]] - 1;
          for (j=0; j<nz; j++) rtmp[pj[j]] -= multiplier * pv[j];
          ierr = PetscLogFlops(2.0*nz);CHKERRQ(ierr);
        }
        row = *ajtmp++;
      }
      /* finished row so overwrite it onto a->a */
      pv   = a->a + ai[r[i]] ;
      pj   = aj + ai[r[i]] ;
      nz   = ai[r[i]+1] - ai[r[i]];
      nbdiag = diag[r[i]] - ai[r[i]]; /* num of entries before the diagonal */
      
      rs   = 0.0;
      for (j=0; j<nz; j++) {
        pv[j] = rtmp[pj[j]];
        if (j != nbdiag) rs += PetscAbsScalar(pv[j]);
      }

      /* 9/13/02 Victor Eijkhout suggested scaling zeropivot by rs for matrices with funny scalings */
      sctx.rs  = rs;
      sctx.pv  = pv[nbdiag];
      ierr = MatLUCheckShift_inline(info,sctx,i,newshift);CHKERRQ(ierr);
      if (newshift == 1) break;
    } 

    if (info->shiftpd && !sctx.lushift && sctx.shift_fraction>0 && sctx.nshift<sctx.nshift_max) {
      /*
       * if no shift in this attempt & shifting & started shifting & can refine,
       * then try lower shift
       */
      sctx.shift_hi        = sctx.shift_fraction;
      sctx.shift_fraction = (sctx.shift_hi+sctx.shift_lo)/2.;
      sctx.shift_amount    = sctx.shift_fraction * sctx.shift_top;
      sctx.lushift         = PETSC_TRUE;
      sctx.nshift++;
    }
  } while (sctx.lushift);

  /* invert diagonal entries for simplier triangular solves */
  for (i=0; i<n; i++) {
    a->a[diag[r[i]]] = 1.0/a->a[diag[r[i]]];
  }

  ierr = PetscFree(rtmp);CHKERRQ(ierr);
  ierr = ISRestoreIndices(isicol,&ic);CHKERRQ(ierr);
  ierr = ISRestoreIndices(isrow,&r);CHKERRQ(ierr);
  A->ops->solve             = MatSolve_SeqAIJ_InplaceWithPerm;
  A->ops->solveadd          = MatSolveAdd_SeqAIJ;
  A->ops->solvetranspose    = MatSolveTranspose_SeqAIJ;
  A->ops->solvetransposeadd = MatSolveTransposeAdd_SeqAIJ;
  A->assembled = PETSC_TRUE;
  A->preallocated = PETSC_TRUE;
  ierr = PetscLogFlops(A->cmap->n);CHKERRQ(ierr);
  if (sctx.nshift){
    if (info->shiftpd) {
      ierr = PetscInfo4(A,"number of shift_pd tries %D, shift_amount %G, diagonal shifted up by %e fraction top_value %e\n",sctx.nshift,sctx.shift_amount,sctx.shift_fraction,sctx.shift_top);CHKERRQ(ierr);
    } else if (info->shiftnz) {
      ierr = PetscInfo2(A,"number of shift_nz tries %D, shift_amount %G\n",sctx.nshift,sctx.shift_amount);CHKERRQ(ierr);
    } 
  }
  PetscFunctionReturn(0);
}

/* ----------------------------------------------------------- */
#undef __FUNCT__  
#define __FUNCT__ "MatLUFactor_SeqAIJ"
PetscErrorCode MatLUFactor_SeqAIJ(Mat A,IS row,IS col,const MatFactorInfo *info)
{
  PetscErrorCode ierr;
  Mat            C;

  PetscFunctionBegin;
  ierr = MatGetFactor(A,MAT_SOLVER_PETSC,MAT_FACTOR_LU,&C);CHKERRQ(ierr);
  ierr = MatLUFactorSymbolic(C,A,row,col,info);CHKERRQ(ierr);
  ierr = MatLUFactorNumeric(C,A,info);CHKERRQ(ierr);
  A->ops->solve            = C->ops->solve;
  A->ops->solvetranspose   = C->ops->solvetranspose;
  ierr = MatHeaderCopy(A,C);CHKERRQ(ierr);
  ierr = PetscLogObjectParent(A,((Mat_SeqAIJ*)(A->data))->icol);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
/* ----------------------------------------------------------- */


#undef __FUNCT__  
#define __FUNCT__ "MatSolve_SeqAIJ"
PetscErrorCode MatSolve_SeqAIJ(Mat A,Vec bb,Vec xx)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)A->data;
  IS                iscol = a->col,isrow = a->row;
  PetscErrorCode    ierr;
  PetscInt          i, n = A->rmap->n,*vi,*ai = a->i,*aj = a->j;
  PetscInt          nz;
  const PetscInt    *rout,*cout,*r,*c;
  PetscScalar       *x,*tmp,*tmps,sum;
  const PetscScalar *b;
  const MatScalar   *aa = a->a,*v;
 
  PetscFunctionBegin;
  if (!n) PetscFunctionReturn(0);

  ierr = VecGetArray(bb,(PetscScalar**)&b);CHKERRQ(ierr); 
  ierr = VecGetArray(xx,&x);CHKERRQ(ierr);
  tmp  = a->solve_work;

  ierr = ISGetIndices(isrow,&rout);CHKERRQ(ierr); r = rout;
  ierr = ISGetIndices(iscol,&cout);CHKERRQ(ierr); c = cout + (n-1);

  /* forward solve the lower triangular */
  tmp[0] = b[*r++];
  tmps   = tmp;
  for (i=1; i<n; i++) {
    v   = aa + ai[i] ;
    vi  = aj + ai[i] ;
    nz  = a->diag[i] - ai[i];
    sum = b[*r++];
    PetscSparseDenseMinusDot(sum,tmps,v,vi,nz); 
    tmp[i] = sum;
  }

  /* backward solve the upper triangular */
  for (i=n-1; i>=0; i--){
    v   = aa + a->diag[i] + 1;
    vi  = aj + a->diag[i] + 1;
    nz  = ai[i+1] - a->diag[i] - 1;
    sum = tmp[i];
    PetscSparseDenseMinusDot(sum,tmps,v,vi,nz); 
    x[*c--] = tmp[i] = sum*aa[a->diag[i]];
  }

  ierr = ISRestoreIndices(isrow,&rout);CHKERRQ(ierr);
  ierr = ISRestoreIndices(iscol,&cout);CHKERRQ(ierr);
  ierr = VecRestoreArray(bb,(PetscScalar**)&b);CHKERRQ(ierr); 
  ierr = VecRestoreArray(xx,&x);CHKERRQ(ierr);
  ierr = PetscLogFlops(2.0*a->nz - A->cmap->n);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatMatSolve_SeqAIJ"
PetscErrorCode MatMatSolve_SeqAIJ(Mat A,Mat B,Mat X)
{
  Mat_SeqAIJ      *a = (Mat_SeqAIJ*)A->data;
  IS              iscol = a->col,isrow = a->row;
  PetscErrorCode  ierr;
  PetscInt        i, n = A->rmap->n,*vi,*ai = a->i,*aj = a->j;
  PetscInt        nz,neq; 
  const PetscInt  *rout,*cout,*r,*c;
  PetscScalar     *x,*b,*tmp,*tmps,sum;
  const MatScalar *aa = a->a,*v;
  PetscTruth      bisdense,xisdense;

  PetscFunctionBegin;
  if (!n) PetscFunctionReturn(0);

  ierr = PetscTypeCompare((PetscObject)B,MATSEQDENSE,&bisdense);CHKERRQ(ierr);
  if (!bisdense) SETERRQ(PETSC_ERR_ARG_INCOMP,"B matrix must be a SeqDense matrix");
  ierr = PetscTypeCompare((PetscObject)X,MATSEQDENSE,&xisdense);CHKERRQ(ierr);
  if (!xisdense) SETERRQ(PETSC_ERR_ARG_INCOMP,"X matrix must be a SeqDense matrix");

  ierr = MatGetArray(B,&b);CHKERRQ(ierr); 
  ierr = MatGetArray(X,&x);CHKERRQ(ierr);
  
  tmp  = a->solve_work;
  ierr = ISGetIndices(isrow,&rout);CHKERRQ(ierr); r = rout;
  ierr = ISGetIndices(iscol,&cout);CHKERRQ(ierr); c = cout;

  for (neq=0; neq<B->cmap->n; neq++){
    /* forward solve the lower triangular */
    tmp[0] = b[r[0]];
    tmps   = tmp;
    for (i=1; i<n; i++) {
      v   = aa + ai[i] ;
      vi  = aj + ai[i] ;
      nz  = a->diag[i] - ai[i];
      sum = b[r[i]];
      PetscSparseDenseMinusDot(sum,tmps,v,vi,nz); 
      tmp[i] = sum;
    }
    /* backward solve the upper triangular */
    for (i=n-1; i>=0; i--){
      v   = aa + a->diag[i] + 1;
      vi  = aj + a->diag[i] + 1;
      nz  = ai[i+1] - a->diag[i] - 1;
      sum = tmp[i];
      PetscSparseDenseMinusDot(sum,tmps,v,vi,nz); 
      x[c[i]] = tmp[i] = sum*aa[a->diag[i]];
    }

    b += n;
    x += n;
  }
  ierr = ISRestoreIndices(isrow,&rout);CHKERRQ(ierr);
  ierr = ISRestoreIndices(iscol,&cout);CHKERRQ(ierr);
  ierr = MatRestoreArray(B,&b);CHKERRQ(ierr); 
  ierr = MatRestoreArray(X,&x);CHKERRQ(ierr);
  ierr = PetscLogFlops(B->cmap->n*(2.0*a->nz - n));CHKERRQ(ierr);
  PetscFunctionReturn(0);
}  

#undef __FUNCT__  
#define __FUNCT__ "MatSolve_SeqAIJ_InplaceWithPerm"
PetscErrorCode MatSolve_SeqAIJ_InplaceWithPerm(Mat A,Vec bb,Vec xx)
{
  Mat_SeqAIJ      *a = (Mat_SeqAIJ*)A->data;
  IS              iscol = a->col,isrow = a->row;
  PetscErrorCode  ierr;
  const PetscInt  *r,*c,*rout,*cout;
  PetscInt        i, n = A->rmap->n,*vi,*ai = a->i,*aj = a->j;
  PetscInt        nz,row;
  PetscScalar     *x,*b,*tmp,*tmps,sum;
  const MatScalar *aa = a->a,*v;

  PetscFunctionBegin;
  if (!n) PetscFunctionReturn(0);

  ierr = VecGetArray(bb,&b);CHKERRQ(ierr); 
  ierr = VecGetArray(xx,&x);CHKERRQ(ierr);
  tmp  = a->solve_work;

  ierr = ISGetIndices(isrow,&rout);CHKERRQ(ierr); r = rout;
  ierr = ISGetIndices(iscol,&cout);CHKERRQ(ierr); c = cout + (n-1);

  /* forward solve the lower triangular */
  tmp[0] = b[*r++];
  tmps   = tmp;
  for (row=1; row<n; row++) {
    i   = rout[row]; /* permuted row */
    v   = aa + ai[i] ;
    vi  = aj + ai[i] ;
    nz  = a->diag[i] - ai[i];
    sum = b[*r++];
    PetscSparseDenseMinusDot(sum,tmps,v,vi,nz); 
    tmp[row] = sum;
  }

  /* backward solve the upper triangular */
  for (row=n-1; row>=0; row--){
    i   = rout[row]; /* permuted row */
    v   = aa + a->diag[i] + 1;
    vi  = aj + a->diag[i] + 1;
    nz  = ai[i+1] - a->diag[i] - 1;
    sum = tmp[row];
    PetscSparseDenseMinusDot(sum,tmps,v,vi,nz); 
    x[*c--] = tmp[row] = sum*aa[a->diag[i]];
  }

  ierr = ISRestoreIndices(isrow,&rout);CHKERRQ(ierr);
  ierr = ISRestoreIndices(iscol,&cout);CHKERRQ(ierr);
  ierr = VecRestoreArray(bb,&b);CHKERRQ(ierr); 
  ierr = VecRestoreArray(xx,&x);CHKERRQ(ierr);
  ierr = PetscLogFlops(2.0*a->nz - A->cmap->n);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/* ----------------------------------------------------------- */
#undef __FUNCT__  
#define __FUNCT__ "MatSolve_SeqAIJ_NaturalOrdering"
PetscErrorCode MatSolve_SeqAIJ_NaturalOrdering(Mat A,Vec bb,Vec xx)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)A->data;
  PetscErrorCode    ierr;
  PetscInt          n = A->rmap->n;
  const PetscInt    *ai = a->i,*aj = a->j,*adiag = a->diag,*vi;
  PetscScalar       *x;
  const PetscScalar *b;
  const MatScalar   *aa = a->a;
#if !defined(PETSC_USE_FORTRAN_KERNEL_SOLVEAIJ)
  PetscInt          adiag_i,i,nz,ai_i;
  const MatScalar   *v;
  PetscScalar       sum;
#endif

  PetscFunctionBegin;
  if (!n) PetscFunctionReturn(0);

  ierr = VecGetArray(bb,(PetscScalar**)&b);CHKERRQ(ierr);
  ierr = VecGetArray(xx,&x);CHKERRQ(ierr);

#if defined(PETSC_USE_FORTRAN_KERNEL_SOLVEAIJ)
  fortransolveaij_(&n,x,ai,aj,adiag,aa,b);
#else
  /* forward solve the lower triangular */
  x[0] = b[0];
  for (i=1; i<n; i++) {
    ai_i = ai[i];
    v    = aa + ai_i;
    vi   = aj + ai_i;
    nz   = adiag[i] - ai_i;
    sum  = b[i];
    PetscSparseDenseMinusDot(sum,x,v,vi,nz);    
    x[i] = sum;
  }

  /* backward solve the upper triangular */
  for (i=n-1; i>=0; i--){
    adiag_i = adiag[i];
    v       = aa + adiag_i + 1;
    vi      = aj + adiag_i + 1;
    nz      = ai[i+1] - adiag_i - 1;
    sum     = x[i];
    PetscSparseDenseMinusDot(sum,x,v,vi,nz);
    x[i]    = sum*aa[adiag_i];
  }
#endif
  ierr = PetscLogFlops(2.0*a->nz - A->cmap->n);CHKERRQ(ierr);
  ierr = VecRestoreArray(bb,(PetscScalar**)&b);CHKERRQ(ierr);
  ierr = VecRestoreArray(xx,&x);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatSolveAdd_SeqAIJ"
PetscErrorCode MatSolveAdd_SeqAIJ(Mat A,Vec bb,Vec yy,Vec xx)
{
  Mat_SeqAIJ      *a = (Mat_SeqAIJ*)A->data;
  IS              iscol = a->col,isrow = a->row;
  PetscErrorCode  ierr;
  PetscInt        i, n = A->rmap->n,*vi,*ai = a->i,*aj = a->j;
  PetscInt        nz;
  const PetscInt  *rout,*cout,*r,*c;
  PetscScalar     *x,*b,*tmp,sum;
  const MatScalar *aa = a->a,*v;

  PetscFunctionBegin;
  if (yy != xx) {ierr = VecCopy(yy,xx);CHKERRQ(ierr);}

  ierr = VecGetArray(bb,&b);CHKERRQ(ierr);
  ierr = VecGetArray(xx,&x);CHKERRQ(ierr);
  tmp  = a->solve_work;

  ierr = ISGetIndices(isrow,&rout);CHKERRQ(ierr); r = rout;
  ierr = ISGetIndices(iscol,&cout);CHKERRQ(ierr); c = cout + (n-1);

  /* forward solve the lower triangular */
  tmp[0] = b[*r++];
  for (i=1; i<n; i++) {
    v   = aa + ai[i] ;
    vi  = aj + ai[i] ;
    nz  = a->diag[i] - ai[i];
    sum = b[*r++];
    while (nz--) sum -= *v++ * tmp[*vi++ ];
    tmp[i] = sum;
  }

  /* backward solve the upper triangular */
  for (i=n-1; i>=0; i--){
    v   = aa + a->diag[i] + 1;
    vi  = aj + a->diag[i] + 1;
    nz  = ai[i+1] - a->diag[i] - 1;
    sum = tmp[i];
    while (nz--) sum -= *v++ * tmp[*vi++ ];
    tmp[i] = sum*aa[a->diag[i]];
    x[*c--] += tmp[i];
  }

  ierr = ISRestoreIndices(isrow,&rout);CHKERRQ(ierr);
  ierr = ISRestoreIndices(iscol,&cout);CHKERRQ(ierr);
  ierr = VecRestoreArray(bb,&b);CHKERRQ(ierr);
  ierr = VecRestoreArray(xx,&x);CHKERRQ(ierr);
  ierr = PetscLogFlops(2.0*a->nz);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}
/* -------------------------------------------------------------------*/
#undef __FUNCT__  
#define __FUNCT__ "MatSolveTranspose_SeqAIJ"
PetscErrorCode MatSolveTranspose_SeqAIJ(Mat A,Vec bb,Vec xx)
{
  Mat_SeqAIJ      *a = (Mat_SeqAIJ*)A->data;
  IS              iscol = a->col,isrow = a->row;
  PetscErrorCode  ierr;
  const PetscInt  *rout,*cout,*r,*c;
  PetscInt        i,n = A->rmap->n,*vi,*ai = a->i,*aj = a->j;
  PetscInt        nz,*diag = a->diag;
  PetscScalar     *x,*b,*tmp,s1;
  const MatScalar *aa = a->a,*v;

  PetscFunctionBegin;
  ierr = VecGetArray(bb,&b);CHKERRQ(ierr);
  ierr = VecGetArray(xx,&x);CHKERRQ(ierr);
  tmp  = a->solve_work;

  ierr = ISGetIndices(isrow,&rout);CHKERRQ(ierr); r = rout;
  ierr = ISGetIndices(iscol,&cout);CHKERRQ(ierr); c = cout;

  /* copy the b into temp work space according to permutation */
  for (i=0; i<n; i++) tmp[i] = b[c[i]]; 

  /* forward solve the U^T */
  for (i=0; i<n; i++) {
    v   = aa + diag[i] ;
    vi  = aj + diag[i] + 1;
    nz  = ai[i+1] - diag[i] - 1;
    s1  = tmp[i];
    s1 *= (*v++);  /* multiply by inverse of diagonal entry */
    while (nz--) {
      tmp[*vi++ ] -= (*v++)*s1;
    }
    tmp[i] = s1;
  }

  /* backward solve the L^T */
  for (i=n-1; i>=0; i--){
    v   = aa + diag[i] - 1 ;
    vi  = aj + diag[i] - 1 ;
    nz  = diag[i] - ai[i];
    s1  = tmp[i];
    while (nz--) {
      tmp[*vi-- ] -= (*v--)*s1;
    }
  }

  /* copy tmp into x according to permutation */
  for (i=0; i<n; i++) x[r[i]] = tmp[i];

  ierr = ISRestoreIndices(isrow,&rout);CHKERRQ(ierr);
  ierr = ISRestoreIndices(iscol,&cout);CHKERRQ(ierr);
  ierr = VecRestoreArray(bb,&b);CHKERRQ(ierr);
  ierr = VecRestoreArray(xx,&x);CHKERRQ(ierr);

  ierr = PetscLogFlops(2.0*a->nz-A->cmap->n);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatSolveTransposeAdd_SeqAIJ"
PetscErrorCode MatSolveTransposeAdd_SeqAIJ(Mat A,Vec bb,Vec zz,Vec xx)
{
  Mat_SeqAIJ      *a = (Mat_SeqAIJ*)A->data;
  IS              iscol = a->col,isrow = a->row;
  PetscErrorCode  ierr;
  const PetscInt  *r,*c,*rout,*cout;
  PetscInt        i,n = A->rmap->n,*vi,*ai = a->i,*aj = a->j;
  PetscInt        nz,*diag = a->diag;
  PetscScalar     *x,*b,*tmp;
  const MatScalar *aa = a->a,*v;

  PetscFunctionBegin;
  if (zz != xx) {ierr = VecCopy(zz,xx);CHKERRQ(ierr);}

  ierr = VecGetArray(bb,&b);CHKERRQ(ierr);
  ierr = VecGetArray(xx,&x);CHKERRQ(ierr);
  tmp = a->solve_work;

  ierr = ISGetIndices(isrow,&rout);CHKERRQ(ierr); r = rout;
  ierr = ISGetIndices(iscol,&cout);CHKERRQ(ierr); c = cout;

  /* copy the b into temp work space according to permutation */
  for (i=0; i<n; i++) tmp[i] = b[c[i]]; 

  /* forward solve the U^T */
  for (i=0; i<n; i++) {
    v   = aa + diag[i] ;
    vi  = aj + diag[i] + 1;
    nz  = ai[i+1] - diag[i] - 1;
    tmp[i] *= *v++;
    while (nz--) {
      tmp[*vi++ ] -= (*v++)*tmp[i];
    }
  }

  /* backward solve the L^T */
  for (i=n-1; i>=0; i--){
    v   = aa + diag[i] - 1 ;
    vi  = aj + diag[i] - 1 ;
    nz  = diag[i] - ai[i];
    while (nz--) {
      tmp[*vi-- ] -= (*v--)*tmp[i];
    }
  }

  /* copy tmp into x according to permutation */
  for (i=0; i<n; i++) x[r[i]] += tmp[i]; 

  ierr = ISRestoreIndices(isrow,&rout);CHKERRQ(ierr);
  ierr = ISRestoreIndices(iscol,&cout);CHKERRQ(ierr);
  ierr = VecRestoreArray(bb,&b);CHKERRQ(ierr);
  ierr = VecRestoreArray(xx,&x);CHKERRQ(ierr);

  ierr = PetscLogFlops(2.0*a->nz);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
/* ----------------------------------------------------------------*/
EXTERN PetscErrorCode Mat_CheckInode(Mat,PetscTruth);
EXTERN PetscErrorCode MatDuplicateNoCreate_SeqAIJ(Mat,Mat,MatDuplicateOption);

#undef __FUNCT__  
#define __FUNCT__ "MatILUFactorSymbolic_SeqAIJ"
PetscErrorCode MatILUFactorSymbolic_SeqAIJ(Mat fact,Mat A,IS isrow,IS iscol,const MatFactorInfo *info)
{
  Mat_SeqAIJ         *a = (Mat_SeqAIJ*)A->data,*b;
  IS                 isicol;
  PetscErrorCode     ierr;
  const PetscInt     *r,*ic;
  PetscInt           n=A->rmap->n,*ai=a->i,*aj=a->j,d;
  PetscInt           *bi,*cols,nnz,*cols_lvl;
  PetscInt           *bdiag,prow,fm,nzbd,reallocs=0,dcount=0;
  PetscInt           i,levels,diagonal_fill;
  PetscTruth         col_identity,row_identity;
  PetscReal          f;
  PetscInt           nlnk,*lnk,*lnk_lvl=PETSC_NULL;
  PetscBT            lnkbt;
  PetscInt           nzi,*bj,**bj_ptr,**bjlvl_ptr; 
  PetscFreeSpaceList free_space=PETSC_NULL,current_space=PETSC_NULL; 
  PetscFreeSpaceList free_space_lvl=PETSC_NULL,current_space_lvl=PETSC_NULL; 
  PetscTruth         missing;

  PetscFunctionBegin;
  if (A->rmap->n != A->cmap->n) SETERRQ2(PETSC_ERR_ARG_WRONG,"Must be square matrix, rows %D columns %D",A->rmap->n,A->cmap->n);
  f             = info->fill;
  levels        = (PetscInt)info->levels;
  diagonal_fill = (PetscInt)info->diagonal_fill;
  ierr = ISInvertPermutation(iscol,PETSC_DECIDE,&isicol);CHKERRQ(ierr);

  /* special case that simply copies fill pattern */
  ierr = ISIdentity(isrow,&row_identity);CHKERRQ(ierr);
  ierr = ISIdentity(iscol,&col_identity);CHKERRQ(ierr);
  if (!levels && row_identity && col_identity) {
    ierr = MatDuplicateNoCreate_SeqAIJ(fact,A,MAT_DO_NOT_COPY_VALUES);CHKERRQ(ierr);
    fact->factor = MAT_FACTOR_ILU;
    (fact)->info.factor_mallocs    = 0;
    (fact)->info.fill_ratio_given  = info->fill;
    (fact)->info.fill_ratio_needed = 1.0;
    b               = (Mat_SeqAIJ*)(fact)->data;
    ierr = MatMissingDiagonal(A,&missing,&d);CHKERRQ(ierr);
    if (missing) SETERRQ1(PETSC_ERR_ARG_WRONGSTATE,"Matrix is missing diagonal entry %D",d);
    b->row              = isrow;
    b->col              = iscol;
    b->icol             = isicol;
    ierr                = PetscMalloc(((fact)->rmap->n+1)*sizeof(PetscScalar),&b->solve_work);CHKERRQ(ierr);
    ierr                = PetscObjectReference((PetscObject)isrow);CHKERRQ(ierr);
    ierr                = PetscObjectReference((PetscObject)iscol);CHKERRQ(ierr);
    (fact)->ops->lufactornumeric =  MatLUFactorNumeric_SeqAIJ;
    ierr = MatILUFactorSymbolic_Inode(fact,A,isrow,iscol,info);CHKERRQ(ierr); 
    PetscFunctionReturn(0);
  }

  ierr = ISGetIndices(isrow,&r);CHKERRQ(ierr);
  ierr = ISGetIndices(isicol,&ic);CHKERRQ(ierr);

  /* get new row pointers */
  ierr = PetscMalloc((n+1)*sizeof(PetscInt),&bi);CHKERRQ(ierr);
  bi[0] = 0;
  /* bdiag is location of diagonal in factor */
  ierr = PetscMalloc((n+1)*sizeof(PetscInt),&bdiag);CHKERRQ(ierr);
  bdiag[0]  = 0;

  ierr = PetscMalloc((2*n+1)*sizeof(PetscInt**),&bj_ptr);CHKERRQ(ierr); 
  bjlvl_ptr = (PetscInt**)(bj_ptr + n);

  /* create a linked list for storing column indices of the active row */
  nlnk = n + 1;
  ierr = PetscIncompleteLLCreate(n,n,nlnk,lnk,lnk_lvl,lnkbt);CHKERRQ(ierr);

  /* initial FreeSpace size is f*(ai[n]+1) */
  ierr = PetscFreeSpaceGet((PetscInt)(f*(ai[n]+1)),&free_space);CHKERRQ(ierr);
  current_space = free_space;
  ierr = PetscFreeSpaceGet((PetscInt)(f*(ai[n]+1)),&free_space_lvl);CHKERRQ(ierr);
  current_space_lvl = free_space_lvl;
 
  for (i=0; i<n; i++) {
    nzi = 0;
    /* copy current row into linked list */
    nnz  = ai[r[i]+1] - ai[r[i]];
    if (!nnz) SETERRQ2(PETSC_ERR_MAT_LU_ZRPVT,"Empty row in matrix: row in original ordering %D in permuted ordering %D",r[i],i);
    cols = aj + ai[r[i]];
    lnk[i] = -1; /* marker to indicate if diagonal exists */
    ierr = PetscIncompleteLLInit(nnz,cols,n,ic,nlnk,lnk,lnk_lvl,lnkbt);CHKERRQ(ierr);
    nzi += nlnk;

    /* make sure diagonal entry is included */
    if (diagonal_fill && lnk[i] == -1) {
      fm = n;
      while (lnk[fm] < i) fm = lnk[fm];
      lnk[i]     = lnk[fm]; /* insert diagonal into linked list */
      lnk[fm]    = i;
      lnk_lvl[i] = 0;
      nzi++; dcount++; 
    }

    /* add pivot rows into the active row */
    nzbd = 0;
    prow = lnk[n];
    while (prow < i) {
      nnz      = bdiag[prow];
      cols     = bj_ptr[prow] + nnz + 1;
      cols_lvl = bjlvl_ptr[prow] + nnz + 1; 
      nnz      = bi[prow+1] - bi[prow] - nnz - 1;
      ierr = PetscILULLAddSorted(nnz,cols,levels,cols_lvl,prow,nlnk,lnk,lnk_lvl,lnkbt,prow);CHKERRQ(ierr);
      nzi += nlnk;
      prow = lnk[prow];
      nzbd++;
    }
    bdiag[i] = nzbd;
    bi[i+1]  = bi[i] + nzi;

    /* if free space is not available, make more free space */
    if (current_space->local_remaining<nzi) {
      nnz = nzi*(n - i); /* estimated and max additional space needed */
      ierr = PetscFreeSpaceGet(nnz,&current_space);CHKERRQ(ierr);
      ierr = PetscFreeSpaceGet(nnz,&current_space_lvl);CHKERRQ(ierr);
      reallocs++;
    }

    /* copy data into free_space and free_space_lvl, then initialize lnk */
    ierr = PetscIncompleteLLClean(n,n,nzi,lnk,lnk_lvl,current_space->array,current_space_lvl->array,lnkbt);CHKERRQ(ierr);
    bj_ptr[i]    = current_space->array;
    bjlvl_ptr[i] = current_space_lvl->array;

    /* make sure the active row i has diagonal entry */
    if (*(bj_ptr[i]+bdiag[i]) != i) {
      SETERRQ1(PETSC_ERR_MAT_LU_ZRPVT,"Row %D has missing diagonal in factored matrix\n\
    try running with -pc_factor_nonzeros_along_diagonal or -pc_factor_diagonal_fill",i);
    }

    current_space->array           += nzi;
    current_space->local_used      += nzi;
    current_space->local_remaining -= nzi;
    current_space_lvl->array           += nzi;
    current_space_lvl->local_used      += nzi;
    current_space_lvl->local_remaining -= nzi;
  } 

  ierr = ISRestoreIndices(isrow,&r);CHKERRQ(ierr);
  ierr = ISRestoreIndices(isicol,&ic);CHKERRQ(ierr);

  /* destroy list of free space and other temporary arrays */
  ierr = PetscMalloc((bi[n]+1)*sizeof(PetscInt),&bj);CHKERRQ(ierr);
  ierr = PetscFreeSpaceContiguous(&free_space,bj);CHKERRQ(ierr); 
  ierr = PetscIncompleteLLDestroy(lnk,lnkbt);CHKERRQ(ierr);
  ierr = PetscFreeSpaceDestroy(free_space_lvl);CHKERRQ(ierr); 
  ierr = PetscFree(bj_ptr);CHKERRQ(ierr);

#if defined(PETSC_USE_INFO)
  {
    PetscReal af = ((PetscReal)bi[n])/((PetscReal)ai[n]);
    ierr = PetscInfo3(A,"Reallocs %D Fill ratio:given %G needed %G\n",reallocs,f,af);CHKERRQ(ierr);
    ierr = PetscInfo1(A,"Run with -[sub_]pc_factor_fill %G or use \n",af);CHKERRQ(ierr);
    ierr = PetscInfo1(A,"PCFactorSetFill([sub]pc,%G);\n",af);CHKERRQ(ierr);
    ierr = PetscInfo(A,"for best performance.\n");CHKERRQ(ierr);
    if (diagonal_fill) {
      ierr = PetscInfo1(A,"Detected and replaced %D missing diagonals",dcount);CHKERRQ(ierr);
    }
  }
#endif

  /* put together the new matrix */
  ierr = MatSeqAIJSetPreallocation_SeqAIJ(fact,MAT_SKIP_ALLOCATION,PETSC_NULL);CHKERRQ(ierr);
  ierr = PetscLogObjectParent(fact,isicol);CHKERRQ(ierr);
  b = (Mat_SeqAIJ*)(fact)->data;
  b->free_a       = PETSC_TRUE;
  b->free_ij      = PETSC_TRUE;
  b->singlemalloc = PETSC_FALSE;
  ierr = PetscMalloc( (bi[n] )*sizeof(PetscScalar),&b->a);CHKERRQ(ierr);
  b->j          = bj;
  b->i          = bi;
  for (i=0; i<n; i++) bdiag[i] += bi[i];
  b->diag       = bdiag;
  b->ilen       = 0;
  b->imax       = 0;
  b->row        = isrow;
  b->col        = iscol;
  ierr          = PetscObjectReference((PetscObject)isrow);CHKERRQ(ierr);
  ierr          = PetscObjectReference((PetscObject)iscol);CHKERRQ(ierr);
  b->icol       = isicol;
  ierr = PetscMalloc((n+1)*sizeof(PetscScalar),&b->solve_work);CHKERRQ(ierr);
  /* In b structure:  Free imax, ilen, old a, old j.  
     Allocate bdiag, solve_work, new a, new j */
  ierr = PetscLogObjectMemory(fact,(bi[n]-n) * (sizeof(PetscInt)+sizeof(PetscScalar)));CHKERRQ(ierr);
  b->maxnz             = b->nz = bi[n] ;
  (fact)->info.factor_mallocs    = reallocs;
  (fact)->info.fill_ratio_given  = f;
  (fact)->info.fill_ratio_needed = ((PetscReal)bi[n])/((PetscReal)ai[n]);
  (fact)->ops->lufactornumeric =  MatLUFactorNumeric_SeqAIJ;
  ierr = MatILUFactorSymbolic_Inode(fact,A,isrow,iscol,info);CHKERRQ(ierr); 
  PetscFunctionReturn(0); 
}

#include "../src/mat/impls/sbaij/seq/sbaij.h"
#undef __FUNCT__  
#define __FUNCT__ "MatCholeskyFactorNumeric_SeqAIJ"
PetscErrorCode MatCholeskyFactorNumeric_SeqAIJ(Mat B,Mat A,const MatFactorInfo *info)
{
  Mat            C = B;
  Mat_SeqAIJ     *a=(Mat_SeqAIJ*)A->data;
  Mat_SeqSBAIJ   *b=(Mat_SeqSBAIJ*)C->data;
  IS             ip=b->row,iip = b->icol;
  PetscErrorCode ierr;
  const PetscInt *rip,*riip;
  PetscInt       i,j,mbs=A->rmap->n,*bi=b->i,*bj=b->j,*bcol;
  PetscInt       *ai=a->i,*aj=a->j;
  PetscInt       k,jmin,jmax,*jl,*il,col,nexti,ili,nz;
  MatScalar      *rtmp,*ba=b->a,*bval,*aa=a->a,dk,uikdi;
  PetscReal      zeropivot,rs,shiftnz;
  PetscReal      shiftpd;
  ChShift_Ctx    sctx;
  PetscInt       newshift;
  PetscTruth     perm_identity;

  PetscFunctionBegin;

  shiftnz   = info->shiftnz;
  shiftpd   = info->shiftpd;
  zeropivot = info->zeropivot; 

  ierr  = ISGetIndices(ip,&rip);CHKERRQ(ierr);
  ierr  = ISGetIndices(iip,&riip);CHKERRQ(ierr);
  
  /* initialization */
  nz   = (2*mbs+1)*sizeof(PetscInt)+mbs*sizeof(MatScalar);
  ierr = PetscMalloc(nz,&il);CHKERRQ(ierr);
  jl   = il + mbs;
  rtmp = (MatScalar*)(jl + mbs);

  sctx.shift_amount = 0;
  sctx.nshift       = 0;
  do {
    sctx.chshift = PETSC_FALSE;
    for (i=0; i<mbs; i++) {
      rtmp[i] = 0.0; jl[i] = mbs; il[0] = 0;
    } 
 
    for (k = 0; k<mbs; k++){
      bval = ba + bi[k];
      /* initialize k-th row by the perm[k]-th row of A */
      jmin = ai[rip[k]]; jmax = ai[rip[k]+1];
      for (j = jmin; j < jmax; j++){
        col = riip[aj[j]];
        if (col >= k){ /* only take upper triangular entry */
          rtmp[col] = aa[j];
          *bval++  = 0.0; /* for in-place factorization */
        }
      } 
      /* shift the diagonal of the matrix */
      if (sctx.nshift) rtmp[k] += sctx.shift_amount; 

      /* modify k-th row by adding in those rows i with U(i,k)!=0 */
      dk = rtmp[k];
      i = jl[k]; /* first row to be added to k_th row  */  

      while (i < k){
        nexti = jl[i]; /* next row to be added to k_th row */

        /* compute multiplier, update diag(k) and U(i,k) */
        ili = il[i];  /* index of first nonzero element in U(i,k:bms-1) */
        uikdi = - ba[ili]*ba[bi[i]];  /* diagonal(k) */ 
        dk += uikdi*ba[ili];
        ba[ili] = uikdi; /* -U(i,k) */

        /* add multiple of row i to k-th row */
        jmin = ili + 1; jmax = bi[i+1];
        if (jmin < jmax){
          for (j=jmin; j<jmax; j++) rtmp[bj[j]] += uikdi*ba[j];         
          /* update il and jl for row i */
          il[i] = jmin;             
          j = bj[jmin]; jl[i] = jl[j]; jl[j] = i; 
        }      
        i = nexti;         
      }

      /* shift the diagonals when zero pivot is detected */
      /* compute rs=sum of abs(off-diagonal) */
      rs   = 0.0;
      jmin = bi[k]+1; 
      nz   = bi[k+1] - jmin; 
      bcol = bj + jmin;
      while (nz--){
        rs += PetscAbsScalar(rtmp[*bcol]);
        bcol++;
      }

      sctx.rs = rs;
      sctx.pv = dk;
      ierr = MatCholeskyCheckShift_inline(info,sctx,k,newshift);CHKERRQ(ierr); 

      if (newshift == 1) {
        if (!sctx.shift_amount) {
          sctx.shift_amount = 1e-5;
        }
        break;
      }
   
      /* copy data into U(k,:) */
      ba[bi[k]] = 1.0/dk; /* U(k,k) */
      jmin = bi[k]+1; jmax = bi[k+1];
      if (jmin < jmax) {
        for (j=jmin; j<jmax; j++){
          col = bj[j]; ba[j] = rtmp[col]; rtmp[col] = 0.0;
        }       
        /* add the k-th row into il and jl */
        il[k] = jmin;
        i = bj[jmin]; jl[k] = jl[i]; jl[i] = k;
      }        
    } 
  } while (sctx.chshift);
  ierr = PetscFree(il);CHKERRQ(ierr);

  ierr = ISRestoreIndices(ip,&rip);CHKERRQ(ierr);
  ierr = ISRestoreIndices(iip,&riip);CHKERRQ(ierr);

  ierr = ISIdentity(ip,&perm_identity);CHKERRQ(ierr);
  if (perm_identity){
    (B)->ops->solve           = MatSolve_SeqSBAIJ_1_NaturalOrdering;
    (B)->ops->solvetranspose  = MatSolve_SeqSBAIJ_1_NaturalOrdering;
    (B)->ops->forwardsolve    = MatForwardSolve_SeqSBAIJ_1_NaturalOrdering;
    (B)->ops->backwardsolve   = MatBackwardSolve_SeqSBAIJ_1_NaturalOrdering;
  } else {
    (B)->ops->solve           = MatSolve_SeqSBAIJ_1;
    (B)->ops->solvetranspose  = MatSolve_SeqSBAIJ_1;
    (B)->ops->forwardsolve    = MatForwardSolve_SeqSBAIJ_1;
    (B)->ops->backwardsolve   = MatBackwardSolve_SeqSBAIJ_1;
  }

  C->assembled    = PETSC_TRUE; 
  C->preallocated = PETSC_TRUE;
  ierr = PetscLogFlops(C->rmap->n);CHKERRQ(ierr);
  if (sctx.nshift){
    if (shiftnz) {
      ierr = PetscInfo2(A,"number of shiftnz tries %D, shift_amount %G\n",sctx.nshift,sctx.shift_amount);CHKERRQ(ierr);
    } else if (shiftpd) {
      ierr = PetscInfo2(A,"number of shiftpd tries %D, shift_amount %G\n",sctx.nshift,sctx.shift_amount);CHKERRQ(ierr);
    }
  }
  PetscFunctionReturn(0); 
}

#undef __FUNCT__  
#define __FUNCT__ "MatICCFactorSymbolic_SeqAIJ"
PetscErrorCode MatICCFactorSymbolic_SeqAIJ(Mat fact,Mat A,IS perm,const MatFactorInfo *info)
{
  Mat_SeqAIJ         *a = (Mat_SeqAIJ*)A->data;
  Mat_SeqSBAIJ       *b;
  PetscErrorCode     ierr;
  PetscTruth         perm_identity,missing;
  PetscInt           reallocs=0,i,*ai=a->i,*aj=a->j,am=A->rmap->n,*ui;
  const PetscInt     *rip,*riip;
  PetscInt           jmin,jmax,nzk,k,j,*jl,prow,*il,nextprow;
  PetscInt           nlnk,*lnk,*lnk_lvl=PETSC_NULL,d;
  PetscInt           ncols,ncols_upper,*cols,*ajtmp,*uj,**uj_ptr,**uj_lvl_ptr;
  PetscReal          fill=info->fill,levels=info->levels;
  PetscFreeSpaceList free_space=PETSC_NULL,current_space=PETSC_NULL;
  PetscFreeSpaceList free_space_lvl=PETSC_NULL,current_space_lvl=PETSC_NULL;
  PetscBT            lnkbt;
  IS                 iperm;  
  
  PetscFunctionBegin;   
  if (A->rmap->n != A->cmap->n) SETERRQ2(PETSC_ERR_ARG_WRONG,"Must be square matrix, rows %D columns %D",A->rmap->n,A->cmap->n);
  ierr = MatMissingDiagonal(A,&missing,&d);CHKERRQ(ierr);
  if (missing) SETERRQ1(PETSC_ERR_ARG_WRONGSTATE,"Matrix is missing diagonal entry %D",d);
  ierr = ISIdentity(perm,&perm_identity);CHKERRQ(ierr);
  ierr = ISInvertPermutation(perm,PETSC_DECIDE,&iperm);CHKERRQ(ierr);

  ierr = PetscMalloc((am+1)*sizeof(PetscInt),&ui);CHKERRQ(ierr); 
  ui[0] = 0;

  /* ICC(0) without matrix ordering: simply copies fill pattern */
  if (!levels && perm_identity) { 

    for (i=0; i<am; i++) {
      ui[i+1] = ui[i] + ai[i+1] - a->diag[i]; 
    }
    ierr = PetscMalloc((ui[am]+1)*sizeof(PetscInt),&uj);CHKERRQ(ierr); 
    cols = uj;
    for (i=0; i<am; i++) {
      aj    = a->j + a->diag[i];  
      ncols = ui[i+1] - ui[i];
      for (j=0; j<ncols; j++) *cols++ = *aj++; 
    }
  } else { /* case: levels>0 || (levels=0 && !perm_identity) */
    ierr = ISGetIndices(iperm,&riip);CHKERRQ(ierr);
    ierr = ISGetIndices(perm,&rip);CHKERRQ(ierr);

    /* initialization */
    ierr  = PetscMalloc((am+1)*sizeof(PetscInt),&ajtmp);CHKERRQ(ierr); 

    /* jl: linked list for storing indices of the pivot rows 
       il: il[i] points to the 1st nonzero entry of U(i,k:am-1) */
    ierr = PetscMalloc((2*am+1)*sizeof(PetscInt)+2*am*sizeof(PetscInt**),&jl);CHKERRQ(ierr); 
    il         = jl + am;
    uj_ptr     = (PetscInt**)(il + am);
    uj_lvl_ptr = (PetscInt**)(uj_ptr + am);
    for (i=0; i<am; i++){
      jl[i] = am; il[i] = 0;
    }

    /* create and initialize a linked list for storing column indices of the active row k */
    nlnk = am + 1;
    ierr = PetscIncompleteLLCreate(am,am,nlnk,lnk,lnk_lvl,lnkbt);CHKERRQ(ierr);

    /* initial FreeSpace size is fill*(ai[am]+1) */
    ierr = PetscFreeSpaceGet((PetscInt)(fill*(ai[am]+1)),&free_space);CHKERRQ(ierr);
    current_space = free_space;
    ierr = PetscFreeSpaceGet((PetscInt)(fill*(ai[am]+1)),&free_space_lvl);CHKERRQ(ierr);
    current_space_lvl = free_space_lvl;

    for (k=0; k<am; k++){  /* for each active row k */
      /* initialize lnk by the column indices of row rip[k] of A */
      nzk   = 0;
      ncols = ai[rip[k]+1] - ai[rip[k]]; 
      if (!ncols) SETERRQ2(PETSC_ERR_MAT_CH_ZRPVT,"Empty row in matrix: row in original ordering %D in permuted ordering %D",rip[k],k);
      ncols_upper = 0;
      for (j=0; j<ncols; j++){
        i = *(aj + ai[rip[k]] + j); /* unpermuted column index */
        if (riip[i] >= k){ /* only take upper triangular entry */
          ajtmp[ncols_upper] = i; 
          ncols_upper++;
        }
      }
      ierr = PetscIncompleteLLInit(ncols_upper,ajtmp,am,riip,nlnk,lnk,lnk_lvl,lnkbt);CHKERRQ(ierr);
      nzk += nlnk;

      /* update lnk by computing fill-in for each pivot row to be merged in */
      prow = jl[k]; /* 1st pivot row */
   
      while (prow < k){
        nextprow = jl[prow];
      
        /* merge prow into k-th row */
        jmin = il[prow] + 1;  /* index of the 2nd nzero entry in U(prow,k:am-1) */
        jmax = ui[prow+1]; 
        ncols = jmax-jmin;
        i     = jmin - ui[prow];
        cols  = uj_ptr[prow] + i; /* points to the 2nd nzero entry in U(prow,k:am-1) */
        uj    = uj_lvl_ptr[prow] + i; /* levels of cols */
        j     = *(uj - 1); 
        ierr = PetscICCLLAddSorted(ncols,cols,levels,uj,am,nlnk,lnk,lnk_lvl,lnkbt,j);CHKERRQ(ierr); 
        nzk += nlnk;

        /* update il and jl for prow */
        if (jmin < jmax){
          il[prow] = jmin;
          j = *cols; jl[prow] = jl[j]; jl[j] = prow;  
        } 
        prow = nextprow; 
      }  

      /* if free space is not available, make more free space */
      if (current_space->local_remaining<nzk) {
        i = am - k + 1; /* num of unfactored rows */
        i = PetscMin(i*nzk, i*(i-1)); /* i*nzk, i*(i-1): estimated and max additional space needed */
        ierr = PetscFreeSpaceGet(i,&current_space);CHKERRQ(ierr);
        ierr = PetscFreeSpaceGet(i,&current_space_lvl);CHKERRQ(ierr);
        reallocs++;
      }

      /* copy data into free_space and free_space_lvl, then initialize lnk */
      if (nzk == 0) SETERRQ1(PETSC_ERR_ARG_WRONG,"Empty row %D in ICC matrix factor",k);
      ierr = PetscIncompleteLLClean(am,am,nzk,lnk,lnk_lvl,current_space->array,current_space_lvl->array,lnkbt);CHKERRQ(ierr);

      /* add the k-th row into il and jl */
      if (nzk > 1){
        i = current_space->array[1]; /* col value of the first nonzero element in U(k, k+1:am-1) */    
        jl[k] = jl[i]; jl[i] = k;
        il[k] = ui[k] + 1;
      } 
      uj_ptr[k]     = current_space->array;
      uj_lvl_ptr[k] = current_space_lvl->array; 

      current_space->array           += nzk;
      current_space->local_used      += nzk;
      current_space->local_remaining -= nzk;

      current_space_lvl->array           += nzk;
      current_space_lvl->local_used      += nzk;
      current_space_lvl->local_remaining -= nzk;

      ui[k+1] = ui[k] + nzk;  
    } 

#if defined(PETSC_USE_INFO)
    if (ai[am] != 0) {
      PetscReal af = (PetscReal)ui[am]/((PetscReal)ai[am]);
      ierr = PetscInfo3(A,"Reallocs %D Fill ratio:given %G needed %G\n",reallocs,fill,af);CHKERRQ(ierr);
      ierr = PetscInfo1(A,"Run with -pc_factor_fill %G or use \n",af);CHKERRQ(ierr);
      ierr = PetscInfo1(A,"PCFactorSetFill(pc,%G) for best performance.\n",af);CHKERRQ(ierr);
    } else {
      ierr = PetscInfo(A,"Empty matrix.\n");CHKERRQ(ierr);
    }
#endif

    ierr = ISRestoreIndices(perm,&rip);CHKERRQ(ierr);
    ierr = ISRestoreIndices(iperm,&riip);CHKERRQ(ierr);
    ierr = PetscFree(jl);CHKERRQ(ierr);
    ierr = PetscFree(ajtmp);CHKERRQ(ierr);

    /* destroy list of free space and other temporary array(s) */
    ierr = PetscMalloc((ui[am]+1)*sizeof(PetscInt),&uj);CHKERRQ(ierr);
    ierr = PetscFreeSpaceContiguous(&free_space,uj);CHKERRQ(ierr);
    ierr = PetscIncompleteLLDestroy(lnk,lnkbt);CHKERRQ(ierr);
    ierr = PetscFreeSpaceDestroy(free_space_lvl);CHKERRQ(ierr);

  } /* end of case: levels>0 || (levels=0 && !perm_identity) */

  /* put together the new matrix in MATSEQSBAIJ format */

  b    = (Mat_SeqSBAIJ*)(fact)->data;
  b->singlemalloc = PETSC_FALSE;
  ierr = PetscMalloc((ui[am]+1)*sizeof(MatScalar),&b->a);CHKERRQ(ierr);
  b->j    = uj;
  b->i    = ui;
  b->diag = 0;
  b->ilen = 0;
  b->imax = 0;
  b->row  = perm;
  b->col  = perm;
  ierr    = PetscObjectReference((PetscObject)perm);CHKERRQ(ierr); 
  ierr    = PetscObjectReference((PetscObject)perm);CHKERRQ(ierr); 
  b->icol = iperm;
  b->pivotinblocks = PETSC_FALSE; /* need to get from MatFactorInfo */
  ierr    = PetscMalloc((am+1)*sizeof(PetscScalar),&b->solve_work);CHKERRQ(ierr);
  ierr = PetscLogObjectMemory((fact),(ui[am]-am)*(sizeof(PetscInt)+sizeof(MatScalar)));CHKERRQ(ierr);
  b->maxnz   = b->nz = ui[am];
  b->free_a  = PETSC_TRUE; 
  b->free_ij = PETSC_TRUE; 
  
  (fact)->info.factor_mallocs    = reallocs;
  (fact)->info.fill_ratio_given  = fill;
  if (ai[am] != 0) {
    (fact)->info.fill_ratio_needed = ((PetscReal)ui[am])/((PetscReal)ai[am]);
  } else {
    (fact)->info.fill_ratio_needed = 0.0;
  }
  (fact)->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqAIJ;
  PetscFunctionReturn(0); 
}

#undef __FUNCT__  
#define __FUNCT__ "MatCholeskyFactorSymbolic_SeqAIJ"
PetscErrorCode MatCholeskyFactorSymbolic_SeqAIJ(Mat fact,Mat A,IS perm,const MatFactorInfo *info)
{
  Mat_SeqAIJ         *a = (Mat_SeqAIJ*)A->data;
  Mat_SeqSBAIJ       *b;
  PetscErrorCode     ierr;
  PetscTruth         perm_identity;
  PetscReal          fill = info->fill;
  const PetscInt     *rip,*riip;
  PetscInt           i,am=A->rmap->n,*ai=a->i,*aj=a->j,reallocs=0,prow;
  PetscInt           *jl,jmin,jmax,nzk,*ui,k,j,*il,nextprow;
  PetscInt           nlnk,*lnk,ncols,ncols_upper,*cols,*uj,**ui_ptr,*uj_ptr;
  PetscFreeSpaceList free_space=PETSC_NULL,current_space=PETSC_NULL;
  PetscBT            lnkbt;
  IS                 iperm;  

  PetscFunctionBegin;
  if (A->rmap->n != A->cmap->n) SETERRQ2(PETSC_ERR_ARG_WRONG,"Must be square matrix, rows %D columns %D",A->rmap->n,A->cmap->n);
  /* check whether perm is the identity mapping */
  ierr = ISIdentity(perm,&perm_identity);CHKERRQ(ierr);  
  ierr = ISInvertPermutation(perm,PETSC_DECIDE,&iperm);CHKERRQ(ierr);
  ierr = ISGetIndices(iperm,&riip);CHKERRQ(ierr);  
  ierr = ISGetIndices(perm,&rip);CHKERRQ(ierr);

  /* initialization */
  ierr  = PetscMalloc((am+1)*sizeof(PetscInt),&ui);CHKERRQ(ierr);
  ui[0] = 0; 

  /* jl: linked list for storing indices of the pivot rows 
     il: il[i] points to the 1st nonzero entry of U(i,k:am-1) */
  ierr = PetscMalloc((3*am+1)*sizeof(PetscInt)+am*sizeof(PetscInt**),&jl);CHKERRQ(ierr); 
  il     = jl + am;
  cols   = il + am;
  ui_ptr = (PetscInt**)(cols + am);
  for (i=0; i<am; i++){
    jl[i] = am; il[i] = 0;
  }

  /* create and initialize a linked list for storing column indices of the active row k */
  nlnk = am + 1;
  ierr = PetscLLCreate(am,am,nlnk,lnk,lnkbt);CHKERRQ(ierr);

  /* initial FreeSpace size is fill*(ai[am]+1) */
  ierr = PetscFreeSpaceGet((PetscInt)(fill*(ai[am]+1)),&free_space);CHKERRQ(ierr);
  current_space = free_space;

  for (k=0; k<am; k++){  /* for each active row k */
    /* initialize lnk by the column indices of row rip[k] of A */
    nzk   = 0;
    ncols = ai[rip[k]+1] - ai[rip[k]]; 
    if (!ncols) SETERRQ2(PETSC_ERR_MAT_CH_ZRPVT,"Empty row in matrix: row in original ordering %D in permuted ordering %D",rip[k],k);
    ncols_upper = 0;
    for (j=0; j<ncols; j++){
      i = riip[*(aj + ai[rip[k]] + j)];  
      if (i >= k){ /* only take upper triangular entry */
        cols[ncols_upper] = i;
        ncols_upper++;
      }
    }
    ierr = PetscLLAdd(ncols_upper,cols,am,nlnk,lnk,lnkbt);CHKERRQ(ierr);
    nzk += nlnk;

    /* update lnk by computing fill-in for each pivot row to be merged in */
    prow = jl[k]; /* 1st pivot row */
   
    while (prow < k){
      nextprow = jl[prow];
      /* merge prow into k-th row */
      jmin = il[prow] + 1;  /* index of the 2nd nzero entry in U(prow,k:am-1) */
      jmax = ui[prow+1]; 
      ncols = jmax-jmin;
      uj_ptr = ui_ptr[prow] + jmin - ui[prow]; /* points to the 2nd nzero entry in U(prow,k:am-1) */
      ierr = PetscLLAddSorted(ncols,uj_ptr,am,nlnk,lnk,lnkbt);CHKERRQ(ierr); 
      nzk += nlnk;

      /* update il and jl for prow */
      if (jmin < jmax){
        il[prow] = jmin;
        j = *uj_ptr; jl[prow] = jl[j]; jl[j] = prow;  
      } 
      prow = nextprow; 
    }  

    /* if free space is not available, make more free space */
    if (current_space->local_remaining<nzk) {
      i = am - k + 1; /* num of unfactored rows */
      i = PetscMin(i*nzk, i*(i-1)); /* i*nzk, i*(i-1): estimated and max additional space needed */
      ierr = PetscFreeSpaceGet(i,&current_space);CHKERRQ(ierr);
      reallocs++;
    }

    /* copy data into free space, then initialize lnk */
    ierr = PetscLLClean(am,am,nzk,lnk,current_space->array,lnkbt);CHKERRQ(ierr); 

    /* add the k-th row into il and jl */
    if (nzk-1 > 0){
      i = current_space->array[1]; /* col value of the first nonzero element in U(k, k+1:am-1) */    
      jl[k] = jl[i]; jl[i] = k;
      il[k] = ui[k] + 1;
    } 
    ui_ptr[k] = current_space->array;
    current_space->array           += nzk;
    current_space->local_used      += nzk;
    current_space->local_remaining -= nzk;

    ui[k+1] = ui[k] + nzk;  
  } 

#if defined(PETSC_USE_INFO)
  if (ai[am] != 0) {
    PetscReal af = (PetscReal)(ui[am])/((PetscReal)ai[am]);
    ierr = PetscInfo3(A,"Reallocs %D Fill ratio:given %G needed %G\n",reallocs,fill,af);CHKERRQ(ierr);
    ierr = PetscInfo1(A,"Run with -pc_factor_fill %G or use \n",af);CHKERRQ(ierr);
    ierr = PetscInfo1(A,"PCFactorSetFill(pc,%G) for best performance.\n",af);CHKERRQ(ierr);
  } else {
     ierr = PetscInfo(A,"Empty matrix.\n");CHKERRQ(ierr);
  }
#endif

  ierr = ISRestoreIndices(perm,&rip);CHKERRQ(ierr);
  ierr = ISRestoreIndices(iperm,&riip);CHKERRQ(ierr);
  ierr = PetscFree(jl);CHKERRQ(ierr);

  /* destroy list of free space and other temporary array(s) */
  ierr = PetscMalloc((ui[am]+1)*sizeof(PetscInt),&uj);CHKERRQ(ierr);
  ierr = PetscFreeSpaceContiguous(&free_space,uj);CHKERRQ(ierr);
  ierr = PetscLLDestroy(lnk,lnkbt);CHKERRQ(ierr);

  /* put together the new matrix in MATSEQSBAIJ format */

  b = (Mat_SeqSBAIJ*)(fact)->data;
  b->singlemalloc = PETSC_FALSE;
  b->free_a       = PETSC_TRUE;
  b->free_ij      = PETSC_TRUE;
  ierr = PetscMalloc((ui[am]+1)*sizeof(MatScalar),&b->a);CHKERRQ(ierr);
  b->j    = uj;
  b->i    = ui;
  b->diag = 0;
  b->ilen = 0;
  b->imax = 0;
  b->row  = perm;
  b->col  = perm;
  ierr    = PetscObjectReference((PetscObject)perm);CHKERRQ(ierr); 
  ierr    = PetscObjectReference((PetscObject)perm);CHKERRQ(ierr); 
  b->icol = iperm;
  b->pivotinblocks = PETSC_FALSE; /* need to get from MatFactorInfo */
  ierr    = PetscMalloc((am+1)*sizeof(PetscScalar),&b->solve_work);CHKERRQ(ierr);
  ierr    = PetscLogObjectMemory(fact,(ui[am]-am)*(sizeof(PetscInt)+sizeof(MatScalar)));CHKERRQ(ierr);
  b->maxnz = b->nz = ui[am];
  
  (fact)->info.factor_mallocs    = reallocs;
  (fact)->info.fill_ratio_given  = fill;
  if (ai[am] != 0) {
    (fact)->info.fill_ratio_needed = ((PetscReal)ui[am])/((PetscReal)ai[am]);
  } else {
    (fact)->info.fill_ratio_needed = 0.0;
  }
  (fact)->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqAIJ;
  PetscFunctionReturn(0); 
}

#undef __FUNCT__
#define __FUNCT__ "MatSolve_SeqAIJ_NaturalOrdering_iludt"
PetscErrorCode MatSolve_SeqAIJ_NaturalOrdering_iludt(Mat A,Vec bb,Vec xx)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)A->data;
  PetscErrorCode    ierr;
  PetscInt          n = A->rmap->n;
  const PetscInt    *ai = a->i,*aj = a->j,*adiag = a->diag,*vi;
  PetscScalar       *x,sum;
  const PetscScalar *b;
  const MatScalar   *aa = a->a,*v;
  PetscInt          i,nz;

  PetscFunctionBegin;
  if (!n) PetscFunctionReturn(0);

  ierr = VecGetArray(bb,(PetscScalar**)&b);CHKERRQ(ierr);
  ierr = VecGetArray(xx,&x);CHKERRQ(ierr);

  /* forward solve the lower triangular */
  x[0] = b[0];
  v    = aa;
  vi   = aj;
  for (i=1; i<n; i++) {
    nz  = ai[i+1] - ai[i];
    sum = b[i];
    PetscSparseDenseMinusDot(sum,x,v,vi,nz);
    /*    while (nz--) sum -= *v++ * x[*vi++];*/
    v  += nz;
    vi += nz;
    x[i] = sum;
  }

  /* backward solve the upper triangular */
  v   = aa + adiag[n] + 1;
  vi  = aj + adiag[n] + 1;
  for (i=n-1; i>=0; i--){
    nz  = adiag[i] - adiag[i+1] - 1; 
    sum = x[i];
    PetscSparseDenseMinusDot(sum,x,v,vi,nz);
    /* while (nz--) sum -= *v++ * x[*vi++]; */
    v   += nz;
    vi  += nz;
    x[i] = sum*aa[adiag[i]];
    v++; vi++;
  }

  ierr = PetscLogFlops(2.0*a->nz - A->cmap->n);CHKERRQ(ierr);
  ierr = VecRestoreArray(bb,(PetscScalar**)&b);CHKERRQ(ierr);
  ierr = VecRestoreArray(xx,&x);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatSolve_SeqAIJ_iludt"
PetscErrorCode MatSolve_SeqAIJ_iludt(Mat A,Vec bb,Vec xx)
{
  Mat_SeqAIJ        *a = (Mat_SeqAIJ*)A->data;
  IS                iscol = a->col,isrow = a->row;
  PetscErrorCode    ierr;
  PetscInt          i,n=A->rmap->n,*vi,*ai = a->i,*aj = a->j,*adiag=a->diag;
  PetscInt          nz;
  const PetscInt    *rout,*cout,*r,*c;
  PetscScalar       *x,*tmp,*tmps;
  const PetscScalar *b;
  const MatScalar   *aa = a->a,*v;

  PetscFunctionBegin;
  if (!n) PetscFunctionReturn(0);

  ierr = VecGetArray(bb,(PetscScalar**)&b);CHKERRQ(ierr); 
  ierr = VecGetArray(xx,&x);CHKERRQ(ierr);
  tmp  = a->solve_work;

  ierr = ISGetIndices(isrow,&rout);CHKERRQ(ierr); r = rout;
  ierr = ISGetIndices(iscol,&cout);CHKERRQ(ierr); c = cout + (n-1);

  /* forward solve the lower triangular */
  tmp[0] = b[*r++];
  tmps   = tmp;
  v      = aa;
  vi     = aj;
  for (i=1; i<n; i++) {
    nz  = ai[i+1] - ai[i];
    tmp[i] = b[*r++];
    PetscSparseDenseMinusDot(tmp[i],tmps,v,vi,nz); 
    v += nz; vi += nz;
  }

  /* backward solve the upper triangular */
  v   = aa + adiag[n] + 1;
  vi  = aj + adiag[n] + 1;
  for (i=n-1; i>=0; i--){
    nz  = adiag[i] - adiag[i+1] - 1; 
    PetscSparseDenseMinusDot(tmp[i],tmps,v,vi,nz); 
    x[*c--] = tmp[i] = tmp[i]*aa[adiag[i]];
    v += nz+1; vi += nz+1;
  }

  ierr = ISRestoreIndices(isrow,&rout);CHKERRQ(ierr);
  ierr = ISRestoreIndices(iscol,&cout);CHKERRQ(ierr);
  ierr = VecRestoreArray(bb,(PetscScalar**)&b);CHKERRQ(ierr); 
  ierr = VecRestoreArray(xx,&x);CHKERRQ(ierr);
  ierr = PetscLogFlops(2*a->nz - A->cmap->n);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatILUDTFactor_SeqAIJ"
PetscErrorCode MatILUDTFactor_SeqAIJ(Mat A,IS isrow,IS iscol,const MatFactorInfo *info,Mat *fact)
{
  Mat                B = *fact;
  Mat_SeqAIJ         *a=(Mat_SeqAIJ*)A->data,*b;
  IS                 isicol;  
  PetscErrorCode     ierr;
  const PetscInt     *r,*ic;
  PetscInt           i,n=A->rmap->n,*ai=a->i,*aj=a->j,*ajtmp,*adiag;
  PetscInt           *bi,*bj,*bdiag;
  PetscInt           row,nzi,nzi_bl,nzi_bu,*im,dtcount,nzi_al,nzi_au;
  PetscInt           nlnk,*lnk;
  PetscBT            lnkbt;
  PetscTruth         row_identity,icol_identity,both_identity;
  MatScalar          *aatmp,*pv,*batmp,*ba,*rtmp,*pc,multiplier,*vtmp,diag_tmp;
  const PetscInt     *ics;
  PetscInt           j,nz,*pj,*bjtmp,k,ncut,*jtmp;
  PetscReal          dt=info->dt,shift=info->shiftinblocks; 
  PetscInt           nnz_max;
  PetscTruth         missing;

  PetscFunctionBegin;
  /* printf("MatILUDTFactor_SeqAIJ is callled ...\n"); */
  /* ------- symbolic factorization, can be reused ---------*/
  ierr = MatMissingDiagonal(A,&missing,&i);CHKERRQ(ierr);
  if (missing) SETERRQ1(PETSC_ERR_ARG_WRONGSTATE,"Matrix is missing diagonal entry %D",i);
  adiag=a->diag;

  ierr = ISInvertPermutation(iscol,PETSC_DECIDE,&isicol);CHKERRQ(ierr);

  /* bdiag is location of diagonal in factor */
  ierr = PetscMalloc((n+1)*sizeof(PetscInt),&bdiag);CHKERRQ(ierr);

  /* allocate row pointers bi */
  ierr = PetscMalloc((n+1)*sizeof(PetscInt),&bi);CHKERRQ(ierr);

  /* allocate bj and ba; max num of nonzero entries is (ai[n]+2*n*dtcount+2) */
  dtcount = (PetscInt)info->dtcount;
  if (dtcount > n/2) dtcount = n/2;
  nnz_max  = ai[n]+2*n*dtcount+2;
  ierr = PetscMalloc(nnz_max*sizeof(PetscInt),&bj);CHKERRQ(ierr);
  ierr = PetscMalloc(nnz_max*sizeof(MatScalar),&ba);CHKERRQ(ierr);

  /* put together the new matrix */
  ierr = MatSeqAIJSetPreallocation_SeqAIJ(B,MAT_SKIP_ALLOCATION,PETSC_NULL);CHKERRQ(ierr);
  ierr = PetscLogObjectParent(B,isicol);CHKERRQ(ierr);
  b    = (Mat_SeqAIJ*)(B)->data;
  b->free_a       = PETSC_TRUE;
  b->free_ij      = PETSC_TRUE;
  b->singlemalloc = PETSC_FALSE;
  b->a          = ba;
  b->j          = bj; 
  b->i          = bi;
  b->diag       = bdiag;
  b->ilen       = 0;
  b->imax       = 0;
  b->row        = isrow;
  b->col        = iscol;
  ierr          = PetscObjectReference((PetscObject)isrow);CHKERRQ(ierr);
  ierr          = PetscObjectReference((PetscObject)iscol);CHKERRQ(ierr);
  b->icol       = isicol;
  ierr = PetscMalloc((n+1)*sizeof(PetscScalar),&b->solve_work);CHKERRQ(ierr);

  ierr = PetscLogObjectMemory(B,nnz_max*(sizeof(PetscInt)+sizeof(MatScalar)));CHKERRQ(ierr);
  b->maxnz = nnz_max;

  (B)->factor                = MAT_FACTOR_ILUDT;
  (B)->info.factor_mallocs   = 0;
  (B)->info.fill_ratio_given = ((PetscReal)nnz_max)/((PetscReal)ai[n]);
  CHKMEMQ;
  /* ------- end of symbolic factorization ---------*/

  ierr = ISGetIndices(isrow,&r);CHKERRQ(ierr);
  ierr = ISGetIndices(isicol,&ic);CHKERRQ(ierr);
  ics  = ic;

  /* linked list for storing column indices of the active row */
  nlnk = n + 1;
  ierr = PetscLLCreate(n,n,nlnk,lnk,lnkbt);CHKERRQ(ierr);

  /* im: used by PetscLLAddSortedLU(); jtmp: working array for column indices of active row */
  ierr = PetscMalloc((2*n+1)*sizeof(PetscInt),&im);CHKERRQ(ierr);
  jtmp = im + n;
  /* rtmp, vtmp: working arrays for sparse and contiguous row entries of active row */
  ierr = PetscMalloc((2*n+1)*sizeof(MatScalar),&rtmp);CHKERRQ(ierr);
  ierr = PetscMemzero(rtmp,(n+1)*sizeof(PetscScalar));CHKERRQ(ierr); 
  vtmp = rtmp + n;

  bi[0]    = 0;
  bdiag[0] = nnz_max-1; /* location of diagonal in factor B */
  for (i=0; i<n; i++) {
    /* copy initial fill into linked list */
    nzi = 0; /* nonzeros for active row i */
    nzi = ai[r[i]+1] - ai[r[i]];
    if (!nzi) SETERRQ2(PETSC_ERR_MAT_LU_ZRPVT,"Empty row in matrix: row in original ordering %D in permuted ordering %D",r[i],i);
    nzi_al = adiag[r[i]] - ai[r[i]];
    nzi_au = ai[r[i]+1] - adiag[r[i]] -1;
    ajtmp = aj + ai[r[i]]; 
    ierr = PetscLLAddPerm(nzi,ajtmp,ic,n,nlnk,lnk,lnkbt);CHKERRQ(ierr);
    
    /* load in initial (unfactored row) */
    aatmp = a->a + ai[r[i]];
    for (j=0; j<nzi; j++) {
      rtmp[ics[*ajtmp++]] = *aatmp++;
    }
    
    /* add pivot rows into linked list */
    row = lnk[n]; 
    while (row < i) {
      nzi_bl = bi[row+1] - bi[row] + 1;
      bjtmp = bj + bdiag[row+1]+1; /* points to 1st column next to the diagonal in U */
      ierr  = PetscLLAddSortedLU(bjtmp,row,nlnk,lnk,lnkbt,i,nzi_bl,im);CHKERRQ(ierr);
      nzi  += nlnk;
      row   = lnk[row];
    }
    
    /* copy data from lnk into jtmp, then initialize lnk */
    ierr = PetscLLClean(n,n,nzi,lnk,jtmp,lnkbt);CHKERRQ(ierr);

    /* numerical factorization */
    bjtmp = jtmp;
    row   = *bjtmp++; /* 1st pivot row */
    while  (row < i) {
      pc         = rtmp + row;
      pv         = ba + bdiag[row]; /* 1./(diag of the pivot row) */
      multiplier = (*pc) * (*pv); 
      *pc        = multiplier; 
      if (PetscAbsScalar(*pc) > dt){ /* apply tolerance dropping rule */
        pj         = bj + bdiag[row+1] + 1; /* point to 1st entry of U(row,:) */
        pv         = ba + bdiag[row+1] + 1;
        /* if (multiplier < -1.0 or multiplier >1.0) printf("row/prow %d, %d, multiplier %g\n",i,row,multiplier); */
        nz         = bdiag[row] - bdiag[row+1] - 1; /* num of entries in U(row,:), excluding diagonal */
        for (j=0; j<nz; j++) rtmp[*pj++] -= multiplier * (*pv++);
        ierr = PetscLogFlops(2.0*nz);CHKERRQ(ierr);
      } 
      row = *bjtmp++;
    }

    /* copy sparse rtmp into contiguous vtmp; separate L and U part */
    diag_tmp = rtmp[i];  /* save diagonal value - may not needed?? */
    nzi_bl = 0; j = 0;
    while (jtmp[j] < i){ /* Note: jtmp is sorted */
      vtmp[j] = rtmp[jtmp[j]]; rtmp[jtmp[j]]=0.0;
      nzi_bl++; j++;
    }
    nzi_bu = nzi - nzi_bl -1;
    while (j < nzi){
      vtmp[j] = rtmp[jtmp[j]]; rtmp[jtmp[j]]=0.0;
      j++;
    }
    
    bjtmp = bj + bi[i];
    batmp = ba + bi[i];
    /* apply level dropping rule to L part */
    ncut = nzi_al + dtcount; 
    if (ncut < nzi_bl){ 
      ierr = PetscSortSplit(ncut,nzi_bl,vtmp,jtmp);CHKERRQ(ierr);
      ierr = PetscSortIntWithScalarArray(ncut,jtmp,vtmp);CHKERRQ(ierr);
    } else {
      ncut = nzi_bl;
    }
    for (j=0; j<ncut; j++){
      bjtmp[j] = jtmp[j];
      batmp[j] = vtmp[j];
      /* printf(" (%d,%g),",bjtmp[j],batmp[j]); */
    }
    bi[i+1] = bi[i] + ncut;
    nzi = ncut + 1;
      
    /* apply level dropping rule to U part */
    ncut = nzi_au + dtcount; 
    if (ncut < nzi_bu){
      ierr = PetscSortSplit(ncut,nzi_bu,vtmp+nzi_bl+1,jtmp+nzi_bl+1);CHKERRQ(ierr);
      ierr = PetscSortIntWithScalarArray(ncut,jtmp+nzi_bl+1,vtmp+nzi_bl+1);CHKERRQ(ierr);
    } else {
      ncut = nzi_bu;
    }
    nzi += ncut;

    /* mark bdiagonal */
    bdiag[i+1]   = bdiag[i] - (ncut + 1);
    bjtmp = bj + bdiag[i];
    batmp = ba + bdiag[i];
    *bjtmp = i; 
    *batmp = diag_tmp; /* rtmp[i]; */
    if (*batmp == 0.0) *batmp = dt+shift;
    *batmp = 1.0/(*batmp); /* invert diagonal entries for simplier triangular solves */
    /* printf(" (%d,%g),",*bjtmp,*batmp); */
    
    bjtmp = bj + bdiag[i+1]+1;
    batmp = ba + bdiag[i+1]+1;
    for (k=0; k<ncut; k++){
      bjtmp[k] = jtmp[nzi_bl+1+k];
      batmp[k] = vtmp[nzi_bl+1+k];
      /* printf(" (%d,%g),",bjtmp[k],batmp[k]); */
    }
    /* printf("\n"); */
      
    im[i]   = nzi; /* used by PetscLLAddSortedLU() */
    /*
    printf("row %d: bi %d, bdiag %d\n",i,bi[i],bdiag[i]);
    printf(" ----------------------------\n");
    */
  } /* for (i=0; i<n; i++) */
  /* printf("end of L %d, beginning of U %d\n",bi[n],bdiag[n]); */
  if (bi[n] >= bdiag[n]) SETERRQ2(PETSC_ERR_ARG_SIZ,"end of L array %d cannot >= the beginning of U array %d",bi[n],bdiag[n]);

  ierr = ISRestoreIndices(isrow,&r);CHKERRQ(ierr);
  ierr = ISRestoreIndices(isicol,&ic);CHKERRQ(ierr);

  ierr = PetscLLDestroy(lnk,lnkbt);CHKERRQ(ierr);
  ierr = PetscFree(im);CHKERRQ(ierr);
  ierr = PetscFree(rtmp);CHKERRQ(ierr);

  ierr = PetscLogFlops(B->cmap->n);CHKERRQ(ierr);
  b->maxnz = b->nz = bi[n] + bdiag[0] - bdiag[n]; 

  ierr = ISIdentity(isrow,&row_identity);CHKERRQ(ierr);
  ierr = ISIdentity(isicol,&icol_identity);CHKERRQ(ierr);
  both_identity = (PetscTruth) (row_identity && icol_identity);
  if (row_identity && icol_identity) {
    B->ops->solve = MatSolve_SeqAIJ_NaturalOrdering_iludt;
  } else {
    B->ops->solve = MatSolve_SeqAIJ_iludt;
  }
  
  B->ops->lufactorsymbolic  = MatILUDTFactorSymbolic_SeqAIJ;
  B->ops->lufactornumeric   = MatILUDTFactorNumeric_SeqAIJ;
  B->ops->solveadd          = 0;
  B->ops->solvetranspose    = 0;
  B->ops->solvetransposeadd = 0;
  B->ops->matsolve          = 0;
  B->assembled              = PETSC_TRUE;
  B->preallocated           = PETSC_TRUE;
  PetscFunctionReturn(0); 
}

/* a wraper of MatILUDTFactor_SeqAIJ() */
#undef __FUNCT__  
#define __FUNCT__ "MatILUDTFactorSymbolic_SeqAIJ"
PetscErrorCode PETSCMAT_DLLEXPORT MatILUDTFactorSymbolic_SeqAIJ(Mat fact,Mat A,IS row,IS col,const MatFactorInfo *info)
{
  PetscErrorCode     ierr;

  PetscFunctionBegin;
  /* printf("MatILUDTFactorSymbolic_SeqAIJ is called...\n"); */
  ierr = MatILUDTFactor_SeqAIJ(A,row,col,info,&fact);CHKERRQ(ierr);

  fact->ops->lufactornumeric = MatILUDTFactorNumeric_SeqAIJ;
  PetscFunctionReturn(0); 
}

/* 
   same as MatLUFactorNumeric_SeqAIJ(), except using contiguous array matrix factors 
   - intend to replace existing MatLUFactorNumeric_SeqAIJ() 
*/
#undef __FUNCT__  
#define __FUNCT__ "MatILUDTFactorNumeric_SeqAIJ"
PetscErrorCode PETSCMAT_DLLEXPORT MatILUDTFactorNumeric_SeqAIJ(Mat fact,Mat A,const MatFactorInfo *info)
{
  Mat            C=fact;
  Mat_SeqAIJ     *a=(Mat_SeqAIJ*)A->data,*b=(Mat_SeqAIJ *)C->data;
  IS             isrow = b->row,isicol = b->icol;
  PetscErrorCode ierr;
  const PetscInt *r,*ic,*ics;
  PetscInt       i,j,k,n=A->rmap->n,*ai=a->i,*aj=a->j,*bi=b->i,*bj=b->j;
  PetscInt       *ajtmp,*bjtmp,nz,nzl,nzu,row,*bdiag = b->diag,*pj;
  MatScalar      *rtmp,*pc,multiplier,*v,*pv,*aa=a->a;
  PetscReal      dt=info->dt,shift=info->shiftinblocks;
  PetscTruth     row_identity, col_identity;

  PetscFunctionBegin;
  /* printf("MatILUDTFactorNumeric_SeqAIJ is called...\n"); */
  ierr = ISGetIndices(isrow,&r);CHKERRQ(ierr);
  ierr = ISGetIndices(isicol,&ic);CHKERRQ(ierr);
  ierr = PetscMalloc((n+1)*sizeof(MatScalar),&rtmp);CHKERRQ(ierr);
  ics  = ic;

  for (i=0; i<n; i++){     
    /* initialize rtmp array */
    nzl   = bi[i+1] - bi[i];       /* num of nozeros in L(i,:) */
    bjtmp = bj + bi[i];
    for  (j=0; j<nzl; j++) rtmp[*bjtmp++] = 0.0;
    rtmp[i] = 0.0; 
    nzu   = bdiag[i] - bdiag[i+1]; /* num of nozeros in U(i,:) */
    bjtmp = bj + bdiag[i+1] + 1;
    for  (j=0; j<nzu; j++) rtmp[*bjtmp++] = 0.0;     

    /* load in initial unfactored row of A */
    /* printf("row %d\n",i); */
    nz    = ai[r[i]+1] - ai[r[i]];
    ajtmp = aj + ai[r[i]];
    v     = aa + ai[r[i]];
    for (j=0; j<nz; j++) {
      rtmp[ics[*ajtmp++]] = v[j];
      /* printf(" (%d,%g),",ics[ajtmp[j]],rtmp[ics[ajtmp[j]]]); */
    }
    /* printf("\n"); */

    /* numerical factorization */
    bjtmp = bj + bi[i]; /* point to 1st entry of L(i,:) */
    nzl   = bi[i+1] - bi[i]; /* num of entries in L(i,:) */   
    k = 0;
    while (k < nzl){
      row   = *bjtmp++;
      /* printf("  prow %d\n",row); */
      pc         = rtmp + row;
      pv         = b->a + bdiag[row]; /* 1./(diag of the pivot row) */
      multiplier = (*pc) * (*pv); 
      *pc        = multiplier; 
      if (PetscAbsScalar(multiplier) > dt){ 
        pj         = bj + bdiag[row+1] + 1; /* point to 1st entry of U(row,:) */
        pv         = b->a + bdiag[row+1] + 1;
        nz         = bdiag[row] - bdiag[row+1] - 1; /* num of entries in U(row,:), excluding diagonal */
        for (j=0; j<nz; j++) rtmp[*pj++] -= multiplier * (*pv++);
        /* ierr = PetscLogFlops(2.0*nz);CHKERRQ(ierr); */
      } 
      k++;
    }
    
    /* finished row so stick it into b->a */
    /* L-part */
    pv = b->a + bi[i] ;
    pj = bj + bi[i] ;
    nzl = bi[i+1] - bi[i];
    for (j=0; j<nzl; j++) {
      pv[j] = rtmp[pj[j]];
      /* printf(" (%d,%g),",pj[j],pv[j]); */
    }

    /* diagonal: invert diagonal entries for simplier triangular solves */
    if (rtmp[i] == 0.0) rtmp[i] = dt+shift;
    b->a[bdiag[i]] = 1.0/rtmp[i];
    /* printf(" (%d,%g),",i,b->a[bdiag[i]]); */

    /* U-part */
    pv = b->a + bdiag[i+1] + 1;
    pj = bj + bdiag[i+1] + 1;
    nzu = bdiag[i] - bdiag[i+1] - 1;
    for (j=0; j<nzu; j++) {
      pv[j] = rtmp[pj[j]];
      /* printf(" (%d,%g),",pj[j],pv[j]); */
    }
    /* printf("\n"); */
  } 

  ierr = PetscFree(rtmp);CHKERRQ(ierr);
  ierr = ISRestoreIndices(isicol,&ic);CHKERRQ(ierr);
  ierr = ISRestoreIndices(isrow,&r);CHKERRQ(ierr);
  
  ierr = ISIdentity(isrow,&row_identity);CHKERRQ(ierr);
  ierr = ISIdentity(isicol,&col_identity);CHKERRQ(ierr);
  if (row_identity && col_identity) {
    C->ops->solve   = MatSolve_SeqAIJ_NaturalOrdering_iludt;
  } else {
    C->ops->solve   = MatSolve_SeqAIJ_iludt;
  }
  C->ops->solveadd           = 0;
  C->ops->solvetranspose     = 0;
  C->ops->solvetransposeadd  = 0;
  C->ops->matsolve           = 0;
  C->assembled    = PETSC_TRUE;
  C->preallocated = PETSC_TRUE;
  ierr = PetscLogFlops(C->cmap->n);CHKERRQ(ierr);
  PetscFunctionReturn(0); 
}
