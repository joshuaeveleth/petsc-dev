
/*
    Defines the basic matrix operations for the SBAIJ (compressed row)
  matrix storage format.
*/
#include "src/mat/impls/baij/seq/baij.h"         /*I "petscmat.h" I*/
#include "src/inline/spops.h"
#include "src/mat/impls/sbaij/seq/sbaij.h"

#define CHUNKSIZE  10

/*
     Checks for missing diagonals
*/
#undef __FUNCT__  
#define __FUNCT__ "MatMissingDiagonal_SeqSBAIJ"
int MatMissingDiagonal_SeqSBAIJ(Mat A)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data; 
  int         *diag,*jj = a->j,i,ierr;

  PetscFunctionBegin;
  ierr = MatMarkDiagonal_SeqSBAIJ(A);CHKERRQ(ierr);
  diag = a->diag;
  for (i=0; i<a->mbs; i++) {
    if (jj[diag[i]] != i) SETERRQ1(1,"Matrix is missing diagonal number %d",i);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatMarkDiagonal_SeqSBAIJ"
int MatMarkDiagonal_SeqSBAIJ(Mat A)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data; 
  int          i,mbs = a->mbs,ierr;

  PetscFunctionBegin;
  if (a->diag) PetscFunctionReturn(0);

  ierr = PetscMalloc((mbs+1)*sizeof(int),&a->diag);CHKERRQ(ierr); 
  PetscLogObjectMemory(A,(mbs+1)*sizeof(int));
  for (i=0; i<mbs; i++) a->diag[i] = a->i[i];  
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatGetRowIJ_SeqSBAIJ"
static int MatGetRowIJ_SeqSBAIJ(Mat A,int oshift,PetscTruth symmetric,int *nn,int *ia[],int *ja[],PetscTruth *done)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;
  int         n = a->mbs,i;

  PetscFunctionBegin;
  *nn = n;
  if (!ia) PetscFunctionReturn(0);

  if (oshift == 1) {
    /* temporarily add 1 to i and j indices */
    int nz = a->i[n]; 
    for (i=0; i<nz; i++) a->j[i]++;
    for (i=0; i<n+1; i++) a->i[i]++;
    *ia = a->i; *ja = a->j;
  } else {
    *ia = a->i; *ja = a->j;
  }
  PetscFunctionReturn(0); 
}

#undef __FUNCT__  
#define __FUNCT__ "MatRestoreRowIJ_SeqSBAIJ" 
static int MatRestoreRowIJ_SeqSBAIJ(Mat A,int oshift,PetscTruth symmetric,int *nn,int *ia[],int *ja[],PetscTruth *done)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;
  int         i,n = a->mbs;

  PetscFunctionBegin;
  if (!ia) PetscFunctionReturn(0);

  if (oshift == 1) {
    int nz = a->i[n]-1; 
    for (i=0; i<nz; i++) a->j[i]--;
    for (i=0; i<n+1; i++) a->i[i]--;
  }
  PetscFunctionReturn(0); 
}

