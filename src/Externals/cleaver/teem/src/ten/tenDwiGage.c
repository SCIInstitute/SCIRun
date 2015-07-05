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

#if TEEM_LEVMAR
#include <lm.h>
#endif

/* --------------------------------------------------------------------- */

char
_tenDwiGageStr[][AIR_STRLEN_SMALL] = {
  "(unknown tenDwiGage)",
  "all",
  "b0",
  "jdwi",
  "adc",
  "mdwi",
  "tlls",
  "tllserr",
  "tllserrlog",
  "tllslike",
  "twls",
  "twlserr",
  "twlserrlog",
  "twlslike",
  "tnls",
  "tnlserr",
  "tnlserrlog",
  "tnlslike",
  "tmle",
  "tmleerr",
  "tmleerrlog",
  "tmlelike",
  "t",
  "terr",
  "terrlog",
  "tlike",
  "c",
  "fa",
  "adwie",
  "2qs",
  "2qserr",
  "2qsnerr",
  "2peled",
  "2pelederr",
  "2pelednerr",
  "2peledlminfo",
};

int
_tenDwiGageVal[] = {
  tenDwiGageUnknown,
  tenDwiGageAll,
  tenDwiGageB0,
  tenDwiGageJustDWI,
  tenDwiGageADC,
  tenDwiGageMeanDWIValue,
  tenDwiGageTensorLLS,
  tenDwiGageTensorLLSError,
  tenDwiGageTensorLLSErrorLog,
  tenDwiGageTensorLLSLikelihood,
  tenDwiGageTensorWLS,
  tenDwiGageTensorWLSError,
  tenDwiGageTensorWLSErrorLog,
  tenDwiGageTensorWLSLikelihood,
  tenDwiGageTensorNLS,
  tenDwiGageTensorNLSError,
  tenDwiGageTensorNLSErrorLog,
  tenDwiGageTensorNLSLikelihood,
  tenDwiGageTensorMLE,
  tenDwiGageTensorMLEError,
  tenDwiGageTensorMLEErrorLog,
  tenDwiGageTensorMLELikelihood,
  tenDwiGageTensor,
  tenDwiGageTensorError,
  tenDwiGageTensorErrorLog,
  tenDwiGageTensorLikelihood,
  tenDwiGageConfidence,
  tenDwiGageFA,
  tenDwiGageTensorAllDWIError,
  tenDwiGage2TensorQSeg,
  tenDwiGage2TensorQSegError,
  tenDwiGage2TensorQSegAndError,
  tenDwiGage2TensorPeled,
  tenDwiGage2TensorPeledError,
  tenDwiGage2TensorPeledAndError,
  tenDwiGage2TensorPeledLevmarInfo
};

airEnum
_tenDwiGage = {
  "tenDwiGage",
  TEN_DWI_GAGE_ITEM_MAX+1,
  _tenDwiGageStr, _tenDwiGageVal,
  NULL,
  NULL, NULL,
  AIR_FALSE
};
airEnum *
tenDwiGage = &_tenDwiGage;

/* --------------------------------------------------------------------- */

gageItemEntry
_tenDwiGageTable[TEN_DWI_GAGE_ITEM_MAX+1] = {
  /* enum value                     len,deriv, prereqs,                           parent item, parent index, needData */
  {tenDwiGageUnknown,                 0,  0,  {0},                                                    0,  0, AIR_TRUE},
  /* len == 0 for tenDwiGage{All,JustDWI,ADC} means "learn later at run-time" */
  {tenDwiGageAll,                     0,  0,  {0},                                                    0,  0, AIR_TRUE},
  {tenDwiGageB0,                      1,  0,  {tenDwiGageAll},                            tenDwiGageAll,  0, AIR_TRUE},
  {tenDwiGageJustDWI,                 0,  0,  {tenDwiGageAll},                            tenDwiGageAll,  1, AIR_TRUE},
  {tenDwiGageADC,                     0,  0,  {tenDwiGageB0, tenDwiGageJustDWI},                      0,  0, AIR_TRUE},

  {tenDwiGageMeanDWIValue,            1,  0,  {tenDwiGageAll},                                        0,  0, AIR_TRUE},

  {tenDwiGageTensorLLS,               7,  0,  {tenDwiGageAll, tenDwiGageMeanDWIValue},                0,  0, AIR_TRUE},
  {tenDwiGageTensorLLSError,          1,  0,  {tenDwiGageTensorLLS},                                  0,  0, AIR_TRUE},
  {tenDwiGageTensorLLSErrorLog,       1,  0,  {tenDwiGageTensorLLS},                                  0,  0, AIR_TRUE},
  {tenDwiGageTensorLLSLikelihood,     1,  0,  {tenDwiGageTensorLLS},                                  0,  0, AIR_TRUE},

  {tenDwiGageTensorWLS,               7,  0,  {tenDwiGageAll, tenDwiGageMeanDWIValue},                0,  0, AIR_TRUE},
  {tenDwiGageTensorWLSError,          1,  0,  {tenDwiGageTensorWLS},                                  0,  0, AIR_TRUE},
  {tenDwiGageTensorWLSErrorLog,       1,  0,  {tenDwiGageTensorWLS},                                  0,  0, AIR_TRUE},
  {tenDwiGageTensorWLSLikelihood,     1,  0,  {tenDwiGageTensorWLS},                                  0,  0, AIR_TRUE},

  {tenDwiGageTensorNLS,               7,  0,  {tenDwiGageAll, tenDwiGageMeanDWIValue},                0,  0, AIR_TRUE},
  {tenDwiGageTensorNLSError,          1,  0,  {tenDwiGageTensorNLS},                                  0,  0, AIR_TRUE},
  {tenDwiGageTensorNLSErrorLog,       1,  0,  {tenDwiGageTensorNLS},                                  0,  0, AIR_TRUE},
  {tenDwiGageTensorNLSLikelihood,     1,  0,  {tenDwiGageTensorNLS},                                  0,  0, AIR_TRUE},

  {tenDwiGageTensorMLE,               7,  0,  {tenDwiGageAll, tenDwiGageMeanDWIValue},                0,  0, AIR_TRUE},
  {tenDwiGageTensorMLEError,          1,  0,  {tenDwiGageTensorMLE},                                  0,  0, AIR_TRUE},
  {tenDwiGageTensorMLEErrorLog,       1,  0,  {tenDwiGageTensorMLE},                                  0,  0, AIR_TRUE},
  {tenDwiGageTensorMLELikelihood,     1,  0,  {tenDwiGageTensorMLE},                                  0,  0, AIR_TRUE},

  /* these are NOT sub-items: they are copies, as controlled by the
     kind->data, but not the query: the query can't capture the kind
     of dependency implemented by having a dynamic kind */
  {tenDwiGageTensor,                  7,  0,  {0}, /* 0 == "learn later at run time" */               0,  0, AIR_TRUE},
  {tenDwiGageTensorError,             1,  0,  {0},                                                    0,  0, AIR_TRUE},
  {tenDwiGageTensorErrorLog,          1,  0,  {0},                                                    0,  0, AIR_TRUE},
  {tenDwiGageTensorLikelihood,        1,  0,  {0},                                                    0,  0, AIR_TRUE},

  /* back to normal non-run-time items */
  {tenDwiGageConfidence,              1,  0,  {tenDwiGageTensor},                      tenDwiGageTensor,  0, AIR_TRUE},
  {tenDwiGageFA,                      1,  0,  {tenDwiGageTensor},                                     0,  0, AIR_TRUE},
  
  {tenDwiGageTensorAllDWIError,       0,  0,  {tenDwiGageTensor, tenDwiGageJustDWI},                  0,  0, AIR_TRUE},

  /* it actually doesn't make sense for tenDwiGage2TensorQSegAndError to be the parent,
     because of the situations where you want the q-seg result, but don't care about error */
  {tenDwiGage2TensorQSeg,            14,  0,  {tenDwiGageAll},                                        0,  0, AIR_TRUE},
  {tenDwiGage2TensorQSegError,        1,  0,  {tenDwiGageAll, tenDwiGage2TensorQSeg},                 0,  0, AIR_TRUE},
  {tenDwiGage2TensorQSegAndError,    15,  0,  {tenDwiGage2TensorQSeg, tenDwiGage2TensorQSegError},    0,  0, AIR_TRUE},

  {tenDwiGage2TensorPeled,           14,  0,  {tenDwiGageAll},                                        0,  0, AIR_TRUE},
  {tenDwiGage2TensorPeledError,       1,  0,  {tenDwiGageAll, tenDwiGage2TensorPeled},                0,  0, AIR_TRUE},
  {tenDwiGage2TensorPeledAndError,   15,  0,  {tenDwiGage2TensorPeled, tenDwiGage2TensorPeledError},  0,  0, AIR_TRUE},
  
  {tenDwiGage2TensorPeledLevmarInfo,  5,  0,  {tenDwiGage2TensorPeled},                               0,  0, AIR_TRUE}
};

