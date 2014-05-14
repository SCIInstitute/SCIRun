/*
  Teem: Tools to process and visualize scientific data and images              
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "ten.h"
#include "privateTen.h"

tenInterpParm *
tenInterpParmNew(void) {
  tenInterpParm *tip;

  tip = AIR_CAST(tenInterpParm *, malloc(sizeof(tenInterpParm)));
  if (tip) {
    tip->verbose = AIR_FALSE;
    tip->convStep = 0.2;
    tip->minNorm = 0.0;
    tip->convEps = 0.0000000001;
    tip->wghtSumEps = 0.0000001;
    tip->enableRecurse = AIR_TRUE;
    tip->maxIter = 20;
    tip->numSteps = 100;
    tip->lengthFancy = AIR_FALSE;

    tip->allocLen = 0;
    tip->eval = NULL;
    tip->evec = NULL;
    tip->rtIn = NULL;
    tip->rtLog = NULL;
    tip->qIn = NULL;
    tip->qBuff = NULL;
    tip->qInter = NULL;

    tip->numIter = 0;
    tip->convFinal = AIR_NAN;
    tip->lengthShape = AIR_NAN;
    tip->lengthOrient = AIR_NAN;
  }
  return tip;
}



/*
** handles allocating all the various buffers that are needed for QGL
** interpolation, so that they are repeatedly allocated and freed
** between calls
*/
int
tenInterpParmBufferAlloc(tenInterpParm *tip, unsigned int num) {
  char me[]="tenInterpParmBufferAlloc", err[BIFF_STRLEN];

  if (0 == num) {
    /* user wants to free buffers for some reason */
    tip->eval = airFree(tip->eval);
    tip->evec = airFree(tip->evec);
    tip->rtIn = airFree(tip->rtIn);
    tip->rtLog = airFree(tip->rtLog);
    tip->qIn = airFree(tip->qIn);
    tip->qBuff = airFree(tip->qBuff);
    tip->qInter = airFree(tip->qInter);
    tip->allocLen = 0;
  } else if (1 == num) {
    sprintf(err, "%s: need num >= 2 (not %u)", me, num);
    biffAdd(TEN, err); return 1;
  } else if (num != tip->allocLen) {
    tip->eval = airFree(tip->eval);
    tip->evec = airFree(tip->evec);
    tip->rtIn = airFree(tip->rtIn);
    tip->rtLog = airFree(tip->rtLog);
    tip->qIn = airFree(tip->qIn);
    tip->qBuff = airFree(tip->qBuff);
    tip->qInter = airFree(tip->qInter);
    tip->eval = AIR_CAST(double *, calloc(3*num, sizeof(double)));
    tip->evec = AIR_CAST(double *, calloc(9*num, sizeof(double)));
    tip->rtIn = AIR_CAST(double *, calloc(3*num, sizeof(double)));
    tip->rtLog = AIR_CAST(double *, calloc(3*num, sizeof(double)));
    tip->qIn = AIR_CAST(double *, calloc(4*num, sizeof(double)));
    tip->qBuff = AIR_CAST(double *, calloc(4*num, sizeof(double)));
    tip->qInter = AIR_CAST(double *, calloc(num*num, sizeof(double)));
    if (!(tip->eval && tip->evec && 
          tip->rtIn && tip->rtLog &&
          tip->qIn && tip->qBuff && tip->qInter)) {
      sprintf(err, "%s: didn't alloc buffers (%p,%p,%p %p %p %p %p)", me,
              AIR_CAST(void *, tip->eval), AIR_CAST(void *, tip->evec),
              AIR_CAST(void *, tip->rtIn), AIR_CAST(void *, tip->rtLog),
              AIR_CAST(void *, tip->qIn), AIR_CAST(void *, tip->qBuff),
              AIR_CAST(void *, tip->qInter));
      biffAdd(TEN, err); return 1;
    }
    tip->allocLen = num;
  } 
  return 0;
}

tenInterpParm *
tenInterpParmCopy(tenInterpParm *tip) {
  char me[]="tenInterpParmCopy", err[BIFF_STRLEN];
  tenInterpParm *newtip;
  unsigned int num;

  num = tip->allocLen;
  newtip = tenInterpParmNew();
  if (newtip) {
    memcpy(newtip, tip, sizeof(tenInterpParm));
    /* manually set all pointers */
    newtip->allocLen = 0;
    newtip->eval = NULL;
    newtip->evec = NULL;
    newtip->rtIn = NULL;
    newtip->rtLog = NULL;
    newtip->qIn = NULL;
    newtip->qBuff = NULL;
    newtip->qInter = NULL;
    if (tenInterpParmBufferAlloc(newtip, num)) {
      sprintf(err, "%s: trouble allocating output", me);
      biffAdd(TEN, err); return NULL;
    }
    memcpy(newtip->eval, tip->eval, 3*num*sizeof(double));
    memcpy(newtip->evec, tip->evec, 9*num*sizeof(double));
    memcpy(newtip->rtIn, tip->rtIn, 3*num*sizeof(double));
    memcpy(newtip->rtLog, tip->rtLog, 3*num*sizeof(double));
    memcpy(newtip->qIn, tip->qIn, 4*num*sizeof(double));
    memcpy(newtip->qBuff, tip->qBuff, 4*num*sizeof(double));
    memcpy(newtip->qInter, tip->qInter, num*num*sizeof(double));
  }
  return newtip;
}

