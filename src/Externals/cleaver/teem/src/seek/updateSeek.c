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

#include "seek.h"
#include "privateSeek.h"

static int
updateNinEtAl(seekContext *sctx) {
  char me[]="updateNinEtAl", err[BIFF_STRLEN];

  if (sctx->verbose > 5) {
    fprintf(stderr, "%s: --------------------\n", me);
    fprintf(stderr, "%s: flagData = %d\n", me, sctx->flag[flagData]);
  }

  if (!( sctx->ninscl || sctx->pvl )) {
    sprintf(err, "%s: data never set", me);
    biffAdd(SEEK, err); return 1;
  }

  if (sctx->flag[flagData]) {
    if (sctx->ninscl) {
      sctx->nin = sctx->ninscl;
      sctx->baseDim = 0;
      if (gageShapeSet(sctx->_shape, sctx->ninscl, 0)) {
        sprintf(err, "%s: trouble with scalar volume", me);
        biffMove(SEEK, err, GAGE); return 1;
      }
      sctx->shape = sctx->_shape;
    } else {
      sctx->nin = sctx->pvl->nin;
      sctx->baseDim = sctx->pvl->kind->baseDim;
      sctx->shape = sctx->gctx->shape;
    }
    sctx->flag[flagData] = AIR_FALSE;
    sctx->flag[flagNinEtAl] = AIR_TRUE;
  }
  return 0;
}