void
_tenDwiGageIv3Print(FILE *file, gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_tenDwiGageIv3Print";

  AIR_UNUSED(ctx);
  AIR_UNUSED(pvl);
  fprintf(file, "%s: sorry, unimplemented\n", me);
  return;
}

void
_tenDwiGageFilter(gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_tenDwiGageFilter";
  double *fw00, *fw11, *fw22, *dwi;
  int fd;
  tenDwiGageKindData *kindData;
  gageScl3PFilter_t *filter[5] = {NULL, gageScl3PFilter2, gageScl3PFilter4,
                                  gageScl3PFilter6, gageScl3PFilter8};
  unsigned int J, dwiNum;

  fd = 2*ctx->radius;
  dwi = pvl->directAnswer[tenDwiGageAll];
  kindData = AIR_CAST(tenDwiGageKindData *, pvl->kind->data);
  dwiNum = pvl->kind->valLen;
  if (!ctx->parm.k3pack) {
    fprintf(stderr, "!%s: sorry, 6pack filtering not implemented\n", me);
    return;
  }
  fw00 = ctx->fw + fd*3*gageKernel00;
  fw11 = ctx->fw + fd*3*gageKernel11;
  fw22 = ctx->fw + fd*3*gageKernel22;
  /* HEY: these will have to be updated if there is ever any use for
     derivatives in DWIs: can't pass NULL pointers for gradient info,
     and the needD[1] has to be passed */
  if (fd <= 8) {
    for (J=0; J<dwiNum; J++) {
      filter[ctx->radius](ctx->shape, pvl->iv3 + J*fd*fd*fd,
                          pvl->iv2 + J*fd*fd,
                          pvl->iv1 + J*fd,
                          fw00, fw11, fw22,
                          dwi + J, NULL, NULL,
                          pvl->needD[0], AIR_FALSE, AIR_FALSE);
    }
  } else {
    for (J=0; J<dwiNum; J++) {
      gageScl3PFilterN(ctx->shape, fd, pvl->iv3 + J*fd*fd*fd,
                       pvl->iv2 + J*fd*fd, pvl->iv1 + J*fd,
                       fw00, fw11, fw22,
                       dwi + J, NULL, NULL,
                       pvl->needD[0], AIR_FALSE, AIR_FALSE);
    }
  }

  return;
}

/* Returns the Akaike Information Criterion */

/* 
** residual: is the variance
** n: number of observations: number of DWI's in our case
** k: number of parameters: number of tensor components in our case
*/
double 
_tenComputeAIC(double residual, int n, int k) {
   double AIC = 0;

   if (residual == 0) {
     return 0;
   }

   /* AIC, RSS used when doing regression */
   AIC = 2*k + n*log(residual);
   /* Always use bias adjustment */
   /* if (n/k < 40) { */
   AIC = AIC + ((2*k*(k + 1))/(n - k - 1));
   /* } */

   return AIC;
}

/* Form a 2D tensor from the parameters */
void
_tenPeledRotate2D(double ten[7], double lam1, double lam3, double phi) {
  double cc, ss, d3, d1, d2;

  cc = cos(phi);
  ss = sin(phi);
  d1 = cc*cc*lam1 + ss*ss*lam3;
  d3 = cc*ss*(lam1 - lam3);
  d2 = ss*ss*lam1 + cc*cc*lam3;

  TEN_T_SET(ten, 1.0,    d1, d3, 0,    d2, 0,    lam3);
  return;
}

/* The main callback function that is iterated during levmar */