tenInterpParm *
tenInterpParmNix(tenInterpParm *tip) {

  if (tip) {
    airFree(tip->eval);
    airFree(tip->evec);
    airFree(tip->rtIn);
    airFree(tip->rtLog);
    airFree(tip->qIn);
    airFree(tip->qBuff);
    airFree(tip->qInter);
    free(tip);
  }
  return NULL;
}

/*
******** tenInterpTwo_d
**
** interpolates between two tensors, in various ways
**
** this is really only used for demo purposes; its not useful for
** doing real work in DTI fields.  So: its okay that its slow
** (e.g. for tenInterpTypeQuatGeoLox{K,R}, it recomputes the
** eigensystems at the endpoints for every call, even though they are
** apt to be the same between calls.
**
** this 
*/
void
tenInterpTwo_d(double oten[7], 
               const double tenA[7], const double tenB[7],
               int ptype, double aa,
               tenInterpParm *tip) {
  char me[]="tenInterpTwo_d";
  double logA[7], logB[7], tmp1[7], tmp2[7], logMean[7],
    mat1[9], mat2[9], mat3[9], sqrtA[7], isqrtA[7],
    mean[7], sqrtB[7], isqrtB[7],
    oeval[3], evalA[3], evalB[3], oevec[9], evecA[9], evecB[9];


  if (!( oten && tenA && tenB )) {
    /* got NULL pointer, but not using biff */
    if (oten) {
      TEN_T_SET(oten, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN,
                AIR_NAN, AIR_NAN, AIR_NAN);
    }
    return;
  }

  switch (ptype) {
  case tenInterpTypeLinear:
    TEN_T_LERP(oten, aa, tenA, tenB);
    break;
  case tenInterpTypeLogLinear:
    tenLogSingle_d(logA, tenA);
    tenLogSingle_d(logB, tenB);
    TEN_T_LERP(logMean, aa, logA, logB);
    tenExpSingle_d(oten, logMean);
    break;
  case tenInterpTypeAffineInvariant:
    tenSqrtSingle_d(sqrtA, tenA);
    tenInv_d(isqrtA, sqrtA);
    TEN_T2M(mat1, tenB);
    TEN_T2M(mat2, isqrtA);
    ELL_3M_MUL(mat3, mat1, mat2);   /*  B * is(A) */
    ELL_3M_MUL(mat1, mat2, mat3);   /*  is(A) * B * is(A) */
    TEN_M2T(tmp2, mat1);
    tenPowSingle_d(tmp1, tmp2, aa); /*  m = (is(A) * B * is(A))^aa */
    TEN_T2M(mat1, tmp1);
    TEN_T2M(mat2, sqrtA);
    ELL_3M_MUL(mat3, mat1, mat2);   /*  m * sqrt(A) */
    ELL_3M_MUL(mat1, mat2, mat3);   /*  sqrt(A) * m * sqrt(A) */
    TEN_M2T(oten, mat1);
    oten[0] = AIR_LERP(aa, tenA[0], tenB[0]);
    if (tip->verbose) {
      fprintf(stderr, "%s:\nA= %g %g %g   %g %g  %g\n"
              "B = %g %g %g   %g %g  %g\n"
              "foo = %g %g %g   %g %g  %g\n"
              "bar(%g) = %g %g %g   %g %g  %g\n", me,
              tenA[1], tenA[2], tenA[3], tenA[4], tenA[5], tenA[6],
              tenB[1], tenB[2], tenB[3], tenB[4], tenB[5], tenB[6],
              tmp1[1], tmp1[2], tmp1[3], tmp1[4], tmp1[5], tmp1[6],
              aa, oten[1], oten[2], oten[3], oten[4], oten[5], oten[6]);
    }
    break;
  case tenInterpTypeWang:
    /* HEY: this seems to be broken */
    TEN_T_LERP(mean, aa, tenA, tenB);    /* "A" = mean */
    tenLogSingle_d(logA, tenA);
    tenLogSingle_d(logB, tenB);
    TEN_T_LERP(logMean, aa, logA, logB); /* "B" = logMean */
    tenSqrtSingle_d(sqrtB, logMean);
    tenInv_d(isqrtB, sqrtB);
    TEN_T2M(mat1, mean);
    TEN_T2M(mat2, isqrtB);
    ELL_3M_MUL(mat3, mat1, mat2);
    ELL_3M_MUL(mat1, mat2, mat3);
    TEN_M2T(tmp1, mat1);
    tenSqrtSingle_d(oten, tmp1);
    oten[0] = AIR_LERP(aa, tenA[0], tenB[0]);
    break;
  case tenInterpTypeQuatGeoLoxK:
  case tenInterpTypeQuatGeoLoxR:
    tenEigensolve_d(evalA, evecA, tenA);
    tenEigensolve_d(evalB, evecB, tenB);
    if (tenInterpTypeQuatGeoLoxK == ptype) {
      tenQGLInterpTwoEvalK(oeval, evalA, evalB, aa);
    } else {
      tenQGLInterpTwoEvalR(oeval, evalA, evalB, aa);
    }
    tenQGLInterpTwoEvec(oevec, evecA, evecB, aa);
    tenMakeSingle_d(oten, AIR_LERP(aa, tenA[0], tenB[0]), oeval, oevec);
    break;
  case tenInterpTypeGeoLoxK:
  case tenInterpTypeGeoLoxR:
  case tenInterpTypeLoxK:
  case tenInterpTypeLoxR:
  default:
    /* (currently) no closed-form expression for these */
    TEN_T_SET(oten, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN,
              AIR_NAN, AIR_NAN, AIR_NAN);
    break;
  }
  return;
}

