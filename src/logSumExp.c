/***************************************************************************
 Public methods:
 SEXP logSumExp(SEXP lx, SEXP naRm, SEXP hasNA)
 SEXP rowLogSumExps(SEXP lx, SEXP naRm, SEXP hasNA, SEXP byRow)

 Arguments:
   lx   : numeric vector
   naRm : a logical scalar
   hasNA: a logical scalar


 Authors: Henrik Bengtsson

 Copyright Henrik Bengtsson, 2013
 **************************************************************************/
#include <Rdefines.h>
#include <Rmath.h>
#include "types.h"
#include "utils.h"

/* 
 logSumExp_double(x):

  1. Scans for the maximum value of x=(x[0], x[1], ..., x[n-1])
  2. Computes result from 'x'.

  NOTE: The above sweeps the "contiguous" 'x' vector twice.
*/
double logSumExp_double(double *x, R_xlen_t nx, int narm, int hasna) {
  R_xlen_t ii, iMax;
  double xii, xMax;
  LDOUBLE sum;

  /* Quick return? */
  if (nx == 0) {
    return(R_NegInf);
  } else if (nx == 1) {
    if (narm && ISNAN(x[0])) {
      return(R_NegInf);
    } else {
      return(x[0]);
    }
  }

  /* Find the maximum value */
  iMax = 0;
  xMax = x[0];
  for (ii=1; ii < nx; ii++) {
    /* Get the ii:th value */
    xii = x[ii];

    if (hasna && ISNAN(xii)) {
      if (narm) {
        continue;
      } else {
        return(R_NaReal);
      }
    }

    if (xii > xMax) {
      iMax = ii;
      xMax = xii;
    }

    if (ii % 1000000 == 0) {
      R_CheckUserInterrupt();
    }
  } /* for (ii ...) */

  /* Sum differences */
  sum = 0.0;
  for (ii=0; ii < nx; ii++) {
    if (ii == iMax) {
      continue;
    }

    /* Get the ii:th value */
    xii = x[ii];

    if (hasna && ISNAN(xii)) {
      if (narm) {
        continue;
      } else {
        return(R_NaReal);
      }
    } else {
      sum += exp(xii - xMax);
    }

    if (ii % 1000000 == 0) {
      R_CheckUserInterrupt();
    }
  } /* for (ii ...) */

  sum = xMax + log1p(sum);

  return(sum);
} /* logSumExp_double() */



/* 
 logSumExp_double_by(x):

  1. Scans for the maximum value of x=(x[0], x[by], ..., x[(n-1)*by])
     and copies the values to xx = (xx[0], xx[1], xx[2], ..., xx[n-1]),
     which *must* be preallocated.
  2. Computes result from 'xx'.

  NOTE: The above sweeps the "scattered" 'x' vector only once, and then 
  the "contigous" 'xx' vector once.  This is more likely to create 
  cache hits.
*/
double logSumExp_double_by(double *x, R_xlen_t nx, int narm, int hasna, int by, double *xx) {
  R_xlen_t ii, iMax, idx;
  double xii, xMax;
  LDOUBLE sum;

  /* Quick return? */
  if (nx == 0) {
    return(R_NegInf);
  } else if (nx == 1) {
    if (narm && ISNAN(x[0])) {
      return(R_NegInf);
    } else {
      return(x[0]);
    }
  }


  /* To increase the chances for cache hits below, which 
     sweeps through the data twice, we copy data into a
     temporary contigous vector while scanning for the
     maximum value. */

  /* Find the maximum value (and copy) */
  iMax = 0;
  xMax = x[0];
  xx[0] = xMax;
  idx = 0;
  for (ii=1; ii < nx; ii++) {
    /* Get the ii:th value */
    idx = idx + by;
    xii = x[idx];

    /* Copy */
    xx[ii] = xii;

    if (hasna && ISNAN(xii)) {
      if (narm) {
        continue;
      } else {
        return(R_NaReal);
      }
    }

    if (xii > xMax) {
      iMax = ii;
      xMax = xii;
    }

    if (ii % 1000000 == 0) {
      R_CheckUserInterrupt();
    }
  } /* for (ii ...) */


  /* Sum differences */
  sum = 0.0;
  for (ii=0; ii < nx; ii++) {
    if (ii == iMax) {
      continue;
    }

    /* Get the ii:th value */
    xii = xx[ii];

    if (hasna && ISNAN(xii)) {
      if (narm) {
        continue;
      } else {
        return(R_NaReal);
      }
    } else {
      sum += exp(xii - xMax);
    }

    if (ii % 1000000 == 0) {
      R_CheckUserInterrupt();
    }
  } /* for (ii ...) */

  sum = xMax + log1p(sum);

  return(sum);
} /* logSumExp_double_by() */