/* vector pp of parameters is as follows:
** pp[0]: principal eigenvalue 
** pp[1]: fraction of 1st tensor
** pp[2]: phi for 1st tensor
** pp[3]: phi for 2nd tensor
*/
void
_tenLevmarPeledCB(double *pp, double *xx, int mm, int nn, void *_pvlData) {
  /* char me[]="_tenLevmarPeledCB"; */
  double tenA[7], tenB[7];
  int ii;
  tenDwiGagePvlData *pvlData;
  double *egrad;
 
  AIR_UNUSED(mm);
  pvlData = AIR_CAST(tenDwiGagePvlData *, _pvlData);

  /* Form the tensors using the estimated parms */
  _tenPeledRotate2D(tenA, pp[0], pvlData->ten1Eval[2], pp[2]);
  _tenPeledRotate2D(tenB, pp[0], pvlData->ten1Eval[2], pp[3]);

  egrad = AIR_CAST(double *, pvlData->nten1EigenGrads->data);
  /* skip past b0 gradient, HEY: not general purpose */
  egrad += 3; 
  for (ii=0; ii<nn; ii++) {
    double argA, argB, sigA, sigB;
    argA = -pvlData->tec1->bValue*TEN_T3V_CONTR(tenA, egrad + 3*ii);
    argB = -pvlData->tec1->bValue*TEN_T3V_CONTR(tenB, egrad + 3*ii);
    if (pvlData->levmarUseFastExp) {
      sigA = airFastExp(argA);
      sigB = airFastExp(argB);
    } else {
      sigA = exp(argA);
      sigB = exp(argB);
    }
    xx[ii] = pvlData->tec1->knownB0*(pp[1]*sigA + (1-pp[1])*sigB);
  }
  return;
}