/*
** this NON-optionally uses biff
**
** for simplicity, a pre-allocated tenInterpParm MUST be passed,
** regardless of the interpolation requested
*/
int
tenInterpN_d(double tenOut[7],
             const double *tenIn, const double *wght,
             unsigned int num, int ptype, tenInterpParm *tip) {
  char me[]="tenInterpN_d", err[BIFF_STRLEN];
  unsigned int ii;
  double ww, cc, tenErr[7], tmp[7], wghtSum, eval[3], evec[9];

  TEN_T_SET(tenErr, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN, 
            AIR_NAN, AIR_NAN, AIR_NAN);
  /* wght can be NULL ==> equal 1/num weight for all */
  if (!(tenOut && tenIn && tip)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( num >= 2 )) {
    sprintf(err, "%s: need num >= 2 (not %u)", me, num);
    biffAdd(TEN, err); TEN_T_COPY(tenOut, tenErr); return 1;
  }
  if (airEnumValCheck(tenInterpType, ptype)) {
    sprintf(err, "%s: invalid %s %d", me, tenInterpType->name, ptype);
    biffAdd(TEN, err); TEN_T_COPY(tenOut, tenErr); return 1;
  }
  wghtSum = 0;
  for (ii=0; ii<num; ii++) {
    ww = wght ? wght[ii] : 1.0/num;
    wghtSum += ww;
  }
  if (!( AIR_IN_CL(1 - tip->wghtSumEps, wghtSum, 1 + tip->wghtSumEps) )) {
    sprintf(err, "%s: wght sum %g not within %g of 1.0", me,
            wghtSum, tip->wghtSumEps);
    biffAdd(TEN, err); TEN_T_COPY(tenOut, tenErr); return 1;
  }

  switch (ptype) {
  case tenInterpTypeLinear:
    TEN_T_SET(tenOut, 0,   0, 0, 0,   0, 0,   0);
    cc = 0;
    for (ii=0; ii<num; ii++) {
      ww = wght ? wght[ii] : 1.0/num;
      TEN_T_SCALE_INCR(tenOut, ww, tenIn + 7*ii);
      cc += ww*(tenIn + 7*ii)[0];
    }
    tenOut[0] = cc;
    break;
  case tenInterpTypeLogLinear:
    TEN_T_SET(tenOut, 0,   0, 0, 0,   0, 0,   0);
    cc = 0;
    for (ii=0; ii<num; ii++) {
      ww = wght ? wght[ii] : 1.0/num;
      tenLogSingle_d(tmp, tenIn + 7*ii);
      TEN_T_SCALE_INCR(tenOut, ww, tmp);
      cc += ww*(tenIn + 7*ii)[0];
    }
    tenOut[0] = cc;
    TEN_T_COPY(tmp, tenOut);
    tenExpSingle_d(tenOut, tmp);
    break;
  case tenInterpTypeAffineInvariant:
  case tenInterpTypeWang:
    sprintf(err, "%s: sorry, not implemented", me);
    biffAdd(TEN, err); TEN_T_COPY(tenOut, tenErr); return 1;
    break;
  case tenInterpTypeGeoLoxK:
  case tenInterpTypeGeoLoxR:
  case tenInterpTypeLoxK:
  case tenInterpTypeLoxR:
    sprintf(err, "%s: %s doesn't support averaging multiple tensors", me, 
            airEnumStr(tenInterpType, ptype));
    biffAdd(TEN, err); TEN_T_COPY(tenOut, tenErr); return 1;
    break;
  case tenInterpTypeQuatGeoLoxK:
  case tenInterpTypeQuatGeoLoxR:
    if (tenInterpParmBufferAlloc(tip, num)) {
      sprintf(err, "%s: trouble getting buffers", me);
      biffAdd(TEN, err); TEN_T_COPY(tenOut, tenErr); return 1;
    } else {
      cc = 0;
      for (ii=0; ii<num; ii++) {
        tenEigensolve_d(tip->eval + 3*ii, tip->evec + 9*ii, tenIn + 7*ii);
        ww = wght ? wght[ii] : 1.0/num;
        cc += ww*(tenIn + 7*ii)[0];
      }
      if (_tenQGLInterpNEval(eval, tip->eval, wght, num, ptype, tip)
          || _tenQGLInterpNEvec(evec, tip->evec, wght, num, tip)) {
        sprintf(err, "%s: trouble computing", me);
        biffAdd(TEN, err); TEN_T_COPY(tenOut, tenErr); return 1;
      }
      tenMakeSingle_d(tenOut, cc, eval, evec);
    }
    break;
  }
  
  return 0;
}