static int
updateAnswerPointers(seekContext *sctx) {
  char me[]="updateAnswerPointers", err[BIFF_STRLEN];

  if (sctx->verbose > 5) {
    fprintf(stderr, "%s: --------------------\n", me);
    fprintf(stderr, "%s: flagItemValue = %d\n", me,
            sctx->flag[flagItemValue]);
    fprintf(stderr, "%s: flagItemStrength = %d\n", me,
            sctx->flag[flagItemStrength]);
    fprintf(stderr, "%s: flagItemNormal = %d\n", me,
            sctx->flag[flagItemNormal]);
    fprintf(stderr, "%s: flagItemGradient = %d\n", me,
            sctx->flag[flagItemGradient]);
    fprintf(stderr, "%s: flagItemEigensystem = %d\n", me,
            sctx->flag[flagItemEigensystem]);
    fprintf(stderr, "%s: flagNinEtAl = %d\n", me,
            sctx->flag[flagNinEtAl]);
    fprintf(stderr, "%s: flagNormalsFind = %d\n", me,
            sctx->flag[flagNormalsFind]);
    fprintf(stderr, "%s: flagStrengthUse = %d\n", me,
            sctx->flag[flagStrengthUse]);
    fprintf(stderr, "%s: flagType = %d\n", me,
            sctx->flag[flagType]);
    fprintf(stderr, "%s: flagData = %d\n", me,
            sctx->flag[flagData]);
  }

  if (seekTypeUnknown == sctx->type) {
    sprintf(err, "%s: feature type never set", me);
    biffAdd(SEEK, err); return 1;
  }
  
  if (sctx->flag[flagItemValue]
      || sctx->flag[flagItemStrength]
      || sctx->flag[flagItemNormal]
      || sctx->flag[flagItemGradient]
      || sctx->flag[flagItemEigensystem]
      || sctx->flag[flagNinEtAl]
      || sctx->flag[flagNormalsFind]
      || sctx->flag[flagStrengthUse]
      || sctx->flag[flagType]) {

    /* this is apt regardless of feature type */
    if (sctx->strengthUse) {
      if (-1 == sctx->stngItem) {
        sprintf(err, "%s: need to set strength item to use strength", me);
        biffAdd(SEEK, err); return 1;
      }
      sctx->stngAns = (gageAnswerPointer(sctx->gctx, sctx->pvl,
                                         sctx->stngItem));
    } else {
      sctx->stngAns = NULL;
    }

    switch (sctx->type) {
    case seekTypeIsocontour:
      if (!( sctx->ninscl || -1 != sctx->sclvItem )) {
        sprintf(err, "%s: need either scalar volume or value item set for %s",
                me, airEnumStr(seekType, seekTypeIsocontour));
        biffAdd(SEEK, err); return 1;
      }
      if (sctx->normalsFind) {
        /* NOTE simplifying assumption described in seek.h */
        if (!( sctx->ninscl || -1 != sctx->normItem )) {
          sprintf(err, "%s: need either scalar volume or "
                  "normal item set for normals for %s",
                  me, airEnumStr(seekType, seekTypeIsocontour));
          biffAdd(SEEK, err); return 1;
        }
      }
      if (sctx->ninscl) {
        sctx->sclvAns = NULL;
        sctx->normAns = NULL;
      } else {
        sctx->sclvAns = gageAnswerPointer(sctx->gctx, sctx->pvl,
                                          sctx->sclvItem);
        sctx->normAns = (sctx->normalsFind
                         ? gageAnswerPointer(sctx->gctx, sctx->pvl,
                                             sctx->normItem)
                         : NULL);
      }
      if (sctx->flag[flagItemGradient]
          || sctx->flag[flagItemEigensystem]) {
        sprintf(err, "%s: can't set gradient or eigensystem for %s",
                me, airEnumStr(seekType, seekTypeIsocontour));
        biffAdd(SEEK, err); return 1;
      }
      sctx->gradAns = NULL;
      sctx->evalAns = NULL;
      sctx->evecAns = NULL;
      break;
    case seekTypeRidgeSurface:
    case seekTypeValleySurface:
    case seekTypeMaximalSurface:
    case seekTypeMinimalSurface:
      if ( !sctx->pvl ) {
        sprintf(err, "%s: can't find %s without a gage context",
                me, airEnumStr(seekType, sctx->type));
        biffAdd(SEEK, err); return 1;
      }
      if (!( -1 != sctx->gradItem
             && -1 != sctx->evalItem
             && -1 != sctx->evecItem )) {
        sprintf(err, "%s: grad, eval, evec items not all set", me);
        biffAdd(SEEK, err); return 1;
      }
      if (sctx->normalsFind) {
        /* NOTE simplifying assumption described in seek.h */
        if (-1 == sctx->normItem) {
          sprintf(err, "%s: need normal item set for normals for %s",
                  me, airEnumStr(seekType, sctx->type));
          biffAdd(SEEK, err); return 1;
        }
        sctx->normAns = (gageAnswerPointer(sctx->gctx, sctx->pvl,
                                           sctx->normItem));
      } else {
        sctx->normAns = NULL;
      }
      sctx->sclvAns = NULL;
      sctx->gradAns = gageAnswerPointer(sctx->gctx, sctx->pvl, sctx->gradItem);
      sctx->evalAns = gageAnswerPointer(sctx->gctx, sctx->pvl, sctx->evalItem);
      sctx->evecAns = gageAnswerPointer(sctx->gctx, sctx->pvl, sctx->evecItem);
      break;
    default:
      sprintf(err, "%s: sorry, %s extraction not implemented", me,
              airEnumStr(seekType, sctx->type));
      biffAdd(SEEK, err); return 1;
      break;
    }

    sctx->flag[flagItemValue] = AIR_FALSE;
    sctx->flag[flagItemStrength] = AIR_FALSE;
    sctx->flag[flagItemNormal] = AIR_FALSE;
    sctx->flag[flagItemGradient] = AIR_FALSE;
    sctx->flag[flagItemEigensystem] = AIR_FALSE;
    sctx->flag[flagNormalsFind] = AIR_FALSE;
    sctx->flag[flagAnswerPointers] = AIR_TRUE;
  }

  return 0;
}

