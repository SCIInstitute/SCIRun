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


#include "pull.h"
#include "privatePull.h"

pullContext *
pullContextNew(void) {
  pullContext *pctx;
  unsigned int ii;

  pctx = (pullContext *)calloc(1, sizeof(pullContext));
  if (!pctx) {
    return NULL;
  }
  
  pctx->verbose = 0;
  pctx->pointNumInitial = 0;
  pctx->npos = NULL;
  for (ii=0; ii<PULL_VOLUME_MAXNUM; ii++) {
    pctx->vol[ii] = NULL;
  }
  pctx->volNum = 0;
  for (ii=0; ii<=PULL_INFO_MAX; ii++) {
    pctx->ispec[ii] = NULL;
    pctx->infoIdx[ii] = UINT_MAX;
  }

  pctx->stepInitial = 1;
  pctx->radiusSpace = 1;
  pctx->radiusScale = 1;
  pctx->neighborTrueProb = 1.0;
  pctx->probeProb = 1.0;
  pctx->opporStepScale = 1.0;
  pctx->stepScale = 0.5;
  pctx->energyImprovMin = 0.01;
  pctx->constraintStepMin = 0.0001;
  pctx->wall = 1;

  pctx->pointPerVoxel = 0;
  pctx->rngSeed = 42;
  pctx->threadNum = 1;
  pctx->iterMax = 0;
  pctx->constraintIterMax = 15;
  pctx->snap = 0;
  
  pctx->energySpec = pullEnergySpecNew();
  pctx->alpha = 0.5;
  pctx->beta = 1.0;
  pctx->jitter = 1.0;
  pctx->radiusSingle = AIR_TRUE;

  pctx->binSingle = AIR_FALSE;
  pctx->binIncr = 32;

  ELL_4V_SET(pctx->bboxMin, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_4V_SET(pctx->bboxMax, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN);
  pctx->infoTotalLen = 0; /* will be set later */
  pctx->idtagNext = 0;
  pctx->haveScale = AIR_FALSE;
  pctx->constraint = 0;
  pctx->finished = AIR_FALSE;
  pctx->maxDist = AIR_NAN;
  pctx->constraintVoxelSize = AIR_NAN;

  pctx->bin = NULL;
  ELL_3V_SET(pctx->binsEdge, 0, 0, 0);
  pctx->binNum = 0;
  pctx->binNextIdx = 0;
  pctx->pointPerm = NULL;
  pctx->pointBuff = NULL;
  pctx->binMutex = NULL;

  pctx->task = NULL;
  pctx->iterBarrierA = NULL;
  pctx->iterBarrierB = NULL;

  pctx->timeIteration = 0;
  pctx->timeRun = 0;
  pctx->stuckNum = 0;
  pctx->iter = 0;
  pctx->energy = AIR_NAN;
  pctx->noutPos = nrrdNew();
  return pctx;
}

/*
** this should only nix things created by pullContextNew, or the things
** (vols and ispecs) that were explicitly added to this context
*/
pullContext *
pullContextNix(pullContext *pctx) {
  unsigned int ii;
  
  if (pctx) {
    for (ii=0; ii<pctx->volNum; ii++) {
      pctx->vol[ii] = pullVolumeNix(pctx->vol[ii]);
    }
    pctx->volNum = 0;
    for (ii=0; ii<=PULL_INFO_MAX; ii++) {
      if (pctx->ispec[ii]) {
        pctx->ispec[ii] = pullInfoSpecNix(pctx->ispec[ii]);
      }
    }
    pctx->energySpec = pullEnergySpecNix(pctx->energySpec);
    /* handled elsewhere: bin, task, iterBarrierA, iterBarrierB */
    pctx->noutPos = nrrdNuke(pctx->noutPos);
    airFree(pctx);
  }
  return NULL;
}

int
_pullContextCheck(pullContext *pctx) {
  char me[]="_pullContextCheck", err[BIFF_STRLEN];
  unsigned int ii, sclvi;
  int gotIspec, gotConstr;

  if (!pctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(PULL, err); return 1;
  }
  if (pctx->npos) {
    if (nrrdCheck(pctx->npos)) {
      sprintf(err, "%s: got a broken npos", me);
      biffMove(PULL, err, NRRD); return 1;
    }
    if (!( 2 == pctx->npos->dim 
           && 4 == pctx->npos->axis[0].size
           && (nrrdTypeDouble == pctx->npos->type 
               || nrrdTypeFloat == pctx->npos->type) )) {
      sprintf(err, "%s: npos not a 2-D 4-by-N array of %s or %s"
              "(got %u-D %u-by-X of %s)", me,
              airEnumStr(nrrdType, nrrdTypeFloat),
              airEnumStr(nrrdType, nrrdTypeDouble),
              pctx->npos->dim,
              AIR_CAST(unsigned int, pctx->npos->axis[0].size),
              airEnumStr(nrrdType, pctx->npos->type));
      biffAdd(PULL, err); return 1;
    }
  } else {
    if (!( pctx->pointNumInitial >= 1 )) {
      sprintf(err, "%s: pctx->pointNumInitial (%d) not >= 1\n", me,
              pctx->pointNumInitial);
      biffAdd(PULL, err); return 1;
    }
  }
  if (!pctx->volNum) {
    sprintf(err, "%s: have no volumes set", me);
    biffAdd(PULL, err); return 1;
  }
  for (ii=0; ii<pctx->volNum; ii++) {
    if (pctx->vol[ii]->ninScale) {
      sclvi = ii;
      for (ii=sclvi+1; ii<pctx->volNum; ii++) {
        if (pctx->vol[ii]->ninScale) {
          sprintf(err, "%s: can have only 1 scale volume (not both %u and %u)",
                  me, ii, sclvi);
          biffAdd(PULL, err); return 1;
        }
      }
    }
  }
  gotConstr = 0;
  gotIspec = AIR_FALSE;
  for (ii=0; ii<=PULL_INFO_MAX; ii++) {
    if (pctx->ispec[ii]) {
      if (pctx->ispec[ii]->constraint) {
        if (1 != pullInfoAnswerLen(ii)) {
          sprintf(err, "%s: can't use non-scalar (len %u) %s as constraint",
                  me, pullInfoAnswerLen(ii), airEnumStr(pullInfo, ii));
          biffAdd(PULL, err); return 1;
        }
        if (gotConstr) {
          sprintf(err, "%s: can't also have %s constraint, already have "
                  "constraint on %s ", me, airEnumStr(pullInfo, ii),
                  airEnumStr(pullInfo, gotConstr));
          biffAdd(PULL, err); return 1;
        }
        /* elso no problems having constraint on ii */
        gotConstr = ii;
      }
      /* make sure we have extra info as necessary */
      switch (ii) {
      case pullInfoInside:
      case pullInfoHeight:
      case pullInfoHeightLaplacian:
      case pullInfoSeedThresh:
      case pullInfoTangentMode:
      case pullInfoIsovalue:
      case pullInfoStrength:
        if (!( AIR_EXISTS(pctx->ispec[ii]->scale)
               && AIR_EXISTS(pctx->ispec[ii]->zero) )) {
          sprintf(err, "%s: %s info needs scale (%g) and zero (%g)", me, 
                  airEnumStr(pullInfo, ii),
                  pctx->ispec[ii]->scale, pctx->ispec[ii]->zero);
          biffAdd(PULL, err); return 1;
        }
        break;
      }
      gotIspec = AIR_TRUE;
    }
  }

  if (!gotIspec) {
    sprintf(err, "%s: have no infos set", me);
    biffAdd(PULL, err); return 1;
  }
  if (pctx->ispec[pullInfoInside]) {
    if (!pctx->ispec[pullInfoInsideGradient]) {
      sprintf(err, "%s: want %s but don't have %s set", me, 
              airEnumStr(pullInfo, pullInfoInside),
              airEnumStr(pullInfo, pullInfoInsideGradient));
      biffAdd(PULL, err); return 1;
    }
  }
  if (pctx->ispec[pullInfoTangent2]) {
    if (!pctx->ispec[pullInfoTangent1]) {
      sprintf(err, "%s: want %s but don't have %s set", me, 
              airEnumStr(pullInfo, pullInfoTangent2),
              airEnumStr(pullInfo, pullInfoTangent1));
      biffAdd(PULL, err); return 1;
    }
  }
  if (pctx->ispec[pullInfoTangentMode]) {
    if (!( pctx->ispec[pullInfoTangent1]
           && pctx->ispec[pullInfoTangent2] )) {
      sprintf(err, "%s: want %s but don't have %s and %s set", me, 
              airEnumStr(pullInfo, pullInfoTangentMode),
              airEnumStr(pullInfo, pullInfoTangent1),
              airEnumStr(pullInfo, pullInfoTangent2));
      biffAdd(PULL, err); return 1;
    }
  }
  if (pctx->ispec[pullInfoHeight]) {
    if (!( pctx->ispec[pullInfoHeightGradient] )) {
      sprintf(err, "%s: want %s but don't have %s set", me, 
              airEnumStr(pullInfo, pullInfoHeight),
              airEnumStr(pullInfo, pullInfoHeightGradient));
      biffAdd(PULL, err); return 1;
    }
    if (pctx->ispec[pullInfoHeight]->constraint) {
      if (!pctx->ispec[pullInfoHeightHessian]) {
        sprintf(err, "%s: want constrained %s but don't have %s set", me,
                airEnumStr(pullInfo, pullInfoHeight),
                airEnumStr(pullInfo, pullInfoHeightHessian));
        biffAdd(PULL, err); return 1;
      }
      if (!pctx->ispec[pullInfoTangent1]) {
        sprintf(err, "%s: want constrained %s but need at least %s set", me,
                airEnumStr(pullInfo, pullInfoHeight),
                airEnumStr(pullInfo, pullInfoTangent1));
        biffAdd(PULL, err); return 1;
      }
    }
  }
  if (pctx->ispec[pullInfoHeightLaplacian]) {
    if (!( pctx->ispec[pullInfoHeight] )) {
      sprintf(err, "%s: want %s but don't have %s set", me, 
              airEnumStr(pullInfo, pullInfoHeightLaplacian),
              airEnumStr(pullInfo, pullInfoHeight));
      biffAdd(PULL, err); return 1;
    }
  }
  if (pctx->ispec[pullInfoIsovalue]) {
    if (!( pctx->ispec[pullInfoIsovalueGradient]
           && pctx->ispec[pullInfoIsovalueHessian] )) {
      sprintf(err, "%s: want %s but don't have %s and %s set", me, 
              airEnumStr(pullInfo, pullInfoIsovalue),
              airEnumStr(pullInfo, pullInfoIsovalueGradient),
              airEnumStr(pullInfo, pullInfoIsovalueHessian));
      biffAdd(PULL, err); return 1;
    }
  }
  if (pctx->pointPerVoxel) {
    if (!( pctx->ispec[pullInfoSeedThresh] )) {
      sprintf(err, "%s: sorry, need %s info set to use pointPerVoxel",
              me, airEnumStr(pullInfo, pullInfoSeedThresh));
      biffAdd(PULL, err); return 1;
    }
  }
  
  if (!( AIR_IN_CL(1, pctx->threadNum, PULL_THREAD_MAXNUM) )) {
    sprintf(err, "%s: pctx->threadNum (%d) outside valid range [1,%d]", me,
            pctx->threadNum, PULL_THREAD_MAXNUM);
    biffAdd(PULL, err); return 1;
  }

#define CHECK(thing, min, max)                                   \
  if (!( AIR_EXISTS(pctx->thing)                                 \
         && min <= pctx->thing && pctx->thing <= max )) {        \
    sprintf(err, "%s: pctx->" #thing " %g not in range [%g,%g]", \
            me, pctx->thing, min, max);                          \
    biffAdd(PULL, err); return 1;                                \
  }
  /* these reality-check bounds are somewhat arbitrary */
  CHECK(radiusScale, 0.000001, 15.0);
  CHECK(radiusSpace, 0.000001, 15.0);
  CHECK(neighborTrueProb, 0.02, 1.0);
  CHECK(probeProb, 0.02, 1.0);
  CHECK(opporStepScale, 1.0, 10.0);
  CHECK(stepScale, 0.01, 0.99);
  CHECK(energyImprovMin, -0.2, 1.0);
  CHECK(constraintStepMin, 0.00000000000000001, 0.1);
  CHECK(wall, 0.0, 100.0);
  CHECK(alpha, 0.0, 1.0);
  CHECK(beta, 0.0, 1.0);
  CHECK(jitter, 0.0, 1.0);
#undef CHECK
  if (!( 1 <= pctx->constraintIterMax
         && pctx->constraintIterMax <= 50 )) {
    sprintf(err, "%s: pctx->constraintIterMax %u not in range [%u,%u]",
            me, pctx->constraintIterMax, 1, 50);
    biffAdd(PULL, err); return 1;
  }

  if (0 == pctx->jitter && 1 < pctx->pointPerVoxel) {
    sprintf(err, "%s: must have jitter > 0 if pointPerVoxel (%u) > 1", me,
            pctx->pointPerVoxel);
    biffAdd(PULL, err); return 1;
  }
  
  return 0;
}

int
pullOutputGet(Nrrd *nPosOut, Nrrd *nTenOut, pullContext *pctx) {
  char me[]="pullOutputGet", err[BIFF_STRLEN];
  unsigned int binIdx, pointNum, pointIdx, outIdx;
  int E;
  double *posOut, *tenOut;
  pullBin *bin;
  pullPoint *point;

  pointNum = _pullPointNumber(pctx);
  fprintf(stderr, "!%s: pointNum = %u\n", me, pointNum);
  E = AIR_FALSE;
  if (nPosOut) {
    E |= nrrdMaybeAlloc_va(nPosOut, nrrdTypeDouble, 2,
                           AIR_CAST(size_t, 4),
                           AIR_CAST(size_t, pointNum));
  }
  if (nTenOut) {
    E |= nrrdMaybeAlloc_va(nTenOut, nrrdTypeDouble, 2, 
                           AIR_CAST(size_t, 7),
                           AIR_CAST(size_t, pointNum));
  }
  if (E) {
    sprintf(err, "%s: trouble allocating outputs", me);
    biffMove(PULL, err, NRRD); return 1;
  }
  posOut = nPosOut ? (double*)(nPosOut->data) : NULL;
  tenOut = nTenOut ? (double*)(nTenOut->data) : NULL;

  outIdx = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      if (nPosOut) {
        ELL_4V_COPY(posOut + 4*outIdx, point->pos);
      }
      if (nTenOut) {
        double scl, tout[7];
        scl = 1;
        if (pctx->ispec[pullInfoHeightHessian]) {
          double *hess, eval[3], evec[9], eceil, len;
          hess = point->info + pctx->infoIdx[pullInfoHeightHessian];
          ell_3m_eigensolve_d(eval, evec, hess, 10);
          eval[0] = AIR_ABS(eval[0]);
          eval[1] = AIR_ABS(eval[1]);
          eval[2] = AIR_ABS(eval[2]);
          eceil = 7/ELL_3V_LEN(eval);
          eval[0] = AIR_MIN(eceil, 1.0/eval[0]);
          eval[1] = AIR_MIN(eceil, 1.0/eval[1]);
          eval[2] = AIR_MIN(eceil, 1.0/eval[2]);
          ELL_3V_NORM(eval, eval, len);
          tenMakeSingle_d(tout, 1, eval, evec);
        } else if (pctx->constraint
                   && (pctx->ispec[pullInfoHeightGradient]
                       || pctx->ispec[pullInfoIsovalueGradient])) {
          double *grad, norm[3], len, mat[9], out[9];
          grad = point->info + (pctx->ispec[pullInfoHeightGradient]
                                ? pctx->infoIdx[pullInfoHeightGradient]
                                : pctx->infoIdx[pullInfoIsovalueGradient]);
          ELL_3V_NORM(norm, grad, len);
          ELL_3MV_OUTER(out, norm, norm);
          ELL_3M_IDENTITY_SET(mat);
          ELL_3M_SCALE_INCR(mat, -0.95, out);
          TEN_M2T(tout, mat);
          tout[0] = 1;
        } else {
          TEN_T_SET(tout, 1, 1, 0, 0, 1, 0, 1);
        }
        TEN_T_SCALE(tout, scl, tout);
        if (nTenOut) {
          TEN_T_COPY(tenOut + 7*outIdx, tout);
        }
      } /* if (nTenOut) */
      ++outIdx;
    }
  }

  fprintf(stderr, "!%s: final outIdx %u, # %u\n", me, outIdx, pointNum);
  return 0;
}