int
_tenInterpGeoLoxRelaxOne(Nrrd *nodata, Nrrd *ntdata, Nrrd *nigrtdata,
                       unsigned int ii, int rotnoop, double scl,
                       tenInterpParm *tip) {
  char me[]="_tenInterpGeoLoxRelaxOne", err[BIFF_STRLEN];
  double *tdata, *odata, *igrtdata, *tt[5], *igrt[5][6], d02[7], d24[7],
    len02, len24, tmp, tng[7], correct, half, update[7];
  unsigned int jj, NN;

  NN = (ntdata->axis[1].size-1)/2;
  half = (NN+1)/2;

  if (tip->verbose) {
    fprintf(stderr, "---- %u --> %u %u %u %u %u\n", ii,
            2*ii - 2, 2*ii - 1, 2*ii, 2*ii + 1, 2*ii + 2);
  }
  tdata = AIR_CAST(double *, ntdata->data);
  odata = AIR_CAST(double *, nodata->data);
  tt[0] = tdata + 7*(2*ii - 2);
  tt[1] = tdata + 7*(2*ii - 1); /* unused */
  tt[2] = tdata + 7*(2*ii + 0);
  tt[3] = tdata + 7*(2*ii + 1); /* unused */
  tt[4] = tdata + 7*(2*ii + 2);
  igrtdata = AIR_CAST(double *, nigrtdata->data);
  for (jj=0; jj<6; jj++) {
    igrt[0][jj] = igrtdata + 7*(jj + 6*(2*ii - 2)); /* unused */
    igrt[1][jj] = igrtdata + 7*(jj + 6*(2*ii - 1));
    igrt[2][jj] = igrtdata + 7*(jj + 6*(2*ii + 0));
    igrt[3][jj] = igrtdata + 7*(jj + 6*(2*ii + 1));
    igrt[4][jj] = igrtdata + 7*(jj + 6*(2*ii + 2)); /* unused */
  }

  /* re-align [1] and [3] bases relative to [2] */
  /* HEY: should I be worrying about aligning the mode normal
     when it had to be computed from eigenvectors? */
  for (jj=3; jj<6; jj++) {
    if (TEN_T_DOT(igrt[1][jj], igrt[2][jj]) < 0) {
      TEN_T_SCALE(igrt[1][jj], -1, igrt[1][jj]);
    }
    if (TEN_T_DOT(igrt[3][jj], igrt[2][jj]) < 0) {
      TEN_T_SCALE(igrt[3][jj], -1, igrt[1][jj]);
    }
  }  

  TEN_T_SUB(tng, tt[4], tt[0]);
  tmp = 1.0/TEN_T_NORM(tng);
  TEN_T_SCALE(tng, tmp, tng);

  TEN_T_SUB(d02, tt[2], tt[0]);
  TEN_T_SUB(d24, tt[4], tt[2]);
  TEN_T_SET(update, 1,   0, 0, 0,   0, 0,   0);
  for (jj=0; jj<(rotnoop ? 3u : 6u); jj++) {
    len02 = TEN_T_DOT(igrt[1][jj], d02);
    len24 = TEN_T_DOT(igrt[3][jj], d24);
    correct = (len24 - len02)/2;
    TEN_T_SCALE_INCR(update, correct*scl, igrt[2][jj]);
    if (tip->verbose) {
      fprintf(stderr, "igrt[1][%u] = %g %g %g   %g %g   %g\n", jj,
              igrt[1][jj][1], igrt[1][jj][2], igrt[1][jj][3],
              igrt[1][jj][4], igrt[1][jj][5], igrt[1][jj][6]);
      fprintf(stderr, "igrt[3][%u] = %g %g %g   %g %g   %g\n", jj,
              igrt[3][jj][1], igrt[3][jj][2], igrt[3][jj][3],
              igrt[3][jj][4], igrt[3][jj][5], igrt[3][jj][6]);
      fprintf(stderr, "(jj=%u) len = %g %g --> (d = %g) "
              "update = %g %g %g     %g %g   %g\n",
              jj, len02, len24,
              TEN_T_DOT(igrt[2][0], update),
              update[1], update[2], update[3],
              update[4], update[5], update[6]);
    }
  }
  if (rotnoop) {
    double avg[7], diff[7], len;
    TEN_T_LERP(avg, 0.5, tt[0], tt[4]);
    TEN_T_SUB(diff, avg, tt[2]);
    for (jj=0; jj<3; jj++) {
      len = TEN_T_DOT(igrt[2][jj], diff);
      TEN_T_SCALE_INCR(diff, -len, igrt[2][jj]);
    }
    TEN_T_SCALE_INCR(update, scl*0.2, diff);  /* HEY: scaling is a hack */
    if (tip->verbose) {
      fprintf(stderr, "(rotnoop) (d = %g) "
              "update = %g %g %g     %g %g   %g\n",
              TEN_T_DOT(igrt[2][0], update),
              update[1], update[2], update[3],
              update[4], update[5], update[6]);
    }
  }
  /*
  TEN_T_SUB(d02, tt[2], tt[0]);
  TEN_T_SUB(d24, tt[4], tt[2]);
  len02 = TEN_T_DOT(tng, d02);
  len24 = TEN_T_DOT(tng, d24);
  correct = (len24 - len02);
  TEN_T_SCALE_INCR(update, scl*correct, tng);
  */

  if (!TEN_T_EXISTS(update)) {
    sprintf(err, "%s: computed non-existant update (step-size too big?)", me);
    biffAdd(TEN, err); return 1;
  }

  TEN_T_ADD(odata + 7*(2*ii + 0), tt[2], update);

  return 0;
}