static int
updateSxSySz(seekContext *sctx) {
  char me[]="updateSxSySz", err[BIFF_STRLEN];
  size_t sizeIn[3], sizeOut[3];
  double min, max, scl[3], off[3];
  unsigned int axi;

  if (sctx->verbose > 5) {
    fprintf(stderr, "%s: --------------------\n", me);
    fprintf(stderr, "%s: flagSamples = %d\n", me, sctx->flag[flagSamples]);
    fprintf(stderr, "%s: flagNinEtAl = %d\n", me, sctx->flag[flagNinEtAl]);
  }

  sizeIn[0] = sctx->nin->axis[sctx->baseDim+0].size;
  sizeIn[1] = sctx->nin->axis[sctx->baseDim+1].size;
  sizeIn[2] = sctx->nin->axis[sctx->baseDim+2].size;

  if (sctx->flag[flagSamples]
      || sctx->flag[flagNinEtAl]) {
    if (0 == sctx->samples[0]
        || 0 == sctx->samples[1]
        || 0 == sctx->samples[2]) {
      ELL_3V_COPY(sizeOut, sizeIn);
    } else {
      if (!sctx->pvl) {
        sprintf(err, "%s: can't specify # samples (%u,%u,%u) independent of "
                "volume dimensions (%u,%u,%u) without a gage context", me,
                AIR_CAST(unsigned int, sctx->samples[0]),
                AIR_CAST(unsigned int, sctx->samples[1]),
                AIR_CAST(unsigned int, sctx->samples[2]),
                AIR_CAST(unsigned int, sizeIn[0]),
                AIR_CAST(unsigned int, sizeIn[1]),
                AIR_CAST(unsigned int, sizeIn[2]));
        biffAdd(SEEK, err); return 1;
      }
      ELL_3V_COPY(sizeOut, sctx->samples);
    }
    /* want to make absolutely sure txfIdx was being set ... 
    if (sctx->sx != sizeOut[0]
        || sctx->sy != sizeOut[1]
        || sctx->sz != sizeOut[2]) { */
    sctx->sx = sizeOut[0];
    sctx->sy = sizeOut[1];
    sctx->sz = sizeOut[2];
    /* there has got to be a better way of doing this... */
    /* perhaps refer to the way origin is calculated in new nrrd resampler? */
    for (axi=0; axi<3; axi++) {
      min = (nrrdCenterCell == sctx->shape->center
             ? -0.5
             : 0.0);
      max = (nrrdCenterCell == sctx->shape->center
             ? sizeIn[axi] - 0.5
             : sizeIn[axi] - 1.0);
      off[axi] = NRRD_POS(sctx->shape->center, min, max, sizeOut[axi], 0);
      scl[axi] = (NRRD_POS(sctx->shape->center, min, max, sizeOut[axi], 1)
                  - off[axi]);
    }
    ELL_4V_SET(sctx->txfIdx + 0*4, scl[0],    0.0,    0.0, off[0]);
    ELL_4V_SET(sctx->txfIdx + 1*4,    0.0, scl[1],    0.0, off[1]);
    ELL_4V_SET(sctx->txfIdx + 2*4,    0.0,    0.0, scl[2], off[2]);
    ELL_4V_SET(sctx->txfIdx + 3*4,    0.0,    0.0,    0.0,    1.0);
    sctx->flag[flagSxSySz] = AIR_TRUE;
    sctx->flag[flagSamples] = AIR_FALSE;
  }
  return 0;
}

static int
updateReverse(seekContext *sctx) {
  char me[]="updateReverse" /* , err[BIFF_STRLEN] */;

  if (sctx->verbose > 5) {
    fprintf(stderr, "%s: --------------------\n", me);
    fprintf(stderr, "%s: flagNinEtAl = %d\n", me,
            sctx->flag[flagNinEtAl]);
    fprintf(stderr, "%s: flagLowerInside = %d\n", me,
            sctx->flag[flagLowerInside]);
  }

  if (sctx->flag[flagNinEtAl]
      || sctx->flag[flagLowerInside]) {
    double mat[9];
    int reverse;

    ELL_34M_EXTRACT(mat, sctx->shape->ItoW);
    reverse = (!!sctx->lowerInside) ^ (ELL_3M_DET(mat) < 0);
    if (sctx->reverse != reverse) {
      sctx->reverse = reverse;
      sctx->flag[flagReverse] = AIR_TRUE;
    }
  }
  return 0;
}

