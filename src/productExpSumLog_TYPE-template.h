/***********************************************************************
 TEMPLATE:
  LDOUBLE productExpSumLog_<Integer|Real>(X_C_TYPE *x, R_xlen_t nx, int narm, int hasna)

 GENERATES:
  LDOUBLE productExpSumLog_Real(double *x, R_xlen_t nx, int narm, int hasna)
  LDOUBLE productExpSumLog_Integer(int *x, R_xlen_t nx, int narm, int hasna)

 Arguments:
   The following macros ("arguments") should be defined for the 
   template to work as intended.

  - METHOD_NAME: the name of the resulting function
  - X_TYPE: 'i' or 'r'

 Copyright: Henrik Bengtsson, 2014
 ***********************************************************************/ 
#include <Rdefines.h>
#include <Rmath.h>
#include <float.h>
#include <stdlib.h> /* abs() */
#include "types.h" 

#ifdef HAVE_LONG_DOUBLE
#define LDOUBLE long double
#else
#define LDOUBLE double
#endif

/* Expand arguments:
    X_TYPE => (X_C_TYPE, X_IN_C, [METHOD_NAME])
 */
#include "templates-types.h" 


LDOUBLE METHOD_NAME(X_C_TYPE *x, R_xlen_t nx, int narm, int hasna) {
  LDOUBLE y = 0.0, t;
  R_xlen_t ii;
  int isneg = 0;

  /* Calculate sum(log(abs(x))) */
  for (ii = 0 ; ii < nx; ii++) {
    t = x[ii];
    /* Skip missing values? */
    if (narm) {
      if (X_ISNAN(t)) continue;
    }

#if X_TYPE == 'i'
    /* Early stopping? */
    if (t < 0) {
      isneg = !isneg;
    } else if (t == 0) {
      y = R_NegInf;
      break;
    }
#elif X_TYPE == 'r'
    if (t < 0) isneg = !isneg;
#endif
    t = X_ABS(t);
    t = log(t);
    y += t;
    /*
      Rprintf("#%d: x=%g, is.nan(x)=%d, abs(x)=%g, is.nan(abs(x))=%d, log(abs(x))=%g, is.nan(log(abs(x)))=%d, sum=%g, is.nan(sum)=%d\n", ii, x[ii], R_IsNaN(x[ii]), abs(x[ii]), R_IsNaN(abs(x[ii])), t, R_IsNaN(y), y, R_IsNaN(y));  */
  }

  if (X_ISNAN(y)) {
    /* If there where NA and/or NaN elements, then 'y' will at this
       point be NaN. The information on an NA value is lost when 
       calculating fabs(NA), which returns NaN. For consistency with
       integers, we return NA in all cases. */
    y = NA_REAL;
  } else {
    y = exp(y);
  
    /* Update sign */
    if (isneg) {
      y = -y;
    }
  
    /* Overflow or underflow? */
    if (y > DBL_MAX) {
      y = R_PosInf;
    } else if (y < -DBL_MAX) {
      y = R_NegInf; 
    }
  }

  return(y);
}

/* Undo template macros */
#include "templates-types_undef.h" 


/***************************************************************************
 HISTORY:
 2014-11-06 [HB]
  o CLEANUP: Moving away from R data types in low-level C functions.
 2014-06-04 [HB]
  o Created.
 **************************************************************************/