void
_tenInterpGeoLoxIGRT(double *igrt, double *ten, int useK, int rotNoop,
                   double minnorm) {
  /* char me[]="_tenInterpGeoLoxIGRT"; */
  double eval[3], evec[9];

  if (useK) {
    tenInvariantGradientsK_d(igrt + 7*0, igrt + 7*1, igrt + 7*2, ten, minnorm);
  } else {
    tenInvariantGradientsR_d(igrt + 7*0, igrt + 7*1, igrt + 7*2, ten, minnorm);
  }
  if (rotNoop) {
    /* these shouldn't be used */
    TEN_T_SET(igrt + 7*3, 1, AIR_NAN, AIR_NAN, AIR_NAN,
              AIR_NAN, AIR_NAN, AIR_NAN);
    TEN_T_SET(igrt + 7*4, 1, AIR_NAN, AIR_NAN, AIR_NAN,
              AIR_NAN, AIR_NAN, AIR_NAN);
    TEN_T_SET(igrt + 7*5, 1, AIR_NAN, AIR_NAN, AIR_NAN,
              AIR_NAN, AIR_NAN, AIR_NAN);
  } else {
    tenEigensolve_d(eval, evec, ten);
    tenRotationTangents_d(igrt + 7*3, igrt + 7*4, igrt + 7*5, evec);
  }
  return;
}

/*
** if "doubling" is non-zero, this assumes that the real
** vertices are on the even-numbered indices:
** (0   1   2   3   4)
**  0   2   4   6   8 --> size=9 --> NN=4
**    1   3   5   7
*/
double
tenInterpPathLength(Nrrd *ntt, int doubleVerts, int fancy, int shape) {
  double *tt, len, diff[7], *tenA, *tenB;
  unsigned int ii, NN;

  tt = AIR_CAST(double *, ntt->data);
  if (doubleVerts) {
    NN = AIR_CAST(unsigned int, (ntt->axis[1].size-1)/2);
  } else {
    NN = AIR_CAST(unsigned int, ntt->axis[1].size-1);
  }
  len = 0;
  for (ii=0; ii<NN; ii++) {
    if (doubleVerts) {
      tenA = tt + 7*2*(ii + 1);
      tenB = tt + 7*2*(ii + 0);
    } else {
      tenA = tt + 7*(ii + 1);
      tenB = tt + 7*(ii + 0);
    }
    TEN_T_SUB(diff, tenA, tenB);
    if (fancy) {
      double mean[7], igrt[7*6], dot, incr;
      unsigned int ii, lo, hi;

      TEN_T_LERP(mean, 0.5, tenA, tenB);
      _tenInterpGeoLoxIGRT(igrt, mean, AIR_FALSE, AIR_FALSE, 0.0);
      if (shape) {
        lo = 0;
        hi = 2;
      } else {
        lo = 3;
        hi = 5;
      }
      incr = 0;
      for (ii=lo; ii<=hi; ii++) {
        dot = TEN_T_DOT(igrt + 7*ii, diff);
        incr += dot*dot;
      }
      len += sqrt(incr);
    } else {
      len += TEN_T_NORM(diff);
    }
  }
  return len;
}

double
_tenPathSpacingEqualize(Nrrd *nout, Nrrd *nin) {
  /* char me[]="_tenPathSpacingEqualize"; */
  double *in, *out, len, diff[7],
    lenTotal,  /* total length of input */
    lenStep,   /* correct length on input polyline between output vertices */
    lenIn,     /* length along input processed so far */
    lenHere,   /* length of segment associated with current input index */
    lenRmn,    /* length along past input segments as yet unmapped to output */
    *tenHere, *tenNext;
  unsigned int idxIn, idxOut, NN;

  in = AIR_CAST(double *, nin->data);
  out = AIR_CAST(double *, nout->data);
  NN = (nin->axis[1].size-1)/2;
  lenTotal = tenInterpPathLength(nin, AIR_TRUE, AIR_FALSE, AIR_FALSE);
  lenStep = lenTotal/NN;
  /*
  fprintf(stderr, "!%s: lenTotal/NN = %g/%u = %g = lenStep\n", me,
          lenTotal, NN, lenStep);
  */
  TEN_T_COPY(out + 7*2*(0 + 0), in + 7*2*(0 + 0));
  lenIn = lenRmn = 0;
  idxOut = 1;
  for (idxIn=0; idxIn<NN; idxIn++) {
    tenNext = in + 7*2*(idxIn + 1);
    tenHere = in + 7*2*(idxIn + 0);
    TEN_T_SUB(diff, tenNext, tenHere);
    lenHere = TEN_T_NORM(diff);
    /*
    fprintf(stderr, "!%s(%u): %g + %g >(%s)= %g\n", me, idxIn,
            lenRmn, lenHere, 
            (lenRmn + lenHere >= lenStep ? "yes" : "no"),
            lenStep);
    */
    if (lenRmn + lenHere >= lenStep) {
      len = lenRmn + lenHere;
      while (len > lenStep) {
        len -= lenStep;
        /*
        fprintf(stderr, "!%s(%u): len = %g -> %g\n", me, idxIn,
                len + lenStep, len);
        */
        TEN_T_AFFINE(out + 7*(2*idxOut + 0),
                     lenHere, len, 0, tenHere, tenNext);
        /*
        fprintf(stderr, "!%s(%u): out[%u] ~ %g\n", me, idxIn, idxOut,
                AIR_AFFINE(lenHere, len, 0, 0, 1));
        */
        idxOut++;
      }
      lenRmn = len;
    } else {
      lenRmn += lenHere;
      /*
      fprintf(stderr, "!%s(%u):   (==> lenRmn = %g -> %g)\n", me, idxIn,
              lenRmn - lenHere, lenRmn);
      */
    }
    /* now lenRmn < lenStep */
    lenIn += lenHere;
  }
  /* copy very last one in case we didn't get to it somehow */
  TEN_T_COPY(out + 7*2*(NN + 0), in + 7*2*(NN + 0));

  /* fill in vertex mid-points */
  for (idxOut=0; idxOut<NN; idxOut++) {
    TEN_T_LERP(out + 7*(2*idxOut + 1), 
               0.5, out + 7*(2*idxOut + 0), out + 7*(2*idxOut + 2));
  }
  return lenTotal;
}