void
_tenDwiGageAnswer(gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_tenDwiGageAnswer";
  unsigned int dwiIdx;
  tenDwiGageKindData *kindData;
  tenDwiGagePvlData *pvlData;
  double *dwiAll, dwiMean=0, tentmp[7];

  kindData = AIR_CAST(tenDwiGageKindData *, pvl->kind->data);
  pvlData = AIR_CAST(tenDwiGagePvlData *, pvl->data);

  dwiAll = pvl->directAnswer[tenDwiGageAll];
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageAll)) {
    /* done if doV */
    if (ctx->verbose) {
      for (dwiIdx=0; dwiIdx<pvl->kind->valLen; dwiIdx++) {
        fprintf(stderr, "%s(%d+%g,%d+%g,%d+%g): dwi[%u] = %g\n", me,
                ctx->point.xi, ctx->point.xf,
                ctx->point.yi, ctx->point.yf,
                ctx->point.zi, ctx->point.zf,
                dwiIdx, dwiAll[dwiIdx]);
      }
      fprintf(stderr, "%s: type(ngrad) = %d = %s\n", me,
              kindData->ngrad->type,
              airEnumStr(nrrdType, kindData->ngrad->type));
    }
  }

  /*
    if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageB0)) {
    if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageJustDWI)) {
    done if doV
    }
  */
  /* HEY this isn't valid for multiple b-values */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageADC)) {
    double logdwi, logb0;
    logb0 = log(AIR_MAX(kindData->valueMin,
                        pvl->directAnswer[tenDwiGageB0][0]));
    for (dwiIdx=1; dwiIdx<pvl->kind->valLen; dwiIdx++) {
      logdwi = log(AIR_MAX(kindData->valueMin,
                           pvl->directAnswer[tenDwiGageJustDWI][dwiIdx-1]));
      pvl->directAnswer[tenDwiGageADC][dwiIdx-1]
        = (logb0 - logdwi)/kindData->bval;
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageMeanDWIValue)) {
    dwiMean = 0;
    for (dwiIdx=1; dwiIdx<pvl->kind->valLen; dwiIdx++) {
      dwiMean += dwiAll[dwiIdx];
    }
    dwiMean /= pvl->kind->valLen;
    pvl->directAnswer[tenDwiGageMeanDWIValue][0] = dwiMean;
  }

  /* note: the gage interface to tenEstimate functionality 
     allows you exactly one kind of tensor estimation (per kind),
     so the function call to do the estimation is actually
     repeated over and over again; the copy into the answer
     buffer is what changes... */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorLLS)) {
    tenEstimate1TensorSingle_d(pvlData->tec1, tentmp, dwiAll);
    TEN_T_COPY(pvl->directAnswer[tenDwiGageTensorLLS], tentmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorLLSError)) {
    pvl->directAnswer[tenDwiGageTensorLLSError][0] = pvlData->tec1->errorDwi;
  }  
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorLLSErrorLog)) {
    pvl->directAnswer[tenDwiGageTensorLLSErrorLog][0] 
      = pvlData->tec1->errorLogDwi;
  }  
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorWLS)) {
    tenEstimate1TensorSingle_d(pvlData->tec1, tentmp, dwiAll);
    TEN_T_COPY(pvl->directAnswer[tenDwiGageTensorWLS], tentmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorNLS)) {
    tenEstimate1TensorSingle_d(pvlData->tec1, tentmp, dwiAll);
    TEN_T_COPY(pvl->directAnswer[tenDwiGageTensorNLS], tentmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorMLE)) {
    tenEstimate1TensorSingle_d(pvlData->tec1, tentmp, dwiAll);
    TEN_T_COPY(pvl->directAnswer[tenDwiGageTensorMLE], tentmp);
  }
  /* HEY: have to implement all the different kinds of errors */

  /* BEGIN sneakiness ........ */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensor)) {
    gageItemEntry *item;
    item = pvl->kind->table + tenDwiGageTensor;
    TEN_T_COPY(pvl->directAnswer[tenDwiGageTensor],
               pvl->directAnswer[item->prereq[0]]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorError)) {
    gageItemEntry *item;
    item = pvl->kind->table + tenDwiGageTensorError;
    pvl->directAnswer[tenDwiGageTensorError][0]
      = pvl->directAnswer[item->prereq[0]][0];
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorErrorLog)) {
    gageItemEntry *item;
    item = pvl->kind->table + tenDwiGageTensorErrorLog;
    pvl->directAnswer[tenDwiGageTensorErrorLog][0]
      = pvl->directAnswer[item->prereq[0]][0];
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorLikelihood)) {
    gageItemEntry *item;
    item = pvl->kind->table + tenDwiGageTensorLikelihood;
    pvl->directAnswer[tenDwiGageTensorLikelihood][0]
      = pvl->directAnswer[item->prereq[0]][0];
  }
  /* END sneakiness ........ */

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageFA)) {
    pvl->directAnswer[tenDwiGageFA][0]
      = pvl->directAnswer[tenDwiGageTensor][0]
      * tenAnisoTen_d(pvl->directAnswer[tenDwiGageTensor],
                      tenAniso_FA);
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorAllDWIError)) {
    const double *grads;
    int gradcount;
    double *ten, d;
    int i;
    
    /* HEY: should switch to tenEstimate-based DWI simulation */
    ten = pvl->directAnswer[tenDwiGageTensor];
    gradcount = pvl->kind->valLen -1; /* Dont count b0 */
    grads = ((const double*) kindData->ngrad->data) +3; /* Ignore b0 grad */
    for( i=0; i < gradcount; i++ ) {
      d = dwiAll[0]*exp(- pvlData->tec1->bValue 
                        * TEN_T3V_CONTR(ten, grads + 3*i));
      pvl->directAnswer[tenDwiGageTensorAllDWIError][i] = dwiAll[i+1] - d;
    }
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGage2TensorQSeg)) {
    const double *grads;
    int gradcount;
    double *twoten;
    unsigned int valIdx, E;
    
    twoten = pvl->directAnswer[tenDwiGage2TensorQSeg];
    
    gradcount = pvl->kind->valLen -1; /* Dont count b0 */
    grads = ((const double*) kindData->ngrad->data) +3; /* Ignore b0 grad */
    if (dwiAll[0] != 0) { /*  S0 = 0 */
      _tenQball(pvlData->tec2->bValue, gradcount, dwiAll, grads,
                pvlData->qvals);
      _tenQvals2points(gradcount, pvlData->qvals, grads, pvlData->qpoints);
      _tenSegsamp2(gradcount, pvlData->qvals, grads, pvlData->qpoints,
                   pvlData->wght + 1, pvlData->dists );
    } else {
      /* stupid; should really return right here since data is garbage */
      for (valIdx=1; valIdx < AIR_CAST(unsigned int, gradcount+1); valIdx++) {
        pvlData->wght[valIdx] = valIdx % 2;
      }
    }
    
    E = 0;
    for (valIdx=1; valIdx<pvl->kind->valLen; valIdx++) {
      if (!E) E |= tenEstimateSkipSet(pvlData->tec2, valIdx,
                                      pvlData->wght[valIdx]);
    }
    if (!E) E |= tenEstimateUpdate(pvlData->tec2);
    if (!E) E |= tenEstimate1TensorSingle_d(pvlData->tec2,
                                            twoten + 0, dwiAll);
    for (valIdx=1; valIdx<pvl->kind->valLen; valIdx++) {
      if (!E) E |= tenEstimateSkipSet(pvlData->tec2, valIdx,
                                      1 - pvlData->wght[valIdx]);
    }
    if (!E) E |= tenEstimateUpdate(pvlData->tec2);
    if (!E) E |= tenEstimate1TensorSingle_d(pvlData->tec2,
                                            twoten + 7, dwiAll);
    if (E) {
      fprintf(stderr, "!%s: (trouble) %s\n", me, biffGetDone(TEN));
    }
    
    /* hack: confidence for two-tensor fit */
    twoten[0] = (twoten[0] + twoten[7])/2;
    twoten[7] = 0.5; /* fraction that is the first tensor (initial value) */
    /* twoten[1 .. 6] = first tensor */
    /* twoten[8 .. 13] = second tensor */
    
    /* Compute fraction between tensors if not garbage in this voxel */
    if (twoten[0] > 0.5) {
      double exp0,exp1,d,e=0,g=0, a=0,b=0;
      int i;
      
      for( i=0; i < gradcount; i++ ) {
        exp0 = exp(-pvlData->tec2->bValue * TEN_T3V_CONTR(twoten + 0,
                                                          grads + 3*i));
        exp1 = exp(-pvlData->tec2->bValue * TEN_T3V_CONTR(twoten + 7,
                                                          grads + 3*i));
        
        d = dwiAll[i+1] / dwiAll[0];
        e = exp0 - exp1;
        g = d - exp1;
        
        a += .5*e*e;
        b += e*g;
      }
      
      twoten[7] = AIR_CLAMP(0, 0.5*(b/a), 1);
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGage2TensorQSegError)) {
    const double *grads;
    int gradcount;
    double *twoten, d;
    int i;
    
    /* HEY: should switch to tenEstimate-based DWI simulation */
    if (dwiAll[0] != 0) { /* S0 = 0 */
      twoten = pvl->directAnswer[tenDwiGage2TensorQSeg];
      gradcount = pvl->kind->valLen -1; /* Dont count b0 */
      grads = ((const double*) kindData->ngrad->data) +3; /* Ignore b0 grad */
      
      pvl->directAnswer[tenDwiGage2TensorQSegError][0] = 0;
      for( i=0; i < gradcount; i++ ) {
        d = twoten[7]*exp(-pvlData->tec2->bValue * TEN_T3V_CONTR(twoten + 0,
                                                                 grads + 3*i));
        d += (1 - twoten[7])*exp(-pvlData->tec2->bValue 
                                 *TEN_T3V_CONTR(twoten + 7, grads + 3*i));
        d = dwiAll[i+1]/dwiAll[0] - d;
        pvl->directAnswer[tenDwiGage2TensorQSegError][0] += d*d;
      }
      pvl->directAnswer[tenDwiGage2TensorQSegError][0] = 
        sqrt( pvl->directAnswer[tenDwiGage2TensorQSegError][0] );
    } else {
      /* HEY: COMPLETELY WRONG!! An error is not defined! */
      pvl->directAnswer[tenDwiGage2TensorQSegError][0] = 0;
    }
    /* printf("%f\n",pvl->directAnswer[tenDwiGage2TensorQSegError][0]); */
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGage2TensorQSegAndError)) {
    double *twoten, *err, *twotenerr;
    
    twoten = pvl->directAnswer[tenDwiGage2TensorQSeg];
    err = pvl->directAnswer[tenDwiGage2TensorQSegError];
    twotenerr = pvl->directAnswer[tenDwiGage2TensorQSegAndError];
    TEN_T_COPY(twotenerr + 0, twoten + 0);
    TEN_T_COPY(twotenerr + 7, twoten + 7);
    twotenerr[14] = err[0];
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGage2TensorPeled)) {
#if TEEM_LEVMAR
#define PARAMS 4
    double *twoTen, Cp /* , residual, AICSingFit, AICTwoFit */;
    /* Vars for the NLLS */
    double guess[PARAMS], loBnd[PARAMS], upBnd[PARAMS],
      opts[LM_OPTS_SZ], *grad, *egrad, tenA[7], tenB[7],
      matA[9], matB[9], matTmp[9], rott[9];
    unsigned int gi;
    int lmret;
    
    /* Pointer to the location where the two tensor will be written */
    twoTen = pvl->directAnswer[tenDwiGage2TensorPeled];
    /* Estimate the DWI error, error is given as standard deviation */
    pvlData->tec1->recordErrorDwi = AIR_FALSE;
    /* Estimate the single tensor */
    tenEstimate1TensorSingle_d(pvlData->tec1, pvlData->ten1, dwiAll);
    /* Get the eigenValues and eigen vectors for this tensor */
    tenEigensolve_d(pvlData->ten1Eval, pvlData->ten1Evec, pvlData->ten1);
    /* Get westins Cp */
    Cp = tenAnisoEval_d(pvlData->ten1Eval, tenAniso_Cp1);
    
    /* Only do two-tensor fitting if CP is greater or equal to than a
       user-defined threshold */
    if (Cp >= pvlData->levmarMinCp) {
      /* Calculate the residual, need the variance to sqr it */
      /* residual = pvlData->tec1->errorDwi*pvlData->tec1->errorDwi; */
      /* Calculate the AIC for single tensor fit */
      /* AICSingFit = _tenComputeAIC(residual, pvlData->tec1->dwiNum, 6); */

      /* the CP-based test is gone; caller's responsibility */
      
      /* rotate DW gradients by inverse of eigenvector column matrix
         and place into pvlData->nten1EigenGrads (which has been
         allocated by _tenDwiGagePvlDataNew()) */
      grad = AIR_CAST(double *, kindData->ngrad->data);
      egrad = AIR_CAST(double *, pvlData->nten1EigenGrads->data);
      for (gi=0; gi<kindData->ngrad->axis[1].size; gi++) {
        /* yes, this is also transforming some zero-length (B0) gradients;
           that's harmless */
        ELL_3MV_MUL(egrad, pvlData->ten1Evec, grad);
        grad += 3;
        egrad += 3;
      }
    
      /* Lower and upper bounds for the NLLS routine */
      loBnd[0] = 0.0;
      loBnd[1] = 0.0;       
      loBnd[2] = -AIR_PI/2;
      loBnd[3] = -AIR_PI/2;
      upBnd[0] = pvlData->ten1Eval[0]*5;
      upBnd[1] = 1.0;
      upBnd[2] = AIR_PI/2;
      upBnd[3] = AIR_PI/2;
      /* Starting point for the NLLS */
      guess[0] = pvlData->ten1Eval[0];
      guess[1] = 0.5;

      guess[2] = AIR_PI/4;
      guess[3] = -AIR_PI/4;
      /*
        guess[2] = AIR_AFFINE(0, airDrandMT_r(pvlData->randState), 1,
        AIR_PI/6, AIR_PI/3);
        guess[3] = AIR_AFFINE(0, airDrandMT_r(pvlData->randState), 1,
        -AIR_PI/6, -AIR_PI/3);
      */
      /* Fill in the constraints for the LM optimization, 
         the threshold of error difference */
      opts[0] = pvlData->levmarTau;
      opts[1] = pvlData->levmarEps1;
      opts[2] = pvlData->levmarEps2;
      opts[3] = pvlData->levmarEps3;
      /* Very imp to set this opt, note that only forward
         differences are used to approx Jacobian */
      opts[4] = pvlData->levmarDelta;
    
      /* run NLLS, results are stored back into guess[] */
      pvlData->levmarUseFastExp = AIR_FALSE;
      lmret = dlevmar_bc_dif(_tenLevmarPeledCB, guess, pvlData->tec1->dwi,
                             PARAMS, pvlData->tec1->dwiNum, loBnd, upBnd,
                             pvlData->levmarMaxIter, opts,
                             pvlData->levmarInfo,
                             NULL, NULL, pvlData);
      if (-1 == lmret) {
        ctx->errNum = 1;
        sprintf(ctx->errStr, "%s: dlevmar_bc_dif() failed!", me);
      } else {
        /* Get the AIC for the two tensor fit, use the levmarinfo
           to get the residual */
        /*
          residual = pvlData->levmarInfo[1]/pvlData->tec1->dwiNum;
          AICTwoFit = _tenComputeAIC(residual, pvlData->tec1->dwiNum, 12);
        */
        /* Form the tensors using the estimated pp, returned in guess */
        _tenPeledRotate2D(tenA, guess[0], pvlData->ten1Eval[2], guess[2]);
        _tenPeledRotate2D(tenB, guess[0], pvlData->ten1Eval[2], guess[3]);
        TEN_T2M(matA, tenA);
        TEN_T2M(matB, tenB);
      
        ELL_3M_TRANSPOSE(rott, pvlData->ten1Evec);
        ELL_3M_MUL(matTmp, matA, pvlData->ten1Evec);
        ELL_3M_MUL(matA, rott, matTmp);
        ELL_3M_MUL(matTmp, matB, pvlData->ten1Evec);
        ELL_3M_MUL(matB, rott, matTmp);
        
        /* Copy two two tensors */
        /* guess[1] is population fraction of first tensor */
        if (guess[1] > 0.5) {
          twoTen[7] = guess[1];
          TEN_M2T(twoTen + 0, matA);
          TEN_M2T(twoTen + 7, matB);
        } else {
          twoTen[7] = 1 - guess[1];
          TEN_M2T(twoTen + 0, matB);
          TEN_M2T(twoTen + 7, matA);
        }
        twoTen[0] = 1;
      }
    } else {
      /* its too planar- just do single tensor fit */
      TEN_T_COPY(twoTen, pvlData->ten1);
      TEN_T_SET(twoTen + 7, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    }
#undef PARAMS
#else
    double *twoTen;
    twoTen = pvl->directAnswer[tenDwiGage2TensorPeled];
    TEN_T_SET(twoTen + 0, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN,
              AIR_NAN, AIR_NAN, AIR_NAN);
    TEN_T_SET(twoTen + 7, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN,
              AIR_NAN, AIR_NAN, AIR_NAN);
    fprintf(stderr, "%s: sorry, not compiled with TEEM_LEVMAR\n", me);
#endif
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGage2TensorPeledError)) {
    double *info;
    info = pvlData->levmarInfo;
    pvl->directAnswer[tenDwiGage2TensorPeledError][0] = 0;

    if (info[1] > 0) {
      /* Returning the standard deviation */
      pvl->directAnswer[tenDwiGage2TensorPeledError][0] = 
        sqrt(info[1]/pvlData->tec1->dwiNum);
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGage2TensorPeledAndError)) {
    double *twoten, *err, *twotenerr;
    /* HEY cut and paste */
    twoten = pvl->directAnswer[tenDwiGage2TensorPeled];
    err = pvl->directAnswer[tenDwiGage2TensorPeledError];
    twotenerr = pvl->directAnswer[tenDwiGage2TensorPeledAndError];
    TEN_T_COPY(twotenerr + 0, twoten + 0);
    TEN_T_COPY(twotenerr + 7, twoten + 7);
    twotenerr[14] = err[0];
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGage2TensorPeledLevmarInfo)) {
    double *info;
    unsigned int ii, alen;
    alen = gageKindAnswerLength(pvl->kind, tenDwiGage2TensorPeledLevmarInfo);
    info = pvl->directAnswer[tenDwiGage2TensorPeledLevmarInfo];
    for (ii=0; ii<alen; ii++) {
      info[ii] = pvlData->levmarInfo[ii];
    }
  }

  return;
}

