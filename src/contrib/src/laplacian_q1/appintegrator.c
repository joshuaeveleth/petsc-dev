
#include "appctx.h"

/* REMEMBER quadrature weights  */

int ComputeRHS( AppElement *phi ){
  int i,j,k; 
  int bn, qn; /* basis count, quadrature count */
  bn = 4;
  qn = 4;
  /* need to go over each element , then each variable */
 for( i = 0; i < bn; i++ ){ /* loop over basis functions */
   phi->rhsresult[i] = 0.0; 
   for( j = 0; j < qn; j++ ){ /* loop over Gauss points */
     phi->rhsresult[i] +=  phi->weights[j] *f(phi->x[j], phi->y[j]) 
       *(phi->RefVal[i][j])*PetscAbsDouble(phi->detDh[j]); 
   }
 }
PetscFunctionReturn(0);
}

/* ComputeStiffness: computes integrals of gradients of local phi_i and phi_j on the given quadrangle by changing variables to the reference quadrangle and reference basis elements phi_i and phi_j.  The formula used is

integral (given element) of <grad phi_j', grad phi_i'> =
integral over (ref element) of 
    <(grad phi_j composed with h)*(grad h)^-1,
     (grad phi_i composed with h)*(grad h)^-1>*det(grad h).
this is evaluated by quadrature:
= sum over gauss points, above evaluated at gauss pts
*/
int ComputeStiffness( AppElement *phi ){
   int i,j,k;
   int bn, qn; /* basis count, quadrature count */
   bn = 4;  
   qn = 4;
   /* Stiffness Terms */
   /* could even do half as many by exploiting symmetry  */
   for( i=0;i<bn;i++ ){ /* loop over first basis fn */
     for( j=0; j<bn; j++){ /* loop over second */
       phi->stiffnessresult[i][j] = 0;
     }
   }

  /* Now Integral.  term is <DphiDhinv[i],DphiDhinv[j]>*abs(detDh) */
   for( i=0;i<bn;i++ ){ /* loop over first basis fn */
     for( j=0; j<bn; j++){ /* loop over second */
       for(k=0;k<qn;k++){ /* loop over gauss points */
	 phi->stiffnessresult[i][j] +=
	  - phi->weights[k]*
	   (phi->dx[i][k]*phi->dx[j][k] + 
	     phi->dy[i][k]*phi->dy[j][k])*
	   PetscAbsDouble(phi->detDh[k]);
       }
     }
   }
   PetscFunctionReturn(0);
}
