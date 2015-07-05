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

typedef struct {
  double weight[3], amount, target;            /* tenSizeNormalize */
  /* amount: tenSizeScale */
  double scale; int fixDet; int makePositive;  /* tenAnisoScale */
  double min, max;                             /* tenEigenvalueClamp */
  double expo;                                 /* tenEigenvaluePower */
  double val;                                  /* tenEigenvalueAdd */
} funcParm;

enum {
  funcUnknown,
  funcSizeNormalize,
  funcSizeScale,
  funcAnisoScale,
  funcEigenvalueClamp,
  funcEigenvaluePower,
  funcEigenvalueAdd,
  funcLog,
  funcExp,
  funcLast
};

static int
theFunc(Nrrd *nout, const Nrrd *nin, int func, funcParm *parm) {
  char me[]="theFunc", err[BIFF_STRLEN];
  float *tin, *tout, eval[3], evec[9], weight[3], size, mean;
  size_t NN, II;
  unsigned int ri;

  if (!AIR_IN_OP(funcUnknown, func, funcLast)) {
    sprintf(err, "%s: given func %d out of range [%d,%d]", me, func,
            funcUnknown+1, funcLast-1);
    biffAdd(TEN, err); return 1;
  }
  if (!(nout && nin && parm)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: couldn't allocate output", me);
      biffMove(TEN, err, NRRD); return 1;
    }
  }

  tin = (float*)(nin->data);
  tout = (float*)(nout->data);
  NN = nrrdElementNumber(nin)/7;
  switch(func) {
  case funcSizeNormalize:
    ELL_3V_COPY_TT(weight, float, parm->weight);
    size = weight[0] + weight[1] + weight[2];
    if (!size) {
      sprintf(err, "%s: some of eigenvalue weights is zero", me);
      biffAdd(TEN, err); return 1;
    }
    weight[0] /= size;
    weight[1] /= size;
    weight[2] /= size;
    for (II=0; II<=NN-1; II++) {
      tenEigensolve_f(eval, evec, tin);
      size = (weight[0]*AIR_ABS(eval[0])
              + weight[1]*AIR_ABS(eval[1])
              + weight[2]*AIR_ABS(eval[2]));
      ELL_3V_SET_TT(eval, float,
                    AIR_AFFINE(0, parm->amount, 1,
                               eval[0], parm->target*eval[0]/size),
                    AIR_AFFINE(0, parm->amount, 1,
                               eval[1], parm->target*eval[1]/size),
                    AIR_AFFINE(0, parm->amount, 1,
                               eval[2], parm->target*eval[2]/size));
      tenMakeSingle_f(tout, tin[0], eval, evec);
      tin += 7;
      tout += 7;
    }
    break;
  case funcSizeScale:
    for (II=0; II<=NN-1; II++) {
      TEN_T_SET_TT(tout, float,
                   tin[0],
                   parm->amount*tin[1],
                   parm->amount*tin[2],
                   parm->amount*tin[3],
                   parm->amount*tin[4],
                   parm->amount*tin[5],
                   parm->amount*tin[6]);
      tin += 7;
      tout += 7;
    }
    break;
  case funcAnisoScale:
    for (II=0; II<=NN-1; II++) {
      tenEigensolve_f(eval, evec, tin);
      if (parm->fixDet) {
        eval[0] = AIR_MAX(eval[0], 0.00001f);
        eval[1] = AIR_MAX(eval[1], 0.00001f);
        eval[2] = AIR_MAX(eval[2], 0.00001f);
        ELL_3V_SET_TT(eval, float, log(eval[0]), log(eval[1]), log(eval[2]));
      }
      mean = (eval[0] + eval[1] + eval[2])/3.0f;
      ELL_3V_SET_TT(eval, float,
                    AIR_LERP(parm->scale, mean, eval[0]),
                    AIR_LERP(parm->scale, mean, eval[1]),
                    AIR_LERP(parm->scale, mean, eval[2]));
      if (parm->fixDet) {
        ELL_3V_SET_TT(eval, float, exp(eval[0]), exp(eval[1]), exp(eval[2]));
      }
      if (eval[2] < 0 && parm->makePositive) {
        eval[0] = AIR_MAX(eval[0], 0.0f);
        eval[1] = AIR_MAX(eval[1], 0.0f);
        eval[2] = AIR_MAX(eval[2], 0.0f);
      }
      tenMakeSingle_f(tout, tin[0], eval, evec);
      tin += 7;
      tout += 7;
    }
    break;
  case funcEigenvalueClamp:
    for (II=0; II<=NN-1; II++) {
      tenEigensolve_f(eval, evec, tin);
      if (AIR_EXISTS(parm->min)) {
        ELL_3V_SET_TT(eval, float,
                      AIR_MAX(eval[0], parm->min),
                      AIR_MAX(eval[1], parm->min),
                      AIR_MAX(eval[2], parm->min));
      }
      if (AIR_EXISTS(parm->max)) {
        ELL_3V_SET_TT(eval, float,
                      AIR_MIN(eval[0], parm->max),
                      AIR_MIN(eval[1], parm->max),
                      AIR_MIN(eval[2], parm->max));
      }
      tenMakeSingle_f(tout, tin[0], eval, evec);
      tin += 7;
      tout += 7;
    }
    break;
  case funcEigenvaluePower:
    for (II=0; II<=NN-1; II++) {
      tenEigensolve_f(eval, evec, tin);
      ELL_3V_SET_TT(eval, float,
                    pow(eval[0], parm->expo),
                    pow(eval[1], parm->expo),
                    pow(eval[2], parm->expo));
      tenMakeSingle_f(tout, tin[0], eval, evec);
      tin += 7;
      tout += 7;
    }
    break;
  case funcEigenvalueAdd:
    for (II=0; II<=NN-1; II++) {
      /* HEY: this doesn't require eigensolve */
      tenEigensolve_f(eval, evec, tin); 
      ELL_3V_SET_TT(eval, float,
                    eval[0] + parm->val,
                    eval[1] + parm->val,
                    eval[2] + parm->val);
      tenMakeSingle_f(tout, tin[0], eval, evec);
      tin += 7;
      tout += 7;
    }
    break;
  case funcLog:
    for (II=0; II<=NN-1; II++) {
      tenEigensolve_f(eval, evec, tin);
      for (ri=0; ri<3; ri++) {
        eval[ri] = AIR_CAST(float, log(eval[ri]));
        eval[ri] = AIR_EXISTS(eval[ri]) ? eval[ri] : -1000000;
      }
      tenMakeSingle_f(tout, tin[0], eval, evec);
      tin += 7;
      tout += 7;
    }
    break;
  case funcExp:
    for (II=0; II<=NN-1; II++) {
      tenEigensolve_f(eval, evec, tin);
      for (ri=0; ri<3; ri++) {
        eval[ri] = AIR_CAST(float, exp(eval[ri]));
        eval[ri] = AIR_EXISTS(eval[ri]) ? eval[ri] : 0;
      }
      tenMakeSingle_f(tout, tin[0], eval, evec);
      tin += 7;
      tout += 7;
    }
    break;
  }
  
  /* basic and per-axis info handled by nrrdCopy above */
  return 0;
}