/* --------------------- pvlData */

/* note use of the GAGE biff key */
void *
_tenDwiGagePvlDataNew(const gageKind *kind) {
  char me[]="_tenDwiGagePvlDataNew", err[BIFF_STRLEN];
  tenDwiGagePvlData *pvlData;
  tenDwiGageKindData *kindData;
  const int segcount = 2;
  unsigned int num;
  int E;
  
  if (tenDwiGageKindCheck(kind)) {
    sprintf(err, "%s: kindData not ready for use", me);
    biffMove(GAGE, err, TEN); return NULL;
  }
  kindData = AIR_CAST(tenDwiGageKindData *, kind->data);
  
  pvlData = AIR_CAST(tenDwiGagePvlData *,
                     malloc(sizeof(tenDwiGagePvlData)));
  if (!pvlData) {
    sprintf(err, "%s: couldn't allocate pvl data!", me);
    biffAdd(GAGE, err); return NULL;
  }
  pvlData->tec1 = tenEstimateContextNew();
  pvlData->tec2 = tenEstimateContextNew();
  for (num=1; num<=2; num++) {
    tenEstimateContext *tec;
    tec = (1 == num ? pvlData->tec1 : pvlData->tec2);
    E = 0;
    if (!E) tenEstimateVerboseSet(tec, 0);
    if (!E) tenEstimateNegEvalShiftSet(tec, AIR_TRUE);
    if (!E) E |= tenEstimateMethodSet(tec, 1 == num
                                      ? kindData->est1Method
                                      : kindData->est2Method);
    if (!E) E |= tenEstimateValueMinSet(tec, kindData->valueMin);
    if (kindData->ngrad->data) {
      if (!E) E |= tenEstimateGradientsSet(tec, kindData->ngrad,
                                           kindData->bval, AIR_FALSE);
    } else {
      if (!E) E |= tenEstimateBMatricesSet(tec, kindData->nbmat,
                                           kindData->bval, AIR_FALSE);
    }
    if (!E) E |= tenEstimateThresholdSet(tec, 
                                         kindData->thresh, kindData->soft);
    if (!E) E |= tenEstimateUpdate(tec);
    if (E) {
      fprintf(stderr, "%s: trouble setting %u estimation", me, num);
      biffMove(GAGE, err, TEN); return NULL;
    }
  }
  pvlData->vbuf = AIR_CAST(double *,
                           calloc(kind->valLen, sizeof(double)));
  pvlData->wght = AIR_CAST(unsigned int *,
                           calloc(kind->valLen, sizeof(unsigned int)));
  /* HEY: this is where we act on the the assumption about
     having val[0] be T2 baseline and all subsequent val[i] be DWIs */
  pvlData->wght[0] = 1;
  pvlData->qvals = AIR_CAST(double *,
                            calloc(kind->valLen-1, sizeof(double)));
  pvlData->qpoints = AIR_CAST(double *,
                              calloc(kind->valLen-1,  3*sizeof(double)));
  pvlData->dists = AIR_CAST(double *,
                            calloc(segcount*(kind->valLen-1), 
                                   sizeof(double)));
  pvlData->weights = AIR_CAST(double *,
                              calloc(segcount*(kind->valLen-1),
                                     sizeof(double)));

  if (kindData->ngrad->data) {
    pvlData->nten1EigenGrads = nrrdNew();
    /* this is for allocation only; values will get over-written */
    nrrdCopy(pvlData->nten1EigenGrads, kindData->ngrad);
  } else {
    /* HEY: currently don't handle general B-matrices here */
    pvlData->nten1EigenGrads = NULL;
  }

  pvlData->randSeed = kindData->randSeed;
  pvlData->randState = airRandMTStateNew(pvlData->randSeed);

  /* initialize single-tensor info to all NaNs */
  TEN_T_SET(pvlData->ten1, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN,
            AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_3V_SET(pvlData->ten1Evec + 0, AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_3V_SET(pvlData->ten1Evec + 3, AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_3V_SET(pvlData->ten1Evec + 6, AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_3V_SET(pvlData->ten1Eval, AIR_NAN, AIR_NAN, AIR_NAN);

  /* here's an okay spot to check our compile-time assumptions 
     about the levmar library */
#if TEEM_LEVMAR
  /* this is needed to make sure that the tenDwiGage2TensorPeledLevmarInfo
     item definition above is valid */
  if (5 != LM_OPTS_SZ) {
    sprintf(err, "%s: LM_OPTS_SZ (%d) != expected 5\n", me, LM_OPTS_SZ);
    biffAdd(GAGE, err); return NULL;
  }
#endif
  pvlData->levmarUseFastExp = AIR_FALSE;
  pvlData->levmarMaxIter = 200;
  pvlData->levmarTau = 1E-03; /* LM_INIT_MU; */
  pvlData->levmarEps1 = 1E-8;
  pvlData->levmarEps2 = 1E-8;
  pvlData->levmarEps3 = 1E-8;
  pvlData->levmarDelta = 1E-8;
  pvlData->levmarMinCp = 0.1;

  /* pvlData->levmarInfo[] is output; not initialized */

  return AIR_CAST(void *, pvlData);
}

void *
_tenDwiGagePvlDataCopy(const gageKind *kind, const void *_pvlDataOld) {
  tenDwiGagePvlData *pvlDataOld, *pvlDataNew;

  pvlDataOld = AIR_CAST(tenDwiGagePvlData *, _pvlDataOld);
  pvlDataNew = AIR_CAST(tenDwiGagePvlData *, _tenDwiGagePvlDataNew(kind));

  /* HEY: no error checking? */
  if (pvlDataOld->nten1EigenGrads) {
    nrrdCopy(pvlDataNew->nten1EigenGrads, pvlDataOld->nten1EigenGrads);
  }
  /* need to copy randState or randSeed? */

  TEN_T_COPY(pvlDataNew->ten1, pvlDataOld->ten1);
  ELL_3M_COPY(pvlDataNew->ten1Evec, pvlDataOld->ten1Evec);
  ELL_3V_COPY(pvlDataNew->ten1Eval, pvlDataOld->ten1Eval);

  pvlDataNew->levmarUseFastExp = pvlDataOld->levmarUseFastExp;
  pvlDataNew->levmarMaxIter = pvlDataOld->levmarMaxIter;
  pvlDataNew->levmarTau = pvlDataOld->levmarTau;
  pvlDataNew->levmarEps1 = pvlDataOld->levmarEps1;
  pvlDataNew->levmarEps2 = pvlDataOld->levmarEps2;
  pvlDataNew->levmarEps3 = pvlDataOld->levmarEps3;
  pvlDataNew->levmarDelta = pvlDataOld->levmarDelta;
  pvlDataNew->levmarMinCp = pvlDataOld->levmarMinCp;
  /* pvlData->levmarInfo[] is output; not copied */

  return pvlDataNew;
}

int
_tenDwiGagePvlDataUpdate(const gageKind *kind,
                         const gageContext *ctx,
                         const gagePerVolume *pvl, const void *_pvlData) {
  /* char me[]="_tenDwiGagePvlDataUpdate"; */
  tenDwiGagePvlData *pvlData;

  AIR_UNUSED(ctx);
  pvlData = AIR_CAST(tenDwiGagePvlData *, _pvlData);
  AIR_UNUSED(kind);
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorLLSError)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorWLSError)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorNLSError)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorMLEError)) {
    pvlData->tec1->recordErrorDwi = AIR_TRUE;
  } else {
    pvlData->tec1->recordErrorDwi = AIR_FALSE;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorLLSErrorLog)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorWLSErrorLog)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorNLSErrorLog)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorMLEErrorLog)) {
    pvlData->tec1->recordErrorLogDwi = AIR_TRUE;
  } else {
    pvlData->tec1->recordErrorLogDwi = AIR_FALSE;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorLLSLikelihood)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorWLSLikelihood)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorNLSLikelihood)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenDwiGageTensorMLELikelihood)) {
    pvlData->tec1->recordLikelihoodDwi = AIR_TRUE;
  } else {
    pvlData->tec1->recordLikelihoodDwi = AIR_FALSE;
  }
  /*
  fprintf(stderr, "!%s: record %d %d %d\n", me,
          pvlData->tec1->recordErrorDwi,
          pvlData->tec1->recordErrorLogDwi,
          pvlData->tec1->recordLikelihoodDwi);
  */
  return 0;
}