SEXP logSumExp(SEXP lx, SEXP naRm, SEXP hasNA) {
  int narm, hasna;
  double *x;
  R_xlen_t nx;

  /* Argument 'lx': */
  if (!isReal(lx)) {
    error("Argument 'lx' must be a numeric vector.");
  }

  /* Argument 'naRm': */
  if (!isLogical(naRm))
    error("Argument 'naRm' must be a single logical.");

  if (length(naRm) != 1)
    error("Argument 'naRm' must be a single logical.");

  narm = LOGICAL(naRm)[0];
  if (narm != TRUE && narm != FALSE)
    error("Argument 'naRm' must be either TRUE or FALSE.");

  /* Argument 'hasNA': */
  hasna = LOGICAL(hasNA)[0];


  /* Get the values */
  x = REAL(lx);
  nx = xlength(lx);

  return(Rf_ScalarReal(logSumExp_double(x, nx, narm, hasna)));
} /* logSumExp() */



SEXP rowLogSumExps(SEXP lx, SEXP dim, SEXP naRm, SEXP hasNA, SEXP byRow) {
  SEXP ans;
  int narm, hasna, byrow;
  R_xlen_t nrow, ncol, len, ii;
  double *x, *xx, *ans_ptr;

  /* Argument 'lx' and 'dim': */
  assertArgMatrix(lx, dim);
  nrow = INTEGER(dim)[0];
  ncol = INTEGER(dim)[1];
  if (!isReal(lx)) {
    error("Argument 'lx' must be a numeric vector.");
  }

  /* Argument 'naRm': */
  if (!isLogical(naRm))
    error("Argument 'naRm' must be a single logical.");

  if (length(naRm) != 1)
    error("Argument 'naRm' must be a single logical.");

  narm = asLogical(naRm);
  if (narm != TRUE && narm != FALSE)
    error("Argument 'naRm' must be either TRUE or FALSE.");

  /* Argument 'hasNA': */
  hasna = asLogical(hasNA);

  /* Argument 'byRow': */
  byrow = asInteger(byRow);

  /* R allocate a double vector of length 'nrow'
     Note that 'nrow' means 'ncol' if byrow=FALSE. */ 
  if (byrow) { len = nrow; } else { len = ncol; }
  PROTECT(ans = allocVector(REALSXP, len));
  ans_ptr = REAL(ans);

  /* Get the values */
  x = REAL(lx);

  if (byrow) {
    /* R allocate memory for row-vector 'xx' of length 'ncol'. 
       This will be taken care of by the R garbage collector later on. */
    xx = (double *) R_alloc(ncol, sizeof(double));

    for (ii=0; ii < nrow; ii++) {
      ans_ptr[ii] = logSumExp_double_by(x, ncol, narm, hasna, nrow, xx);
      /* Move to the beginning next row */
      x++;
    }
  } else {
    for (ii=0; ii < ncol; ii++) {
      ans_ptr[ii] = logSumExp_double(x, nrow, narm, hasna);
      /* Move to the beginning next column */
      x += nrow;
    }
  }

  UNPROTECT(1); /* PROTECT(ans = ...) */

  return(ans);
} /* rowLogSumExps() */



/***************************************************************************
 HISTORY:
 2013-05-02 [HB]
 o BUG FIX: Incorrectly used ISNAN() on an int variable as caught by the
   'cc' compiler on Solaris.  Reported by Brian Ripley upon CRAN submission.
 2013-04-30 [HB]
 o Created.
 **************************************************************************/