int
tenSizeNormalize(Nrrd *nout, const Nrrd *nin, double _weight[3],
                 double amount, double target) {
  char me[]="tenSizeNormalize", err[BIFF_STRLEN];
  funcParm parm;

  ELL_3V_COPY(parm.weight, _weight);
  parm.amount = amount;
  parm.target = target;
  if (theFunc(nout, nin, funcSizeNormalize, &parm)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

int
tenSizeScale(Nrrd *nout, const Nrrd *nin, double amount) {
  char me[]="tenSizeScale", err[BIFF_STRLEN];
  funcParm parm;

  parm.amount = amount;
  if (theFunc(nout, nin, funcSizeScale, &parm)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}


/*
******** tenAnisoScale
**
** scales the "deviatoric" part of a tensor up or down
*/
int
tenAnisoScale(Nrrd *nout, const Nrrd *nin, double scale,
              int fixDet, int makePositive) {
  char me[]="tenAnisoScale", err[BIFF_STRLEN];
  funcParm parm;

  parm.scale = scale;
  parm.fixDet = fixDet;
  parm.makePositive = makePositive;
  if (theFunc(nout, nin, funcAnisoScale, &parm)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

/*
******** tenEigenvalueClamp
**
** enstates the given value as the lowest eigenvalue
*/
int
tenEigenvalueClamp(Nrrd *nout, const Nrrd *nin, double min, double max) {
  char me[]="tenEigenvalueClamp", err[BIFF_STRLEN];
  funcParm parm;

  parm.min = min;
  parm.max = max;
  if (theFunc(nout, nin, funcEigenvalueClamp, &parm)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

/*
******** tenEigenvaluePower
**
** raises the eigenvalues to some power
*/
int
tenEigenvaluePower(Nrrd *nout, const Nrrd *nin, double expo) {
  char me[]="tenEigenvaluePower", err[BIFF_STRLEN];
  funcParm parm;

  parm.expo = expo;
  if (theFunc(nout, nin, funcEigenvaluePower, &parm)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

/*
******** tenEigenvalueAdd
**
** adds something to all eigenvalues
*/
int
tenEigenvalueAdd(Nrrd *nout, const Nrrd *nin, double val) {
  char me[]="tenEigenvalueAdd", err[BIFF_STRLEN];
  funcParm parm;
  
  parm.val = val;
  if (theFunc(nout, nin, funcEigenvalueAdd, &parm)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

/*
******** tenLog
**
** takes the logarithm (by taking the log of the eigenvalues)
*/
int
tenLog(Nrrd *nout, const Nrrd *nin) {
  char me[]="tenLog", err[BIFF_STRLEN];
  funcParm parm;

  if (theFunc(nout, nin, funcLog, &parm)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

/*
******** tenExp
**
** takes the exp()  (by taking exp() of the eigenvalues)
*/
int
tenExp(Nrrd *nout, const Nrrd *nin) {
  char me[]="tenExp", err[BIFF_STRLEN];
  funcParm parm;

  if (theFunc(nout, nin, funcExp, &parm)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}