void *
_tenDwiGagePvlDataNix(const gageKind *kind, void *_pvlData) {
  tenDwiGagePvlData *pvlData;
  
  AIR_UNUSED(kind);
  pvlData = AIR_CAST(tenDwiGagePvlData *, _pvlData);
  if (pvlData) {
    tenEstimateContextNix(pvlData->tec1);
    tenEstimateContextNix(pvlData->tec2);
    airFree(pvlData->vbuf);
    airFree(pvlData->wght);
    airFree(pvlData->qvals);
    airFree(pvlData->qpoints);
    airFree(pvlData->dists);
    airFree(pvlData->weights);
    nrrdNuke(pvlData->nten1EigenGrads);
    airRandMTStateNix(pvlData->randState);
    airFree(pvlData);
  }
  return NULL;
}

/* --------------------- kindData */

tenDwiGageKindData*
tenDwiGageKindDataNew() {
  tenDwiGageKindData *ret;
  
  ret = AIR_CAST(tenDwiGageKindData *, malloc(sizeof(tenDwiGageKindData)));
  if (ret) {
    /* it may be that only one of these is actually filled */
    ret->ngrad = nrrdNew();
    ret->nbmat = nrrdNew();
    ret->thresh = ret->soft = ret->bval = AIR_NAN;
    ret->est1Method = tenEstimate1MethodUnknown;
    ret->est2Method = tenEstimate2MethodUnknown;
    ret->randSeed = 42;
  }
  return ret;
}

