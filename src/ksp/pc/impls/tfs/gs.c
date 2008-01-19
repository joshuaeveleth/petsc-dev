#define PETSCKSP_DLL

/***********************************gs.c***************************************

Author: Henry M. Tufo III

e-mail: hmt@cs.brown.edu

snail-mail:
Division of Applied Mathematics
Brown University
Providence, RI 02912

Last Modification: 
6.21.97
************************************gs.c**************************************/

/***********************************gs.c***************************************
File Description:
-----------------

************************************gs.c**************************************/

#include "src/ksp/pc/impls/tfs/tfs.h"

/* default length of number of items via tree - doubles if exceeded */
#define TREE_BUF_SZ 2048;
#define GS_VEC_SZ   1



/***********************************gs.c***************************************
Type: struct gather_scatter_id 
------------------------------

************************************gs.c**************************************/
typedef struct gather_scatter_id {
  int id;
  int nel_min;
  int nel_max;
  int nel_sum;
  int negl;
  int gl_max;
  int gl_min;
  int repeats;
  int ordered;
  int positive;
  PetscScalar *vals;

  /* bit mask info */
  int *my_proc_mask;
  int mask_sz;
  int *ngh_buf;
  int ngh_buf_sz;
  int *nghs;
  int num_nghs;
  int max_nghs;
  int *pw_nghs;
  int num_pw_nghs;
  int *tree_nghs;
  int num_tree_nghs;

  int num_loads;

  /* repeats == true -> local info */
  int nel;         /* number of unique elememts */
  int *elms;       /* of size nel */
  int nel_total;
  int *local_elms; /* of size nel_total */
  int *companion;  /* of size nel_total */

  /* local info */
  int num_local_total;
  int local_strength;
  int num_local;
  int *num_local_reduce;
  int **local_reduce;
  int num_local_gop;
  int *num_gop_local_reduce;
  int **gop_local_reduce;

  /* pairwise info */
  int level;
  int num_pairs;
  int max_pairs;
  int loc_node_pairs;
  int max_node_pairs;
  int min_node_pairs;
  int avg_node_pairs;
  int *pair_list;
  int *msg_sizes;
  int **node_list;
  int len_pw_list;
  int *pw_elm_list;
  PetscScalar *pw_vals;

  MPI_Request *msg_ids_in;
  MPI_Request *msg_ids_out;

  PetscScalar *out;
  PetscScalar *in;
  int msg_total;

  /* tree - crystal accumulator info */
  int max_left_over;
  int *pre;
  int *in_num;
  int *out_num;
  int **in_list;
  int **out_list;

  /* new tree work*/
  int  tree_nel;
  int *tree_elms;
  PetscScalar *tree_buf;
  PetscScalar *tree_work;

  int  tree_map_sz;
  int *tree_map_in;
  int *tree_map_out;

  /* current memory status */
  int gl_bss_min;
  int gl_perm_min;

  /* max segment size for gs_gop_vec() */
  int vec_sz;

  /* hack to make paul happy */
  MPI_Comm gs_comm;

} gs_id;


/* to be made public */

/* PRIVATE - and definitely not exported */
/*static void gs_print_template( gs_id* gs, int who);*/
/*static void gs_print_stemplate( gs_id* gs, int who);*/

static gs_id *gsi_check_args(int *elms, int nel, int level);
static PetscErrorCode gsi_via_bit_mask(gs_id *gs);
static PetscErrorCode get_ngh_buf(gs_id *gs);
static PetscErrorCode set_pairwise(gs_id *gs);
static gs_id * gsi_new(void);
static PetscErrorCode set_tree(gs_id *gs);

/* same for all but vector flavor */
static PetscErrorCode gs_gop_local_out(gs_id *gs, PetscScalar *vals);
/* vector flavor */
static PetscErrorCode gs_gop_vec_local_out(gs_id *gs, PetscScalar *vals, int step);

static PetscErrorCode gs_gop_vec_plus(gs_id *gs, PetscScalar *in_vals, int step);
static PetscErrorCode gs_gop_vec_pairwise_plus(gs_id *gs, PetscScalar *in_vals, int step);
static PetscErrorCode gs_gop_vec_local_plus(gs_id *gs, PetscScalar *vals, int step);
static PetscErrorCode gs_gop_vec_local_in_plus(gs_id *gs, PetscScalar *vals, int step);
static PetscErrorCode gs_gop_vec_tree_plus(gs_id *gs, PetscScalar *vals, int step);