static int
updateTxfNormal(seekContext *sctx) {
  char me[]="updateTxfNormal" /*, err[BIFF_STRLEN] */ ;

  if (sctx->verbose > 5) {
    fprintf(stderr, "%s: --------------------\n", me);
    fprintf(stderr, "%s: flagNinEtAl = %d\n", me,
            sctx->flag[flagNinEtAl]);
    fprintf(stderr, "%s: flagLowerInside = %d\n", me,
            sctx->flag[flagLowerInside]);
  }

  if (sctx->flag[flagNinEtAl]
      || sctx->flag[flagLowerInside]) {
    double matA[9], matB[9];

    ELL_34M_EXTRACT(matA, sctx->shape->ItoW);
    ell_3m_inv_d(matB, matA);
    ELL_3M_TRANSPOSE(sctx->txfNormal, matB);
    if (!sctx->lowerInside) {
      ELL_3M_SCALE(sctx->txfNormal, -1, sctx->txfNormal);
    }
    sctx->flag[flagTxfNormal] = AIR_TRUE;
  }
  sctx->flag[flagLowerInside] = AIR_FALSE;
  return 0;
}

static int
updateSlabCacheAlloc(seekContext *sctx) {
  char me[]="updateSlabCacheAlloc", err[BIFF_STRLEN];
  int E;

  if (sctx->verbose > 5) {
    fprintf(stderr, "%s: --------------------\n", me);
    fprintf(stderr, "%s: flagType = %d (type = %s)\n", me,
            sctx->flag[flagType], airEnumStr(seekType, sctx->type));
    fprintf(stderr, "%s: flagStrengthUse = %d\n", me,
            sctx->flag[flagStrengthUse]);
    fprintf(stderr, "%s: flagSxSySz = %d\n", me,
            sctx->flag[flagSxSySz]);
  }

  E = 0;
  if (sctx->flag[flagType]
      || sctx->flag[flagStrengthUse]  /* kind of sloppy/overkill */
      || sctx->flag[flagSxSySz]) {
    if (!E) E |= nrrdMaybeAlloc_va(sctx->nvidx, nrrdTypeInt, 3,
                                   AIR_CAST(size_t, 5),
                                   sctx->sx,
                                   sctx->sy);
    if (!E) sctx->vidx = AIR_CAST(int*, sctx->nvidx->data);
    if (sctx->strengthUse) {
      if (!E) E |= nrrdMaybeAlloc_va(sctx->nstng, nrrdTypeDouble, 3,
                                     AIR_CAST(size_t, 2),
                                     sctx->sx,
                                     sctx->sy);
      if (!E) sctx->stng = AIR_CAST(double*, sctx->nstng->data);
    }
    if (seekTypeIsocontour == sctx->type) {
      if (!E) E |= nrrdMaybeAlloc_va(sctx->nsclv, nrrdTypeDouble, 3,
                                     AIR_CAST(size_t, 4),
                                     sctx->sx + 2,
                                     sctx->sy + 2);
      if (!E) sctx->sclv = AIR_CAST(double*, sctx->nsclv->data);
    }
    if (seekTypeRidgeSurface == sctx->type
        || seekTypeValleySurface == sctx->type
        || seekTypeMaximalSurface == sctx->type
        || seekTypeMinimalSurface == sctx->type) {
      if (!E) E |= nrrdMaybeAlloc_va(sctx->ngrad, nrrdTypeDouble, 4, 
                                     AIR_CAST(size_t, 3),
                                     AIR_CAST(size_t, 2),
                                     sctx->sx,
                                     sctx->sy);
      if (!E) sctx->grad = AIR_CAST(double*, sctx->ngrad->data);
      if (!E) E |= nrrdMaybeAlloc_va(sctx->neval, nrrdTypeDouble, 4, 
                                     AIR_CAST(size_t, 3),
                                     AIR_CAST(size_t, 2),
                                     sctx->sx,
                                     sctx->sy);
      if (!E) sctx->eval = AIR_CAST(double*, sctx->neval->data);
      if (!E) E |= nrrdMaybeAlloc_va(sctx->nevec, nrrdTypeDouble, 4, 
                                     AIR_CAST(size_t, 9),
                                     AIR_CAST(size_t, 2),
                                     sctx->sx,
                                     sctx->sy);
      if (!E) sctx->evec = AIR_CAST(double*, sctx->nevec->data);
      if (!E) E |= nrrdMaybeAlloc_va(sctx->nflip, nrrdTypeChar, 3, 
                                     AIR_CAST(size_t, 5),
                                     sctx->sx,
                                     sctx->sy);
      if (!E) sctx->flip = AIR_CAST(signed char*, sctx->nflip->data);
    } else {
      sctx->grad = NULL;
      sctx->eval = NULL;
      sctx->evec = NULL;
      sctx->flip = NULL;
    }
    sctx->flag[flagSlabCacheAlloc] = AIR_TRUE;
  }
  if (E) {
    sprintf(err, "%s: couldn't allocate all slab caches", me);
    biffMove(SEEK, err, NRRD); return 1;
  }
  sctx->flag[flagStrengthUse] = AIR_FALSE;
  sctx->flag[flagSxSySz] = AIR_FALSE;
  return 0;
}