tenDwiGageKindData*
tenDwiGageKindDataNix(tenDwiGageKindData *kindData) {
  
  if (kindData) {
    nrrdNuke(kindData->ngrad);
    nrrdNuke(kindData->nbmat);
    airFree(kindData);
  }
  return NULL;
}

/* --------------------- dwiKind, and dwiKind->data setting*/

/*
** Because this kind has to be dynamically allocated,
** this is not the kind, but just the template for it
*/
gageKind
_tenDwiGageKindTmpl = {
  AIR_TRUE, /* dynamically allocated */
  TEN_DWI_GAGE_KIND_NAME,
  &_tenDwiGage,
  1,
  0, /* NOT: set later by tenDwiGageKindSet() */
  TEN_DWI_GAGE_ITEM_MAX,
  NULL, /* NOT: modified copy of _tenDwiGageTable,
           allocated by tenDwiGageKindNew(), and
           set by _tenDwiGageKindSet() */
  _tenDwiGageIv3Print,
  _tenDwiGageFilter,
  _tenDwiGageAnswer,
  _tenDwiGagePvlDataNew,
  _tenDwiGagePvlDataCopy,
  _tenDwiGagePvlDataNix,
  _tenDwiGagePvlDataUpdate,
  NULL /* NOT: allocated by tenDwiGageKindNew(),
          insides set by tenDwiGageKindSet() */
};

gageKind *
tenDwiGageKindNew() {
  gageKind *kind;
  
  kind = AIR_CAST(gageKind *, malloc(sizeof(gageKind)));
  if (kind) {
    memcpy(kind, &_tenDwiGageKindTmpl, sizeof(gageKind));
    kind->valLen = 0; /* still has to be set later */
    kind->table = AIR_CAST(gageItemEntry *,
                           malloc(sizeof(_tenDwiGageTable)));
    memcpy(kind->table, _tenDwiGageTable, sizeof(_tenDwiGageTable));
    kind->data = AIR_CAST(void *, tenDwiGageKindDataNew());
  }
  return kind;
}

gageKind *
tenDwiGageKindNix(gageKind *kind) {
  
  if (kind) {
    airFree(kind->table);
    tenDwiGageKindDataNix(AIR_CAST(tenDwiGageKindData *, kind->data));
    airFree(kind);
  }
  return NULL;
}