#undef __FUNCT__  
#define __FUNCT__ "MatGetBlockSize_SeqSBAIJ"
int MatGetBlockSize_SeqSBAIJ(Mat mat,int *bs)
{
  Mat_SeqSBAIJ *sbaij = (Mat_SeqSBAIJ*)mat->data;

  PetscFunctionBegin;
  *bs = sbaij->bs;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatDestroy_SeqSBAIJ"
int MatDestroy_SeqSBAIJ(Mat A)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;
  int         ierr;

  PetscFunctionBegin;
#if defined(PETSC_USE_LOG)
  PetscLogObjectState((PetscObject)A,"Rows=%d, NZ=%d",A->m,a->nz);
#endif
  ierr = PetscFree(a->a);CHKERRQ(ierr);
  if (!a->singlemalloc) {
    ierr = PetscFree(a->i);CHKERRQ(ierr);
    ierr = PetscFree(a->j);CHKERRQ(ierr); 
  }
  if (a->row) {
    ierr = ISDestroy(a->row);CHKERRQ(ierr);
  }
  if (a->diag) {ierr = PetscFree(a->diag);CHKERRQ(ierr);}
  if (a->ilen) {ierr = PetscFree(a->ilen);CHKERRQ(ierr);}
  if (a->imax) {ierr = PetscFree(a->imax);CHKERRQ(ierr);}
  if (a->icol) {ierr = ISDestroy(a->icol);CHKERRQ(ierr);}
  if (a->solve_work)  {ierr = PetscFree(a->solve_work);CHKERRQ(ierr);}
  if (a->solves_work) {ierr = PetscFree(a->solves_work);CHKERRQ(ierr);}
  if (a->mult_work)   {ierr = PetscFree(a->mult_work);CHKERRQ(ierr);}
  if (a->saved_values) {ierr = PetscFree(a->saved_values);CHKERRQ(ierr);}
  if (a->xtoy) {ierr = PetscFree(a->xtoy);CHKERRQ(ierr);}

  if (a->inew){
    ierr = PetscFree(a->inew);CHKERRQ(ierr);
    a->inew = 0;
  }
  ierr = PetscFree(a);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatSetOption_SeqSBAIJ"
int MatSetOption_SeqSBAIJ(Mat A,MatOption op)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;

  PetscFunctionBegin;
  switch (op) {
  case MAT_ROW_ORIENTED:
    a->roworiented    = PETSC_TRUE;
    break;
  case MAT_COLUMN_ORIENTED:
    a->roworiented    = PETSC_FALSE;
    break;
  case MAT_COLUMNS_SORTED:
    a->sorted         = PETSC_TRUE;
    break;
  case MAT_COLUMNS_UNSORTED:
    a->sorted         = PETSC_FALSE;
    break;
  case MAT_KEEP_ZEROED_ROWS:
    a->keepzeroedrows = PETSC_TRUE;
    break;
  case MAT_NO_NEW_NONZERO_LOCATIONS:
    a->nonew          = 1;
    break;
  case MAT_NEW_NONZERO_LOCATION_ERR:
    a->nonew          = -1;
    break;
  case MAT_NEW_NONZERO_ALLOCATION_ERR:
    a->nonew          = -2;
    break;
  case MAT_YES_NEW_NONZERO_LOCATIONS:
    a->nonew          = 0;
    break;
  case MAT_ROWS_SORTED:
  case MAT_ROWS_UNSORTED:
  case MAT_YES_NEW_DIAGONALS:
  case MAT_IGNORE_OFF_PROC_ENTRIES:
  case MAT_USE_HASH_TABLE:
    PetscLogInfo(A,"MatSetOption_SeqSBAIJ:Option ignored\n");
    break;
  case MAT_NO_NEW_DIAGONALS:
    SETERRQ(PETSC_ERR_SUP,"MAT_NO_NEW_DIAGONALS");
  case MAT_NOT_SYMMETRIC:
  case MAT_NOT_STRUCTURALLY_SYMMETRIC:
  case MAT_HERMITIAN:
    SETERRQ(PETSC_ERR_SUP,"Matrix must be symmetric");
  case MAT_SYMMETRIC:
  case MAT_STRUCTURALLY_SYMMETRIC:
  case MAT_NOT_HERMITIAN:
  case MAT_SYMMETRY_ETERNAL:
  case MAT_NOT_SYMMETRY_ETERNAL:
    break;
  default:
    SETERRQ(PETSC_ERR_SUP,"unknown option");
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatGetRow_SeqSBAIJ"
int MatGetRow_SeqSBAIJ(Mat A,int row,int *ncols,int **cols,PetscScalar **v)
{
  Mat_SeqSBAIJ  *a = (Mat_SeqSBAIJ*)A->data;
  int          itmp,i,j,k,M,*ai,*aj,bs,bn,bp,*cols_i,bs2,ierr;
  MatScalar    *aa,*aa_i;
  PetscScalar  *v_i;

  PetscFunctionBegin;
  bs  = a->bs;
  ai  = a->i;
  aj  = a->j;
  aa  = a->a;
  bs2 = a->bs2;
  
  if (row < 0 || row >= A->m) SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE, "Row %d out of range", row);
  
  bn  = row/bs;   /* Block number */
  bp  = row % bs; /* Block position */
  M   = ai[bn+1] - ai[bn];
  *ncols = bs*M;
  
  if (v) {
    *v = 0;
    if (*ncols) {
      ierr = PetscMalloc((*ncols+row)*sizeof(PetscScalar),v);CHKERRQ(ierr);
      for (i=0; i<M; i++) { /* for each block in the block row */
        v_i  = *v + i*bs;
        aa_i = aa + bs2*(ai[bn] + i);
        for (j=bp,k=0; j<bs2; j+=bs,k++) {v_i[k] = aa_i[j];}
      }
    }
  }

  if (cols) {
    *cols = 0;
    if (*ncols) {
      ierr = PetscMalloc((*ncols+row)*sizeof(int),cols);CHKERRQ(ierr);
      for (i=0; i<M; i++) { /* for each block in the block row */
        cols_i = *cols + i*bs;
        itmp  = bs*aj[ai[bn] + i];
        for (j=0; j<bs; j++) {cols_i[j] = itmp++;}
      }
    }
  }

  /*search column A(0:row-1,row) (=A(row,0:row-1)). Could be expensive! */
  /* this segment is currently removed, so only entries in the upper triangle are obtained */
#ifdef column_search
  v_i    = *v    + M*bs;
  cols_i = *cols + M*bs;
  for (i=0; i<bn; i++){ /* for each block row */
    M = ai[i+1] - ai[i];
    for (j=0; j<M; j++){
      itmp = aj[ai[i] + j];    /* block column value */
      if (itmp == bn){ 
        aa_i   = aa    + bs2*(ai[i] + j) + bs*bp;
        for (k=0; k<bs; k++) {
          *cols_i++ = i*bs+k; 
          *v_i++    = aa_i[k]; 
        }
        *ncols += bs;
        break;
      }
    }
  }
#endif

  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatRestoreRow_SeqSBAIJ"
int MatRestoreRow_SeqSBAIJ(Mat A,int row,int *nz,int **idx,PetscScalar **v)
{
  int ierr;

  PetscFunctionBegin;
  if (idx) {if (*idx) {ierr = PetscFree(*idx);CHKERRQ(ierr);}}
  if (v)   {if (*v)   {ierr = PetscFree(*v);CHKERRQ(ierr);}}
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatTranspose_SeqSBAIJ"
int MatTranspose_SeqSBAIJ(Mat A,Mat *B)
{ 
  int ierr;
  PetscFunctionBegin;
  ierr = MatDuplicate(A,MAT_COPY_VALUES,B);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatView_SeqSBAIJ_ASCII"
static int MatView_SeqSBAIJ_ASCII(Mat A,PetscViewer viewer)
{
  Mat_SeqSBAIJ      *a = (Mat_SeqSBAIJ*)A->data;
  int               ierr,i,j,bs = a->bs,k,l,bs2=a->bs2;
  char              *name;
  PetscViewerFormat format;

  PetscFunctionBegin;
  ierr = PetscObjectGetName((PetscObject)A,&name);CHKERRQ(ierr);
  ierr = PetscViewerGetFormat(viewer,&format);CHKERRQ(ierr);
  if (format == PETSC_VIEWER_ASCII_INFO || format == PETSC_VIEWER_ASCII_INFO_DETAIL) {
    ierr = PetscViewerASCIIPrintf(viewer,"  block size is %d\n",bs);CHKERRQ(ierr);
  } else if (format == PETSC_VIEWER_ASCII_MATLAB) {
    SETERRQ(PETSC_ERR_SUP,"Matlab format not supported");
  } else if (format == PETSC_VIEWER_ASCII_COMMON) {
    ierr = PetscViewerASCIIUseTabs(viewer,PETSC_NO);CHKERRQ(ierr);
    for (i=0; i<a->mbs; i++) {
      for (j=0; j<bs; j++) {
        ierr = PetscViewerASCIIPrintf(viewer,"row %d:",i*bs+j);CHKERRQ(ierr);
        for (k=a->i[i]; k<a->i[i+1]; k++) {
          for (l=0; l<bs; l++) {
#if defined(PETSC_USE_COMPLEX)
            if (PetscImaginaryPart(a->a[bs2*k + l*bs + j]) > 0.0 && PetscRealPart(a->a[bs2*k + l*bs + j]) != 0.0) {
              ierr = PetscViewerASCIIPrintf(viewer," (%d, %g + %g i) ",bs*a->j[k]+l,
                      PetscRealPart(a->a[bs2*k + l*bs + j]),PetscImaginaryPart(a->a[bs2*k + l*bs + j]));CHKERRQ(ierr);
            } else if (PetscImaginaryPart(a->a[bs2*k + l*bs + j]) < 0.0 && PetscRealPart(a->a[bs2*k + l*bs + j]) != 0.0) {
              ierr = PetscViewerASCIIPrintf(viewer," (%d, %g - %g i) ",bs*a->j[k]+l,
                      PetscRealPart(a->a[bs2*k + l*bs + j]),-PetscImaginaryPart(a->a[bs2*k + l*bs + j]));CHKERRQ(ierr);
            } else if (PetscRealPart(a->a[bs2*k + l*bs + j]) != 0.0) {
              ierr = PetscViewerASCIIPrintf(viewer," (%d, %g) ",bs*a->j[k]+l,PetscRealPart(a->a[bs2*k + l*bs + j]));CHKERRQ(ierr);
            }
#else
            if (a->a[bs2*k + l*bs + j] != 0.0) {
              ierr = PetscViewerASCIIPrintf(viewer," (%d, %g) ",bs*a->j[k]+l,a->a[bs2*k + l*bs + j]);CHKERRQ(ierr);
            }
#endif
          }
        }
        ierr = PetscViewerASCIIPrintf(viewer,"\n");CHKERRQ(ierr);
      }
    } 
    ierr = PetscViewerASCIIUseTabs(viewer,PETSC_YES);CHKERRQ(ierr);
  } else {
    ierr = PetscViewerASCIIUseTabs(viewer,PETSC_NO);CHKERRQ(ierr);
    for (i=0; i<a->mbs; i++) {
      for (j=0; j<bs; j++) {
        ierr = PetscViewerASCIIPrintf(viewer,"row %d:",i*bs+j);CHKERRQ(ierr);
        for (k=a->i[i]; k<a->i[i+1]; k++) {
          for (l=0; l<bs; l++) {
#if defined(PETSC_USE_COMPLEX)
            if (PetscImaginaryPart(a->a[bs2*k + l*bs + j]) > 0.0) {
              ierr = PetscViewerASCIIPrintf(viewer," (%d, %g + %g i) ",bs*a->j[k]+l,
                PetscRealPart(a->a[bs2*k + l*bs + j]),PetscImaginaryPart(a->a[bs2*k + l*bs + j]));CHKERRQ(ierr);
            } else if (PetscImaginaryPart(a->a[bs2*k + l*bs + j]) < 0.0) {
              ierr = PetscViewerASCIIPrintf(viewer," (%d, %g - %g i) ",bs*a->j[k]+l,
                PetscRealPart(a->a[bs2*k + l*bs + j]),-PetscImaginaryPart(a->a[bs2*k + l*bs + j]));CHKERRQ(ierr);
            } else {
              ierr = PetscViewerASCIIPrintf(viewer," (%d, %g) ",bs*a->j[k]+l,PetscRealPart(a->a[bs2*k + l*bs + j]));CHKERRQ(ierr);
            }
#else
            ierr = PetscViewerASCIIPrintf(viewer," %d %g ",bs*a->j[k]+l,a->a[bs2*k + l*bs + j]);CHKERRQ(ierr);
#endif
          }
        }
        ierr = PetscViewerASCIIPrintf(viewer,"\n");CHKERRQ(ierr);
      }
    } 
    ierr = PetscViewerASCIIUseTabs(viewer,PETSC_YES);CHKERRQ(ierr);
  }
  ierr = PetscViewerFlush(viewer);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatView_SeqSBAIJ_Draw_Zoom"
static int MatView_SeqSBAIJ_Draw_Zoom(PetscDraw draw,void *Aa)
{
  Mat          A = (Mat) Aa;
  Mat_SeqSBAIJ *a=(Mat_SeqSBAIJ*)A->data;
  int          row,ierr,i,j,k,l,mbs=a->mbs,color,bs=a->bs,bs2=a->bs2,rank;
  PetscReal    xl,yl,xr,yr,x_l,x_r,y_l,y_r;
  MatScalar    *aa;
  MPI_Comm     comm;
  PetscViewer  viewer;

  PetscFunctionBegin; 
  /*
      This is nasty. If this is called from an originally parallel matrix
   then all processes call this,but only the first has the matrix so the
   rest should return immediately.
  */
  ierr = PetscObjectGetComm((PetscObject)draw,&comm);CHKERRQ(ierr);
  ierr = MPI_Comm_rank(comm,&rank);CHKERRQ(ierr);
  if (rank) PetscFunctionReturn(0);

  ierr = PetscObjectQuery((PetscObject)A,"Zoomviewer",(PetscObject*)&viewer);CHKERRQ(ierr); 

  ierr = PetscDrawGetCoordinates(draw,&xl,&yl,&xr,&yr);CHKERRQ(ierr);
  PetscDrawString(draw, .3*(xl+xr), .3*(yl+yr), PETSC_DRAW_BLACK, "symmetric");

  /* loop over matrix elements drawing boxes */
  color = PETSC_DRAW_BLUE;
  for (i=0,row=0; i<mbs; i++,row+=bs) {
    for (j=a->i[i]; j<a->i[i+1]; j++) {
      y_l = A->m - row - 1.0; y_r = y_l + 1.0;
      x_l = a->j[j]*bs; x_r = x_l + 1.0;
      aa = a->a + j*bs2;
      for (k=0; k<bs; k++) {
        for (l=0; l<bs; l++) {
          if (PetscRealPart(*aa++) >=  0.) continue;
          ierr = PetscDrawRectangle(draw,x_l+k,y_l-l,x_r+k,y_r-l,color,color,color,color);CHKERRQ(ierr);
        }
      }
    } 
  }
  color = PETSC_DRAW_CYAN;
  for (i=0,row=0; i<mbs; i++,row+=bs) {
    for (j=a->i[i]; j<a->i[i+1]; j++) {
      y_l = A->m - row - 1.0; y_r = y_l + 1.0;
      x_l = a->j[j]*bs; x_r = x_l + 1.0;
      aa = a->a + j*bs2;
      for (k=0; k<bs; k++) {
        for (l=0; l<bs; l++) {
          if (PetscRealPart(*aa++) != 0.) continue;
          ierr = PetscDrawRectangle(draw,x_l+k,y_l-l,x_r+k,y_r-l,color,color,color,color);CHKERRQ(ierr);
        }
      }
    } 
  }

  color = PETSC_DRAW_RED;
  for (i=0,row=0; i<mbs; i++,row+=bs) {
    for (j=a->i[i]; j<a->i[i+1]; j++) {
      y_l = A->m - row - 1.0; y_r = y_l + 1.0;
      x_l = a->j[j]*bs; x_r = x_l + 1.0;
      aa = a->a + j*bs2;
      for (k=0; k<bs; k++) {
        for (l=0; l<bs; l++) {
          if (PetscRealPart(*aa++) <= 0.) continue;
          ierr = PetscDrawRectangle(draw,x_l+k,y_l-l,x_r+k,y_r-l,color,color,color,color);CHKERRQ(ierr);
        }
      }
    } 
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatView_SeqSBAIJ_Draw"
static int MatView_SeqSBAIJ_Draw(Mat A,PetscViewer viewer)
{
  int          ierr;
  PetscReal    xl,yl,xr,yr,w,h;
  PetscDraw    draw;
  PetscTruth   isnull;

  PetscFunctionBegin; 

  ierr = PetscViewerDrawGetDraw(viewer,0,&draw);CHKERRQ(ierr);
  ierr = PetscDrawIsNull(draw,&isnull);CHKERRQ(ierr); if (isnull) PetscFunctionReturn(0);

  ierr = PetscObjectCompose((PetscObject)A,"Zoomviewer",(PetscObject)viewer);CHKERRQ(ierr);
  xr  = A->m; yr = A->m; h = yr/10.0; w = xr/10.0; 
  xr += w;    yr += h;  xl = -w;     yl = -h;
  ierr = PetscDrawSetCoordinates(draw,xl,yl,xr,yr);CHKERRQ(ierr);
  ierr = PetscDrawZoom(draw,MatView_SeqSBAIJ_Draw_Zoom,A);CHKERRQ(ierr);
  ierr = PetscObjectCompose((PetscObject)A,"Zoomviewer",PETSC_NULL);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatView_SeqSBAIJ"
int MatView_SeqSBAIJ(Mat A,PetscViewer viewer)
{
  int        ierr;
  PetscTruth isascii,isdraw;

  PetscFunctionBegin;
  ierr = PetscTypeCompare((PetscObject)viewer,PETSC_VIEWER_ASCII,&isascii);CHKERRQ(ierr);
  ierr = PetscTypeCompare((PetscObject)viewer,PETSC_VIEWER_DRAW,&isdraw);CHKERRQ(ierr);
  if (isascii){
    ierr = MatView_SeqSBAIJ_ASCII(A,viewer);CHKERRQ(ierr);
  } else if (isdraw) {
    ierr = MatView_SeqSBAIJ_Draw(A,viewer);CHKERRQ(ierr);
  } else {
    Mat B;
    ierr = MatConvert(A,MATSEQAIJ,&B);CHKERRQ(ierr);
    ierr = MatView(B,viewer);CHKERRQ(ierr);
    ierr = MatDestroy(B);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}


#undef __FUNCT__  
#define __FUNCT__ "MatGetValues_SeqSBAIJ"
int MatGetValues_SeqSBAIJ(Mat A,int m,const int im[],int n,const int in[],PetscScalar v[])
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;
  int        *rp,k,low,high,t,row,nrow,i,col,l,*aj = a->j;
  int        *ai = a->i,*ailen = a->ilen;
  int        brow,bcol,ridx,cidx,bs=a->bs,bs2=a->bs2;
  MatScalar  *ap,*aa = a->a,zero = 0.0;

  PetscFunctionBegin;
  for (k=0; k<m; k++) { /* loop over rows */
    row  = im[k]; brow = row/bs;  
    if (row < 0) SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,"Negative row: %d",row);
    if (row >= A->m) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Row too large: row %d max %d",row,A->m-1);
    rp   = aj + ai[brow] ; ap = aa + bs2*ai[brow] ;
    nrow = ailen[brow]; 
    for (l=0; l<n; l++) { /* loop over columns */
      if (in[l] < 0) SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,"Negative column: %d",in[l]);
      if (in[l] >= A->n) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Column too large: col %d max %d",in[l],A->n-1);
      col  = in[l] ; 
      bcol = col/bs;
      cidx = col%bs; 
      ridx = row%bs;
      high = nrow; 
      low  = 0; /* assume unsorted */
      while (high-low > 5) {
        t = (low+high)/2;
        if (rp[t] > bcol) high = t;
        else             low  = t;
      }
      for (i=low; i<high; i++) {
        if (rp[i] > bcol) break;
        if (rp[i] == bcol) {
          *v++ = ap[bs2*i+bs*cidx+ridx];
          goto finished;
        }
      } 
      *v++ = zero;
      finished:;
    }
  }
  PetscFunctionReturn(0);
} 


#undef __FUNCT__  
#define __FUNCT__ "MatSetValuesBlocked_SeqSBAIJ"
int MatSetValuesBlocked_SeqSBAIJ(Mat A,int m,const int im[],int n,const int in[],const PetscScalar v[],InsertMode is)
{
  Mat_SeqSBAIJ    *a = (Mat_SeqSBAIJ*)A->data;
  int             *rp,k,low,high,t,ii,jj,row,nrow,i,col,l,rmax,N,sorted=a->sorted;
  int             *imax=a->imax,*ai=a->i,*ailen=a->ilen;
  int             *aj=a->j,nonew=a->nonew,bs2=a->bs2,bs=a->bs,stepval,ierr;
  PetscTruth      roworiented=a->roworiented; 
  const MatScalar *value = v;
  MatScalar       *ap,*aa = a->a,*bap;
  
  PetscFunctionBegin;
  if (roworiented) { 
    stepval = (n-1)*bs;
  } else {
    stepval = (m-1)*bs;
  }
  for (k=0; k<m; k++) { /* loop over added rows */
    row  = im[k]; 
    if (row < 0) continue;
#if defined(PETSC_USE_BOPT_g)  
    if (row >= a->mbs) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Row too large: row %d max %d",row,a->mbs-1);
#endif
    rp   = aj + ai[row]; 
    ap   = aa + bs2*ai[row];
    rmax = imax[row]; 
    nrow = ailen[row]; 
    low  = 0;
    for (l=0; l<n; l++) { /* loop over added columns */
      if (in[l] < 0) continue;
      col = in[l]; 
#if defined(PETSC_USE_BOPT_g)  
      if (col >= a->nbs) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Column too large: col %d max %d",col,a->nbs-1);
#endif
      if (col < row) continue; /* ignore lower triangular block */
      if (roworiented) { 
        value = v + k*(stepval+bs)*bs + l*bs;
      } else {
        value = v + l*(stepval+bs)*bs + k*bs;
      }
      if (!sorted) low = 0; high = nrow;
      while (high-low > 7) {
        t = (low+high)/2;
        if (rp[t] > col) high = t;
        else             low  = t;
      }
      for (i=low; i<high; i++) {
        if (rp[i] > col) break;
        if (rp[i] == col) {
          bap  = ap +  bs2*i;
          if (roworiented) { 
            if (is == ADD_VALUES) {
              for (ii=0; ii<bs; ii++,value+=stepval) {
                for (jj=ii; jj<bs2; jj+=bs) {
                  bap[jj] += *value++; 
                }
              }
            } else {
              for (ii=0; ii<bs; ii++,value+=stepval) {
                for (jj=ii; jj<bs2; jj+=bs) {
                  bap[jj] = *value++; 
                }
              }
            }
          } else {
            if (is == ADD_VALUES) {
              for (ii=0; ii<bs; ii++,value+=stepval) {
                for (jj=0; jj<bs; jj++) {
                  *bap++ += *value++; 
                }
              }
            } else {
              for (ii=0; ii<bs; ii++,value+=stepval) {
                for (jj=0; jj<bs; jj++) {
                  *bap++  = *value++; 
                }
              }
            }
          }
          goto noinsert2;
        }
      } 
      if (nonew == 1) goto noinsert2;
      else if (nonew == -1) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Inserting a new nonzero (%d, %d) in the matrix", row, col);
      if (nrow >= rmax) {
        /* there is no extra room in row, therefore enlarge */
        int       new_nz = ai[a->mbs] + CHUNKSIZE,len,*new_i,*new_j;
        MatScalar *new_a;

        if (nonew == -2) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Inserting a new nonzero (%d, %d) in the matrix", row, col);

        /* malloc new storage space */
        len     = new_nz*(sizeof(int)+bs2*sizeof(MatScalar))+(a->mbs+1)*sizeof(int);
	ierr    = PetscMalloc(len,&new_a);CHKERRQ(ierr);
        new_j   = (int*)(new_a + bs2*new_nz);
        new_i   = new_j + new_nz;

        /* copy over old data into new slots */
        for (ii=0; ii<row+1; ii++) {new_i[ii] = ai[ii];}
        for (ii=row+1; ii<a->mbs+1; ii++) {new_i[ii] = ai[ii]+CHUNKSIZE;}
        ierr = PetscMemcpy(new_j,aj,(ai[row]+nrow)*sizeof(int));CHKERRQ(ierr);
        len  = (new_nz - CHUNKSIZE - ai[row] - nrow);
        ierr = PetscMemcpy(new_j+ai[row]+nrow+CHUNKSIZE,aj+ai[row]+nrow,len*sizeof(int));CHKERRQ(ierr);
        ierr = PetscMemcpy(new_a,aa,(ai[row]+nrow)*bs2*sizeof(MatScalar));CHKERRQ(ierr);
        ierr = PetscMemzero(new_a+bs2*(ai[row]+nrow),bs2*CHUNKSIZE*sizeof(MatScalar));CHKERRQ(ierr);
        ierr = PetscMemcpy(new_a+bs2*(ai[row]+nrow+CHUNKSIZE),aa+bs2*(ai[row]+nrow),bs2*len*sizeof(MatScalar));CHKERRQ(ierr);
        /* free up old matrix storage */
        ierr = PetscFree(a->a);CHKERRQ(ierr);
        if (!a->singlemalloc) {
          ierr = PetscFree(a->i);CHKERRQ(ierr);
          ierr = PetscFree(a->j);CHKERRQ(ierr);
        }
        aa = a->a = new_a; ai = a->i = new_i; aj = a->j = new_j; 
        a->singlemalloc = PETSC_TRUE;

        rp   = aj + ai[row]; ap = aa + bs2*ai[row];
        rmax = imax[row] = imax[row] + CHUNKSIZE;
        PetscLogObjectMemory(A,CHUNKSIZE*(sizeof(int) + bs2*sizeof(MatScalar)));
        a->maxnz += bs2*CHUNKSIZE;
        a->reallocs++;
        a->nz++;
      }
      N = nrow++ - 1; 
      /* shift up all the later entries in this row */
      for (ii=N; ii>=i; ii--) {
        rp[ii+1] = rp[ii];
        ierr = PetscMemcpy(ap+bs2*(ii+1),ap+bs2*(ii),bs2*sizeof(MatScalar));CHKERRQ(ierr);
      }
      if (N >= i) {
        ierr = PetscMemzero(ap+bs2*i,bs2*sizeof(MatScalar));CHKERRQ(ierr);
      }
      rp[i] = col; 
      bap   = ap +  bs2*i;
      if (roworiented) { 
        for (ii=0; ii<bs; ii++,value+=stepval) {
          for (jj=ii; jj<bs2; jj+=bs) {
            bap[jj] = *value++; 
          }
        }
      } else {
        for (ii=0; ii<bs; ii++,value+=stepval) {
          for (jj=0; jj<bs; jj++) {
            *bap++  = *value++; 
          }
        }
      }
      noinsert2:;
      low = i;
    }
    ailen[row] = nrow;
  }
  PetscFunctionReturn(0);
} 

#undef __FUNCT__  
#define __FUNCT__ "MatAssemblyEnd_SeqSBAIJ"
int MatAssemblyEnd_SeqSBAIJ(Mat A,MatAssemblyType mode)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;
  int        fshift = 0,i,j,*ai = a->i,*aj = a->j,*imax = a->imax;
  int        m = A->m,*ip,N,*ailen = a->ilen;
  int        mbs = a->mbs,bs2 = a->bs2,rmax = 0,ierr;
  MatScalar  *aa = a->a,*ap;

  PetscFunctionBegin;
  if (mode == MAT_FLUSH_ASSEMBLY) PetscFunctionReturn(0);

  if (m) rmax = ailen[0];
  for (i=1; i<mbs; i++) {
    /* move each row back by the amount of empty slots (fshift) before it*/
    fshift += imax[i-1] - ailen[i-1];
    rmax   = PetscMax(rmax,ailen[i]);
    if (fshift) {
      ip = aj + ai[i]; ap = aa + bs2*ai[i];
      N = ailen[i];
      for (j=0; j<N; j++) {
        ip[j-fshift] = ip[j];
        ierr = PetscMemcpy(ap+(j-fshift)*bs2,ap+j*bs2,bs2*sizeof(MatScalar));CHKERRQ(ierr);
      }
    } 
    ai[i] = ai[i-1] + ailen[i-1];
  }
  if (mbs) {
    fshift += imax[mbs-1] - ailen[mbs-1];
    ai[mbs] = ai[mbs-1] + ailen[mbs-1];
  }
  /* reset ilen and imax for each row */
  for (i=0; i<mbs; i++) {
    ailen[i] = imax[i] = ai[i+1] - ai[i];
  }
  a->nz = ai[mbs]; 

  /* diagonals may have moved, reset it */
  if (a->diag) {
    ierr = PetscMemcpy(a->diag,ai,(mbs+1)*sizeof(int));CHKERRQ(ierr);
  } 
  PetscLogInfo(A,"MatAssemblyEnd_SeqSBAIJ:Matrix size: %d X %d, block size %d; storage space: %d unneeded, %d used\n",
           m,A->m,a->bs,fshift*bs2,a->nz*bs2);
  PetscLogInfo(A,"MatAssemblyEnd_SeqSBAIJ:Number of mallocs during MatSetValues is %d\n",
           a->reallocs);
  PetscLogInfo(A,"MatAssemblyEnd_SeqSBAIJ:Most nonzeros blocks in any row is %d\n",rmax);
  a->reallocs          = 0;
  A->info.nz_unneeded  = (PetscReal)fshift*bs2;
  
  PetscFunctionReturn(0);
}

/* 
   This function returns an array of flags which indicate the locations of contiguous
   blocks that should be zeroed. for eg: if bs = 3  and is = [0,1,2,3,5,6,7,8,9]
   then the resulting sizes = [3,1,1,3,1] correspondig to sets [(0,1,2),(3),(5),(6,7,8),(9)]
   Assume: sizes should be long enough to hold all the values.
*/
#undef __FUNCT__  
#define __FUNCT__ "MatZeroRows_SeqSBAIJ_Check_Blocks"
int MatZeroRows_SeqSBAIJ_Check_Blocks(int idx[],int n,int bs,int sizes[], int *bs_max)
{
  int        i,j,k,row;
  PetscTruth flg;

  PetscFunctionBegin;
  for (i=0,j=0; i<n; j++) {
    row = idx[i];
    if (row%bs!=0) { /* Not the begining of a block */
      sizes[j] = 1;
      i++; 
    } else if (i+bs > n) { /* Beginning of a block, but complete block doesn't exist (at idx end) */
      sizes[j] = 1;         /* Also makes sure atleast 'bs' values exist for next else */
      i++; 
    } else { /* Begining of the block, so check if the complete block exists */
      flg = PETSC_TRUE;
      for (k=1; k<bs; k++) {
        if (row+k != idx[i+k]) { /* break in the block */
          flg = PETSC_FALSE;
          break;
        }
      }
      if (flg == PETSC_TRUE) { /* No break in the bs */
        sizes[j] = bs;
        i+= bs;
      } else {
        sizes[j] = 1;
        i++;
      }
    }
  }
  *bs_max = j;
  PetscFunctionReturn(0);
}
  
#undef __FUNCT__  
#define __FUNCT__ "MatZeroRows_SeqSBAIJ"
int MatZeroRows_SeqSBAIJ(Mat A,IS is,const PetscScalar *diag)
{
  PetscFunctionBegin;
  SETERRQ(PETSC_ERR_SUP,"No support for this function yet");
}

/* Only add/insert a(i,j) with i<=j (blocks). 
   Any a(i,j) with i>j input by user is ingored. 
*/

#undef __FUNCT__  
#define __FUNCT__ "MatSetValues_SeqSBAIJ"
int MatSetValues_SeqSBAIJ(Mat A,int m,const int im[],int n,const int in[],const PetscScalar v[],InsertMode is)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;
  int         *rp,k,low,high,t,ii,row,nrow,i,col,l,rmax,N,sorted=a->sorted;
  int         *imax=a->imax,*ai=a->i,*ailen=a->ilen,roworiented=a->roworiented;
  int         *aj=a->j,nonew=a->nonew,bs=a->bs,brow,bcol;
  int         ridx,cidx,bs2=a->bs2,ierr;
  MatScalar   *ap,value,*aa=a->a,*bap;

  PetscFunctionBegin;

  for (k=0; k<m; k++) { /* loop over added rows */
    row  = im[k];       /* row number */ 
    brow = row/bs;      /* block row number */ 
    if (row < 0) continue;
#if defined(PETSC_USE_BOPT_g)  
    if (row >= A->m) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Row too large: row %d max %d",row,A->m-1);
#endif
    rp   = aj + ai[brow]; /*ptr to beginning of column value of the row block*/
    ap   = aa + bs2*ai[brow]; /*ptr to beginning of element value of the row block*/
    rmax = imax[brow];  /* maximum space allocated for this row */
    nrow = ailen[brow]; /* actual length of this row */
    low  = 0;

    for (l=0; l<n; l++) { /* loop over added columns */
      if (in[l] < 0) continue;
#if defined(PETSC_USE_BOPT_g)  
      if (in[l] >= A->m) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Column too large: col %d max %d",in[l],A->m-1);
#endif
      col = in[l]; 
      bcol = col/bs;              /* block col number */

      if (brow <= bcol){
        ridx = row % bs; cidx = col % bs; /*row and col index inside the block */
        if ((brow==bcol && ridx<=cidx) || (brow<bcol)){    
          /* element value a(k,l) */
          if (roworiented) {
            value = v[l + k*n];             
          } else {
            value = v[k + l*m];
          }

          /* move pointer bap to a(k,l) quickly and add/insert value */
          if (!sorted) low = 0; high = nrow;
          while (high-low > 7) {
            t = (low+high)/2;
            if (rp[t] > bcol) high = t;
            else              low  = t;
          }
          for (i=low; i<high; i++) {
            /* printf("The loop of i=low.., rp[%d]=%d\n",i,rp[i]); */
            if (rp[i] > bcol) break;
            if (rp[i] == bcol) {
              bap  = ap +  bs2*i + bs*cidx + ridx;
              if (is == ADD_VALUES) *bap += value;  
              else                  *bap  = value; 
              /* for diag block, add/insert its symmetric element a(cidx,ridx) */
              if (brow == bcol && ridx < cidx){
                bap  = ap +  bs2*i + bs*ridx + cidx;
                if (is == ADD_VALUES) *bap += value;  
                else                  *bap  = value; 
              }
              goto noinsert1;
            }
          }      
      
      if (nonew == 1) goto noinsert1;
      else if (nonew == -1) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Inserting a new nonzero (%d, %d) in the matrix", row, col);
      if (nrow >= rmax) {
        /* there is no extra room in row, therefore enlarge */
        int       new_nz = ai[a->mbs] + CHUNKSIZE,len,*new_i,*new_j;
        MatScalar *new_a;

        if (nonew == -2) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"Inserting a new nonzero (%d, %d) in the matrix", row, col);

        /* Malloc new storage space */
        len   = new_nz*(sizeof(int)+bs2*sizeof(MatScalar))+(a->mbs+1)*sizeof(int);
        ierr  = PetscMalloc(len,&new_a);CHKERRQ(ierr);
        new_j = (int*)(new_a + bs2*new_nz);
        new_i = new_j + new_nz;

        /* copy over old data into new slots */
        for (ii=0; ii<brow+1; ii++) {new_i[ii] = ai[ii];}
        for (ii=brow+1; ii<a->mbs+1; ii++) {new_i[ii] = ai[ii]+CHUNKSIZE;}
        ierr = PetscMemcpy(new_j,aj,(ai[brow]+nrow)*sizeof(int));CHKERRQ(ierr);
        len  = (new_nz - CHUNKSIZE - ai[brow] - nrow);
        ierr = PetscMemcpy(new_j+ai[brow]+nrow+CHUNKSIZE,aj+ai[brow]+nrow,len*sizeof(int));CHKERRQ(ierr);
        ierr = PetscMemcpy(new_a,aa,(ai[brow]+nrow)*bs2*sizeof(MatScalar));CHKERRQ(ierr);
        ierr = PetscMemzero(new_a+bs2*(ai[brow]+nrow),bs2*CHUNKSIZE*sizeof(MatScalar));CHKERRQ(ierr);
        ierr = PetscMemcpy(new_a+bs2*(ai[brow]+nrow+CHUNKSIZE),aa+bs2*(ai[brow]+nrow),bs2*len*sizeof(MatScalar));CHKERRQ(ierr);
        /* free up old matrix storage */
        ierr = PetscFree(a->a);CHKERRQ(ierr);
        if (!a->singlemalloc) {
          ierr = PetscFree(a->i);CHKERRQ(ierr);
          ierr = PetscFree(a->j);CHKERRQ(ierr);
        }
        aa = a->a = new_a; ai = a->i = new_i; aj = a->j = new_j; 
        a->singlemalloc = PETSC_TRUE;

        rp   = aj + ai[brow]; ap = aa + bs2*ai[brow];
        rmax = imax[brow] = imax[brow] + CHUNKSIZE;
        PetscLogObjectMemory(A,CHUNKSIZE*(sizeof(int) + bs2*sizeof(MatScalar)));
        a->maxnz += bs2*CHUNKSIZE;
        a->reallocs++;
        a->nz++;
      }

      N = nrow++ - 1;     
      /* shift up all the later entries in this row */
      for (ii=N; ii>=i; ii--) {
        rp[ii+1] = rp[ii];
        ierr     = PetscMemcpy(ap+bs2*(ii+1),ap+bs2*(ii),bs2*sizeof(MatScalar));CHKERRQ(ierr);
      }
      if (N>=i) {
        ierr = PetscMemzero(ap+bs2*i,bs2*sizeof(MatScalar));CHKERRQ(ierr);
      }
      rp[i]                      = bcol; 
      ap[bs2*i + bs*cidx + ridx] = value; 
      noinsert1:;
      low = i;      
      /* } */
        }
      } /* end of if .. if..  */
    }                     /* end of loop over added columns */
    ailen[brow] = nrow; 
  }                       /* end of loop over added rows */

  PetscFunctionReturn(0);
} 

extern int MatCholeskyFactorSymbolic_SeqSBAIJ(Mat,IS,MatFactorInfo*,Mat*);
extern int MatCholeskyFactor_SeqSBAIJ(Mat,IS,MatFactorInfo*);
extern int MatIncreaseOverlap_SeqSBAIJ(Mat,int,IS[],int);
extern int MatGetSubMatrix_SeqSBAIJ(Mat,IS,IS,int,MatReuse,Mat*);
extern int MatGetSubMatrices_SeqSBAIJ(Mat,int,const IS[],const IS[],MatReuse,Mat*[]);
extern int MatMultTranspose_SeqSBAIJ(Mat,Vec,Vec);
extern int MatMultTransposeAdd_SeqSBAIJ(Mat,Vec,Vec,Vec);
extern int MatScale_SeqSBAIJ(const PetscScalar*,Mat);
extern int MatNorm_SeqSBAIJ(Mat,NormType,PetscReal *);
extern int MatEqual_SeqSBAIJ(Mat,Mat,PetscTruth*);
extern int MatGetDiagonal_SeqSBAIJ(Mat,Vec);
extern int MatDiagonalScale_SeqSBAIJ(Mat,Vec,Vec);
extern int MatGetInfo_SeqSBAIJ(Mat,MatInfoType,MatInfo *);
extern int MatZeroEntries_SeqSBAIJ(Mat);
extern int MatGetRowMax_SeqSBAIJ(Mat,Vec);

extern int MatSolve_SeqSBAIJ_N(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_1(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_2(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_3(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_4(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_5(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_6(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_7(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_7(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_6(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_5(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_4(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_3(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_2(Mat,Vec,Vec);
extern int MatSolveTranspose_SeqSBAIJ_1(Mat,Vec,Vec);

extern int MatSolves_SeqSBAIJ_1(Mat,Vecs,Vecs);

extern int MatSolve_SeqSBAIJ_1_NaturalOrdering(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_2_NaturalOrdering(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_3_NaturalOrdering(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_4_NaturalOrdering(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_5_NaturalOrdering(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_6_NaturalOrdering(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_7_NaturalOrdering(Mat,Vec,Vec);
extern int MatSolve_SeqSBAIJ_N_NaturalOrdering(Mat,Vec,Vec);

extern int MatCholeskyFactorNumeric_SeqSBAIJ_N(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_1(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_2(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_3(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_4(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_5(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_6(Mat,Mat*);
extern int MatCholeskyFactorNumeric_SeqSBAIJ_7(Mat,Mat*);
extern int MatGetInertia_SeqSBAIJ(Mat,int*,int*,int*);

extern int MatMult_SeqSBAIJ_1(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_2(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_3(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_4(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_5(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_6(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_7(Mat,Vec,Vec);
extern int MatMult_SeqSBAIJ_N(Mat,Vec,Vec);

extern int MatMultAdd_SeqSBAIJ_1(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_2(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_3(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_4(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_5(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_6(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_7(Mat,Vec,Vec,Vec);
extern int MatMultAdd_SeqSBAIJ_N(Mat,Vec,Vec,Vec);

#ifdef HAVE_MatICCFactor
/* modefied from MatILUFactor_SeqSBAIJ, needs further work!  */
#undef __FUNCT__  
#define __FUNCT__ "MatICCFactor_SeqSBAIJ"
int MatICCFactor_SeqSBAIJ(Mat inA,IS row,MatFactorInfo *info)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)inA->data;
  Mat         outA;
  int         ierr;
  PetscTruth  row_identity,col_identity;

  PetscFunctionBegin;
  /*
  if (level != 0) SETERRQ(PETSC_ERR_SUP,"Only levels = 0 supported for in-place ILU"); 
  ierr = ISIdentity(row,&row_identity);CHKERRQ(ierr);
  ierr = ISIdentity(col,&col_identity);CHKERRQ(ierr);
  if (!row_identity || !col_identity) {
    SETERRQ(1,"Row and column permutations must be identity for in-place ICC");
  }
  */

  outA          = inA; 
  inA->factor   = FACTOR_CHOLESKY;

  if (!a->diag) {
    ierr = MatMarkDiagonal_SeqSBAIJ(inA);CHKERRQ(ierr);
  }
  /*
      Blocksize 2, 3, 4, 5, 6 and 7 have a special faster factorization/solver 
      for ILU(0) factorization with natural ordering
  */
  switch (a->bs) {
  case 1:
    inA->ops->solve            = MatSolve_SeqSBAIJ_1_NaturalOrdering;
    inA->ops->solvetranspose   = MatSolve_SeqSBAIJ_1_NaturalOrdering;
    inA->ops->solves           = MatSolves_SeqSBAIJ_1;
    PetscLoginfo(inA,"MatICCFactor_SeqSBAIJ:Using special in-place natural ordering solvetrans BS=1\n");
  case 2:
    inA->ops->lufactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_2_NaturalOrdering;
    inA->ops->solve           = MatSolve_SeqSBAIJ_2_NaturalOrdering;
    inA->ops->solvetranspose  = MatSolve_SeqSBAIJ_2_NaturalOrdering;
    PetscLogInfo(inA,"MatICCFactor_SeqSBAIJ:Using special in-place natural ordering factor and solve BS=2\n");
    break;
  case 3:
    inA->ops->lufactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_3_NaturalOrdering;
    inA->ops->solve           = MatSolve_SeqSBAIJ_3_NaturalOrdering;
    inA->ops->solvetranspose  = MatSolve_SeqSBAIJ_3_NaturalOrdering;
    PetscLogInfo(inA,"MatICCFactor_SeqSBAIJ:Using special in-place natural ordering factor and solve BS=3\n");
    break; 
  case 4:
    inA->ops->lufactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_4_NaturalOrdering;
    inA->ops->solve           = MatSolve_SeqSBAIJ_4_NaturalOrdering;
    inA->ops->solvetranspose  = MatSolve_SeqSBAIJ_4_NaturalOrdering;
    PetscLogInfo(inA,"MatICCFactor_SeqSBAIJ:Using special in-place natural ordering factor and solve BS=4\n"); 
    break;
  case 5:
    inA->ops->lufactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_5_NaturalOrdering;
    inA->ops->solve           = MatSolve_SeqSBAIJ_5_NaturalOrdering;
    inA->ops->solvetranspose  = MatSolve_SeqSBAIJ_5_NaturalOrdering;
    PetscLogInfo(inA,"MatICCFactor_SeqSBAIJ:Using special in-place natural ordering factor and solve BS=5\n"); 
    break;
  case 6: 
    inA->ops->lufactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_6_NaturalOrdering;
    inA->ops->solve           = MatSolve_SeqSBAIJ_6_NaturalOrdering;
    inA->ops->solvetranspose  = MatSolve_SeqSBAIJ_6_NaturalOrdering;
    PetscLogInfo(inA,"MatICCFactor_SeqSBAIJ:Using special in-place natural ordering factor and solve BS=6\n");
    break; 
  case 7:
    inA->ops->lufactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_7_NaturalOrdering;
    inA->ops->solvetranspose  = MatSolve_SeqSBAIJ_7_NaturalOrdering;
    inA->ops->solve           = MatSolve_SeqSBAIJ_7_NaturalOrdering;
    PetscLogInfo(inA,"MatICCFactor_SeqSBAIJ:Using special in-place natural ordering factor and solve BS=7\n");
    break; 
  default:
    a->row        = row;
    a->icol       = col;
    ierr          = PetscObjectReference((PetscObject)row);CHKERRQ(ierr);
    ierr          = PetscObjectReference((PetscObject)col);CHKERRQ(ierr);

    /* Create the invert permutation so that it can be used in MatLUFactorNumeric() */
    ierr = ISInvertPermutation(col,PETSC_DECIDE, &(a->icol));CHKERRQ(ierr);
    PetscLogObjectParent(inA,a->icol);

    if (!a->solve_work) {
      ierr = PetscMalloc((A->m+a->bs)*sizeof(PetscScalar),&a->solve_work);CHKERRQ(ierr);
      PetscLogObjectMemory(inA,(A->m+a->bs)*sizeof(PetscScalar));
    }
  }

  ierr = MatCholeskyFactorNumeric(inA,&outA);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}
#endif

#undef __FUNCT__  
#define __FUNCT__ "MatPrintHelp_SeqSBAIJ"
int MatPrintHelp_SeqSBAIJ(Mat A)
{
  static PetscTruth called = PETSC_FALSE; 
  MPI_Comm          comm = A->comm;
  int               ierr;

  PetscFunctionBegin;
  if (called) {PetscFunctionReturn(0);} else called = PETSC_TRUE;
  ierr = (*PetscHelpPrintf)(comm," Options for MATSEQSBAIJ and MATMPISBAIJ matrix formats (the defaults):\n");CHKERRQ(ierr);
  ierr = (*PetscHelpPrintf)(comm,"  -mat_block_size <block_size>\n");CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "MatSeqSBAIJSetColumnIndices_SeqSBAIJ"
int MatSeqSBAIJSetColumnIndices_SeqSBAIJ(Mat mat,int *indices)
{
  Mat_SeqSBAIJ *baij = (Mat_SeqSBAIJ *)mat->data;
  int         i,nz,n;

  PetscFunctionBegin;
  nz = baij->maxnz;
  n  = mat->n;
  for (i=0; i<nz; i++) {
    baij->j[i] = indices[i];
  }
  baij->nz = nz;
  for (i=0; i<n; i++) {
    baij->ilen[i] = baij->imax[i];
  }

  PetscFunctionReturn(0);
}
EXTERN_C_END

#undef __FUNCT__  
#define __FUNCT__ "MatSeqSBAIJSetColumnIndices"
/*@
    MatSeqSBAIJSetColumnIndices - Set the column indices for all the rows
       in the matrix.

  Input Parameters:
+  mat     - the SeqSBAIJ matrix
-  indices - the column indices

  Level: advanced

  Notes:
    This can be called if you have precomputed the nonzero structure of the 
  matrix and want to provide it to the matrix object to improve the performance
  of the MatSetValues() operation.

    You MUST have set the correct numbers of nonzeros per row in the call to 
  MatCreateSeqSBAIJ().

    MUST be called before any calls to MatSetValues()

.seealso: MatCreateSeqSBAIJ
@*/ 
int MatSeqSBAIJSetColumnIndices(Mat mat,int *indices)
{
  int ierr,(*f)(Mat,int *);

  PetscFunctionBegin;
  PetscValidHeaderSpecific(mat,MAT_COOKIE,1);
  PetscValidPointer(indices,2);
  ierr = PetscObjectQueryFunction((PetscObject)mat,"MatSeqSBAIJSetColumnIndices_C",(void (**)(void))&f);CHKERRQ(ierr);
  if (f) {
    ierr = (*f)(mat,indices);CHKERRQ(ierr);
  } else {
    SETERRQ(1,"Wrong type of matrix to set column indices");
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatSetUpPreallocation_SeqSBAIJ"
int MatSetUpPreallocation_SeqSBAIJ(Mat A)
{
  int        ierr;

  PetscFunctionBegin;
  ierr =  MatSeqSBAIJSetPreallocation(A,1,PETSC_DEFAULT,0);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatGetArray_SeqSBAIJ"
int MatGetArray_SeqSBAIJ(Mat A,PetscScalar *array[])
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data; 
  PetscFunctionBegin;
  *array = a->a;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatRestoreArray_SeqSBAIJ"
int MatRestoreArray_SeqSBAIJ(Mat A,PetscScalar *array[])
{
  PetscFunctionBegin;
  PetscFunctionReturn(0);
}

#include "petscblaslapack.h"
#undef __FUNCT__  
#define __FUNCT__ "MatAXPY_SeqSBAIJ"
int MatAXPY_SeqSBAIJ(const PetscScalar *a,Mat X,Mat Y,MatStructure str)
{
  Mat_SeqSBAIJ *x=(Mat_SeqSBAIJ *)X->data, *y=(Mat_SeqSBAIJ *)Y->data;
  int          ierr,one=1,i,bs=y->bs,bs2,j;

  PetscFunctionBegin;
  if (str == SAME_NONZERO_PATTERN) {
    BLaxpy_(&x->nz,(PetscScalar*)a,x->a,&one,y->a,&one);
  } else if (str == SUBSET_NONZERO_PATTERN) { /* nonzeros of X is a subset of Y's */
    if (y->xtoy && y->XtoY != X) {
      ierr = PetscFree(y->xtoy);CHKERRQ(ierr);
      ierr = MatDestroy(y->XtoY);CHKERRQ(ierr);
    }
    if (!y->xtoy) { /* get xtoy */
      ierr = MatAXPYGetxtoy_Private(x->mbs,x->i,x->j,PETSC_NULL, y->i,y->j,PETSC_NULL, &y->xtoy);CHKERRQ(ierr);
      y->XtoY = X;
    }
    bs2 = bs*bs;
    for (i=0; i<x->nz; i++) {
      j = 0;
      while (j < bs2){
        y->a[bs2*y->xtoy[i]+j] += (*a)*(x->a[bs2*i+j]); 
        j++; 
      }
    }
    PetscLogInfo(0,"MatAXPY_SeqSBAIJ: ratio of nnz_s(X)/nnz_s(Y): %d/%d = %g\n",bs2*x->nz,bs2*y->nz,(PetscReal)(bs2*x->nz)/(bs2*y->nz));
  } else {
    ierr = MatAXPY_Basic(a,X,Y,str);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatIsSymmetric_SeqSBAIJ"
int MatIsSymmetric_SeqSBAIJ(Mat A,PetscTruth *flg)
{
  PetscFunctionBegin;
  *flg = PETSC_TRUE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatIsStructurallySymmetric_SeqSBAIJ"
int MatIsStructurallySymmetric_SeqSBAIJ(Mat A,PetscTruth *flg)
{
  PetscFunctionBegin;
  *flg = PETSC_TRUE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatIsHermitian_SeqSBAIJ"
int MatIsHermitian_SeqSBAIJ(Mat A,PetscTruth *flg)
{
  PetscFunctionBegin;
  *flg = PETSC_FALSE;
  PetscFunctionReturn(0);
}

/* -------------------------------------------------------------------*/
static struct _MatOps MatOps_Values = {MatSetValues_SeqSBAIJ,
       MatGetRow_SeqSBAIJ,
       MatRestoreRow_SeqSBAIJ,
       MatMult_SeqSBAIJ_N,
/* 4*/ MatMultAdd_SeqSBAIJ_N,
       MatMultTranspose_SeqSBAIJ,
       MatMultTransposeAdd_SeqSBAIJ,
       MatSolve_SeqSBAIJ_N,
       0,
       0,
/*10*/ 0,
       0,
       MatCholeskyFactor_SeqSBAIJ,
       MatRelax_SeqSBAIJ,
       MatTranspose_SeqSBAIJ,
/*15*/ MatGetInfo_SeqSBAIJ,
       MatEqual_SeqSBAIJ,
       MatGetDiagonal_SeqSBAIJ,
       MatDiagonalScale_SeqSBAIJ,
       MatNorm_SeqSBAIJ,
/*20*/ 0,
       MatAssemblyEnd_SeqSBAIJ,
       0,
       MatSetOption_SeqSBAIJ,
       MatZeroEntries_SeqSBAIJ,
/*25*/ MatZeroRows_SeqSBAIJ,
       0,
       0,
       MatCholeskyFactorSymbolic_SeqSBAIJ,
       MatCholeskyFactorNumeric_SeqSBAIJ_N,
/*30*/ MatSetUpPreallocation_SeqSBAIJ,
       0,
       MatICCFactorSymbolic_SeqSBAIJ,
       MatGetArray_SeqSBAIJ,
       MatRestoreArray_SeqSBAIJ,
/*35*/ MatDuplicate_SeqSBAIJ,
       0,
       0,
       0,
       0,
/*40*/ MatAXPY_SeqSBAIJ,
       MatGetSubMatrices_SeqSBAIJ,
       MatIncreaseOverlap_SeqSBAIJ,
       MatGetValues_SeqSBAIJ,
       0,
/*45*/ MatPrintHelp_SeqSBAIJ,
       MatScale_SeqSBAIJ,
       0,
       0,
       0,
/*50*/ MatGetBlockSize_SeqSBAIJ,
       MatGetRowIJ_SeqSBAIJ,
       MatRestoreRowIJ_SeqSBAIJ,
       0,
       0,
/*55*/ 0,
       0,
       0,
       0,
       MatSetValuesBlocked_SeqSBAIJ,
/*60*/ MatGetSubMatrix_SeqSBAIJ,
       0,
       0,
       MatGetPetscMaps_Petsc,
       0,
/*65*/ 0,
       0,
       0,
       0,
       0,    
/*70*/ MatGetRowMax_SeqSBAIJ,
       0,
       0,
       0,
       0,
/*75*/ 0,
       0,
       0,
       0,
       0,
/*80*/ 0,
       0,
       0,
#if !defined(PETSC_USE_COMPLEX)
       MatGetInertia_SeqSBAIJ,
#else
       0,
#endif
/*85*/ MatLoad_SeqSBAIJ,
       MatIsSymmetric_SeqSBAIJ,
       MatIsStructurallySymmetric_SeqSBAIJ,
       MatIsHermitian_SeqSBAIJ
};

EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "MatStoreValues_SeqSBAIJ"
int MatStoreValues_SeqSBAIJ(Mat mat)
{
  Mat_SeqSBAIJ *aij = (Mat_SeqSBAIJ *)mat->data;
  int         nz = aij->i[mat->m]*aij->bs*aij->bs2;
  int         ierr;

  PetscFunctionBegin;
  if (aij->nonew != 1) {
    SETERRQ(1,"Must call MatSetOption(A,MAT_NO_NEW_NONZERO_LOCATIONS);first");
  }

  /* allocate space for values if not already there */
  if (!aij->saved_values) {
    ierr = PetscMalloc((nz+1)*sizeof(PetscScalar),&aij->saved_values);CHKERRQ(ierr);
  }

  /* copy values over */
  ierr = PetscMemcpy(aij->saved_values,aij->a,nz*sizeof(PetscScalar));CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END

EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "MatRetrieveValues_SeqSBAIJ"
int MatRetrieveValues_SeqSBAIJ(Mat mat)
{
  Mat_SeqSBAIJ *aij = (Mat_SeqSBAIJ *)mat->data;
  int         nz = aij->i[mat->m]*aij->bs*aij->bs2,ierr;

  PetscFunctionBegin;
  if (aij->nonew != 1) {
    SETERRQ(1,"Must call MatSetOption(A,MAT_NO_NEW_NONZERO_LOCATIONS);first");
  }
  if (!aij->saved_values) {
    SETERRQ(1,"Must call MatStoreValues(A);first");
  }

  /* copy values over */
  ierr = PetscMemcpy(aij->a,aij->saved_values,nz*sizeof(PetscScalar));CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END

EXTERN_C_BEGIN
#undef __FUNCT__
#define __FUNCT__ "MatSeqSBAIJSetPreallocation_SeqSBAIJ"
int MatSeqSBAIJSetPreallocation_SeqSBAIJ(Mat B,int bs,int nz,int *nnz)
{
  Mat_SeqSBAIJ *b = (Mat_SeqSBAIJ*)B->data;
  int          i,len,ierr,mbs,bs2;
  PetscTruth   flg;

  PetscFunctionBegin;
  B->preallocated = PETSC_TRUE;
  ierr = PetscOptionsGetInt(B->prefix,"-mat_block_size",&bs,PETSC_NULL);CHKERRQ(ierr);
  mbs  = B->m/bs;
  bs2  = bs*bs;

  if (mbs*bs != B->m) {
    SETERRQ(PETSC_ERR_ARG_SIZ,"Number rows, cols must be divisible by blocksize");
  }

  if (nz == PETSC_DEFAULT || nz == PETSC_DECIDE) nz = 3;
  if (nz < 0) SETERRQ1(PETSC_ERR_ARG_OUTOFRANGE,"nz cannot be less than 0: value %d",nz);
  if (nnz) {
    for (i=0; i<mbs; i++) {
      if (nnz[i] < 0) SETERRQ2(PETSC_ERR_ARG_OUTOFRANGE,"nnz cannot be less than 0: local row %d value %d",i,nnz[i]);
      if (nnz[i] > mbs) SETERRQ3(PETSC_ERR_ARG_OUTOFRANGE,"nnz cannot be greater than block row length: local row %d value %d rowlength %d",i,nnz[i],mbs);
    }
  }

  ierr    = PetscOptionsHasName(B->prefix,"-mat_no_unroll",&flg);CHKERRQ(ierr);
  if (!flg) {
    switch (bs) {
    case 1:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_1;
      B->ops->solve           = MatSolve_SeqSBAIJ_1;
      B->ops->solves          = MatSolves_SeqSBAIJ_1;
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_1;
      B->ops->mult            = MatMult_SeqSBAIJ_1;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_1;
      break;
    case 2:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_2;  
      B->ops->solve           = MatSolve_SeqSBAIJ_2;
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_2;
      B->ops->mult            = MatMult_SeqSBAIJ_2;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_2;
      break;
    case 3:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_3;  
      B->ops->solve           = MatSolve_SeqSBAIJ_3;
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_3;
      B->ops->mult            = MatMult_SeqSBAIJ_3;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_3;
      break;
    case 4:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_4;  
      B->ops->solve           = MatSolve_SeqSBAIJ_4;
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_4;
      B->ops->mult            = MatMult_SeqSBAIJ_4;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_4;
      break;
    case 5:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_5;  
      B->ops->solve           = MatSolve_SeqSBAIJ_5; 
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_5;
      B->ops->mult            = MatMult_SeqSBAIJ_5;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_5;
      break;
    case 6:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_6;  
      B->ops->solve           = MatSolve_SeqSBAIJ_6; 
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_6;
      B->ops->mult            = MatMult_SeqSBAIJ_6;
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_6;
      break;
    case 7:
      B->ops->choleskyfactornumeric = MatCholeskyFactorNumeric_SeqSBAIJ_7;
      B->ops->solve           = MatSolve_SeqSBAIJ_7;
      B->ops->solvetranspose  = MatSolveTranspose_SeqSBAIJ_7;
      B->ops->mult            = MatMult_SeqSBAIJ_7; 
      B->ops->multadd         = MatMultAdd_SeqSBAIJ_7;
      break;
    }
  }

  b->mbs = mbs;
  b->nbs = mbs; 
  ierr   = PetscMalloc((mbs+1)*sizeof(int),&b->imax);CHKERRQ(ierr);
  if (!nnz) {
    if (nz == PETSC_DEFAULT || nz == PETSC_DECIDE) nz = 5;
    else if (nz <= 0)        nz = 1;
    for (i=0; i<mbs; i++) {
      b->imax[i] = nz; 
    }
    nz = nz*mbs; /* total nz */
  } else {
    nz = 0;
    for (i=0; i<mbs; i++) {b->imax[i] = nnz[i]; nz += nnz[i];}
  }
  /* nz=(nz+mbs)/2; */ /* total diagonal and superdiagonal nonzero blocks */

  /* allocate the matrix space */
  len  = nz*sizeof(int) + nz*bs2*sizeof(MatScalar) + (B->m+1)*sizeof(int);
  ierr = PetscMalloc(len,&b->a);CHKERRQ(ierr);
  ierr = PetscMemzero(b->a,nz*bs2*sizeof(MatScalar));CHKERRQ(ierr);
  b->j = (int*)(b->a + nz*bs2);
  ierr = PetscMemzero(b->j,nz*sizeof(int));CHKERRQ(ierr);
  b->i = b->j + nz;
  b->singlemalloc = PETSC_TRUE;

  /* pointer to beginning of each row */
  b->i[0] = 0;
  for (i=1; i<mbs+1; i++) {
    b->i[i] = b->i[i-1] + b->imax[i-1];
  }

  /* b->ilen will count nonzeros in each block row so far. */
  ierr = PetscMalloc((mbs+1)*sizeof(int),&b->ilen);CHKERRQ(ierr);
  PetscLogObjectMemory(B,len+2*(mbs+1)*sizeof(int)+sizeof(struct _p_Mat)+sizeof(Mat_SeqSBAIJ));
  for (i=0; i<mbs; i++) { b->ilen[i] = 0;}
  
  b->bs               = bs;
  b->bs2              = bs2;
  b->nz             = 0;
  b->maxnz          = nz*bs2;
  
  b->inew             = 0;
  b->jnew             = 0;
  b->anew             = 0;
  b->a2anew           = 0;
  b->permute          = PETSC_FALSE;
  PetscFunctionReturn(0);
}
EXTERN_C_END

EXTERN_C_BEGIN
EXTERN int MatConvert_SeqSBAIJ_SeqAIJ(Mat,const MatType,Mat*); 
EXTERN int MatConvert_SeqSBAIJ_SeqBAIJ(Mat,const MatType,Mat*); 
EXTERN_C_END

/*MC
   MATSEQSBAIJ - MATSEQSBAIJ = "seqsbaij" - A matrix type to be used for sequential symmetric block sparse matrices, 
   based on block compressed sparse row format.  Only the upper triangular portion of the matrix is stored.

   Options Database Keys:
. -mat_type seqsbaij - sets the matrix type to "seqsbaij" during a call to MatSetFromOptions()

  Level: beginner

.seealso: MatCreateSeqSBAIJ
M*/

EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "MatCreate_SeqSBAIJ"
int MatCreate_SeqSBAIJ(Mat B)
{
  Mat_SeqSBAIJ *b;
  int          ierr,size;

  PetscFunctionBegin;
  ierr = MPI_Comm_size(B->comm,&size);CHKERRQ(ierr);
  if (size > 1) SETERRQ(PETSC_ERR_ARG_WRONG,"Comm must be of size 1");
  B->m = B->M = PetscMax(B->m,B->M);
  B->n = B->N = PetscMax(B->n,B->N);

  ierr    = PetscNew(Mat_SeqSBAIJ,&b);CHKERRQ(ierr);
  B->data = (void*)b;
  ierr    = PetscMemzero(b,sizeof(Mat_SeqSBAIJ));CHKERRQ(ierr);
  ierr    = PetscMemcpy(B->ops,&MatOps_Values,sizeof(struct _MatOps));CHKERRQ(ierr);
  B->ops->destroy     = MatDestroy_SeqSBAIJ;
  B->ops->view        = MatView_SeqSBAIJ;
  B->factor           = 0;
  B->lupivotthreshold = 1.0;
  B->mapping          = 0;
  b->row              = 0;
  b->icol             = 0;
  b->reallocs         = 0;
  b->saved_values     = 0;
  
  ierr = PetscMapCreateMPI(B->comm,B->m,B->M,&B->rmap);CHKERRQ(ierr);
  ierr = PetscMapCreateMPI(B->comm,B->n,B->N,&B->cmap);CHKERRQ(ierr);

  b->sorted           = PETSC_FALSE;
  b->roworiented      = PETSC_TRUE;
  b->nonew            = 0;
  b->diag             = 0;
  b->solve_work       = 0;
  b->mult_work        = 0;
  B->spptr            = 0;
  b->keepzeroedrows   = PETSC_FALSE;
  b->xtoy             = 0;
  b->XtoY             = 0;
  
  b->inew             = 0;
  b->jnew             = 0;
  b->anew             = 0;
  b->a2anew           = 0;
  b->permute          = PETSC_FALSE;

  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatStoreValues_C",
                                     "MatStoreValues_SeqSBAIJ",
                                     MatStoreValues_SeqSBAIJ);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatRetrieveValues_C",
                                     "MatRetrieveValues_SeqSBAIJ",
                                     (void*)MatRetrieveValues_SeqSBAIJ);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatSeqSBAIJSetColumnIndices_C",
                                     "MatSeqSBAIJSetColumnIndices_SeqSBAIJ",
                                     MatSeqSBAIJSetColumnIndices_SeqSBAIJ);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatConvert_seqsbaij_seqaij_C",
                                     "MatConvert_SeqSBAIJ_SeqAIJ",
                                      MatConvert_SeqSBAIJ_SeqAIJ);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatConvert_seqsbaij_seqbaij_C",
                                     "MatConvert_SeqSBAIJ_SeqBAIJ",
                                      MatConvert_SeqSBAIJ_SeqBAIJ);CHKERRQ(ierr);
  ierr = PetscObjectComposeFunctionDynamic((PetscObject)B,"MatSeqSBAIJSetPreallocation_C",
                                     "MatSeqSBAIJSetPreallocation_SeqSBAIJ",
                                     MatSeqSBAIJSetPreallocation_SeqSBAIJ);CHKERRQ(ierr);

  B->symmetric                  = PETSC_TRUE;
  B->structurally_symmetric     = PETSC_TRUE;
  B->symmetric_set              = PETSC_TRUE;
  B->structurally_symmetric_set = PETSC_TRUE;
  PetscFunctionReturn(0);
}
EXTERN_C_END

#undef __FUNCT__  
#define __FUNCT__ "MatSeqSBAIJSetPreallocation"
/*@C
   MatSeqSBAIJSetPreallocation - Creates a sparse symmetric matrix in block AIJ (block
   compressed row) format.  For good matrix assembly performance the
   user should preallocate the matrix storage by setting the parameter nz
   (or the array nnz).  By setting these parameters accurately, performance
   during matrix assembly can be increased by more than a factor of 50.

   Collective on Mat

   Input Parameters:
+  A - the symmetric matrix 
.  bs - size of block
.  nz - number of block nonzeros per block row (same for all rows)
-  nnz - array containing the number of block nonzeros in the upper triangular plus
         diagonal portion of each block (possibly different for each block row) or PETSC_NULL

   Options Database Keys:
.   -mat_no_unroll - uses code that does not unroll the loops in the 
                     block calculations (much slower)
.    -mat_block_size - size of the blocks to use

   Level: intermediate

   Notes:
   Specify the preallocated storage with either nz or nnz (not both).
   Set nz=PETSC_DEFAULT and nnz=PETSC_NULL for PETSc to control dynamic memory 
   allocation.  For additional details, see the users manual chapter on
   matrices.

.seealso: MatCreate(), MatCreateSeqAIJ(), MatSetValues(), MatCreateMPISBAIJ()
@*/
int MatSeqSBAIJSetPreallocation(Mat B,int bs,int nz,const int nnz[]) {
  int ierr,(*f)(Mat,int,int,const int[]);

  PetscFunctionBegin;
  ierr = PetscObjectQueryFunction((PetscObject)B,"MatSeqSBAIJSetPreallocation_C",(void (**)(void))&f);CHKERRQ(ierr);
  if (f) {
    ierr = (*f)(B,bs,nz,nnz);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatCreateSeqSBAIJ"
/*@C
   MatCreateSeqSBAIJ - Creates a sparse symmetric matrix in block AIJ (block
   compressed row) format.  For good matrix assembly performance the
   user should preallocate the matrix storage by setting the parameter nz
   (or the array nnz).  By setting these parameters accurately, performance
   during matrix assembly can be increased by more than a factor of 50.

   Collective on MPI_Comm

   Input Parameters:
+  comm - MPI communicator, set to PETSC_COMM_SELF
.  bs - size of block
.  m - number of rows, or number of columns
.  nz - number of block nonzeros per block row (same for all rows)
-  nnz - array containing the number of block nonzeros in the upper triangular plus
         diagonal portion of each block (possibly different for each block row) or PETSC_NULL

   Output Parameter:
.  A - the symmetric matrix 

   Options Database Keys:
.   -mat_no_unroll - uses code that does not unroll the loops in the 
                     block calculations (much slower)
.    -mat_block_size - size of the blocks to use

   Level: intermediate

   Notes:

   Specify the preallocated storage with either nz or nnz (not both).
   Set nz=PETSC_DEFAULT and nnz=PETSC_NULL for PETSc to control dynamic memory 
   allocation.  For additional details, see the users manual chapter on
   matrices.

.seealso: MatCreate(), MatCreateSeqAIJ(), MatSetValues(), MatCreateMPISBAIJ()
@*/
int MatCreateSeqSBAIJ(MPI_Comm comm,int bs,int m,int n,int nz,const int nnz[],Mat *A)
{
  int ierr;
 
  PetscFunctionBegin;
  ierr = MatCreate(comm,m,n,m,n,A);CHKERRQ(ierr);
  ierr = MatSetType(*A,MATSEQSBAIJ);CHKERRQ(ierr);
  ierr = MatSeqSBAIJSetPreallocation(*A,bs,nz,nnz);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatDuplicate_SeqSBAIJ"
int MatDuplicate_SeqSBAIJ(Mat A,MatDuplicateOption cpvalues,Mat *B)
{
  Mat          C;
  Mat_SeqSBAIJ *c,*a = (Mat_SeqSBAIJ*)A->data;
  int          i,len,mbs = a->mbs,nz = a->nz,bs2 =a->bs2,ierr;

  PetscFunctionBegin;
  if (a->i[mbs] != nz) SETERRQ(PETSC_ERR_PLIB,"Corrupt matrix");

  *B = 0;
  ierr = MatCreate(A->comm,A->m,A->n,A->m,A->n,&C);CHKERRQ(ierr);
  ierr = MatSetType(C,A->type_name);CHKERRQ(ierr);
  c    = (Mat_SeqSBAIJ*)C->data;

  C->preallocated   = PETSC_TRUE;
  C->factor         = A->factor;
  c->row            = 0;
  c->icol           = 0;
  c->saved_values   = 0;
  c->keepzeroedrows = a->keepzeroedrows;
  C->assembled      = PETSC_TRUE;

  C->M    = A->M;
  C->N    = A->N;
  c->bs   = a->bs;
  c->bs2  = a->bs2;
  c->mbs  = a->mbs;
  c->nbs  = a->nbs;

  ierr = PetscMalloc((mbs+1)*sizeof(int),&c->imax);CHKERRQ(ierr);
  ierr = PetscMalloc((mbs+1)*sizeof(int),&c->ilen);CHKERRQ(ierr);
  for (i=0; i<mbs; i++) {
    c->imax[i] = a->imax[i];
    c->ilen[i] = a->ilen[i]; 
  }

  /* allocate the matrix space */
  c->singlemalloc = PETSC_TRUE;
  len  = (mbs+1)*sizeof(int) + nz*(bs2*sizeof(MatScalar) + sizeof(int));
  ierr = PetscMalloc(len,&c->a);CHKERRQ(ierr);
  c->j = (int*)(c->a + nz*bs2);
  c->i = c->j + nz;
  ierr = PetscMemcpy(c->i,a->i,(mbs+1)*sizeof(int));CHKERRQ(ierr);
  if (mbs > 0) {
    ierr = PetscMemcpy(c->j,a->j,nz*sizeof(int));CHKERRQ(ierr);
    if (cpvalues == MAT_COPY_VALUES) {
      ierr = PetscMemcpy(c->a,a->a,bs2*nz*sizeof(MatScalar));CHKERRQ(ierr);
    } else {
      ierr = PetscMemzero(c->a,bs2*nz*sizeof(MatScalar));CHKERRQ(ierr);
    }
  }

  PetscLogObjectMemory(C,len+2*(mbs+1)*sizeof(int)+sizeof(struct _p_Mat)+sizeof(Mat_SeqSBAIJ));  
  c->sorted      = a->sorted;
  c->roworiented = a->roworiented;
  c->nonew       = a->nonew;

  if (a->diag) {
    ierr = PetscMalloc((mbs+1)*sizeof(int),&c->diag);CHKERRQ(ierr);
    PetscLogObjectMemory(C,(mbs+1)*sizeof(int));
    for (i=0; i<mbs; i++) {
      c->diag[i] = a->diag[i];
    }
  } else c->diag        = 0;
  c->nz               = a->nz;
  c->maxnz            = a->maxnz;
  c->solve_work         = 0;
  c->mult_work          = 0;
  *B = C;
  ierr = PetscFListDuplicate(A->qlist,&C->qlist);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatLoad_SeqSBAIJ"
int MatLoad_SeqSBAIJ(PetscViewer viewer,const MatType type,Mat *A)
{
  Mat_SeqSBAIJ *a;
  Mat          B;
  int          i,nz,ierr,fd,header[4],size,*rowlengths=0,M,N,bs=1;
  int          *mask,mbs,*jj,j,rowcount,nzcount,k,*s_browlengths,maskcount;
  int          kmax,jcount,block,idx,point,nzcountb,extra_rows;
  int          *masked,nmask,tmp,bs2,ishift;
  PetscScalar  *aa;
  MPI_Comm     comm = ((PetscObject)viewer)->comm;

  PetscFunctionBegin;
  ierr = PetscOptionsGetInt(PETSC_NULL,"-matload_block_size",&bs,PETSC_NULL);CHKERRQ(ierr);
  bs2  = bs*bs;

  ierr = MPI_Comm_size(comm,&size);CHKERRQ(ierr);
  if (size > 1) SETERRQ(PETSC_ERR_ARG_WRONG,"view must have one processor");
  ierr = PetscViewerBinaryGetDescriptor(viewer,&fd);CHKERRQ(ierr);
  ierr = PetscBinaryRead(fd,header,4,PETSC_INT);CHKERRQ(ierr);
  if (header[0] != MAT_FILE_COOKIE) SETERRQ(PETSC_ERR_FILE_UNEXPECTED,"not Mat object");
  M = header[1]; N = header[2]; nz = header[3];

  if (header[3] < 0) {
    SETERRQ(PETSC_ERR_FILE_UNEXPECTED,"Matrix stored in special format, cannot load as SeqSBAIJ");
  }

  if (M != N) SETERRQ(PETSC_ERR_SUP,"Can only do square matrices");

  /* 
     This code adds extra rows to make sure the number of rows is 
    divisible by the blocksize
  */
  mbs        = M/bs;
  extra_rows = bs - M + bs*(mbs);
  if (extra_rows == bs) extra_rows = 0;
  else                  mbs++;
  if (extra_rows) {
    PetscLogInfo(0,"MatLoad_SeqSBAIJ:Padding loaded matrix to match blocksize\n");
  }

  /* read in row lengths */
  ierr = PetscMalloc((M+extra_rows)*sizeof(int),&rowlengths);CHKERRQ(ierr);
  ierr = PetscBinaryRead(fd,rowlengths,M,PETSC_INT);CHKERRQ(ierr);
  for (i=0; i<extra_rows; i++) rowlengths[M+i] = 1;

  /* read in column indices */
  ierr = PetscMalloc((nz+extra_rows)*sizeof(int),&jj);CHKERRQ(ierr);
  ierr = PetscBinaryRead(fd,jj,nz,PETSC_INT);CHKERRQ(ierr);
  for (i=0; i<extra_rows; i++) jj[nz+i] = M+i;

  /* loop over row lengths determining block row lengths */
  ierr     = PetscMalloc(mbs*sizeof(int),&s_browlengths);CHKERRQ(ierr);
  ierr     = PetscMemzero(s_browlengths,mbs*sizeof(int));CHKERRQ(ierr);
  ierr     = PetscMalloc(2*mbs*sizeof(int),&mask);CHKERRQ(ierr);
  ierr     = PetscMemzero(mask,mbs*sizeof(int));CHKERRQ(ierr);
  masked   = mask + mbs;
  rowcount = 0; nzcount = 0;
  for (i=0; i<mbs; i++) {
    nmask = 0;
    for (j=0; j<bs; j++) {
      kmax = rowlengths[rowcount];
      for (k=0; k<kmax; k++) {
        tmp = jj[nzcount++]/bs;   /* block col. index */
        if (!mask[tmp] && tmp >= i) {masked[nmask++] = tmp; mask[tmp] = 1;} 
      }
      rowcount++;
    }
    s_browlengths[i] += nmask;
    
    /* zero out the mask elements we set */
    for (j=0; j<nmask; j++) mask[masked[j]] = 0;
  }

  /* create our matrix */
  ierr = MatCreate(comm,M+extra_rows,N+extra_rows,M+extra_rows,N+extra_rows,&B);CHKERRQ(ierr);
  ierr = MatSetType(B,type);CHKERRQ(ierr);
  ierr = MatSeqSBAIJSetPreallocation(B,bs,0,s_browlengths);CHKERRQ(ierr);
  a = (Mat_SeqSBAIJ*)B->data;

  /* set matrix "i" values */
  a->i[0] = 0;
  for (i=1; i<= mbs; i++) {
    a->i[i]      = a->i[i-1] + s_browlengths[i-1];
    a->ilen[i-1] = s_browlengths[i-1];
  }
  a->nz = a->i[mbs];

  /* read in nonzero values */
  ierr = PetscMalloc((nz+extra_rows)*sizeof(PetscScalar),&aa);CHKERRQ(ierr);
  ierr = PetscBinaryRead(fd,aa,nz,PETSC_SCALAR);CHKERRQ(ierr);
  for (i=0; i<extra_rows; i++) aa[nz+i] = 1.0;

  /* set "a" and "j" values into matrix */
  nzcount = 0; jcount = 0;
  for (i=0; i<mbs; i++) {
    nzcountb = nzcount;
    nmask    = 0;
    for (j=0; j<bs; j++) {
      kmax = rowlengths[i*bs+j];
      for (k=0; k<kmax; k++) {
        tmp = jj[nzcount++]/bs; /* block col. index */
        if (!mask[tmp] && tmp >= i) { masked[nmask++] = tmp; mask[tmp] = 1;} 
      }
    }
    /* sort the masked values */
    ierr = PetscSortInt(nmask,masked);CHKERRQ(ierr);

    /* set "j" values into matrix */
    maskcount = 1;
    for (j=0; j<nmask; j++) {
      a->j[jcount++]  = masked[j];
      mask[masked[j]] = maskcount++; 
    }

    /* set "a" values into matrix */
    ishift = bs2*a->i[i]; 
    for (j=0; j<bs; j++) {
      kmax = rowlengths[i*bs+j];
      for (k=0; k<kmax; k++) {
        tmp       = jj[nzcountb]/bs ; /* block col. index */
        if (tmp >= i){ 
          block     = mask[tmp] - 1;
          point     = jj[nzcountb] - bs*tmp;
          idx       = ishift + bs2*block + j + bs*point;
          a->a[idx] = aa[nzcountb];
        } 
        nzcountb++;
      }
    }
    /* zero out the mask elements we set */
    for (j=0; j<nmask; j++) mask[masked[j]] = 0;
  }
  if (jcount != a->nz) SETERRQ(PETSC_ERR_FILE_UNEXPECTED,"Bad binary matrix");

  ierr = PetscFree(rowlengths);CHKERRQ(ierr);
  ierr = PetscFree(s_browlengths);CHKERRQ(ierr);
  ierr = PetscFree(aa);CHKERRQ(ierr);
  ierr = PetscFree(jj);CHKERRQ(ierr);
  ierr = PetscFree(mask);CHKERRQ(ierr);

  ierr = MatAssemblyBegin(B,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(B,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatView_Private(B);CHKERRQ(ierr);
  *A = B;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "MatRelax_SeqSBAIJ"
int MatRelax_SeqSBAIJ(Mat A,Vec bb,PetscReal omega,MatSORType flag,PetscReal fshift,int its,int lits,Vec xx)
{
  Mat_SeqSBAIJ *a = (Mat_SeqSBAIJ*)A->data;
  MatScalar    *aa=a->a,*v,*v1;
  PetscScalar  *x,*b,*t,sum,d;
  int          m=a->mbs,bs=a->bs,*ai=a->i,*aj=a->j,ierr;
  int          nz,nz1,*vj,*vj1,i;

  PetscFunctionBegin;
  its = its*lits;
  if (its <= 0) SETERRQ2(PETSC_ERR_ARG_WRONG,"Relaxation requires global its %d and local its %d both positive",its,lits);

  if (bs > 1)
    SETERRQ(PETSC_ERR_SUP,"SSOR for block size > 1 is not yet implemented");

  ierr = VecGetArray(xx,&x);CHKERRQ(ierr);
  if (xx != bb) { 
    ierr = VecGetArray(bb,&b);CHKERRQ(ierr);
  } else { 
    b = x;
  } 

  ierr = PetscMalloc(m*sizeof(PetscScalar),&t);CHKERRQ(ierr);
 
  if (flag & SOR_ZERO_INITIAL_GUESS) {
    if (flag & SOR_FORWARD_SWEEP || flag & SOR_LOCAL_FORWARD_SWEEP){ 
      for (i=0; i<m; i++)
        t[i] = b[i];

      for (i=0; i<m; i++){
        d  = *(aa + ai[i]);  /* diag[i] */
        v  = aa + ai[i] + 1; 
        vj = aj + ai[i] + 1;    
        nz = ai[i+1] - ai[i] - 1;       
        x[i] = omega*t[i]/d;
        while (nz--) t[*vj++] -= x[i]*(*v++); /* update rhs */
        PetscLogFlops(2*nz-1);
      }
    } 

    if (flag & SOR_BACKWARD_SWEEP || flag & SOR_LOCAL_BACKWARD_SWEEP){ 
      for (i=0; i<m; i++)
        t[i] = b[i];
  
      for (i=0; i<m-1; i++){  /* update rhs */
        v  = aa + ai[i] + 1; 
        vj = aj + ai[i] + 1;    
        nz = ai[i+1] - ai[i] - 1;
        while (nz--) t[*vj++] -= x[i]*(*v++);
        PetscLogFlops(2*nz-1);
      }
      for (i=m-1; i>=0; i--){
        d  = *(aa + ai[i]);  
        v  = aa + ai[i] + 1; 
        vj = aj + ai[i] + 1;    
        nz = ai[i+1] - ai[i] - 1;
        sum = t[i];
        while (nz--) sum -= x[*vj++]*(*v++);
        PetscLogFlops(2*nz-1);
        x[i] =   (1-omega)*x[i] + omega*sum/d;        
      }
    }
    its--;
  } 

  while (its--) {
    /* 
       forward sweep:
       for i=0,...,m-1:
         sum[i] = (b[i] - U(i,:)x )/d[i];
         x[i]   = (1-omega)x[i] + omega*sum[i];
         b      = b - x[i]*U^T(i,:);
         
    */ 
    if (flag & SOR_FORWARD_SWEEP || flag & SOR_LOCAL_FORWARD_SWEEP){ 
      for (i=0; i<m; i++)
        t[i] = b[i];

      for (i=0; i<m; i++){
        d  = *(aa + ai[i]);  /* diag[i] */
        v  = aa + ai[i] + 1; v1=v;
        vj = aj + ai[i] + 1; vj1=vj;   
        nz = ai[i+1] - ai[i] - 1; nz1=nz;
        sum = t[i];
        while (nz1--) sum -= (*v1++)*x[*vj1++]; 
        x[i] = (1-omega)*x[i] + omega*sum/d;
        while (nz--) t[*vj++] -= x[i]*(*v++); 
        PetscLogFlops(4*nz-2);
      }
    }
  
  if (flag & SOR_BACKWARD_SWEEP || flag & SOR_LOCAL_BACKWARD_SWEEP){ 
      /* 
       backward sweep:
       b = b - x[i]*U^T(i,:), i=0,...,n-2
       for i=m-1,...,0:
         sum[i] = (b[i] - U(i,:)x )/d[i];
         x[i]   = (1-omega)x[i] + omega*sum[i];
      */ 
      for (i=0; i<m; i++)
        t[i] = b[i];
  
      for (i=0; i<m-1; i++){  /* update rhs */
        v  = aa + ai[i] + 1; 
        vj = aj + ai[i] + 1;    
        nz = ai[i+1] - ai[i] - 1;
        while (nz--) t[*vj++] -= x[i]*(*v++);
        PetscLogFlops(2*nz-1);
      }
      for (i=m-1; i>=0; i--){
        d  = *(aa + ai[i]);  
        v  = aa + ai[i] + 1; 
        vj = aj + ai[i] + 1;    
        nz = ai[i+1] - ai[i] - 1;
        sum = t[i];
        while (nz--) sum -= x[*vj++]*(*v++);
        PetscLogFlops(2*nz-1);
        x[i] =   (1-omega)*x[i] + omega*sum/d;        
      }
    }
  } 

  ierr = PetscFree(t); CHKERRQ(ierr);
  ierr = VecRestoreArray(xx,&x);CHKERRQ(ierr);
  if (bb != xx) { 
    ierr = VecRestoreArray(bb,&b);CHKERRQ(ierr);
  } 

  PetscFunctionReturn(0);
} 