static int
updateSclDerived(seekContext *sctx) {
  char me[]="updateSclDerived", err[BIFF_STRLEN], doneStr[AIR_STRLEN_SMALL];
  double *scl, idxIn[4], idxOut[4], val;
  unsigned int xi, yi, zi;

  if (sctx->verbose > 5) {
    fprintf(stderr, "%s: --------------------\n", me);
    fprintf(stderr, "%s: flagType = %d\n", me,
            sctx->flag[flagType]);
    fprintf(stderr, "%s: flagNinEtAl = %d\n", me,
            sctx->flag[flagNinEtAl]);
  }

  if (sctx->flag[flagType]
      || sctx->flag[flagNinEtAl]) {
    if (!( seekTypeIsocontour == sctx->type
           && sctx->pvl )) {
      nrrdEmpty(sctx->nsclDerived);
    } else {
      if (nrrdMaybeAlloc_va(sctx->nsclDerived, nrrdTypeDouble, 3,
                            sctx->sx, sctx->sy, sctx->sz)) {
        sprintf(err, "%s: couldn't allocated derived scalar volume", me);
        biffMove(SEEK, err, NRRD); return 1;
      }
      scl = AIR_CAST(double*, sctx->nsclDerived->data);
      if (sctx->verbose) {
        fprintf(stderr, "%s: pre-computing scalar volume ...       ", me);
      }
      for (zi=0; zi<sctx->sz; zi++) {
        if (sctx->verbose) {
          fprintf(stderr, "%s", airDoneStr(0, zi, sctx->sz-1, doneStr));
          fflush(stderr);
        }
        for (yi=0; yi<sctx->sy; yi++) {
          for (xi=0; xi<sctx->sx; xi++) {
            ELL_4V_SET(idxOut, xi, yi, zi, 1.0);
            ELL_4MV_MUL(idxIn, sctx->txfIdx, idxOut);
            ELL_34V_HOMOG(idxIn, idxIn);
            gageProbe(sctx->gctx, idxIn[0], idxIn[1], idxIn[2]);
            val = sctx->sclvAns[0];
            if (!AIR_EXISTS(val)) {
              sprintf(err, "%s: probed scalar[%u,%u,%u] %g doesn't exist", me,
                      xi, yi, zi, val);
              biffAdd(SEEK, err); return 1;
            }
            scl[xi + sctx->sx*(yi + sctx->sy*zi)] = val;
          }
        }
      }
      if (sctx->verbose) {
        fprintf(stderr, "%s\n", airDoneStr(0, zi, sctx->sz-1, doneStr));
      }
    }
    sctx->flag[flagSclDerived] = AIR_TRUE;
  }
  
  return 0;
}