/*
** NOTE: this sets information in both the kind and kindData
*/
int
tenDwiGageKindSet(gageKind *dwiKind,
                  double thresh, double soft, double bval, double valueMin,
                  const Nrrd *ngrad,
                  const Nrrd *nbmat,
                  int e1method, int e2method,
                  unsigned int randSeed) {
  char me[]="tenDwiGageKindSet", err[BIFF_STRLEN];
  tenDwiGageKindData *kindData;
  double grad[3], (*lup)(const void *, size_t);
  unsigned int gi;

  if (!dwiKind) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 0;
  }
  if (!( !!(ngrad) ^ !!(nbmat) )) {
    sprintf(err, "%s: need exactly one non-NULL in {ngrad,nbmat}", me);
    biffAdd(TEN, err); return 1;
  }
  if (nbmat) {
    sprintf(err, "%s: sorry, B-matrices temporarily disabled", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenGradientCheck(ngrad, nrrdTypeDefault, 7)) {
    sprintf(err, "%s: problem with given gradients", me);
    biffAdd(TEN, err); return 1;
  }
  /* make sure that gradient lengths are as expected */
  lup = nrrdDLookup[ngrad->type];
  grad[0] = lup(ngrad->data, 0);
  grad[1] = lup(ngrad->data, 1);
  grad[2] = lup(ngrad->data, 2);
  if (0.0 != ELL_3V_LEN(grad)) {
    sprintf(err, "%s: sorry, currently need grad[0] to be len 0 (not %g)",
            me, ELL_3V_LEN(grad));
    biffAdd(TEN, err); return 1;
  }
  for (gi=1; gi<ngrad->axis[1].size; gi++) {
    grad[0] = lup(ngrad->data, 0 + 3*gi);
    grad[1] = lup(ngrad->data, 1 + 3*gi);
    grad[2] = lup(ngrad->data, 2 + 3*gi);
    if (0.0 == ELL_3V_LEN(grad)) {
      sprintf(err, "%s: sorry, all but first gradient must be non-zero "
              "(%u is zero)", me, gi);
      biffAdd(TEN, err); return 1;
    }
  }
  if (airEnumValCheck(tenEstimate1Method, e1method)) {
    sprintf(err, "%s: e1method %d is not a valid %s", me, 
            e1method, tenEstimate1Method->name);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenEstimate2Method, e2method)) {
    sprintf(err, "%s: emethod %d is not a valid %s", me, 
            e2method, tenEstimate2Method->name);
    biffAdd(TEN, err); return 1;
  }

  kindData = AIR_CAST(tenDwiGageKindData *, dwiKind->data);
  if (nrrdConvert(kindData->ngrad, ngrad, nrrdTypeDouble)) {
    sprintf(err, "%s: trouble converting", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  dwiKind->valLen = kindData->ngrad->axis[1].size;

  /* fixing up the item table ... */
  dwiKind->table[tenDwiGageAll].answerLength = dwiKind->valLen;
  dwiKind->table[tenDwiGageJustDWI].answerLength = dwiKind->valLen - 1;
  dwiKind->table[tenDwiGageADC].answerLength = dwiKind->valLen - 1;
  dwiKind->table[tenDwiGageTensorAllDWIError].answerLength =
    dwiKind->valLen - 1;
  switch (e1method) {
  case tenEstimate1MethodLLS:
    dwiKind->table[tenDwiGageTensor].prereq[0]
      = tenDwiGageTensorLLS;
    dwiKind->table[tenDwiGageTensorError].prereq[0] 
      = tenDwiGageTensorLLSError;
    dwiKind->table[tenDwiGageTensorErrorLog].prereq[0] 
      = tenDwiGageTensorLLSErrorLog;
    dwiKind->table[tenDwiGageTensorLikelihood].prereq[0]
      = tenDwiGageTensorLLSLikelihood;
    break;
  case tenEstimate1MethodWLS:
    dwiKind->table[tenDwiGageTensor].prereq[0]
      = tenDwiGageTensorWLS;
    dwiKind->table[tenDwiGageTensorError].prereq[0] 
      = tenDwiGageTensorWLSError;
    dwiKind->table[tenDwiGageTensorErrorLog].prereq[0] 
      = tenDwiGageTensorWLSErrorLog;
    dwiKind->table[tenDwiGageTensorLikelihood].prereq[0]
      = tenDwiGageTensorWLSLikelihood;
    break;
  case tenEstimate1MethodNLS:
    dwiKind->table[tenDwiGageTensor].prereq[0]
      = tenDwiGageTensorNLS;
    dwiKind->table[tenDwiGageTensorError].prereq[0] 
      = tenDwiGageTensorNLSError;
    dwiKind->table[tenDwiGageTensorErrorLog].prereq[0] 
      = tenDwiGageTensorNLSErrorLog;
    dwiKind->table[tenDwiGageTensorLikelihood].prereq[0]
      = tenDwiGageTensorNLSLikelihood;
    break;
  case tenEstimate1MethodMLE:
    dwiKind->table[tenDwiGageTensor].prereq[0]
      = tenDwiGageTensorMLE;
    dwiKind->table[tenDwiGageTensorError].prereq[0] 
      = tenDwiGageTensorMLEError;
    dwiKind->table[tenDwiGageTensorErrorLog].prereq[0] 
      = tenDwiGageTensorMLEErrorLog;
    dwiKind->table[tenDwiGageTensorLikelihood].prereq[0]
      = tenDwiGageTensorMLELikelihood;
    break;
  default:
    sprintf(err, "%s: unimplemented %s: %s (%d)", me,
            tenEstimate1Method->name,
            airEnumStr(tenEstimate1Method, e1method), e1method);
    biffAdd(TEN, err); return 1;
    break;
  }
  kindData->thresh = thresh;
  kindData->soft = soft;
  kindData->bval = bval;
  kindData->valueMin = valueMin;
  kindData->est1Method = e1method;
  kindData->est2Method = e2method;
  kindData->randSeed = randSeed;
  return 0;
}

int
tenDwiGageKindCheck(const gageKind *kind) {
  char me[]="tenDwiGageKindCheck", err[BIFF_STRLEN];
  
  if (!kind) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (strcmp(kind->name, TEN_DWI_GAGE_KIND_NAME)) {
    sprintf(err, "%s: got \"%s\" kind, not \"%s\"", me,
            kind->name, TEN_DWI_GAGE_KIND_NAME);
    biffAdd(TEN, err); return 1;
  }
  if (0 == kind->valLen) {
    sprintf(err, "%s: don't yet know valLen", me);
    biffAdd(TEN, err); return 1;
  }
  if (!kind->data) {
    sprintf(err, "%s: kind->data is NULL", me);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}