int
pullPropGet(Nrrd *nprop, int prop, pullContext *pctx) {
  char me[]="pullPropGet", err[BIFF_STRLEN];
  int typeOut;
  size_t size[2];
  unsigned int dim, pointNum, pointIdx, binIdx, *out_ui, outIdx;
  double *out_d;
  unsigned char *out_uc;
  pullBin *bin;
  pullPoint *point;
  
  pointNum = _pullPointNumber(pctx);
  switch(prop) {
  case pullPropEnergy:
  case pullPropStepEnergy:
  case pullPropStepConstr:
    dim = 1;
    size[0] = pointNum;
    typeOut = nrrdTypeDouble;
    break;
  case pullPropIdtag:
    dim = 1;
    size[0] = pointNum;
    typeOut = nrrdTypeUInt;
    break;
  case pullPropStuck:
    dim = 1;
    size[0] = pointNum;
    typeOut = nrrdTypeUChar;
    break;
  case pullPropPosition:
  case pullPropForce:
    dim = 2;
    size[0] = 4;
    size[1] = pointNum;
    typeOut = nrrdTypeDouble;
    break;
  case pullPropNeighDist:
    dim = 1;
    size[0] = pointNum;
    typeOut = nrrdTypeDouble;
    break;
  default:
    sprintf(err, "%s: prop %d unrecognized", me, prop);
    biffAdd(PULL, err); return 1;
    break;
  }
  if (nrrdMaybeAlloc_nva(nprop, typeOut, dim, size)) {
    sprintf(err, "%s: trouble allocating output", me);
    biffMove(PULL, err, NRRD); return 1;
  }
  out_d = AIR_CAST(double *, nprop->data);
  out_ui = AIR_CAST(unsigned int *, nprop->data);
  out_uc = AIR_CAST(unsigned char *, nprop->data);

  outIdx = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      switch(prop) {
      case pullPropEnergy:
        out_d[outIdx] = point->energy;
        break;
      case pullPropStepEnergy:
        out_d[outIdx] = point->stepEnergy;
        break;
      case pullPropStepConstr:
        out_d[outIdx] = point->stepConstr;
        break;
      case pullPropIdtag:
        out_ui[outIdx] = point->idtag;
        break;
      case pullPropStuck:
        out_uc[outIdx] = (point->status & PULL_STATUS_STUCK_BIT);
        break;
      case pullPropPosition:
        ELL_4V_COPY(out_d + 4*outIdx, point->pos);
        break;
      case pullPropForce:
        ELL_4V_COPY(out_d + 4*outIdx, point->force);
        break;
      case pullPropNeighDist:
        out_d[outIdx] = point->neighDist;
        break;
      }
      ++outIdx;
    } /* for (pointIdx) */
  }
  
  return 0;
}