static int
updateSpanSpaceHist(seekContext *sctx) {
  char me[]="updateSpanSpaceHist", err[BIFF_STRLEN];
  unsigned int sx, sy, sz, ss, xi, yi, zi, vi, si, minI, maxI, *spanHist;
  double min, max, val;
  const void *data;
  double (*lup)(const void *, size_t);

  if (sctx->verbose > 5) {
    fprintf(stderr, "%s: --------------------\n", me);
    fprintf(stderr, "%s: flagType = %d\n", me,
            sctx->flag[flagType]);
    fprintf(stderr, "%s: flagSclDerived = %d\n", me,
            sctx->flag[flagSclDerived]);
    fprintf(stderr, "%s: flagNinEtAl = %d\n", me,
            sctx->flag[flagNinEtAl]);
  }

  if (sctx->flag[flagType]
      || sctx->flag[flagSclDerived]
      || sctx->flag[flagNinEtAl]) {
    if (seekTypeIsocontour != sctx->type) {
      nrrdEmpty(sctx->nspanHist);
      sctx->range->min = AIR_NAN;
      sctx->range->max = AIR_NAN;
    } else {
      nrrdRangeSet(sctx->range,
                   (sctx->ninscl ? sctx->ninscl : sctx->nsclDerived),
                   nrrdBlind8BitRangeFalse);
      if (sctx->range->hasNonExist) {
        sprintf(err, "%s: scalar volume has non-existent values", me);
        biffAdd(SEEK, err); return 1;
      }
      sctx->nspanHist->axis[0].min = sctx->range->min;
      sctx->nspanHist->axis[1].min = sctx->range->min;
      sctx->nspanHist->axis[0].max = sctx->range->max;
      sctx->nspanHist->axis[1].max = sctx->range->max;

      if (sctx->ninscl) {
        lup = nrrdDLookup[sctx->ninscl->type];
        data = sctx->ninscl->data;
      } else {
        lup = nrrdDLookup[sctx->nsclDerived->type];
        data = sctx->nsclDerived->data;
      }

      /* calculate the span space histogram */
      if (nrrdMaybeAlloc_va(sctx->nspanHist, nrrdTypeUInt, 2, 
                            AIR_CAST(size_t, sctx->spanSize), 
                            AIR_CAST(size_t, sctx->spanSize))) {
        sprintf(err, "%s: couldn't allocate space space histogram", me);
        biffMove(SEEK, err, NRRD); return 1;
      }
      spanHist = AIR_CAST(unsigned int*, sctx->nspanHist->data);
      sx = sctx->sx;
      sy = sctx->sy;
      sz = sctx->sz;
      ss = sctx->spanSize;
      for (si=0; si<ss*ss; si++) {
        spanHist[si] = 0;
      }
      for (zi=0; zi<sz-1; zi++) {
        for (yi=0; yi<sy-1; yi++) {
          for (xi=0; xi<sx-1; xi++) {
            vi = xi + sx*(yi + sy*zi);
            val = lup(data, vi + 0 + 0*sx + 0*sx*sy);
            min = max = val;
            val = lup(data, vi + 1 + 0*sx + 0*sx*sy);
            min = AIR_MIN(min, val);
            max = AIR_MAX(max, val);
            val = lup(data, vi + 0 + 1*sx + 0*sx*sy);
            min = AIR_MIN(min, val);
            max = AIR_MAX(max, val);
            val = lup(data, vi + 1 + 1*sx + 0*sx*sy);
            min = AIR_MIN(min, val);
            max = AIR_MAX(max, val);
            val = lup(data, vi + 0 + 0*sx + 1*sx*sy);
            min = AIR_MIN(min, val);
            max = AIR_MAX(max, val);
            val = lup(data, vi + 1 + 0*sx + 1*sx*sy);
            min = AIR_MIN(min, val);
            max = AIR_MAX(max, val);
            val = lup(data, vi + 0 + 1*sx + 1*sx*sy);
            min = AIR_MIN(min, val);
            max = AIR_MAX(max, val);
            val = lup(data, vi + 1 + 1*sx + 1*sx*sy);
            min = AIR_MIN(min, val);
            max = AIR_MAX(max, val);
            minI = airIndex(sctx->range->min, min, sctx->range->max, ss);
            maxI = airIndex(sctx->range->min, max, sctx->range->max, ss);
            spanHist[minI + ss*maxI]++;
          }
        }
      }
    }
    sctx->flag[flagSclDerived] = AIR_FALSE;
    sctx->flag[flagSpanSpaceHist] = AIR_TRUE;
  }
  return 0;
}

