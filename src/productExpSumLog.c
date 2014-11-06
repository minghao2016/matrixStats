/***************************************************************************
 Public methods:
 SEXP productExpSumLog(SEXP x, SEXP naRm, SEXP hasNA)

 Copyright Henrik Bengtsson, 2014
 **************************************************************************/
/* Include R packages */
#include <Rdefines.h>

#define METHOD productExpSumLog

#define X_TYPE 'i'
#include "productExpSumLog_TYPE-template.h"

#define X_TYPE 'r'
#include "productExpSumLog_TYPE-template.h"

#undef METHOD



SEXP productExpSumLog(SEXP x, SEXP naRm, SEXP hasNA) {
  SEXP ans = NILSXP;
  double res;
  int narm, hasna;

  /* Argument 'x': */
  if (!isVector(x))
    error("Argument 'x' must be a vector.");

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

  /* Double matrices are more common to use. */
  if (isReal(x)) {
    res = productExpSumLog_Real(REAL(x), XLENGTH(x), narm, hasna);
  } else if (isInteger(x)) {
    res = productExpSumLog_Integer(INTEGER(x), XLENGTH(x), narm, hasna);
  } else {
    error("Argument 'x' must be numeric.");
  }

  /* Return results */
  PROTECT(ans = allocVector(REALSXP, 1));
  REAL(ans)[0] = res;
  UNPROTECT(1);

  return(ans);
} // productExpSumLog()


/***************************************************************************
 HISTORY:
 2014-06-04 [HB]
  o Created.
 **************************************************************************/