int
_tenInterpGeoLoxPolyLine(Nrrd *ngeod, unsigned int *numIter,
                         const double tenA[7], const double tenB[7],
                         unsigned int NN, int useK, int rotnoop,
                         tenInterpParm *tip) {
  char me[]="_tenInterpGeoLoxPolyLine", err[BIFF_STRLEN];
  Nrrd *nigrt, *ntt, *nss, *nsub;
  double *igrt, *geod, *tt, len, newlen;
  unsigned int ii;
  airArray *mop;

  if (!(ngeod && numIter && tenA && tenB)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!(NN >= 2)) {
    sprintf(err, "%s: # steps %u too small", me, NN);
    biffAdd(TEN, err); return 1;
  }

  mop = airMopNew();
  ntt = nrrdNew();
  airMopAdd(mop, ntt, (airMopper)nrrdNuke, airMopAlways);
  nss = nrrdNew();
  airMopAdd(mop, nss, (airMopper)nrrdNuke, airMopAlways);
  nigrt = nrrdNew();
  airMopAdd(mop, nigrt, (airMopper)nrrdNuke, airMopAlways);
  nsub = nrrdNew();
  airMopAdd(mop, nsub, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdMaybeAlloc_va(ngeod, nrrdTypeDouble, 2,
                        AIR_CAST(size_t, 7),
                        AIR_CAST(size_t, NN+1))
      || nrrdMaybeAlloc_va(ntt, nrrdTypeDouble, 2,
                           AIR_CAST(size_t, 7),
                           AIR_CAST(size_t, 2*NN + 1))
      || nrrdMaybeAlloc_va(nigrt, nrrdTypeDouble, 3,
                           AIR_CAST(size_t, 7),
                           AIR_CAST(size_t, 6),
                           AIR_CAST(size_t, 2*NN + 1))) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  geod = AIR_CAST(double *, ngeod->data);
  tt = AIR_CAST(double *, ntt->data);
  igrt = AIR_CAST(double *, nigrt->data);

  *numIter = 0;
  if (NN > 14 && tip->enableRecurse) {
    unsigned int subIter;
    int E;
    NrrdResampleContext *rsmc;
    double kparm[3] = {1.0, 0.0, 0.5};
    /* recurse and find geodesic with smaller number of vertices */
    if (_tenInterpGeoLoxPolyLine(nsub, &subIter, tenA, tenB,
                                 NN/2, useK, rotnoop, tip)) {
      sprintf(err, "%s: problem with recursive call", me);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
    /* upsample coarse geodesic to higher resolution */
    rsmc = nrrdResampleContextNew();
    airMopAdd(mop, rsmc, (airMopper)nrrdResampleContextNix, airMopAlways);
    E = AIR_FALSE;
    if (!E) E |= nrrdResampleDefaultCenterSet(rsmc, nrrdCenterNode);
    if (!E) E |= nrrdResampleNrrdSet(rsmc, nsub);
    if (!E) E |= nrrdResampleKernelSet(rsmc, 0, NULL, NULL);
    if (!E) E |= nrrdResampleKernelSet(rsmc, 1, nrrdKernelTent, kparm);
    if (!E) E |= nrrdResampleSamplesSet(rsmc, 1, 2*NN + 1);
    if (!E) E |= nrrdResampleRangeFullSet(rsmc, 1);
    if (!E) E |= nrrdResampleBoundarySet(rsmc, nrrdBoundaryBleed);
    if (!E) E |= nrrdResampleTypeOutSet(rsmc, nrrdTypeDefault);
    if (!E) E |= nrrdResampleRenormalizeSet(rsmc, AIR_TRUE);
    if (!E) E |= nrrdResampleExecute(rsmc, ntt);
    if (E) {
      sprintf(err, "%s: problem upsampling course solution", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
    *numIter += subIter;
  } else {
    /* initialize the path, including all the segment midpoints */
    for (ii=0; ii<=2*NN; ii++) {
      TEN_T_AFFINE(tt + 7*ii, 0, ii, 2*NN, tenA, tenB);
    }
  }
  for (ii=0; ii<=2*NN; ii++) {
    _tenInterpGeoLoxIGRT(igrt + 7*6*ii, tt + 7*ii, useK, rotnoop,
                         tip->minNorm);
  }
  nrrdCopy(nss, ntt);
  
  newlen = tenInterpPathLength(ntt, AIR_TRUE, AIR_FALSE, AIR_FALSE);
  do {
    unsigned int lo, hi;
    int dd;
    len = newlen;
    if (0 == *numIter % 2) {
      lo = 1;
      hi = NN;
      dd = 1;
    } else {
      lo = NN-1;
      hi = 0;
      dd = -1;
    }
    if (tip->verbose) {
      fprintf(stderr, "%s: ======= iter = %u (NN=%u)\n", me, *numIter, NN);
    }
    for (ii=lo; ii!=hi; ii+=dd) {
      double sclHack;
      sclHack = ii*4.0/NN - ii*ii*4.0/NN/NN;
      if (_tenInterpGeoLoxRelaxOne(nss, ntt, nigrt, ii, rotnoop,
                                   sclHack*tip->convStep, tip)) {
        sprintf(err, "%s: problem on vert %u, iter %u\n", me, ii, *numIter);
        biffAdd(TEN, err); return 1;
      }
    }
    newlen = _tenPathSpacingEqualize(ntt, nss);
    /* try doing this less often */
    for (ii=0; ii<=2*NN; ii++) {
      _tenInterpGeoLoxIGRT(igrt + 7*6*ii, tt + 7*ii, useK, rotnoop,
                           tip->minNorm);
    }
    *numIter += 1;
  } while ((0 == tip->maxIter || *numIter < tip->maxIter)
           && 2*AIR_ABS(newlen - len)/(newlen + len) > tip->convEps);

  /* copy final result to output */
  for (ii=0; ii<=NN; ii++) {
    TEN_T_COPY(geod + 7*ii, tt + 7*2*ii);
  }
  /* values from outer-most recursion will stick */
  tip->numIter = *numIter;
  tip->convFinal = 2*AIR_ABS(newlen - len)/(newlen + len);

  airMopOkay(mop);
  return 0;
}

int
tenInterpTwoDiscrete_d(Nrrd *nout, 
                       const double tenA[7], const double tenB[7],
                       int ptype, unsigned int num,
                       tenInterpParm *_tip) {
  char me[]="tenInterpTwoDiscrete_d", err[BIFF_STRLEN];
  double *out;
  unsigned int ii;
  airArray *mop;
  tenInterpParm *tip;

  if (!nout) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenInterpType, ptype)) {
    sprintf(err, "%s: path type %d not a valid %s", me, ptype, 
            tenInterpType->name);
    biffAdd(TEN, err); return 1;
  }

  mop = airMopNew();
  if (_tip) {
    tip = _tip;
  } else {
    tip = tenInterpParmNew();
    airMopAdd(mop, tip, (airMopper)tenInterpParmNix, airMopAlways);
  }
  if (!( num >= 2 )) {
    sprintf(err, "%s: need num >= 2 (not %u)", me, num);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  if (nrrdMaybeAlloc_va(nout, nrrdTypeDouble, 2, 
                        AIR_CAST(size_t, 7),
                        AIR_CAST(size_t, num))) {
    sprintf(err, "%s: trouble allocating output", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  out = AIR_CAST(double *, nout->data);

  if (ptype == tenInterpTypeLinear
      || ptype == tenInterpTypeLogLinear
      || ptype == tenInterpTypeAffineInvariant
      || ptype == tenInterpTypeWang
      || ptype == tenInterpTypeQuatGeoLoxK
      || ptype == tenInterpTypeQuatGeoLoxR) {
    for (ii=0; ii<num; ii++) {
      double *oten;
      oten = out + 7*ii;
      /* yes, this is often doing a lot of needless recomputations. */
      tenInterpTwo_d(out + 7*ii, tenA, tenB, 
                     ptype, (double)ii/(num-1), tip);
    }
  } else if (ptype == tenInterpTypeGeoLoxK
             || ptype == tenInterpTypeGeoLoxR
             || ptype == tenInterpTypeLoxK
             || ptype == tenInterpTypeLoxR) {
    /* we have slow iterative code for these */
    unsigned int numIter;
    int useK, rotnoop;
    
    useK = (tenInterpTypeGeoLoxK == ptype
            || tenInterpTypeLoxK == ptype);
    rotnoop = (tenInterpTypeGeoLoxK == ptype
               || tenInterpTypeGeoLoxR == ptype);
    fprintf(stderr, "!%s: useK = %d, rotnoop = %d\n", me, useK, rotnoop);
    if (_tenInterpGeoLoxPolyLine(nout, &numIter,
                                 tenA, tenB,
                                 num, useK, rotnoop, tip)) {
      sprintf(err, "%s: trouble finding path", me);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
  } else {
    sprintf(err, "%s: sorry, interp for path %s not implemented", me,
            airEnumStr(tenInterpType, ptype));
    biffAdd(TEN, err); airMopError(mop); return 1;
  }

  airMopOkay(mop); 
  return 0;
}

double
tenInterpDistanceTwo_d(const double tenA[7], const double tenB[7],
                    int ptype, tenInterpParm *_tip) {
  char me[]="tenInterpDistanceTwo_d", *err;
  tenInterpParm *tip;
  airArray *mop;
  double ret, diff[7], logA[7], logB[7], invA[7], det, siA[7],
    mat1[9], mat2[9], mat3[9], logDiff[7];
  Nrrd *npath;
  
  if (!( tenA && tenB && !airEnumValCheck(tenInterpType, ptype) )) {
    return AIR_NAN;
  }

  mop = airMopNew();
  switch (ptype) {
  case tenInterpTypeLinear:
    TEN_T_SUB(diff, tenA, tenB);
    ret = TEN_T_NORM(diff);
    break;
  case tenInterpTypeLogLinear:
    tenLogSingle_d(logA, tenA);
    tenLogSingle_d(logB, tenB);
    TEN_T_SUB(diff, logA, logB);
    ret = TEN_T_NORM(diff);
    break;
  case tenInterpTypeAffineInvariant:
    TEN_T_INV(invA, tenA, det);
    tenSqrtSingle_d(siA, invA);
    TEN_T2M(mat1, tenB);
    TEN_T2M(mat2, siA);
    ell_3m_mul_d(mat3, mat1, mat2);
    ell_3m_mul_d(mat1, mat2, mat3);
    TEN_M2T(diff, mat1);
    tenLogSingle_d(logDiff, diff);
    ret = TEN_T_NORM(logDiff);
    break;
  case tenInterpTypeGeoLoxK:
  case tenInterpTypeGeoLoxR:
  case tenInterpTypeLoxK:
  case tenInterpTypeLoxR:
  case tenInterpTypeQuatGeoLoxK:
  case tenInterpTypeQuatGeoLoxR:
    npath = nrrdNew();
    airMopAdd(mop, npath, (airMopper)nrrdNuke, airMopAlways);
    if (_tip) {
      tip = _tip;
    } else {
      tip = tenInterpParmNew();
      airMopAdd(mop, tip, (airMopper)tenInterpParmNix, airMopAlways);
    }
    if (tenInterpTwoDiscrete_d(npath, tenA, tenB, ptype,
                               tip->numSteps, tip)) {
      airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble computing path:\n%s\n", me, err);
      airMopError(mop); return AIR_NAN;
    }
    ret = tenInterpPathLength(npath, AIR_FALSE, AIR_FALSE, AIR_FALSE);
    if (tip->lengthFancy) {
      tip->lengthShape = tenInterpPathLength(npath, AIR_FALSE,
                                             AIR_TRUE, AIR_TRUE);
      tip->lengthOrient = tenInterpPathLength(npath, AIR_FALSE,
                                              AIR_TRUE, AIR_FALSE);
    }
    break;
  case tenInterpTypeWang:
  default:
    fprintf(stderr, "%s: unimplemented %s %d!!!!\n", me,
            tenInterpType->name, ptype);
    ret = AIR_NAN;
    break;
  }

  airMopOkay(mop);
  return ret;
}

/*
** actually, the input nrrds don't have to be 3D ...
*/
int
tenInterpMulti3D(Nrrd *nout, const Nrrd *const *nin, const double *wght,
                 unsigned int ninLen, int ptype, tenInterpParm *_tip) {
  char me[]="tenInterpMulti3D", err[BIFF_STRLEN];
  unsigned int ninIdx;
  size_t II, NN;
  double (*lup)(const void *, size_t), (*ins)(void *, size_t, double),
    *tbuff;
  tenInterpParm *tip;
  airArray *mop;

  /* allow NULL wght, to signify equal weighting */
  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( ninLen > 0 )) {
    sprintf(err, "%s: need at least 1 nin, not 0", me);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenInterpType, ptype)) {
    sprintf(err, "%s: invalid %s %d", me, 
            tenInterpType->name, ptype);
    biffAdd(TEN, err); return 1;
  }

  if (tenTensorCheck(nin[0], nrrdTypeDefault, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: first nrrd not a tensor array", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( nrrdTypeFloat == nin[0]->type ||
         nrrdTypeDouble == nin[0]->type )) {
    sprintf(err, "%s: need type %s or %s (not %s) in first nrrd", me,
            airEnumStr(nrrdType, nrrdTypeFloat),
            airEnumStr(nrrdType, nrrdTypeDouble),
            airEnumStr(nrrdType, nin[0]->type));
    biffAdd(TEN, err); return 1;
  }
  for (ninIdx=1; ninIdx<ninLen; ninIdx++) {
    if (tenTensorCheck(nin[ninIdx], nrrdTypeDefault, AIR_FALSE, AIR_TRUE)) {
      sprintf(err, "%s: nin[%u] not a tensor array", me, ninIdx);
      biffAdd(TEN, err); return 1;
    }
    if (!nrrdSameSize(nin[0], nin[ninIdx], AIR_TRUE)) {
      sprintf(err, "%s: nin[0] doesn't match nin[%u]", me, ninIdx);
      biffMove(TEN, err, NRRD); return 1;
    }
    if (nin[0]->type != nin[ninIdx]->type) {
      sprintf(err, "%s: nin[0] type (%s) != nin[%u] type (%s)", me, 
              airEnumStr(nrrdType, nin[0]->type),
              ninIdx, airEnumStr(nrrdType, nin[ninIdx]->type));
      biffAdd(TEN, err); return 1;
    }
  }

  mop = airMopNew();
  if (nrrdCopy(nout, nin[0])) {
    sprintf(err, "%s: couldn't initialize output", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }
  if (_tip) {
    tip = _tip;
  } else {
    tip = tenInterpParmNew();
    airMopAdd(mop, tip, (airMopper)tenInterpParmNix, airMopAlways);
  }
  tbuff = AIR_CAST(double *, calloc(7*ninLen, sizeof(double)));
  if (!tbuff) {
    sprintf(err, "%s: couldn't allocate tensor buff", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  ins = nrrdDInsert[nin[0]->type];
  lup = nrrdDLookup[nin[0]->type];
  NN = nrrdElementNumber(nin[0])/7;
  for (II=0; II<NN; II++) {
    double tenOut[7];
    unsigned int tt;
    for (ninIdx=0; ninIdx<ninLen; ninIdx++) {
      for (tt=0; tt<7; tt++) {
        tbuff[tt + 7*ninIdx] = lup(nin[ninIdx]->data, tt + 7*II);
      }
    }
    if (tenInterpN_d(tenOut, tbuff, wght, ninLen, ptype, tip)) {
      sprintf(err, "%s: trouble on sample " _AIR_SIZE_T_CNV, me, II);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
    for (tt=0; tt<7; tt++) {
      ins(nout->data, tt + 7*II, tenOut[tt]);
    }
  }
  
  airMopOkay(mop); 
  return 0;
}