static PetscErrorCode gs_gop_plus(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_pairwise_plus(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_local_plus(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_local_in_plus(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_tree_plus(gs_id *gs, PetscScalar *vals);

static PetscErrorCode gs_gop_plus_hc(gs_id *gs, PetscScalar *in_vals, int dim);
static PetscErrorCode gs_gop_pairwise_plus_hc(gs_id *gs, PetscScalar *in_vals, int dim);
static PetscErrorCode gs_gop_tree_plus_hc(gs_id *gs, PetscScalar *vals, int dim);

static PetscErrorCode gs_gop_times(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_pairwise_times(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_local_times(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_local_in_times(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_tree_times(gs_id *gs, PetscScalar *vals);

static PetscErrorCode gs_gop_min(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_pairwise_min(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_local_min(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_local_in_min(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_tree_min(gs_id *gs, PetscScalar *vals);

static PetscErrorCode gs_gop_min_abs(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_pairwise_min_abs(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_local_min_abs(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_local_in_min_abs(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_tree_min_abs(gs_id *gs, PetscScalar *vals);

static PetscErrorCode gs_gop_max(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_pairwise_max(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_local_max(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_local_in_max(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_tree_max(gs_id *gs, PetscScalar *vals);

static PetscErrorCode gs_gop_max_abs(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_pairwise_max_abs(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_local_max_abs(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_local_in_max_abs(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_tree_max_abs(gs_id *gs, PetscScalar *vals);

static PetscErrorCode gs_gop_exists(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_pairwise_exists(gs_id *gs, PetscScalar *in_vals);
static PetscErrorCode gs_gop_local_exists(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_local_in_exists(gs_id *gs, PetscScalar *vals);
static PetscErrorCode gs_gop_tree_exists(gs_id *gs, PetscScalar *vals);

static PetscErrorCode gs_gop_pairwise_binary(gs_id *gs, PetscScalar *in_vals, rbfp fct);
static PetscErrorCode gs_gop_local_binary(gs_id *gs, PetscScalar *vals, rbfp fct);
static PetscErrorCode gs_gop_local_in_binary(gs_id *gs, PetscScalar *vals, rbfp fct);
static PetscErrorCode gs_gop_tree_binary(gs_id *gs, PetscScalar *vals, rbfp fct);



/* global vars */
/* from comm.c module */

/* module state inf and fortran interface */
static int num_gs_ids = 0;

/* should make this dynamic ... later */
static int msg_buf=MAX_MSG_BUF;
static int vec_sz=GS_VEC_SZ;
static int *tree_buf=NULL;
static int tree_buf_sz=0;
static int ntree=0;


/******************************************************************************
Function: gs_init_()

Input : 
Output: 
Return: 
Description:  
******************************************************************************/
PetscErrorCode gs_init_vec_sz(int size)
{
  PetscFunctionBegin;
  vec_sz = size;
  PetscFunctionReturn(0);
}

/******************************************************************************
Function: gs_init_()

Input : 
Output: 
Return: 
Description:  
******************************************************************************/
PetscErrorCode gs_init_msg_buf_sz(int buf_size)
{
  PetscFunctionBegin;
  msg_buf = buf_size;
  PetscFunctionReturn(0);
}

/******************************************************************************
Function: gs_init()

Input : 

Output: 

RETURN: 

Description:  
******************************************************************************/
gs_id *gs_init( int *elms, int nel, int level)
{
   gs_id *gs;
  MPI_Group gs_group;
  MPI_Comm  gs_comm;

  PetscFunctionBegin;
  /* ensure that communication package has been initialized */
  comm_init();


  /* determines if we have enough dynamic/semi-static memory */
  /* checks input, allocs and sets gd_id template            */
  gs = gsi_check_args(elms,nel,level);

  /* only bit mask version up and working for the moment    */
  /* LATER :: get int list version working for sparse pblms */
  gsi_via_bit_mask(gs);


  MPI_Comm_group(MPI_COMM_WORLD,&gs_group);
  MPI_Comm_create(MPI_COMM_WORLD,gs_group,&gs_comm);
  gs->gs_comm=gs_comm;

  return(gs);
}



/******************************************************************************
Function: gsi_new()

Input : 
Output: 
Return: 
Description: 

elm list must >= 0!!!
elm repeats allowed
******************************************************************************/
static gs_id *gsi_new(void)
{
  gs_id *gs;
  gs = (gs_id *) malloc(sizeof(gs_id));
  PetscMemzero(gs,sizeof(gs_id));
  return(gs);
}



/******************************************************************************
Function: gsi_check_args()

Input : 
Output: 
Return: 
Description: 

elm list must >= 0!!!
elm repeats allowed
local working copy of elms is sorted
******************************************************************************/
static gs_id * gsi_check_args(int *in_elms, int nel, int level)
{
   int i, j, k, t2;
  int *companion, *elms, *unique, *iptr;
  int num_local=0, *num_to_reduce, **local_reduce;
  int oprs[] = {NON_UNIFORM,GL_MIN,GL_MAX,GL_ADD,GL_MIN,GL_MAX,GL_MIN,GL_B_AND};
  int vals[sizeof(oprs)/sizeof(oprs[0])-1];
  int work[sizeof(oprs)/sizeof(oprs[0])-1];
  gs_id *gs;



  if (!in_elms)
    {error_msg_fatal("elms point to nothing!!!\n");}

  if (nel<0)
    {error_msg_fatal("can't have fewer than 0 elms!!!\n");}

  if (nel==0)
    {error_msg_warning("I don't have any elements!!!\n");}

  /* get space for gs template */
  gs = gsi_new();
  gs->id = ++num_gs_ids;

  /* hmt 6.4.99                                            */
  /* caller can set global ids that don't participate to 0 */
  /* gs_init ignores all zeros in elm list                 */
  /* negative global ids are still invalid                 */
  for (i=j=0;i<nel;i++)
    {if (in_elms[i]!=0) {j++;}}

  k=nel; nel=j;

  /* copy over in_elms list and create inverse map */
  elms = (int*) malloc((nel+1)*sizeof(PetscInt));
  companion = (int*) malloc(nel*sizeof(PetscInt));
  /* ivec_c_index(companion,nel); */
  /* ivec_copy(elms,in_elms,nel); */
  for (i=j=0;i<k;i++)
    {
      if (in_elms[i]!=0)
        {elms[j] = in_elms[i]; companion[j++] = i;}
    }

  if (j!=nel)
    {error_msg_fatal("nel j mismatch!\n");}

  /* pre-pass ... check to see if sorted */
  elms[nel] = INT_MAX;
  iptr = elms;
  unique = elms+1;
  j=0;
  while (*iptr!=INT_MAX)
    {
      if (*iptr++>*unique++) 
        {j=1; break;}
    }

  /* set up inverse map */  
  if (j)
    {
      error_msg_warning("gsi_check_args() :: elm list *not* sorted!\n");
      SMI_sort((void*)elms, (void*)companion, nel, SORT_INTEGER);
    }
  else
    {error_msg_warning("gsi_check_args() :: elm list sorted!\n");}
  elms[nel] = INT_MIN;

  /* first pass */
  /* determine number of unique elements, check pd */
  for (i=k=0;i<nel;i+=j)
    {
      t2 = elms[i];
      j=++i;
     
      /* clump 'em for now */ 
      while (elms[j]==t2) {j++;}
      
      /* how many together and num local */
      if (j-=i)
        {num_local++; k+=j;}
    }

  /* how many unique elements? */
  gs->repeats=k;
  gs->nel = nel-k;


  /* number of repeats? */
  gs->num_local = num_local;
  num_local+=2;
  gs->local_reduce=local_reduce=(int **)malloc(num_local*sizeof(PetscInt*));
  gs->num_local_reduce=num_to_reduce=(int*) malloc(num_local*sizeof(PetscInt));

  unique = (int*) malloc((gs->nel+1)*sizeof(PetscInt));
  gs->elms = unique; 
  gs->nel_total = nel;
  gs->local_elms = elms;
  gs->companion = companion;

  /* compess map as well as keep track of local ops */
  for (num_local=i=j=0;i<gs->nel;i++)
    {
      k=j;
      t2 = unique[i] = elms[j];
      companion[i] = companion[j];
     
      while (elms[j]==t2) {j++;}

      if ((t2=(j-k))>1)
        {
          /* number together */
          num_to_reduce[num_local] = t2++;
          iptr = local_reduce[num_local++] = (int*)malloc(t2*sizeof(PetscInt));

          /* to use binary searching don't remap until we check intersection */
          *iptr++ = i;
          
          /* note that we're skipping the first one */
          while (++k<j)
            {*(iptr++) = companion[k];}
          *iptr = -1;
        }
    }

  /* sentinel for ngh_buf */
  unique[gs->nel]=INT_MAX;

  /* for two partition sort hack */
  num_to_reduce[num_local] = 0;
  local_reduce[num_local] = NULL;
  num_to_reduce[++num_local] = 0;
  local_reduce[num_local] = NULL;

  /* load 'em up */
  /* note one extra to hold NON_UNIFORM flag!!! */
  vals[2] = vals[1] = vals[0] = nel;
  if (gs->nel>0)
    {
       vals[3] = unique[0];           /* ivec_lb(elms,nel); */
       vals[4] = unique[gs->nel-1];   /* ivec_ub(elms,nel); */       
    }
  else
    {
       vals[3] = INT_MAX;             /* ivec_lb(elms,nel); */
       vals[4] = INT_MIN;             /* ivec_ub(elms,nel); */       
    }
  vals[5] = level;
  vals[6] = num_gs_ids;

  /* GLOBAL: send 'em out */
  giop(vals,work,sizeof(oprs)/sizeof(oprs[0])-1,oprs);

  /* must be semi-pos def - only pairwise depends on this */
  /* LATER - remove this restriction */
  if (vals[3]<0)
    {error_msg_fatal("gsi_check_args() :: system not semi-pos def ::%d\n",vals[3]);}

  if (vals[4]==INT_MAX)
    {error_msg_fatal("gsi_check_args() :: system ub too large ::%d!\n",vals[4]);}

  gs->nel_min = vals[0];
  gs->nel_max = vals[1];
  gs->nel_sum = vals[2];
  gs->gl_min  = vals[3];
  gs->gl_max  = vals[4];
  gs->negl    = vals[4]-vals[3]+1;

  if (gs->negl<=0)
    {error_msg_fatal("gsi_check_args() :: system empty or neg :: %d\n",gs->negl);}
  
  /* LATER :: add level == -1 -> program selects level */
  if (vals[5]<0)
    {vals[5]=0;}
  else if (vals[5]>num_nodes)
    {vals[5]=num_nodes;}
  gs->level = vals[5];

  return(gs);
}


/******************************************************************************
Function: gsi_via_bit_mask()

Input : 
Output: 
Return: 
Description: 


******************************************************************************/
static PetscErrorCode gsi_via_bit_mask(gs_id *gs)
{
   int i, nel, *elms;
  int t1;
  int **reduce;
  int *map;

  /* totally local removes ... ct_bits == 0 */
  get_ngh_buf(gs);

  if (gs->level)
    {set_pairwise(gs);}

  if (gs->max_left_over)
    {set_tree(gs);}

  /* intersection local and pairwise/tree? */
  gs->num_local_total = gs->num_local;
  gs->gop_local_reduce = gs->local_reduce;
  gs->num_gop_local_reduce = gs->num_local_reduce;

  map = gs->companion;

  /* is there any local compression */
  if (!gs->num_local) {
    gs->local_strength = NONE;
    gs->num_local_gop = 0;
  } else {
      /* ok find intersection */
      map = gs->companion;
      reduce = gs->local_reduce;  
      for (i=0, t1=0; i<gs->num_local; i++, reduce++)
        {
          if ((ivec_binary_search(**reduce,gs->pw_elm_list,gs->len_pw_list)>=0)
              ||
              ivec_binary_search(**reduce,gs->tree_map_in,gs->tree_map_sz)>=0)
            {
              /* printf("C%d :: i=%d, **reduce=%d\n",my_id,i,**reduce); */
              t1++; 
              if (gs->num_local_reduce[i]<=0)
                {error_msg_fatal("nobody in list?");}
              gs->num_local_reduce[i] *= -1;
            }
           **reduce=map[**reduce];
        }

      /* intersection is empty */
      if (!t1)
        {
          gs->local_strength = FULL;
          gs->num_local_gop = 0;          
        }
      /* intersection not empty */
      else
        {
          gs->local_strength = PARTIAL;
          SMI_sort((void*)gs->num_local_reduce, (void*)gs->local_reduce, 
                   gs->num_local + 1, SORT_INT_PTR);

          gs->num_local_gop = t1;
          gs->num_local_total =  gs->num_local;
          gs->num_local    -= t1;
          gs->gop_local_reduce = gs->local_reduce;
          gs->num_gop_local_reduce = gs->num_local_reduce;

          for (i=0; i<t1; i++)
            {
              if (gs->num_gop_local_reduce[i]>=0)
                {error_msg_fatal("they aren't negative?");}
              gs->num_gop_local_reduce[i] *= -1;
              gs->local_reduce++;
              gs->num_local_reduce++;
            }
          gs->local_reduce++;
          gs->num_local_reduce++;
        }
    }

  elms = gs->pw_elm_list;
  nel  = gs->len_pw_list;
  for (i=0; i<nel; i++)
    {elms[i] = map[elms[i]];}

  elms = gs->tree_map_in;
  nel  = gs->tree_map_sz;
  for (i=0; i<nel; i++)
    {elms[i] = map[elms[i]];}

  /* clean up */
  free((void*) gs->local_elms);
  free((void*) gs->companion);
  free((void*) gs->elms);
  free((void*) gs->ngh_buf);
  gs->local_elms = gs->companion = gs->elms = gs->ngh_buf = NULL;
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: place_in_tree()

Input : 
Output: 
Return: 
Description: 


******************************************************************************/
static PetscErrorCode place_in_tree( int elm)
{
   int *tp, n;

  PetscFunctionBegin;
  if (ntree==tree_buf_sz)
    {
      if (tree_buf_sz)
        {
          tp = tree_buf;
          n = tree_buf_sz;
          tree_buf_sz<<=1;
          tree_buf = (int*)malloc(tree_buf_sz*sizeof(PetscInt));
          ivec_copy(tree_buf,tp,n);
          free(tp);
        }
      else
        {
          tree_buf_sz = TREE_BUF_SZ;
          tree_buf = (int*)malloc(tree_buf_sz*sizeof(PetscInt));
        }
    }

  tree_buf[ntree++] = elm;
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: get_ngh_buf()

Input : 
Output: 
Return: 
Description: 


******************************************************************************/
static PetscErrorCode get_ngh_buf(gs_id *gs)
{
   int i, j, npw=0, ntree_map=0;
  int p_mask_size, ngh_buf_size, buf_size;
  int *p_mask, *sh_proc_mask, *pw_sh_proc_mask;
  int *ngh_buf, *buf1, *buf2;
  int offset, per_load, num_loads, or_ct, start, end;
  int *ptr1, *ptr2, i_start, negl, nel, *elms;
  int oper=GL_B_OR;
  int *ptr3, *t_mask, level, ct1, ct2;

  PetscFunctionBegin;
  /* to make life easier */
  nel   = gs->nel;
  elms  = gs->elms;
  level = gs->level;
  
  /* det #bytes needed for processor bit masks and init w/mask cor. to my_id */
  p_mask = (int*) malloc(p_mask_size=len_bit_mask(num_nodes));
  set_bit_mask(p_mask,p_mask_size,my_id);

  /* allocate space for masks and info bufs */
  gs->nghs = sh_proc_mask = (int*) malloc(p_mask_size);
  gs->pw_nghs = pw_sh_proc_mask = (int*) malloc(p_mask_size);
  gs->ngh_buf_sz = ngh_buf_size = p_mask_size*nel;
  t_mask = (int*) malloc(p_mask_size);
  gs->ngh_buf = ngh_buf = (int*) malloc(ngh_buf_size);

  /* comm buffer size ... memory usage bounded by ~2*msg_buf */
  /* had thought I could exploit rendezvous threshold */

  /* default is one pass */
  per_load = negl  = gs->negl;
  gs->num_loads = num_loads = 1;
  i=p_mask_size*negl;

  /* possible overflow on buffer size */
  /* overflow hack                    */
  if (i<0) {i=INT_MAX;}

  buf_size = PetscMin(msg_buf,i);

  /* can we do it? */
  if (p_mask_size>buf_size)
    {error_msg_fatal("get_ngh_buf() :: buf<pms :: %d>%d\n",p_mask_size,buf_size);}

  /* get giop buf space ... make *only* one malloc */
  buf1 = (int*) malloc(buf_size<<1);

  /* more than one gior exchange needed? */
  if (buf_size!=i)
    {
      per_load = buf_size/p_mask_size;
      buf_size = per_load*p_mask_size;
      gs->num_loads = num_loads = negl/per_load + (negl%per_load>0);
    }


  /* convert buf sizes from #bytes to #ints - 32 bit only! */
  p_mask_size/=sizeof(PetscInt); ngh_buf_size/=sizeof(PetscInt); buf_size/=sizeof(PetscInt);
  
  /* find giop work space */
  buf2 = buf1+buf_size;

  /* hold #ints needed for processor masks */
  gs->mask_sz=p_mask_size;

  /* init buffers */ 
  ivec_zero(sh_proc_mask,p_mask_size);
  ivec_zero(pw_sh_proc_mask,p_mask_size);
  ivec_zero(ngh_buf,ngh_buf_size);

  /* HACK reset tree info */
  tree_buf=NULL;
  tree_buf_sz=ntree=0;

  /* queue the tree elements for now */
  /* elms_q = new_queue(); */
  
  /* can also queue tree info for pruned or forest implememtation */
  /*  mask_q = new_queue(); */

  /* ok do it */
  for (ptr1=ngh_buf,ptr2=elms,end=gs->gl_min,or_ct=i=0; or_ct<num_loads; or_ct++)
    {
      /* identity for bitwise or is 000...000 */
      ivec_zero(buf1,buf_size);

      /* load msg buffer */
      for (start=end,end+=per_load,i_start=i; (offset=*ptr2)<end; i++, ptr2++)
        {
          offset = (offset-start)*p_mask_size;
          ivec_copy(buf1+offset,p_mask,p_mask_size); 
        }

      /* GLOBAL: pass buffer */
      giop(buf1,buf2,buf_size,&oper);


      /* unload buffer into ngh_buf */
      ptr2=(elms+i_start);
      for(ptr3=buf1,j=start; j<end; ptr3+=p_mask_size,j++)
        {
          /* I own it ... may have to pairwise it */
          if (j==*ptr2)
            {
              /* do i share it w/anyone? */
              ct1 = ct_bits((char *)ptr3,p_mask_size*sizeof(PetscInt));
              /* guess not */
              if (ct1<2)
                {ptr2++; ptr1+=p_mask_size; continue;}

              /* i do ... so keep info and turn off my bit */
              ivec_copy(ptr1,ptr3,p_mask_size);
              ivec_xor(ptr1,p_mask,p_mask_size);
              ivec_or(sh_proc_mask,ptr1,p_mask_size);
              
              /* is it to be done pairwise? */
              if (--ct1<=level)
                {
                  npw++;
                  
                  /* turn on high bit to indicate pw need to process */
                  *ptr2++ |= TOP_BIT; 
                  ivec_or(pw_sh_proc_mask,ptr1,p_mask_size);
                  ptr1+=p_mask_size; 
                  continue;
                }

              /* get set for next and note that I have a tree contribution */
              /* could save exact elm index for tree here -> save a search */
              ptr2++; ptr1+=p_mask_size; ntree_map++;
            }
          /* i don't but still might be involved in tree */
          else
            {

              /* shared by how many? */
              ct1 = ct_bits((char *)ptr3,p_mask_size*sizeof(PetscInt));

              /* none! */
              if (ct1<2)
                {continue;}

              /* is it going to be done pairwise? but not by me of course!*/
              if (--ct1<=level)
                {continue;}
            }
          /* LATER we're going to have to process it NOW */
          /* nope ... tree it */
          place_in_tree(j);
        }
    }

  free((void*)t_mask);
  free((void*)buf1);

  gs->len_pw_list=npw;
  gs->num_nghs = ct_bits((char *)sh_proc_mask,p_mask_size*sizeof(PetscInt));

  /* expand from bit mask list to int list and save ngh list */
  gs->nghs = (int*) malloc(gs->num_nghs * sizeof(PetscInt));
  bm_to_proc((char *)sh_proc_mask,p_mask_size*sizeof(PetscInt),gs->nghs);

  gs->num_pw_nghs = ct_bits((char *)pw_sh_proc_mask,p_mask_size*sizeof(PetscInt));

  oper = GL_MAX;
  ct1 = gs->num_nghs;
  giop(&ct1,&ct2,1,&oper);
  gs->max_nghs = ct1;

  gs->tree_map_sz  = ntree_map;
  gs->max_left_over=ntree;

  free((void*)p_mask);
  free((void*)sh_proc_mask);
  PetscFunctionReturn(0);
}





/******************************************************************************
Function: pairwise_init()

Input : 
Output: 
Return: 
Description: 

if an element is shared by fewer that level# of nodes do pairwise exch 
******************************************************************************/
static PetscErrorCode set_pairwise(gs_id *gs)
{
   int i, j;
  int p_mask_size;
  int *p_mask, *sh_proc_mask, *tmp_proc_mask;
  int *ngh_buf, *buf2;
  int offset;
  int *msg_list, *msg_size, **msg_nodes, nprs;
  int *pairwise_elm_list, len_pair_list=0;
  int *iptr, t1, i_start, nel, *elms;
  int ct;


  PetscFunctionBegin;
  /* to make life easier */
  nel  = gs->nel;
  elms = gs->elms;
  ngh_buf = gs->ngh_buf;
  sh_proc_mask  = gs->pw_nghs;

  /* need a few temp masks */
  p_mask_size   = len_bit_mask(num_nodes);
  p_mask        = (int*) malloc(p_mask_size);
  tmp_proc_mask = (int*) malloc(p_mask_size);

  /* set mask to my my_id's bit mask */
  set_bit_mask(p_mask,p_mask_size,my_id);

  p_mask_size /= sizeof(PetscInt);
          
  len_pair_list=gs->len_pw_list;
  gs->pw_elm_list=pairwise_elm_list=(int*)malloc((len_pair_list+1)*sizeof(PetscInt));

  /* how many processors (nghs) do we have to exchange with? */
  nprs=gs->num_pairs=ct_bits((char *)sh_proc_mask,p_mask_size*sizeof(PetscInt));


  /* allocate space for gs_gop() info */
  gs->pair_list = msg_list = (int*)  malloc(sizeof(PetscInt)*nprs);
  gs->msg_sizes = msg_size  = (int*)  malloc(sizeof(PetscInt)*nprs);
  gs->node_list = msg_nodes = (int **) malloc(sizeof(PetscInt*)*(nprs+1));

  /* init msg_size list */
  ivec_zero(msg_size,nprs);  

  /* expand from bit mask list to int list */
  bm_to_proc((char *)sh_proc_mask,p_mask_size*sizeof(PetscInt),msg_list);
  
  /* keep list of elements being handled pairwise */
  for (i=j=0;i<nel;i++)
    {
      if (elms[i] & TOP_BIT)
        {elms[i] ^= TOP_BIT; pairwise_elm_list[j++] = i;}
    }
  pairwise_elm_list[j] = -1;

  gs->msg_ids_out = (MPI_Request *)  malloc(sizeof(MPI_Request)*(nprs+1));
  gs->msg_ids_out[nprs] = MPI_REQUEST_NULL;
  gs->msg_ids_in = (MPI_Request *)  malloc(sizeof(MPI_Request)*(nprs+1));
  gs->msg_ids_in[nprs] = MPI_REQUEST_NULL;
  gs->pw_vals = (PetscScalar *) malloc(sizeof(PetscScalar)*len_pair_list*vec_sz);

  /* find who goes to each processor */
  for (i_start=i=0;i<nprs;i++)
    {
      /* processor i's mask */
      set_bit_mask(p_mask,p_mask_size*sizeof(PetscInt),msg_list[i]);

      /* det # going to processor i */
      for (ct=j=0;j<len_pair_list;j++)
        {
          buf2 = ngh_buf+(pairwise_elm_list[j]*p_mask_size);
          ivec_and3(tmp_proc_mask,p_mask,buf2,p_mask_size);
          if (ct_bits((char *)tmp_proc_mask,p_mask_size*sizeof(PetscInt)))
            {ct++;}
        }
      msg_size[i] = ct;
      i_start = PetscMax(i_start,ct);

      /*space to hold nodes in message to first neighbor */
      msg_nodes[i] = iptr = (int*) malloc(sizeof(PetscInt)*(ct+1));

      for (j=0;j<len_pair_list;j++)
        {
          buf2 = ngh_buf+(pairwise_elm_list[j]*p_mask_size);
          ivec_and3(tmp_proc_mask,p_mask,buf2,p_mask_size);
          if (ct_bits((char *)tmp_proc_mask,p_mask_size*sizeof(PetscInt)))
            {*iptr++ = j;}
        }
      *iptr = -1;
    }
  msg_nodes[nprs] = NULL;

  j=gs->loc_node_pairs=i_start;
  t1 = GL_MAX;
  giop(&i_start,&offset,1,&t1);
  gs->max_node_pairs = i_start;

  i_start=j;
  t1 = GL_MIN;
  giop(&i_start,&offset,1,&t1);
  gs->min_node_pairs = i_start;

  i_start=j;
  t1 = GL_ADD;
  giop(&i_start,&offset,1,&t1);
  gs->avg_node_pairs = i_start/num_nodes + 1;

  i_start=nprs;
  t1 = GL_MAX;
  giop(&i_start,&offset,1,&t1);
  gs->max_pairs = i_start;


  /* remap pairwise in tail of gsi_via_bit_mask() */
  gs->msg_total = ivec_sum(gs->msg_sizes,nprs);
  gs->out = (PetscScalar *) malloc(sizeof(PetscScalar)*gs->msg_total*vec_sz);
  gs->in  = (PetscScalar *) malloc(sizeof(PetscScalar)*gs->msg_total*vec_sz);

  /* reset malloc pool */
  free((void*)p_mask);
  free((void*)tmp_proc_mask);
  PetscFunctionReturn(0);
}    



/******************************************************************************
Function: set_tree()

Input : 
Output: 
Return: 
Description: 

to do pruned tree just save ngh buf copy for each one and decode here!
******************************************************************************/
static PetscErrorCode set_tree(gs_id *gs)
{
   int i, j, n, nel;
   int *iptr_in, *iptr_out, *tree_elms, *elms;

  PetscFunctionBegin;
  /* local work ptrs */
  elms = gs->elms;
  nel     = gs->nel;

  /* how many via tree */
  gs->tree_nel  = n = ntree;
  gs->tree_elms = tree_elms = iptr_in = tree_buf;
  gs->tree_buf  = (PetscScalar *) malloc(sizeof(PetscScalar)*n*vec_sz);
  gs->tree_work = (PetscScalar *) malloc(sizeof(PetscScalar)*n*vec_sz);
  j=gs->tree_map_sz;
  gs->tree_map_in = iptr_in  = (int*) malloc(sizeof(PetscInt)*(j+1));
  gs->tree_map_out = iptr_out = (int*) malloc(sizeof(PetscInt)*(j+1));

  /* search the longer of the two lists */
  /* note ... could save this info in get_ngh_buf and save searches */
  if (n<=nel)
    {
      /* bijective fct w/remap - search elm list */
      for (i=0; i<n; i++)
        {
          if ((j=ivec_binary_search(*tree_elms++,elms,nel))>=0)
            {*iptr_in++ = j; *iptr_out++ = i;} 
        }
    }
  else
    {
      for (i=0; i<nel; i++)
        {
          if ((j=ivec_binary_search(*elms++,tree_elms,n))>=0) 
            {*iptr_in++ = i; *iptr_out++ = j;}
        }
    }

  /* sentinel */
  *iptr_in = *iptr_out = -1;
  PetscFunctionReturn(0);
}


/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_out( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar tmp;

  PetscFunctionBegin;
  num    = gs->num_gop_local_reduce;  
  reduce = gs->gop_local_reduce;  
  while ((map = *reduce++))
    {
      /* wall */
      if (*num == 2)
        {
          num ++;
          vals[map[1]] = vals[map[0]];
        }
      /* corner shared by three elements */
      else if (*num == 3)
        {
          num ++;
          vals[map[2]] = vals[map[1]] = vals[map[0]];
        }
      /* corner shared by four elements */
      else if (*num == 4)
        {
          num ++;
          vals[map[3]] = vals[map[2]] = vals[map[1]] = vals[map[0]];
        }
      /* general case ... odd geoms ... 3D*/
      else
        {
          num++;
          tmp = *(vals + *map++);
          while (*map >= 0)
            {*(vals + *map++) = tmp;}
        }
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
PetscErrorCode gs_gop_binary(gs_ADT gs, PetscScalar *vals, rbfp fct)
{
  PetscFunctionBegin;
  /* local only operations!!! */
  if (gs->num_local)
    {gs_gop_local_binary(gs,vals,fct);}
  
  /* if intersection tree/pairwise and local isn't empty */
  if (gs->num_local_gop)
    {
      gs_gop_local_in_binary(gs,vals,fct);
      
      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_binary(gs,vals,fct);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_binary(gs,vals,fct);}
      
      gs_gop_local_out(gs,vals);
    }
  /* if intersection tree/pairwise and local is empty */
  else
    {
      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_binary(gs,vals,fct);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_binary(gs,vals,fct);}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_binary( gs_id *gs,  PetscScalar *vals,  rbfp fct)
{
   int *num, *map, **reduce;
  PetscScalar tmp;

  PetscFunctionBegin;
  num    = gs->num_local_reduce;  
  reduce = gs->local_reduce;  
  while ((map = *reduce))
    {
      num ++;
      (*fct)(&tmp,NULL,1);
      /* tmp = 0.0; */
      while (*map >= 0)
        {(*fct)(&tmp,(vals + *map),1); map++;}
        /*        {tmp = (*fct)(tmp,*(vals + *map)); map++;} */
      
      map = *reduce++;
      while (*map >= 0)
        {*(vals + *map++) = tmp;}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_in_binary( gs_id *gs,  PetscScalar *vals,  rbfp fct) 
{
   int *num, *map, **reduce;
   PetscScalar *base;

  PetscFunctionBegin;
  num    = gs->num_gop_local_reduce;  

  reduce = gs->gop_local_reduce;  
  while ((map = *reduce++))
    {
      num++;
      base = vals + *map++;
      while (*map >= 0)
        {(*fct)(base,(vals + *map),1); map++;}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

VERSION 3 :: 

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_pairwise_binary( gs_id *gs,  PetscScalar *in_vals,
                        rbfp fct)
{
   PetscScalar *dptr1, *dptr2, *dptr3, *in1, *in2;
   int *iptr, *msg_list, *msg_size, **msg_nodes;
   int *pw, *list, *size, **nodes;
  MPI_Request *msg_ids_in, *msg_ids_out, *ids_in, *ids_out;
  MPI_Status status;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* strip and load s */
  msg_list =list         = gs->pair_list;
  msg_size =size         = gs->msg_sizes;
  msg_nodes=nodes        = gs->node_list;
  iptr=pw                = gs->pw_elm_list;  
  dptr1=dptr3            = gs->pw_vals;
  msg_ids_in  = ids_in   = gs->msg_ids_in;
  msg_ids_out = ids_out  = gs->msg_ids_out;
  dptr2                  = gs->out;
  in1=in2                = gs->in;

  /* post the receives */
  /*  msg_nodes=nodes; */
  do 
    {
      /* Should MPI_ANY_SOURCE be replaced by *list ? In that case do the
         second one *list and do list++ afterwards */
      ierr = MPI_Irecv(in1, *size, MPIU_SCALAR, MPI_ANY_SOURCE, MSGTAG1 + *list++, gs->gs_comm, msg_ids_in++);CHKERRQ(ierr); 
      in1 += *size++;
    }
  while (*++msg_nodes);
  msg_nodes=nodes;

  /* load gs values into in out gs buffers */  
  while (*iptr >= 0)
    {*dptr3++ = *(in_vals + *iptr++);}

  /* load out buffers and post the sends */
  while ((iptr = *msg_nodes++))
    {
      dptr3 = dptr2;
      while (*iptr >= 0)
        {*dptr2++ = *(dptr1 + *iptr++);}
      /* CHECK PERSISTENT COMMS MODE FOR ALL THIS STUFF */
      /* is msg_ids_out++ correct? */
      ierr = MPI_Isend(dptr3, *msg_size++, MPIU_SCALAR, *msg_list++, MSGTAG1+my_id, gs->gs_comm, msg_ids_out++);CHKERRQ(ierr);
    }

  if (gs->max_left_over)
    {gs_gop_tree_binary(gs,in_vals,fct);}

  /* process the received data */
  msg_nodes=nodes;
  while ((iptr = *nodes++))
    {
      /* Should I check the return value of MPI_Wait() or status? */
      /* Can this loop be replaced by a call to MPI_Waitall()? */
      ierr = MPI_Wait(ids_in++, &status);CHKERRQ(ierr);
      while (*iptr >= 0)
        {(*fct)((dptr1 + *iptr),in2,1); iptr++; in2++;}
      /* {*(dptr1 + *iptr) = (*fct)(*(dptr1 + *iptr),*in2); iptr++; in2++;} */
    }

  /* replace vals */
  while (*pw >= 0)
    {*(in_vals + *pw++) = *dptr1++;}

  /* clear isend message handles */
  /* This changed for clarity though it could be the same */
  while (*msg_nodes++)
    /* Should I check the return value of MPI_Wait() or status? */
    /* Can this loop be replaced by a call to MPI_Waitall()? */
    {ierr = MPI_Wait(ids_out++, &status);CHKERRQ(ierr);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_tree_binary(gs_id *gs, PetscScalar *vals,  rbfp fct)
{
  int size;
  int *in, *out;  
  PetscScalar *buf, *work;

  PetscFunctionBegin;
  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  buf  = gs->tree_buf;
  work = gs->tree_work;
  size = gs->tree_nel;

  /* load vals vector w/identity */
  (*fct)(buf,NULL,size);
  
  /* load my contribution into val vector */
  while (*in >= 0)
    {(*fct)((buf + *out++),(vals + *in++),-1);}

  gfop(buf,work,size,(vbfp)fct,MPIU_SCALAR,0);

  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  while (*in >= 0)
    {(*fct)((vals + *in++),(buf + *out++),-1);}
  PetscFunctionReturn(0);
}




/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
PetscErrorCode gs_gop( gs_id *gs,  PetscScalar *vals,  const char *op)
{
  PetscFunctionBegin;
  switch (*op) {
  case '+':
    gs_gop_plus(gs,vals);
    break;
  case '*':
    gs_gop_times(gs,vals);
    break;
  case 'a':
    gs_gop_min_abs(gs,vals);
    break;
  case 'A':
    gs_gop_max_abs(gs,vals);
    break;
  case 'e':
    gs_gop_exists(gs,vals);
    break;
  case 'm':
    gs_gop_min(gs,vals);
    break;
  case 'M':
    gs_gop_max(gs,vals); break;
    /*
    if (*(op+1)=='\0')
      {gs_gop_max(gs,vals); break;}
    else if (*(op+1)=='X')
      {gs_gop_max_abs(gs,vals); break;}
    else if (*(op+1)=='N')
      {gs_gop_min_abs(gs,vals); break;}
    */
  default:
    error_msg_warning("gs_gop() :: %c is not a valid op",op[0]);
    error_msg_warning("gs_gop() :: default :: plus");
    gs_gop_plus(gs,vals);    
    break;
  }
  PetscFunctionReturn(0);
}


/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_exists( gs_id *gs,  PetscScalar *vals)
{
  PetscFunctionBegin;
  /* local only operations!!! */
  if (gs->num_local)
    {gs_gop_local_exists(gs,vals);}

  /* if intersection tree/pairwise and local isn't empty */
  if (gs->num_local_gop)
    {
      gs_gop_local_in_exists(gs,vals);

      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_exists(gs,vals);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_exists(gs,vals);}
  
      gs_gop_local_out(gs,vals);
    }
  /* if intersection tree/pairwise and local is empty */
  else
    {
      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_exists(gs,vals);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_exists(gs,vals);}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_exists( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar tmp;

  PetscFunctionBegin;
  num    = gs->num_local_reduce;  
  reduce = gs->local_reduce;  
  while ((map = *reduce))
    {
      num ++;
      tmp = 0.0;
      while (*map >= 0)
        {tmp = EXISTS(tmp,*(vals + *map)); map++;}
      
      map = *reduce++;
      while (*map >= 0)
        {*(vals + *map++) = tmp;}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_in_exists( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar *base;

  PetscFunctionBegin;
  num    = gs->num_gop_local_reduce;  
  reduce = gs->gop_local_reduce;  
  while ((map = *reduce++))
    {
      num++;
      base = vals + *map++;
      while (*map >= 0)
        {*base = EXISTS(*base,*(vals + *map)); map++;}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

VERSION 3 :: 

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_pairwise_exists( gs_id *gs,  PetscScalar *in_vals)
{
   PetscScalar *dptr1, *dptr2, *dptr3, *in1, *in2;
   int *iptr, *msg_list, *msg_size, **msg_nodes;
   int *pw, *list, *size, **nodes;
  MPI_Request *msg_ids_in, *msg_ids_out, *ids_in, *ids_out;
  MPI_Status status;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* strip and load s */
  msg_list =list         = gs->pair_list;
  msg_size =size         = gs->msg_sizes;
  msg_nodes=nodes        = gs->node_list;
  iptr=pw                = gs->pw_elm_list;  
  dptr1=dptr3            = gs->pw_vals;
  msg_ids_in  = ids_in   = gs->msg_ids_in;
  msg_ids_out = ids_out  = gs->msg_ids_out;
  dptr2                  = gs->out;
  in1=in2                = gs->in;

  /* post the receives */
  /*  msg_nodes=nodes; */
  do 
    {
      /* Should MPI_ANY_SOURCE be replaced by *list ? In that case do the
         second one *list and do list++ afterwards */
      ierr = MPI_Irecv(in1, *size, MPIU_SCALAR, MPI_ANY_SOURCE, MSGTAG1 + *list++, gs->gs_comm, msg_ids_in++);CHKERRQ(ierr);
      in1 += *size++;
    }
  while (*++msg_nodes);
  msg_nodes=nodes;

  /* load gs values into in out gs buffers */  
  while (*iptr >= 0)
    {*dptr3++ = *(in_vals + *iptr++);}

  /* load out buffers and post the sends */
  while ((iptr = *msg_nodes++))
    {
      dptr3 = dptr2;
      while (*iptr >= 0)
        {*dptr2++ = *(dptr1 + *iptr++);}
      /* CHECK PERSISTENT COMMS MODE FOR ALL THIS STUFF */
      /* is msg_ids_out++ correct? */
      ierr = MPI_Isend(dptr3, *msg_size++, MPIU_SCALAR, *msg_list++, MSGTAG1+my_id, gs->gs_comm, msg_ids_out++);CHKERRQ(ierr);
    }

  if (gs->max_left_over)
    {gs_gop_tree_exists(gs,in_vals);}

  /* process the received data */
  msg_nodes=nodes;
  while ((iptr = *nodes++))
    {
      /* Should I check the return value of MPI_Wait() or status? */
      /* Can this loop be replaced by a call to MPI_Waitall()? */
      ierr = MPI_Wait(ids_in++, &status);CHKERRQ(ierr);
      while (*iptr >= 0)
        {*(dptr1 + *iptr) = EXISTS(*(dptr1 + *iptr),*in2); iptr++; in2++;}
    }

  /* replace vals */
  while (*pw >= 0)
    {*(in_vals + *pw++) = *dptr1++;}

  /* clear isend message handles */
  /* This changed for clarity though it could be the same */
  while (*msg_nodes++)
    /* Should I check the return value of MPI_Wait() or status? */
    /* Can this loop be replaced by a call to MPI_Waitall()? */
    {ierr = MPI_Wait(ids_out++, &status);CHKERRQ(ierr);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_tree_exists(gs_id *gs, PetscScalar *vals)
{
  int size;
  int *in, *out;  
  PetscScalar *buf, *work;
  int op[] = {GL_EXISTS,0};

  PetscFunctionBegin;
  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  buf  = gs->tree_buf;
  work = gs->tree_work;
  size = gs->tree_nel;

  rvec_zero(buf,size);

  while (*in >= 0)
    { 
      /*
      printf("%d :: out=%d\n",my_id,*out);
      printf("%d :: in=%d\n",my_id,*in);
      */
      *(buf + *out++) = *(vals + *in++);
    }

  grop(buf,work,size,op);

  in   = gs->tree_map_in;
  out  = gs->tree_map_out;

  while (*in >= 0)
    {*(vals + *in++) = *(buf + *out++);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_max_abs( gs_id *gs,  PetscScalar *vals)
{
  PetscFunctionBegin;
  /* local only operations!!! */
  if (gs->num_local)
    {gs_gop_local_max_abs(gs,vals);}

  /* if intersection tree/pairwise and local isn't empty */
  if (gs->num_local_gop)
    {
      gs_gop_local_in_max_abs(gs,vals);

      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_max_abs(gs,vals);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_max_abs(gs,vals);}
  
      gs_gop_local_out(gs,vals);
    }
  /* if intersection tree/pairwise and local is empty */
  else
    {
      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_max_abs(gs,vals);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_max_abs(gs,vals);}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_max_abs( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar tmp;

  PetscFunctionBegin;
  num    = gs->num_local_reduce;  
  reduce = gs->local_reduce;  
  while ((map = *reduce))
    {
      num ++;
      tmp = 0.0;
      while (*map >= 0)
        {tmp = MAX_FABS(tmp,*(vals + *map)); map++;}
      
      map = *reduce++;
      while (*map >= 0)
        {*(vals + *map++) = tmp;}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_in_max_abs( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar *base;

  PetscFunctionBegin;
  num    = gs->num_gop_local_reduce;  
  reduce = gs->gop_local_reduce;  
  while ((map = *reduce++))
    {
      num++;
      base = vals + *map++;
      while (*map >= 0)
        {*base = MAX_FABS(*base,*(vals + *map)); map++;}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

VERSION 3 :: 

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_pairwise_max_abs( gs_id *gs,  PetscScalar *in_vals)
{
   PetscScalar *dptr1, *dptr2, *dptr3, *in1, *in2;
   int *iptr, *msg_list, *msg_size, **msg_nodes;
   int *pw, *list, *size, **nodes;
  MPI_Request *msg_ids_in, *msg_ids_out, *ids_in, *ids_out;
  MPI_Status status;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* strip and load s */
  msg_list =list         = gs->pair_list;
  msg_size =size         = gs->msg_sizes;
  msg_nodes=nodes        = gs->node_list;
  iptr=pw                = gs->pw_elm_list;  
  dptr1=dptr3            = gs->pw_vals;
  msg_ids_in  = ids_in   = gs->msg_ids_in;
  msg_ids_out = ids_out  = gs->msg_ids_out;
  dptr2                  = gs->out;
  in1=in2                = gs->in;

  /* post the receives */
  /*  msg_nodes=nodes; */
  do 
    {
      /* Should MPI_ANY_SOURCE be replaced by *list ? In that case do the
         second one *list and do list++ afterwards */
      ierr = MPI_Irecv(in1, *size, MPIU_SCALAR, MPI_ANY_SOURCE, MSGTAG1 + *list++, gs->gs_comm, msg_ids_in++);CHKERRQ(ierr);
      in1 += *size++;
    }
  while (*++msg_nodes);
  msg_nodes=nodes;

  /* load gs values into in out gs buffers */  
  while (*iptr >= 0)
    {*dptr3++ = *(in_vals + *iptr++);}

  /* load out buffers and post the sends */
  while ((iptr = *msg_nodes++))
    {
      dptr3 = dptr2;
      while (*iptr >= 0)
        {*dptr2++ = *(dptr1 + *iptr++);}
      /* CHECK PERSISTENT COMMS MODE FOR ALL THIS STUFF */
      /* is msg_ids_out++ correct? */
      ierr = MPI_Isend(dptr3, *msg_size++, MPIU_SCALAR, *msg_list++, MSGTAG1+my_id, gs->gs_comm, msg_ids_out++);CHKERRQ(ierr);
    }

  if (gs->max_left_over)
    {gs_gop_tree_max_abs(gs,in_vals);}

  /* process the received data */
  msg_nodes=nodes;
  while ((iptr = *nodes++))
    {
      /* Should I check the return value of MPI_Wait() or status? */
      /* Can this loop be replaced by a call to MPI_Waitall()? */
      ierr = MPI_Wait(ids_in++, &status);CHKERRQ(ierr);
      while (*iptr >= 0)
        {*(dptr1 + *iptr) = MAX_FABS(*(dptr1 + *iptr),*in2); iptr++; in2++;}
    }

  /* replace vals */
  while (*pw >= 0)
    {*(in_vals + *pw++) = *dptr1++;}

  /* clear isend message handles */
  /* This changed for clarity though it could be the same */
  while (*msg_nodes++)
    /* Should I check the return value of MPI_Wait() or status? */
    /* Can this loop be replaced by a call to MPI_Waitall()? */
    {ierr = MPI_Wait(ids_out++, &status);CHKERRQ(ierr);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_tree_max_abs(gs_id *gs, PetscScalar *vals)
{
  int size;
  int *in, *out;  
  PetscScalar *buf, *work;
  int op[] = {GL_MAX_ABS,0};

  PetscFunctionBegin;
  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  buf  = gs->tree_buf;
  work = gs->tree_work;
  size = gs->tree_nel;

  rvec_zero(buf,size);

  while (*in >= 0)
    { 
      /*
      printf("%d :: out=%d\n",my_id,*out);
      printf("%d :: in=%d\n",my_id,*in);
      */
      *(buf + *out++) = *(vals + *in++);
    }

  grop(buf,work,size,op);

  in   = gs->tree_map_in;
  out  = gs->tree_map_out;

  while (*in >= 0)
    {*(vals + *in++) = *(buf + *out++);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_max( gs_id *gs,  PetscScalar *vals)
{
  PetscFunctionBegin;
  /* local only operations!!! */
  if (gs->num_local)
    {gs_gop_local_max(gs,vals);}

  /* if intersection tree/pairwise and local isn't empty */
  if (gs->num_local_gop)
    {
      gs_gop_local_in_max(gs,vals);

      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_max(gs,vals);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_max(gs,vals);}
  
      gs_gop_local_out(gs,vals);
    }
  /* if intersection tree/pairwise and local is empty */
  else
    {
      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_max(gs,vals);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_max(gs,vals);}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_max( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar tmp;

  PetscFunctionBegin;
  num    = gs->num_local_reduce;  
  reduce = gs->local_reduce;  
  while ((map = *reduce))
    {
      num ++;
      tmp = -REAL_MAX;
      while (*map >= 0)
        {tmp = PetscMax(tmp,*(vals + *map)); map++;}
      
      map = *reduce++;
      while (*map >= 0)
        {*(vals + *map++) = tmp;}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_in_max( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar *base;

  PetscFunctionBegin;
  num    = gs->num_gop_local_reduce;  
  reduce = gs->gop_local_reduce;  
  while ((map = *reduce++))
    {
      num++;
      base = vals + *map++;
      while (*map >= 0)
        {*base = PetscMax(*base,*(vals + *map)); map++;}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

VERSION 3 :: 

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_pairwise_max( gs_id *gs,  PetscScalar *in_vals)
{
   PetscScalar *dptr1, *dptr2, *dptr3, *in1, *in2;
   int *iptr, *msg_list, *msg_size, **msg_nodes;
   int *pw, *list, *size, **nodes;
  MPI_Request *msg_ids_in, *msg_ids_out, *ids_in, *ids_out;
  MPI_Status status;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* strip and load s */
  msg_list =list         = gs->pair_list;
  msg_size =size         = gs->msg_sizes;
  msg_nodes=nodes        = gs->node_list;
  iptr=pw                = gs->pw_elm_list;  
  dptr1=dptr3            = gs->pw_vals;
  msg_ids_in  = ids_in   = gs->msg_ids_in;
  msg_ids_out = ids_out  = gs->msg_ids_out;
  dptr2                  = gs->out;
  in1=in2                = gs->in;

  /* post the receives */
  /*  msg_nodes=nodes; */
  do 
    {
      /* Should MPI_ANY_SOURCE be replaced by *list ? In that case do the
         second one *list and do list++ afterwards */
      ierr = MPI_Irecv(in1, *size, MPIU_SCALAR, MPI_ANY_SOURCE, MSGTAG1 + *list++, gs->gs_comm, msg_ids_in++);CHKERRQ(ierr); 
      in1 += *size++;
    }
  while (*++msg_nodes);
  msg_nodes=nodes;

  /* load gs values into in out gs buffers */  
  while (*iptr >= 0)
    {*dptr3++ = *(in_vals + *iptr++);}

  /* load out buffers and post the sends */
  while ((iptr = *msg_nodes++))
    {
      dptr3 = dptr2;
      while (*iptr >= 0)
        {*dptr2++ = *(dptr1 + *iptr++);}
      /* CHECK PERSISTENT COMMS MODE FOR ALL THIS STUFF */
      /* is msg_ids_out++ correct? */
      ierr = MPI_Isend(dptr3, *msg_size++, MPIU_SCALAR, *msg_list++, MSGTAG1+my_id, gs->gs_comm, msg_ids_out++);CHKERRQ(ierr);
    }

  if (gs->max_left_over)
    {gs_gop_tree_max(gs,in_vals);}

  /* process the received data */
  msg_nodes=nodes;
  while ((iptr = *nodes++))
    {
      /* Should I check the return value of MPI_Wait() or status? */
      /* Can this loop be replaced by a call to MPI_Waitall()? */
      ierr = MPI_Wait(ids_in++, &status);CHKERRQ(ierr);
      while (*iptr >= 0)
        {*(dptr1 + *iptr) = PetscMax(*(dptr1 + *iptr),*in2); iptr++; in2++;}
    }

  /* replace vals */
  while (*pw >= 0)
    {*(in_vals + *pw++) = *dptr1++;}

  /* clear isend message handles */
  /* This changed for clarity though it could be the same */
  while (*msg_nodes++)
    /* Should I check the return value of MPI_Wait() or status? */
    /* Can this loop be replaced by a call to MPI_Waitall()? */
    {ierr = MPI_Wait(ids_out++, &status);CHKERRQ(ierr);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_tree_max(gs_id *gs, PetscScalar *vals)
{
  int size;
  int *in, *out;  
  PetscScalar *buf, *work;
  PetscErrorCode ierr;
  
  PetscFunctionBegin;
  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  buf  = gs->tree_buf;
  work = gs->tree_work;
  size = gs->tree_nel;

  rvec_set(buf,-REAL_MAX,size);

  while (*in >= 0)
    {*(buf + *out++) = *(vals + *in++);}

  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  ierr = MPI_Allreduce(buf,work,size,MPIU_SCALAR,MPI_MAX,gs->gs_comm);CHKERRQ(ierr);
  while (*in >= 0)
    {*(vals + *in++) = *(work + *out++);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_min_abs( gs_id *gs,  PetscScalar *vals)
{
  PetscFunctionBegin;
  /* local only operations!!! */
  if (gs->num_local)
    {gs_gop_local_min_abs(gs,vals);}

  /* if intersection tree/pairwise and local isn't empty */
  if (gs->num_local_gop)
    {
      gs_gop_local_in_min_abs(gs,vals);

      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_min_abs(gs,vals);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_min_abs(gs,vals);}
  
      gs_gop_local_out(gs,vals);
    }
  /* if intersection tree/pairwise and local is empty */
  else
    {
      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_min_abs(gs,vals);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_min_abs(gs,vals);}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_min_abs( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar tmp;

  PetscFunctionBegin;
  num    = gs->num_local_reduce;  
  reduce = gs->local_reduce;  
  while ((map = *reduce))
    {
      num ++;
      tmp = REAL_MAX;
      while (*map >= 0)
        {tmp = MIN_FABS(tmp,*(vals + *map)); map++;}
      
      map = *reduce++;
      while (*map >= 0)
        {*(vals + *map++) = tmp;}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_in_min_abs( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar *base;

  PetscFunctionBegin;
  num    = gs->num_gop_local_reduce;  
  reduce = gs->gop_local_reduce;  
  while ((map = *reduce++))
    {
      num++;
      base = vals + *map++;
      while (*map >= 0)
        {*base = MIN_FABS(*base,*(vals + *map)); map++;}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

VERSION 3 :: 

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_pairwise_min_abs( gs_id *gs,  PetscScalar *in_vals)
{
   PetscScalar *dptr1, *dptr2, *dptr3, *in1, *in2;
   int *iptr, *msg_list, *msg_size, **msg_nodes;
   int *pw, *list, *size, **nodes;
  MPI_Request *msg_ids_in, *msg_ids_out, *ids_in, *ids_out;
  MPI_Status status;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* strip and load s */
  msg_list =list         = gs->pair_list;
  msg_size =size         = gs->msg_sizes;
  msg_nodes=nodes        = gs->node_list;
  iptr=pw                = gs->pw_elm_list;  
  dptr1=dptr3            = gs->pw_vals;
  msg_ids_in  = ids_in   = gs->msg_ids_in;
  msg_ids_out = ids_out  = gs->msg_ids_out;
  dptr2                  = gs->out;
  in1=in2                = gs->in;

  /* post the receives */
  /*  msg_nodes=nodes; */
  do 
    {
      /* Should MPI_ANY_SOURCE be replaced by *list ? In that case do the
         second one *list and do list++ afterwards */
      ierr = MPI_Irecv(in1, *size, MPIU_SCALAR, MPI_ANY_SOURCE, MSGTAG1 + *list++, gs->gs_comm, msg_ids_in++);CHKERRQ(ierr);
      in1 += *size++;
    }
  while (*++msg_nodes);
  msg_nodes=nodes;

  /* load gs values into in out gs buffers */  
  while (*iptr >= 0)
    {*dptr3++ = *(in_vals + *iptr++);}

  /* load out buffers and post the sends */
  while ((iptr = *msg_nodes++))
    {
      dptr3 = dptr2;
      while (*iptr >= 0)
        {*dptr2++ = *(dptr1 + *iptr++);}
      /* CHECK PERSISTENT COMMS MODE FOR ALL THIS STUFF */
      /* is msg_ids_out++ correct? */
      ierr = MPI_Isend(dptr3, *msg_size++, MPIU_SCALAR, *msg_list++, MSGTAG1+my_id, gs->gs_comm, msg_ids_out++);CHKERRQ(ierr);
    }

  if (gs->max_left_over)
    {gs_gop_tree_min_abs(gs,in_vals);}

  /* process the received data */
  msg_nodes=nodes;
  while ((iptr = *nodes++))
    {
      /* Should I check the return value of MPI_Wait() or status? */
      /* Can this loop be replaced by a call to MPI_Waitall()? */
      ierr = MPI_Wait(ids_in++, &status);CHKERRQ(ierr);
      while (*iptr >= 0)
        {*(dptr1 + *iptr) = MIN_FABS(*(dptr1 + *iptr),*in2); iptr++; in2++;}
    }

  /* replace vals */
  while (*pw >= 0)
    {*(in_vals + *pw++) = *dptr1++;}

  /* clear isend message handles */
  /* This changed for clarity though it could be the same */
  while (*msg_nodes++)
    /* Should I check the return value of MPI_Wait() or status? */
    /* Can this loop be replaced by a call to MPI_Waitall()? */
    {ierr = MPI_Wait(ids_out++, &status);CHKERRQ(ierr);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_tree_min_abs(gs_id *gs, PetscScalar *vals)
{
  int size;
  int *in, *out;  
  PetscScalar *buf, *work;
  int op[] = {GL_MIN_ABS,0};

  PetscFunctionBegin;
  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  buf  = gs->tree_buf;
  work = gs->tree_work;
  size = gs->tree_nel;

  rvec_set(buf,REAL_MAX,size);

  while (*in >= 0)
    {*(buf + *out++) = *(vals + *in++);}

  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  grop(buf,work,size,op);
  while (*in >= 0)
    {*(vals + *in++) = *(buf + *out++);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_min( gs_id *gs,  PetscScalar *vals)
{
  PetscFunctionBegin;
  /* local only operations!!! */
  if (gs->num_local)
    {gs_gop_local_min(gs,vals);}

  /* if intersection tree/pairwise and local isn't empty */
  if (gs->num_local_gop)
    {
      gs_gop_local_in_min(gs,vals);

      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_min(gs,vals);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_min(gs,vals);}
  
      gs_gop_local_out(gs,vals);
    }
  /* if intersection tree/pairwise and local is empty */
  else
    {
      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_min(gs,vals);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_min(gs,vals);}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_min( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar tmp;
  PetscFunctionBegin;
  num    = gs->num_local_reduce;  
  reduce = gs->local_reduce;  
  while ((map = *reduce))
    {
      num ++;
      tmp = REAL_MAX;
      while (*map >= 0)
        {tmp = PetscMin(tmp,*(vals + *map)); map++;}
      
      map = *reduce++;
      while (*map >= 0)
        {*(vals + *map++) = tmp;}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_in_min( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar *base;

  PetscFunctionBegin;
  num    = gs->num_gop_local_reduce;  
  reduce = gs->gop_local_reduce;  
  while ((map = *reduce++))
    {
      num++;
      base = vals + *map++;
      while (*map >= 0)
        {*base = PetscMin(*base,*(vals + *map)); map++;}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

VERSION 3 :: 

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_pairwise_min( gs_id *gs,  PetscScalar *in_vals)
{
   PetscScalar *dptr1, *dptr2, *dptr3, *in1, *in2;
   int *iptr, *msg_list, *msg_size, **msg_nodes;
   int *pw, *list, *size, **nodes;
  MPI_Request *msg_ids_in, *msg_ids_out, *ids_in, *ids_out;
  MPI_Status status;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* strip and load s */
  msg_list =list         = gs->pair_list;
  msg_size =size         = gs->msg_sizes;
  msg_nodes=nodes        = gs->node_list;
  iptr=pw                = gs->pw_elm_list;  
  dptr1=dptr3            = gs->pw_vals;
  msg_ids_in  = ids_in   = gs->msg_ids_in;
  msg_ids_out = ids_out  = gs->msg_ids_out;
  dptr2                  = gs->out;
  in1=in2                = gs->in;

  /* post the receives */
  /*  msg_nodes=nodes; */
  do 
    {
      /* Should MPI_ANY_SOURCE be replaced by *list ? In that case do the
         second one *list and do list++ afterwards */
      ierr = MPI_Irecv(in1, *size, MPIU_SCALAR, MPI_ANY_SOURCE, MSGTAG1 + *list++, gs->gs_comm, msg_ids_in++);CHKERRQ(ierr);
      in1 += *size++;
    }
  while (*++msg_nodes);
  msg_nodes=nodes;

  /* load gs values into in out gs buffers */  
  while (*iptr >= 0)
    {*dptr3++ = *(in_vals + *iptr++);}

  /* load out buffers and post the sends */
  while ((iptr = *msg_nodes++))
    {
      dptr3 = dptr2;
      while (*iptr >= 0)
        {*dptr2++ = *(dptr1 + *iptr++);}
      /* CHECK PERSISTENT COMMS MODE FOR ALL THIS STUFF */
      /* is msg_ids_out++ correct? */
      ierr = MPI_Isend(dptr3, *msg_size++, MPIU_SCALAR, *msg_list++, MSGTAG1+my_id, gs->gs_comm, msg_ids_out++);CHKERRQ(ierr);
    }

  /* process the received data */
  if (gs->max_left_over)
    {gs_gop_tree_min(gs,in_vals);}

  msg_nodes=nodes;
  while ((iptr = *nodes++))
    {
      /* Should I check the return value of MPI_Wait() or status? */
      /* Can this loop be replaced by a call to MPI_Waitall()? */
      ierr = MPI_Wait(ids_in++, &status);CHKERRQ(ierr);
      while (*iptr >= 0)
        {*(dptr1 + *iptr) = PetscMin(*(dptr1 + *iptr),*in2); iptr++; in2++;}
    }

  /* replace vals */
  while (*pw >= 0)
    {*(in_vals + *pw++) = *dptr1++;}

  /* clear isend message handles */
  /* This changed for clarity though it could be the same */
  while (*msg_nodes++)
    /* Should I check the return value of MPI_Wait() or status? */
    /* Can this loop be replaced by a call to MPI_Waitall()? */
    {ierr = MPI_Wait(ids_out++, &status);CHKERRQ(ierr);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_tree_min(gs_id *gs, PetscScalar *vals)
{
  int size;
  int *in, *out;  
  PetscScalar *buf, *work;
  PetscErrorCode ierr;
  
  PetscFunctionBegin;
  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  buf  = gs->tree_buf;
  work = gs->tree_work;
  size = gs->tree_nel;

  rvec_set(buf,REAL_MAX,size);

  while (*in >= 0)
    {*(buf + *out++) = *(vals + *in++);}

  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  ierr = MPI_Allreduce(buf,work,size,MPIU_SCALAR,MPI_MIN,gs->gs_comm);CHKERRQ(ierr);
  while (*in >= 0)
    {*(vals + *in++) = *(work + *out++);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_times( gs_id *gs,  PetscScalar *vals)
{
  PetscFunctionBegin;
  /* local only operations!!! */
  if (gs->num_local)
    {gs_gop_local_times(gs,vals);}

  /* if intersection tree/pairwise and local isn't empty */
  if (gs->num_local_gop)
    {
      gs_gop_local_in_times(gs,vals);

      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_times(gs,vals);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_times(gs,vals);}
  
      gs_gop_local_out(gs,vals);
    }
  /* if intersection tree/pairwise and local is empty */
  else
    {
      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_pairwise_times(gs,vals);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_times(gs,vals);}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_times( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar tmp;

  PetscFunctionBegin;
  num    = gs->num_local_reduce;  
  reduce = gs->local_reduce;  
  while ((map = *reduce))
    {
      /* wall */
      if (*num == 2)
        {
          num ++; reduce++;
          vals[map[1]] = vals[map[0]] *= vals[map[1]];
        }
      /* corner shared by three elements */
      else if (*num == 3)
        {
          num ++; reduce++;
          vals[map[2]]=vals[map[1]]=vals[map[0]]*=(vals[map[1]]*vals[map[2]]);
        }
      /* corner shared by four elements */
      else if (*num == 4)
        {
          num ++; reduce++;
          vals[map[1]]=vals[map[2]]=vals[map[3]]=vals[map[0]] *= 
                                 (vals[map[1]] * vals[map[2]] * vals[map[3]]);
        }
      /* general case ... odd geoms ... 3D*/
      else
        {
          num ++;
          tmp = 1.0;
          while (*map >= 0)
            {tmp *= *(vals + *map++);}

          map = *reduce++;
          while (*map >= 0)
            {*(vals + *map++) = tmp;}
        }
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_in_times( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar *base;

  PetscFunctionBegin;
  num    = gs->num_gop_local_reduce;  
  reduce = gs->gop_local_reduce;  
  while ((map = *reduce++))
    {
      /* wall */
      if (*num == 2)
        {
          num ++;
          vals[map[0]] *= vals[map[1]];
        }
      /* corner shared by three elements */
      else if (*num == 3)
        {
          num ++;
          vals[map[0]] *= (vals[map[1]] * vals[map[2]]);
        }
      /* corner shared by four elements */
      else if (*num == 4)
        {
          num ++;
          vals[map[0]] *= (vals[map[1]] * vals[map[2]] * vals[map[3]]);
        }
      /* general case ... odd geoms ... 3D*/
      else
        {
          num++;
          base = vals + *map++;
          while (*map >= 0)
            {*base *= *(vals + *map++);}
        }
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

VERSION 3 :: 

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_pairwise_times( gs_id *gs,  PetscScalar *in_vals)
{
   PetscScalar *dptr1, *dptr2, *dptr3, *in1, *in2;
   int *iptr, *msg_list, *msg_size, **msg_nodes;
   int *pw, *list, *size, **nodes;
  MPI_Request *msg_ids_in, *msg_ids_out, *ids_in, *ids_out;
  MPI_Status status;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* strip and load s */
  msg_list =list         = gs->pair_list;
  msg_size =size         = gs->msg_sizes;
  msg_nodes=nodes        = gs->node_list;
  iptr=pw                = gs->pw_elm_list;  
  dptr1=dptr3            = gs->pw_vals;
  msg_ids_in  = ids_in   = gs->msg_ids_in;
  msg_ids_out = ids_out  = gs->msg_ids_out;
  dptr2                  = gs->out;
  in1=in2                = gs->in;

  /* post the receives */
  /*  msg_nodes=nodes; */
  do 
    {
      /* Should MPI_ANY_SOURCE be replaced by *list ? In that case do the
         second one *list and do list++ afterwards */
      ierr = MPI_Irecv(in1, *size, MPIU_SCALAR, MPI_ANY_SOURCE, MSGTAG1 + *list++, gs->gs_comm, msg_ids_in++);CHKERRQ(ierr); 
      in1 += *size++;
    }
  while (*++msg_nodes);
  msg_nodes=nodes;

  /* load gs values into in out gs buffers */  
  while (*iptr >= 0)
    {*dptr3++ = *(in_vals + *iptr++);}

  /* load out buffers and post the sends */
  while ((iptr = *msg_nodes++))
    {
      dptr3 = dptr2;
      while (*iptr >= 0)
        {*dptr2++ = *(dptr1 + *iptr++);}
      /* CHECK PERSISTENT COMMS MODE FOR ALL THIS STUFF */
      /* is msg_ids_out++ correct? */
      ierr = MPI_Isend(dptr3, *msg_size++, MPIU_SCALAR, *msg_list++, MSGTAG1+my_id, gs->gs_comm, msg_ids_out++);CHKERRQ(ierr);
    }

  if (gs->max_left_over)
    {gs_gop_tree_times(gs,in_vals);}

  /* process the received data */
  msg_nodes=nodes;
  while ((iptr = *nodes++))
    {
      /* Should I check the return value of MPI_Wait() or status? */
      /* Can this loop be replaced by a call to MPI_Waitall()? */
      ierr = MPI_Wait(ids_in++, &status);CHKERRQ(ierr);
      while (*iptr >= 0)
        {*(dptr1 + *iptr++) *= *in2++;}
    }

  /* replace vals */
  while (*pw >= 0)
    {*(in_vals + *pw++) = *dptr1++;}

  /* clear isend message handles */
  /* This changed for clarity though it could be the same */
  while (*msg_nodes++)
    /* Should I check the return value of MPI_Wait() or status? */
    /* Can this loop be replaced by a call to MPI_Waitall()? */
    {ierr = MPI_Wait(ids_out++, &status);CHKERRQ(ierr);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_tree_times(gs_id *gs, PetscScalar *vals)
{
  int size;
  int *in, *out;  
  PetscScalar *buf, *work;
  PetscErrorCode ierr;
  
  PetscFunctionBegin;
  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  buf  = gs->tree_buf;
  work = gs->tree_work;
  size = gs->tree_nel;

  rvec_one(buf,size);

  while (*in >= 0)
    {*(buf + *out++) = *(vals + *in++);}

  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  ierr = MPI_Allreduce(buf,work,size,MPIU_SCALAR,MPI_PROD,gs->gs_comm);CHKERRQ(ierr);
  while (*in >= 0)
    {*(vals + *in++) = *(work + *out++);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter


Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_plus( gs_id *gs,  PetscScalar *vals)
{
  PetscFunctionBegin;
  /* local only operations!!! */
  if (gs->num_local)
    {gs_gop_local_plus(gs,vals);}

  /* if intersection tree/pairwise and local isn't empty */
  if (gs->num_local_gop)
    {
      gs_gop_local_in_plus(gs,vals);

      /* pairwise will NOT do tree inside ... */
      if (gs->num_pairs)
        {gs_gop_pairwise_plus(gs,vals);}

      /* tree */
      if (gs->max_left_over)
        {gs_gop_tree_plus(gs,vals);}
      
      gs_gop_local_out(gs,vals);
    }
  /* if intersection tree/pairwise and local is empty */
  else
    {
      /* pairwise will NOT do tree inside */
      if (gs->num_pairs)
        {gs_gop_pairwise_plus(gs,vals);}
      
      /* tree */
      if (gs->max_left_over)
        {gs_gop_tree_plus(gs,vals);}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_plus( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar tmp;

  PetscFunctionBegin;
  num    = gs->num_local_reduce;  
  reduce = gs->local_reduce;  
  while ((map = *reduce))
    {
      /* wall */
      if (*num == 2)
        {
          num ++; reduce++;
          vals[map[1]] = vals[map[0]] += vals[map[1]];
        }
      /* corner shared by three elements */
      else if (*num == 3)
        {
          num ++; reduce++;
          vals[map[2]]=vals[map[1]]=vals[map[0]]+=(vals[map[1]]+vals[map[2]]);
        }
      /* corner shared by four elements */
      else if (*num == 4)
        {
          num ++; reduce++;
          vals[map[1]]=vals[map[2]]=vals[map[3]]=vals[map[0]] += 
                                 (vals[map[1]] + vals[map[2]] + vals[map[3]]);
        }
      /* general case ... odd geoms ... 3D*/
      else
        {
          num ++;
          tmp = 0.0;
          while (*map >= 0)
            {tmp += *(vals + *map++);}

          map = *reduce++;
          while (*map >= 0)
            {*(vals + *map++) = tmp;}
        }
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_local_in_plus( gs_id *gs,  PetscScalar *vals)
{
   int *num, *map, **reduce;
   PetscScalar *base;

  PetscFunctionBegin;
  num    = gs->num_gop_local_reduce;  
  reduce = gs->gop_local_reduce;  
  while ((map = *reduce++))
    {
      /* wall */
      if (*num == 2)
        {
          num ++;
          vals[map[0]] += vals[map[1]];
        }
      /* corner shared by three elements */
      else if (*num == 3)
        {
          num ++;
          vals[map[0]] += (vals[map[1]] + vals[map[2]]);
        }
      /* corner shared by four elements */
      else if (*num == 4)
        {
          num ++;
          vals[map[0]] += (vals[map[1]] + vals[map[2]] + vals[map[3]]);
        }
      /* general case ... odd geoms ... 3D*/
      else
        {
          num++;
          base = vals + *map++;
          while (*map >= 0)
            {*base += *(vals + *map++);}
        }
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

VERSION 3 :: 

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_pairwise_plus( gs_id *gs,  PetscScalar *in_vals)
{
   PetscScalar *dptr1, *dptr2, *dptr3, *in1, *in2;
   int *iptr, *msg_list, *msg_size, **msg_nodes;
   int *pw, *list, *size, **nodes;
  MPI_Request *msg_ids_in, *msg_ids_out, *ids_in, *ids_out;
  MPI_Status status;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* strip and load s */
  msg_list =list         = gs->pair_list;
  msg_size =size         = gs->msg_sizes;
  msg_nodes=nodes        = gs->node_list;
  iptr=pw                = gs->pw_elm_list;  
  dptr1=dptr3            = gs->pw_vals;
  msg_ids_in  = ids_in   = gs->msg_ids_in;
  msg_ids_out = ids_out  = gs->msg_ids_out;
  dptr2                  = gs->out;
  in1=in2                = gs->in;

  /* post the receives */
  /*  msg_nodes=nodes; */
  do 
    {
      /* Should MPI_ANY_SOURCE be replaced by *list ? In that case do the
         second one *list and do list++ afterwards */
      ierr = MPI_Irecv(in1, *size, MPIU_SCALAR, MPI_ANY_SOURCE, MSGTAG1 + *list++, gs->gs_comm, msg_ids_in++);CHKERRQ(ierr);
      in1 += *size++;
    }
  while (*++msg_nodes);
  msg_nodes=nodes;

  /* load gs values into in out gs buffers */  
  while (*iptr >= 0)
    {*dptr3++ = *(in_vals + *iptr++);}

  /* load out buffers and post the sends */
  while ((iptr = *msg_nodes++))
    {
      dptr3 = dptr2;
      while (*iptr >= 0)
        {*dptr2++ = *(dptr1 + *iptr++);}
      /* CHECK PERSISTENT COMMS MODE FOR ALL THIS STUFF */
      /* is msg_ids_out++ correct? */
      ierr = MPI_Isend(dptr3, *msg_size++, MPIU_SCALAR, *msg_list++, MSGTAG1+my_id, gs->gs_comm, msg_ids_out++);CHKERRQ(ierr);
    }

  /* do the tree while we're waiting */
  if (gs->max_left_over)
    {gs_gop_tree_plus(gs,in_vals);}

  /* process the received data */
  msg_nodes=nodes;
  while ((iptr = *nodes++))
    {
      /* Should I check the return value of MPI_Wait() or status? */
      /* Can this loop be replaced by a call to MPI_Waitall()? */
      ierr = MPI_Wait(ids_in++, &status);CHKERRQ(ierr);
      while (*iptr >= 0)
        {*(dptr1 + *iptr++) += *in2++;}
    }

  /* replace vals */
  while (*pw >= 0)
    {*(in_vals + *pw++) = *dptr1++;}

  /* clear isend message handles */
  /* This changed for clarity though it could be the same */
  while (*msg_nodes++)
    /* Should I check the return value of MPI_Wait() or status? */
    /* Can this loop be replaced by a call to MPI_Waitall()? */
    {ierr = MPI_Wait(ids_out++, &status);CHKERRQ(ierr);}
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_tree_plus(gs_id *gs, PetscScalar *vals)
{
  int size;
  int *in, *out;  
  PetscScalar *buf, *work;
  PetscErrorCode ierr;
  
  PetscFunctionBegin;
  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  buf  = gs->tree_buf;
  work = gs->tree_work;
  size = gs->tree_nel;

  rvec_zero(buf,size);

  while (*in >= 0)
    {*(buf + *out++) = *(vals + *in++);}

  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  ierr = MPI_Allreduce(buf,work,size,MPIU_SCALAR,MPI_SUM,gs->gs_comm);CHKERRQ(ierr);
  while (*in >= 0)
    {*(vals + *in++) = *(work + *out++);}
  PetscFunctionReturn(0);
}

/******************************************************************************
Function: gs_free()

Input : 

Output: 

Return: 

Description:  
  if (gs->sss) {free((void*) gs->sss);}
******************************************************************************/
PetscErrorCode gs_free( gs_id *gs)
{
   int i;

  PetscFunctionBegin;
  if (gs->nghs) {free((void*) gs->nghs);}
  if (gs->pw_nghs) {free((void*) gs->pw_nghs);}

  /* tree */
  if (gs->max_left_over)
    {
      if (gs->tree_elms) {free((void*) gs->tree_elms);}
      if (gs->tree_buf) {free((void*) gs->tree_buf);}
      if (gs->tree_work) {free((void*) gs->tree_work);}
      if (gs->tree_map_in) {free((void*) gs->tree_map_in);}
      if (gs->tree_map_out) {free((void*) gs->tree_map_out);}
    }

  /* pairwise info */
  if (gs->num_pairs)
    {    
      /* should be NULL already */
      if (gs->ngh_buf) {free((void*) gs->ngh_buf);}
      if (gs->elms) {free((void*) gs->elms);}
      if (gs->local_elms) {free((void*) gs->local_elms);}
      if (gs->companion) {free((void*) gs->companion);}
      
      /* only set if pairwise */
      if (gs->vals) {free((void*) gs->vals);}
      if (gs->in) {free((void*) gs->in);}
      if (gs->out) {free((void*) gs->out);}
      if (gs->msg_ids_in) {free((void*) gs->msg_ids_in);}  
      if (gs->msg_ids_out) {free((void*) gs->msg_ids_out);}
      if (gs->pw_vals) {free((void*) gs->pw_vals);}
      if (gs->pw_elm_list) {free((void*) gs->pw_elm_list);}
      if (gs->node_list) 
        {
          for (i=0;i<gs->num_pairs;i++)
            {if (gs->node_list[i]) {free((void*) gs->node_list[i]);}}
          free((void*) gs->node_list);
        }
      if (gs->msg_sizes) {free((void*) gs->msg_sizes);}
      if (gs->pair_list) {free((void*) gs->pair_list);}
    }

  /* local info */
  if (gs->num_local_total>=0)
    {
      for (i=0;i<gs->num_local_total+1;i++)
        /*      for (i=0;i<gs->num_local_total;i++) */
        {
          if (gs->num_gop_local_reduce[i]) 
            {free((void*) gs->gop_local_reduce[i]);}
        }
    }

  /* if intersection tree/pairwise and local isn't empty */
  if (gs->gop_local_reduce) {free((void*) gs->gop_local_reduce);}
  if (gs->num_gop_local_reduce) {free((void*) gs->num_gop_local_reduce);}

  free((void*) gs);
  PetscFunctionReturn(0);
}






/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
PetscErrorCode gs_gop_vec( gs_id *gs,  PetscScalar *vals,  const char *op,  int step)
{
  PetscFunctionBegin;
  switch (*op) {
  case '+':
    gs_gop_vec_plus(gs,vals,step);
    break;
  default:
    error_msg_warning("gs_gop_vec() :: %c is not a valid op",op[0]);
    error_msg_warning("gs_gop_vec() :: default :: plus");
    gs_gop_vec_plus(gs,vals,step);    
    break;
  }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_vec_plus( gs_id *gs,  PetscScalar *vals,  int step)
{
  PetscFunctionBegin;
  if (!gs) {error_msg_fatal("gs_gop_vec() passed NULL gs handle!!!");}

  /* local only operations!!! */
  if (gs->num_local)
    {gs_gop_vec_local_plus(gs,vals,step);}

  /* if intersection tree/pairwise and local isn't empty */
  if (gs->num_local_gop)
    {
      gs_gop_vec_local_in_plus(gs,vals,step);

      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_vec_pairwise_plus(gs,vals,step);}

      /* tree */
      else if (gs->max_left_over)
        {gs_gop_vec_tree_plus(gs,vals,step);}

      gs_gop_vec_local_out(gs,vals,step);
    }
  /* if intersection tree/pairwise and local is empty */
  else
    {
      /* pairwise */
      if (gs->num_pairs)
        {gs_gop_vec_pairwise_plus(gs,vals,step);}

      /* tree */
      else if (gs->max_left_over)
        {gs_gop_vec_tree_plus(gs,vals,step);}
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_vec_local_plus( gs_id *gs,  PetscScalar *vals, int step)
{
   int *num, *map, **reduce;
   PetscScalar *base;

  PetscFunctionBegin;
  num    = gs->num_local_reduce;  
  reduce = gs->local_reduce;  
  while ((map = *reduce))
    {
      base = vals + map[0] * step;

      /* wall */
      if (*num == 2)
        {
          num++; reduce++;
          rvec_add (base,vals+map[1]*step,step);
          rvec_copy(vals+map[1]*step,base,step);
        }
      /* corner shared by three elements */
      else if (*num == 3)
        {
          num++; reduce++;
          rvec_add (base,vals+map[1]*step,step);
          rvec_add (base,vals+map[2]*step,step);
          rvec_copy(vals+map[2]*step,base,step);
          rvec_copy(vals+map[1]*step,base,step);
        }
      /* corner shared by four elements */
      else if (*num == 4)
        {
          num++; reduce++;
          rvec_add (base,vals+map[1]*step,step);
          rvec_add (base,vals+map[2]*step,step);
          rvec_add (base,vals+map[3]*step,step);
          rvec_copy(vals+map[3]*step,base,step);
          rvec_copy(vals+map[2]*step,base,step);
          rvec_copy(vals+map[1]*step,base,step);
        }
      /* general case ... odd geoms ... 3D */
      else
        {
          num++;
          while (*++map >= 0)
            {rvec_add (base,vals+*map*step,step);}
              
          map = *reduce;
          while (*++map >= 0)
            {rvec_copy(vals+*map*step,base,step);}
          
          reduce++;
        }
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_vec_local_in_plus( gs_id *gs,  PetscScalar *vals, int step)
{
   int  *num, *map, **reduce;
   PetscScalar *base;
  PetscFunctionBegin;
  num    = gs->num_gop_local_reduce;  
  reduce = gs->gop_local_reduce;  
  while ((map = *reduce++))
    {
      base = vals + map[0] * step;

      /* wall */
      if (*num == 2)
        {
          num ++;
          rvec_add(base,vals+map[1]*step,step);
        }
      /* corner shared by three elements */
      else if (*num == 3)
        {
          num ++;
          rvec_add(base,vals+map[1]*step,step);
          rvec_add(base,vals+map[2]*step,step);
        }
      /* corner shared by four elements */
      else if (*num == 4)
        {
          num ++;
          rvec_add(base,vals+map[1]*step,step);
          rvec_add(base,vals+map[2]*step,step);
          rvec_add(base,vals+map[3]*step,step);
        }
      /* general case ... odd geoms ... 3D*/
      else
        {
          num++;
          while (*++map >= 0)
            {rvec_add(base,vals+*map*step,step);}
        }
    }
  PetscFunctionReturn(0);
}


/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_vec_local_out( gs_id *gs,  PetscScalar *vals, int step)
{
   int *num, *map, **reduce;
   PetscScalar *base;

  PetscFunctionBegin;
  num    = gs->num_gop_local_reduce;  
  reduce = gs->gop_local_reduce;  
  while ((map = *reduce++))
    {
      base = vals + map[0] * step;

      /* wall */
      if (*num == 2)
        {
          num ++;
          rvec_copy(vals+map[1]*step,base,step);
        }
      /* corner shared by three elements */
      else if (*num == 3)
        {
          num ++;
          rvec_copy(vals+map[1]*step,base,step);
          rvec_copy(vals+map[2]*step,base,step);
        }
      /* corner shared by four elements */
      else if (*num == 4)
        {
          num ++;
          rvec_copy(vals+map[1]*step,base,step);
          rvec_copy(vals+map[2]*step,base,step);
          rvec_copy(vals+map[3]*step,base,step);
        }
      /* general case ... odd geoms ... 3D*/
      else
        {
          num++;
          while (*++map >= 0)
            {rvec_copy(vals+*map*step,base,step);}
        }
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

VERSION 3 :: 

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_vec_pairwise_plus( gs_id *gs,  PetscScalar *in_vals, int step)
{
   PetscScalar *dptr1, *dptr2, *dptr3, *in1, *in2;
   int *iptr, *msg_list, *msg_size, **msg_nodes;
   int *pw, *list, *size, **nodes;
  MPI_Request *msg_ids_in, *msg_ids_out, *ids_in, *ids_out;
  MPI_Status status;
  PetscBLASInt i1;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  /* strip and load s */
  msg_list =list         = gs->pair_list;
  msg_size =size         = gs->msg_sizes;
  msg_nodes=nodes        = gs->node_list;
  iptr=pw                = gs->pw_elm_list;  
  dptr1=dptr3            = gs->pw_vals;
  msg_ids_in  = ids_in   = gs->msg_ids_in;
  msg_ids_out = ids_out  = gs->msg_ids_out;
  dptr2                  = gs->out;
  in1=in2                = gs->in;

  /* post the receives */
  /*  msg_nodes=nodes; */
  do 
    {
      /* Should MPI_ANY_SOURCE be replaced by *list ? In that case do the
         second one *list and do list++ afterwards */
      ierr = MPI_Irecv(in1, *size *step, MPIU_SCALAR, MPI_ANY_SOURCE, MSGTAG1 + *list++, gs->gs_comm, msg_ids_in++);CHKERRQ(ierr);
      in1 += *size++ *step;
    }
  while (*++msg_nodes);
  msg_nodes=nodes;

  /* load gs values into in out gs buffers */  
  while (*iptr >= 0)
    {
      rvec_copy(dptr3,in_vals + *iptr*step,step);
      dptr3+=step;
      iptr++;
    }

  /* load out buffers and post the sends */
  while ((iptr = *msg_nodes++))
    {
      dptr3 = dptr2;
      while (*iptr >= 0)
        {
          rvec_copy(dptr2,dptr1 + *iptr*step,step);
          dptr2+=step;
          iptr++;
        }
      ierr = MPI_Isend(dptr3, *msg_size++ *step, MPIU_SCALAR, *msg_list++, MSGTAG1+my_id, gs->gs_comm, msg_ids_out++);CHKERRQ(ierr);
    }

  /* tree */
  if (gs->max_left_over)
    {gs_gop_vec_tree_plus(gs,in_vals,step);}

  /* process the received data */
  msg_nodes=nodes;
  while ((iptr = *nodes++)){
    PetscScalar d1 = 1.0;
      /* Should I check the return value of MPI_Wait() or status? */
      /* Can this loop be replaced by a call to MPI_Waitall()? */
      ierr = MPI_Wait(ids_in++, &status);CHKERRQ(ierr);
      while (*iptr >= 0) {
          BLASaxpy_(&step,&d1,in2,&i1,dptr1 + *iptr*step,&i1);
          in2+=step;
          iptr++;
      }
  }

  /* replace vals */
  while (*pw >= 0)
    {
      rvec_copy(in_vals + *pw*step,dptr1,step);
      dptr1+=step;
      pw++;
    }

  /* clear isend message handles */
  /* This changed for clarity though it could be the same */
  while (*msg_nodes++)
    /* Should I check the return value of MPI_Wait() or status? */
    /* Can this loop be replaced by a call to MPI_Waitall()? */
    {ierr = MPI_Wait(ids_out++, &status);CHKERRQ(ierr);}

  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_vec_tree_plus( gs_id *gs,  PetscScalar *vals,  int step) 
{
  int size, *in, *out;  
  PetscScalar *buf, *work;
  int op[] = {GL_ADD,0};
  PetscBLASInt i1 = 1;

  PetscFunctionBegin;
  /* copy over to local variables */
  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  buf  = gs->tree_buf;
  work = gs->tree_work;
  size = gs->tree_nel*step;

  /* zero out collection buffer */
  rvec_zero(buf,size);


  /* copy over my contributions */
  while (*in >= 0)
    { 
      BLAScopy_(&step,vals + *in++*step,&i1,buf + *out++*step,&i1);
    }

  /* perform fan in/out on full buffer */
  /* must change grop to handle the blas */
  grop(buf,work,size,op);

  /* reset */
  in   = gs->tree_map_in;
  out  = gs->tree_map_out;

  /* get the portion of the results I need */
  while (*in >= 0)
    {
      BLAScopy_(&step,buf + *out++*step,&i1,vals + *in++*step,&i1);
    }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
PetscErrorCode gs_gop_hc( gs_id *gs,  PetscScalar *vals,  const char *op,  int dim)
{
  PetscFunctionBegin;
  switch (*op) {
  case '+':
    gs_gop_plus_hc(gs,vals,dim);
    break;
  default:
    error_msg_warning("gs_gop_hc() :: %c is not a valid op",op[0]);
    error_msg_warning("gs_gop_hc() :: default :: plus\n");
    gs_gop_plus_hc(gs,vals,dim);    
    break;
  }
  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_plus_hc( gs_id *gs,  PetscScalar *vals, int dim)
{
  PetscFunctionBegin;
  /* if there's nothing to do return */
  if (dim<=0)
    {  PetscFunctionReturn(0);}

  /* can't do more dimensions then exist */
  dim = PetscMin(dim,i_log2_num_nodes);

  /* local only operations!!! */
  if (gs->num_local)
    {gs_gop_local_plus(gs,vals);}

  /* if intersection tree/pairwise and local isn't empty */
  if (gs->num_local_gop)
    {
      gs_gop_local_in_plus(gs,vals);

      /* pairwise will do tree inside ... */
      if (gs->num_pairs)
        {gs_gop_pairwise_plus_hc(gs,vals,dim);}

      /* tree only */
      else if (gs->max_left_over)
        {gs_gop_tree_plus_hc(gs,vals,dim);}
      
      gs_gop_local_out(gs,vals);
    }
  /* if intersection tree/pairwise and local is empty */
  else
    {
      /* pairwise will do tree inside */
      if (gs->num_pairs)
        {gs_gop_pairwise_plus_hc(gs,vals,dim);}
      
      /* tree */
      else if (gs->max_left_over)
        {gs_gop_tree_plus_hc(gs,vals,dim);}
    }
  PetscFunctionReturn(0);
}


/******************************************************************************
VERSION 3 :: 

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_pairwise_plus_hc( gs_id *gs,  PetscScalar *in_vals, int dim)
{
   PetscScalar *dptr1, *dptr2, *dptr3, *in1, *in2;
   int *iptr, *msg_list, *msg_size, **msg_nodes;
   int *pw, *list, *size, **nodes;
  MPI_Request *msg_ids_in, *msg_ids_out, *ids_in, *ids_out;
  MPI_Status status;
  int i, mask=1;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  for (i=1; i<dim; i++)
    {mask<<=1; mask++;}


  /* strip and load s */
  msg_list =list         = gs->pair_list;
  msg_size =size         = gs->msg_sizes;
  msg_nodes=nodes        = gs->node_list;
  iptr=pw                = gs->pw_elm_list;  
  dptr1=dptr3            = gs->pw_vals;
  msg_ids_in  = ids_in   = gs->msg_ids_in;
  msg_ids_out = ids_out  = gs->msg_ids_out;
  dptr2                  = gs->out;
  in1=in2                = gs->in;

  /* post the receives */
  /*  msg_nodes=nodes; */
  do 
    {
      /* Should MPI_ANY_SOURCE be replaced by *list ? In that case do the
         second one *list and do list++ afterwards */
      if ((my_id|mask)==(*list|mask))
        {
          ierr = MPI_Irecv(in1, *size, MPIU_SCALAR, MPI_ANY_SOURCE, MSGTAG1 + *list++, gs->gs_comm, msg_ids_in++);CHKERRQ(ierr);
          in1 += *size++;
        }
      else
        {list++; size++;}
    }
  while (*++msg_nodes);

  /* load gs values into in out gs buffers */  
  while (*iptr >= 0)
    {*dptr3++ = *(in_vals + *iptr++);}

  /* load out buffers and post the sends */
  msg_nodes=nodes;
  list = msg_list;
  while ((iptr = *msg_nodes++))
    {
      if ((my_id|mask)==(*list|mask))
        {
          dptr3 = dptr2;
          while (*iptr >= 0)
            {*dptr2++ = *(dptr1 + *iptr++);}
          /* CHECK PERSISTENT COMMS MODE FOR ALL THIS STUFF */
          /* is msg_ids_out++ correct? */
          ierr = MPI_Isend(dptr3, *msg_size++, MPIU_SCALAR, *list++, MSGTAG1+my_id, gs->gs_comm, msg_ids_out++);CHKERRQ(ierr);
        }
      else
        {list++; msg_size++;}
    }

  /* do the tree while we're waiting */
  if (gs->max_left_over)
    {gs_gop_tree_plus_hc(gs,in_vals,dim);}

  /* process the received data */
  msg_nodes=nodes;
  list = msg_list;
  while ((iptr = *nodes++))
    {
      if ((my_id|mask)==(*list|mask))
        {
          /* Should I check the return value of MPI_Wait() or status? */
          /* Can this loop be replaced by a call to MPI_Waitall()? */
          ierr = MPI_Wait(ids_in++, &status);CHKERRQ(ierr);
          while (*iptr >= 0)
            {*(dptr1 + *iptr++) += *in2++;}
        }
      list++;
    }

  /* replace vals */
  while (*pw >= 0)
    {*(in_vals + *pw++) = *dptr1++;}

  /* clear isend message handles */
  /* This changed for clarity though it could be the same */
  while (*msg_nodes++)
    {
      if ((my_id|mask)==(*msg_list|mask))
        {
          /* Should I check the return value of MPI_Wait() or status? */
          /* Can this loop be replaced by a call to MPI_Waitall()? */
          ierr = MPI_Wait(ids_out++, &status);CHKERRQ(ierr);
        }
      msg_list++;
    }

  PetscFunctionReturn(0);
}



/******************************************************************************
Function: gather_scatter

Input : 
Output: 
Return: 
Description: 
******************************************************************************/
static PetscErrorCode gs_gop_tree_plus_hc(gs_id *gs, PetscScalar *vals, int dim)
{
  int size;
  int *in, *out;  
  PetscScalar *buf, *work;
  int op[] = {GL_ADD,0};

  PetscFunctionBegin;
  in   = gs->tree_map_in;
  out  = gs->tree_map_out;
  buf  = gs->tree_buf;
  work = gs->tree_work;
  size = gs->tree_nel;

  rvec_zero(buf,size);

  while (*in >= 0)
    {*(buf + *out++) = *(vals + *in++);}

  in   = gs->tree_map_in;
  out  = gs->tree_map_out;

  grop_hc(buf,work,size,op,dim);

  while (*in >= 0)
    {*(vals + *in++) = *(buf + *out++);}
  PetscFunctionReturn(0);
}