int
pullPositionHistoryGet(limnPolyData *pld, pullContext *pctx) {
  char me[]="pullPositionHistoryGet", err[BIFF_STRLEN];
#if PULL_PHIST
  pullBin *bin;
  pullPoint *point;
  unsigned int binIdx, pointIdx, pointNum, vertNum, vertIdx, 
    primIdx, phistIdx, phistNum;

  if (!(pld && pctx)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(PULL, err); return 1;
  }

  pointNum = 0;
  vertNum = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      vertNum += point->phistArr->len;
      pointNum++;
    }
  }
  if (limnPolyDataAlloc(pld, 1 << limnPolyDataInfoRGBA,
                        vertNum, vertNum, pointNum)) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(PULL, err, LIMN); return 1;
  }
  primIdx = 0;
  vertIdx = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      phistNum = point->phistArr->len;
      for (phistIdx=0; phistIdx<phistNum; phistIdx++) {
        int cond;
        unsigned char rgb[3];
        ELL_3V_SET(rgb, 0, 0, 0);
        ELL_3V_COPY(pld->xyzw + 4*vertIdx, point->phist + 5*phistIdx);
        (pld->xyzw + 4*vertIdx)[3] = 1;
        cond = AIR_CAST(int, (point->phist + 5*phistIdx)[4]);
        switch (cond) {
        case pullCondOld:
          ELL_3V_SET(rgb, 128, 128, 128);
          break;
        case pullCondConstraintSatA:
          ELL_3V_SET(rgb, 0, 255, 0);
          break;
        case pullCondConstraintSatB:
          ELL_3V_SET(rgb, 0, 0, 255);
          break;
        case pullCondEnergyTry:
          ELL_3V_SET(rgb, 255, 255, 255);
          break;
        case pullCondEnergyBad:
          ELL_3V_SET(rgb, 255, 0, 0);
          break;
        case pullCondConstraintFail:
          ELL_3V_SET(rgb, 255, 0, 255);
          break;
        case pullCondNew:
          ELL_3V_SET(rgb, 128, 255, 128);
          break;
        }
        ELL_4V_SET(pld->rgba + 4*vertIdx, rgb[0], rgb[1], rgb[2], 255);
        pld->indx[vertIdx] = vertIdx;
        vertIdx++;
      }
      pld->type[primIdx] = limnPrimitiveLineStrip;
      pld->icnt[primIdx] = phistNum;
      primIdx++;
    }
  }
  

  return 0;
#else 
  AIR_UNUSED(pld);
  AIR_UNUSED(pctx);
  sprintf(err, "%s: sorry, not compiled with PULL_PHIST", me);
  biffAdd(PULL, err);
  return 1;
#endif
}