static int
updateResult(seekContext *sctx) {
  char me[]="updateResult", err[BIFF_STRLEN];

  if (sctx->verbose > 5) {
    fprintf(stderr, "%s: --------------------\n", me);
    fprintf(stderr, "%s: flagIsovalue = %d\n", me,
            sctx->flag[flagIsovalue]);
    fprintf(stderr, "%s: flagAnswerPointers = %d\n", me,
            sctx->flag[flagAnswerPointers]);
    fprintf(stderr, "%s: flagType = %d\n", me,
            sctx->flag[flagType]);
    fprintf(stderr, "%s: flagSlabCacheAlloc = %d\n", me,
            sctx->flag[flagSlabCacheAlloc]);
    fprintf(stderr, "%s: flagSpanSpaceHist = %d\n", me,
            sctx->flag[flagSpanSpaceHist]);
    fprintf(stderr, "%s: flagNinEtAl = %d\n", me,
            sctx->flag[flagNinEtAl]);
    fprintf(stderr, "%s: flagReverse = %d\n", me,
            sctx->flag[flagReverse]);
    fprintf(stderr, "%s: flagTxfNormal = %d\n", me,
            sctx->flag[flagTxfNormal]);
  }

  if (seekTypeIsocontour != sctx->type
      && sctx->flag[flagIsovalue]) {
    sprintf(err, "%s: can't set isovalue for %s (only %s)", me,
            airEnumStr(seekType, sctx->type),
            airEnumStr(seekType, seekTypeIsocontour));
    biffAdd(SEEK, err); return 1;
  }

  if (sctx->strengthUse && !sctx->stngAns) {
    sprintf(err, "%s: can't use feature strength without a strength item", me);
    biffAdd(SEEK, err); return 1;
  }

  /* this seems to be a very pointless exercise */
  if (sctx->flag[flagIsovalue]
      || sctx->flag[flagAnswerPointers]
      || sctx->flag[flagStrengthUse]
      || sctx->flag[flagStrength]
      || sctx->flag[flagType]
      || sctx->flag[flagSlabCacheAlloc]
      || sctx->flag[flagSpanSpaceHist]
      || sctx->flag[flagNinEtAl]
      || sctx->flag[flagReverse]
      || sctx->flag[flagTxfNormal]) {

    sctx->flag[flagResult] = AIR_TRUE;

    sctx->flag[flagIsovalue] = AIR_FALSE;
    sctx->flag[flagAnswerPointers] = AIR_FALSE;
    sctx->flag[flagStrengthUse] = AIR_FALSE;
    sctx->flag[flagStrength] = AIR_FALSE;
    sctx->flag[flagType] = AIR_FALSE;
    sctx->flag[flagSlabCacheAlloc] = AIR_FALSE;
    sctx->flag[flagSpanSpaceHist] = AIR_FALSE;
    sctx->flag[flagNinEtAl] = AIR_FALSE;
    sctx->flag[flagReverse] = AIR_FALSE;
    sctx->flag[flagTxfNormal] = AIR_FALSE;
  }

  return 0;
}

/*
******** seekUpdate
**
** traverses dependency graph for all of seek stuff, which necessarily
** includes nodes specific to, say, isosurfacing, even when isosurfacing
** is not being done.  The idea is to use the same graph for all feature
** types, for error checking if nothing else, leavings steps as no-ops 
** as needed.
*/
int
seekUpdate(seekContext *sctx) {
  char me[]="seekUpdate", err[BIFF_STRLEN];
  int E;

  if (!sctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(SEEK, err); return 1;
  }
  E = 0;
  if (!E) E |= updateNinEtAl(sctx);
  if (!E) E |= updateAnswerPointers(sctx);
  if (!E) E |= updateSxSySz(sctx);
  if (!E) E |= updateReverse(sctx);
  if (!E) E |= updateTxfNormal(sctx);
  if (!E) E |= updateSlabCacheAlloc(sctx);
  if (!E) E |= updateSclDerived(sctx);
  if (!E) E |= updateSpanSpaceHist(sctx);
  if (!E) E |= updateResult(sctx);
  if (E) {
    sprintf(err, "%s: trouble updating", me);
    biffAdd(SEEK, err); return 1;
  }
  
  return 0;
}
