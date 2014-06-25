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

#include "push.h"
#include "privatePush.h"

/* 
** _pushTensorFieldSetup sets:
**** pctx->dimIn
**** pctx->nten
**** pctx->ninv
**** pctx->nmask
** and checks mask range
*/
int
_pushTensorFieldSetup(pushContext *pctx) {
  char me[]="_pushTensorFieldSetup", err[BIFF_STRLEN];
  NrrdRange *nrange;
  airArray *mop;
  Nrrd *ntmp;
  int E;
  float *_ten, *_inv;
  double ten[7], inv[7];
  unsigned int numSingle;
  size_t ii, NN;

  mop = airMopNew();
  ntmp = nrrdNew();
  airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
  pctx->nten = nrrdNew();
  pctx->ninv = nrrdNew();
  pctx->nmask = nrrdNew();
  numSingle = 0;
  numSingle += (1 == pctx->nin->axis[1].size);
  numSingle += (1 == pctx->nin->axis[2].size);
  numSingle += (1 == pctx->nin->axis[3].size);
  if (1 == numSingle) {
    pctx->dimIn = 2;
    pctx->sliceAxis = (1 == pctx->nin->axis[1].size
                       ? 0
                       : (1 == pctx->nin->axis[2].size
                          ? 1
                          : 2));
    fprintf(stderr, "!%s: got 2-D input with sliceAxis %u\n",
            me, pctx->sliceAxis);
  } else {
    pctx->dimIn = 3;
    pctx->sliceAxis = 52; /* HEY: what the heck is 52 ? */
    fprintf(stderr, "!%s: got 3-D input\n", me);
  }
  E = 0;
  if (!E) E |= nrrdConvert(pctx->nten, pctx->nin, nrrdTypeFloat);
  if (!E) E |= nrrdCopy(pctx->ninv, pctx->nten);
  if (E) {
    sprintf(err, "%s: trouble creating 3D tensor input", me);
    biffMove(PUSH, err, NRRD); airMopError(mop); return 1;
  }
  _ten = (float*)pctx->nten->data;
  _inv = (float*)pctx->ninv->data;
  NN = nrrdElementNumber(pctx->nten)/7;
  for (ii=0; ii<NN; ii++) {
    double det;
    TEN_T_COPY(ten, _ten);
    TEN_T_INV(inv, ten, det);
    if (!det || !AIR_EXISTS(det)) {
      fprintf(stderr, "!%s: tensor %u/%u has determinant %g\n", me, 
              AIR_CAST(unsigned int, ii), AIR_CAST(unsigned int, NN), det);
    }
    TEN_T_COPY(_inv, inv);
    _ten += 7;
    _inv += 7;
  }

  if (!E) E |= nrrdSlice(pctx->nmask, pctx->nten, 0, 0);
  if (E) {
    sprintf(err, "%s: trouble creating mask", me);
    biffMove(PUSH, err, NRRD); airMopError(mop); return 1;
  }
  nrange = nrrdRangeNewSet(pctx->nmask, nrrdBlind8BitRangeFalse);
  airMopAdd(mop, nrange, (airMopper)nrrdRangeNix, airMopAlways);
  if (AIR_ABS(1.0 - nrange->max) > 0.005) {
    sprintf(err, "%s: tensor mask max %g not close 1.0", me, nrange->max);
    biffAdd(PUSH, err); airMopError(mop); return 1;
  }

  pctx->nten->axis[1].center = nrrdCenterCell;
  pctx->nten->axis[2].center = nrrdCenterCell;
  pctx->nten->axis[3].center = nrrdCenterCell;
  pctx->ninv->axis[1].center = nrrdCenterCell;
  pctx->ninv->axis[2].center = nrrdCenterCell;
  pctx->ninv->axis[3].center = nrrdCenterCell;
  pctx->nmask->axis[0].center = nrrdCenterCell;
  pctx->nmask->axis[1].center = nrrdCenterCell;
  pctx->nmask->axis[2].center = nrrdCenterCell;

  airMopOkay(mop); 
  return 0;
}

/*
** _pushGageSetup sets:
**** pctx->gctx
*/
int
_pushGageSetup(pushContext *pctx) {
  char me[]="_pushGageSetup", err[BIFF_STRLEN];
  gagePerVolume *mpvl;
  int E;

  pctx->gctx = gageContextNew();
  gageParmSet(pctx->gctx, gageParmRequireAllSpacings, AIR_TRUE);
  E = AIR_FALSE;
  /* set up tensor probing */
  if (!E) E |= !(pctx->tpvl = gagePerVolumeNew(pctx->gctx,
                                               pctx->nten, tenGageKind));
  if (!E) E |= gagePerVolumeAttach(pctx->gctx, pctx->tpvl);
  if (!E) E |= gageKernelSet(pctx->gctx, gageKernel00,
                             pctx->ksp00->kernel, pctx->ksp00->parm);
  if (!E) E |= gageQueryItemOn(pctx->gctx, pctx->tpvl, tenGageTensor);
  if (tenGageUnknown != pctx->gravItem) {
    if (!E) E |= gageQueryItemOn(pctx->gctx, pctx->tpvl, pctx->gravItem);
    if (!E) E |= gageQueryItemOn(pctx->gctx, pctx->tpvl, pctx->gravGradItem);
  }
  /* set up tensor inverse probing */
  if (!E) E |= !(pctx->ipvl = gagePerVolumeNew(pctx->gctx,
                                               pctx->ninv, tenGageKind));
  if (!E) E |= gagePerVolumeAttach(pctx->gctx, pctx->ipvl);
  if (!E) E |= gageQueryItemOn(pctx->gctx, pctx->ipvl, tenGageTensor);
  /* set up mask gradient probing */
  if (!E) E |= !(mpvl = gagePerVolumeNew(pctx->gctx,
                                         pctx->nmask, gageKindScl));
  if (!E) E |= gagePerVolumeAttach(pctx->gctx, mpvl);
  if (!E) E |= gageQueryItemOn(pctx->gctx, mpvl, gageSclGradVec);

  if (!E) E |= gageKernelSet(pctx->gctx, gageKernel11,
                             pctx->ksp11->kernel, pctx->ksp11->parm);
  /* (maybe) turn on seed thresholding */
  if (tenGageUnknown != pctx->seedThreshItem) {
    if (!E) E |= gageQueryItemOn(pctx->gctx, pctx->tpvl, pctx->seedThreshItem);
  }
  /* HEY: seed threshold item should possibly be turned off later! */
  if (!E) E |= gageUpdate(pctx->gctx);
  if (E) {
    sprintf(err, "%s: trouble setting up gage", me);
    biffMove(PUSH, err, GAGE); return 1;
  }

  return 0;
}

pushTask *
_pushTaskNew(pushContext *pctx, int threadIdx) {
  char me[]="_pushTaskNew", err[BIFF_STRLEN];
  pushTask *task;

  task = (pushTask *)calloc(1, sizeof(pushTask));
  if (task) {
    task->pctx = pctx;
    if (!(task->gctx = gageContextCopy(pctx->gctx))) {
      sprintf(err, "%s: trouble copying main gageContext", me);
      biffMove(PUSH, err, GAGE); return NULL;
    }
    /* 
    ** HEY: its a limitation in gage that we have to know a priori
    ** the ordering of per-volumes in the context ...
    */
    task->tenAns = gageAnswerPointer(task->gctx, task->gctx->pvl[0],
                                     tenGageTensor);
    task->invAns = gageAnswerPointer(task->gctx, task->gctx->pvl[1],
                                     tenGageTensor);
    task->cntAns = gageAnswerPointer(task->gctx, task->gctx->pvl[2],
                                     gageSclGradVec);
    if (tenGageUnknown != task->pctx->gravItem) {
      task->gravAns = gageAnswerPointer(task->gctx, task->gctx->pvl[0],
                                        task->pctx->gravItem);
      task->gravGradAns = gageAnswerPointer(task->gctx, task->gctx->pvl[0],
                                            task->pctx->gravGradItem);
    } else {
      task->gravAns = NULL;
      task->gravGradAns = NULL;
    }
    if (tenGageUnknown != task->pctx->seedThreshItem) {
      task->seedThreshAns = gageAnswerPointer(task->gctx, task->gctx->pvl[0],
                                              task->pctx->seedThreshItem);
    } else {
      task->seedThreshAns = NULL;
    }
    if (threadIdx) {
      task->thread = airThreadNew();
    }
    task->rng = airRandMTStateNew(pctx->seedRNG + threadIdx);
    task->threadIdx = threadIdx;
    task->pointNum = 0;
    task->energySum = 0;
    task->deltaFracSum = 0;
    task->returnPtr = NULL;

  }
  return task;
}

pushTask *
_pushTaskNix(pushTask *task) {

  if (task) {
    task->gctx = gageContextNix(task->gctx);
    if (task->threadIdx) {
      task->thread = airThreadNix(task->thread);
    }
    task->rng = airRandMTStateNix(task->rng);
    airFree(task);
  }
  return NULL;
}

/*
** _pushTaskSetup sets:
**** pctx->task
**** pctx->task[]
*/
int
_pushTaskSetup(pushContext *pctx) {
  char me[]="_pushTaskSetup", err[BIFF_STRLEN];
  unsigned int tidx;

  pctx->task = (pushTask **)calloc(pctx->threadNum, sizeof(pushTask *));
  if (!(pctx->task)) {
    sprintf(err, "%s: couldn't allocate array of tasks", me);
    biffAdd(PUSH, err); return 1;
  }
  for (tidx=0; tidx<pctx->threadNum; tidx++) {
    if (pctx->verbose) {
      fprintf(stderr, "%s: creating task %u/%u\n", me, tidx, pctx->threadNum);
    }
    pctx->task[tidx] = _pushTaskNew(pctx, tidx);
    if (!(pctx->task[tidx])) {
      sprintf(err, "%s: couldn't allocate task %d", me, tidx);
      biffAdd(PUSH, err); return 1;
    }
  }
  return 0;
}

/*
** _pushBinSetup sets:
**** pctx->maxDist, pctx->minEval, pctx->maxEval, pctx->maxDet
**** pctx->binsEdge[], pctx->binNum
**** pctx->bin
**** pctx->bin[]
*/
int
_pushBinSetup(pushContext *pctx) {
  char me[]="_pushBinSetup", err[BIFF_STRLEN];
  float eval[3], *tdata;
  unsigned int ii, nn, count;
  double col[3][4], volEdge[3];

  /* ------------------------ find maxEval, maxDet, and set up binning */
  nn = nrrdElementNumber(pctx->nten)/7;
  pctx->maxEval = 0;
  pctx->maxDet = 0;
  pctx->meanEval = 0;
  count = 0;
  tdata = (float*)pctx->nten->data;
  for (ii=0; ii<nn; ii++) {
    tenEigensolve_f(eval, NULL, tdata);
    if (tdata[0] > 0.5) {
      /* HEY: this limitation may be a bad idea */
      count++;
      pctx->meanEval += eval[0];
      pctx->maxEval = AIR_MAX(pctx->maxEval, eval[0]);
      if (2 == pctx->dimIn) {
        double det2d;
        /* HEY! HEY! this assumes not only that the measurement frame
           has been taken care of, but that the volume is axis-aligned */
        det2d = (0 == pctx->sliceAxis
                 ? TEN_T_DET_YZ(tdata)
                 : (1 == pctx->sliceAxis
                    ? TEN_T_DET_XZ(tdata)
                    : TEN_T_DET_XY(tdata)));
        pctx->maxDet = AIR_MAX(pctx->maxDet, det2d);
      } else {
        pctx->maxDet = AIR_MAX(pctx->maxDet, eval[0]*eval[1]*eval[2]);
      }
    }
    tdata += 7;
  }
  fprintf(stderr, "!%s: dimIn = %u(%u) --> maxDet = %g\n", me, 
          pctx->dimIn, pctx->sliceAxis, pctx->maxDet);
  pctx->meanEval /= count;
  pctx->maxDist = (2*pctx->scale*pctx->maxEval
                   *pctx->ensp->energy->support(pctx->ensp->parm));

  if (pctx->binSingle) {
    pctx->binsEdge[0] = 1;
    pctx->binsEdge[1] = 1;
    pctx->binsEdge[2] = 1;
    pctx->binNum = 1;
  } else {
    ELL_4MV_COL0_GET(col[0], pctx->gctx->shape->ItoW); col[0][3] = 0.0;
    ELL_4MV_COL1_GET(col[1], pctx->gctx->shape->ItoW); col[1][3] = 0.0;
    ELL_4MV_COL2_GET(col[2], pctx->gctx->shape->ItoW); col[2][3] = 0.0;
    volEdge[0] = ELL_3V_LEN(col[0])*pctx->gctx->shape->size[0];
    volEdge[1] = ELL_3V_LEN(col[1])*pctx->gctx->shape->size[1];
    volEdge[2] = ELL_3V_LEN(col[2])*pctx->gctx->shape->size[2];
    fprintf(stderr, "!%s: volEdge = %g %g %g\n", me,
            volEdge[0], volEdge[1], volEdge[2]);
    pctx->binsEdge[0] = AIR_CAST(unsigned int,
                                 floor(volEdge[0]/pctx->maxDist));
    pctx->binsEdge[0] = pctx->binsEdge[0] ? pctx->binsEdge[0] : 1;
    pctx->binsEdge[1] = AIR_CAST(unsigned int,
                                 floor(volEdge[1]/pctx->maxDist));
    pctx->binsEdge[1] = pctx->binsEdge[1] ? pctx->binsEdge[1] : 1;
    pctx->binsEdge[2] = AIR_CAST(unsigned int,
                                 floor(volEdge[2]/pctx->maxDist));
    pctx->binsEdge[2] = pctx->binsEdge[2] ? pctx->binsEdge[2] : 1;
    if (2 == pctx->dimIn) {
      pctx->binsEdge[pctx->sliceAxis] = 1;
    }
    fprintf(stderr, "!%s: maxEval=%g -> maxDist=%g -> binsEdge=(%u,%u,%u)\n",
            me, pctx->maxEval, pctx->maxDist,
            pctx->binsEdge[0], pctx->binsEdge[1], pctx->binsEdge[2]);
    pctx->binNum = pctx->binsEdge[0]*pctx->binsEdge[1]*pctx->binsEdge[2];
  }
  pctx->bin = (pushBin *)calloc(pctx->binNum, sizeof(pushBin));
  if (!( pctx->bin )) {
    sprintf(err, "%s: trouble allocating bin arrays", me);
    biffAdd(PUSH, err); return 1;
  }
  for (ii=0; ii<pctx->binNum; ii++) {
    pushBinInit(pctx->bin + ii, pctx->binIncr);
  }
  pushBinAllNeighborSet(pctx);

  return 0;
}

/*
** _pushPointSetup sets:
**** pctx->pointNum (in case pctx->npos)
**
** This is only called by the master thread
** 
** this should set stuff to be like after an update stage and
** just before the rebinning
*/
int
_pushPointSetup(pushContext *pctx) {
  char me[]="_pushPointSetup", err[BIFF_STRLEN];
  double (*lup)(const void *v, size_t I), maxDet;
  unsigned int pointIdx;
  pushPoint *point;
  /*  
  double posIdxHack[2][4] = {
    {49.99999, 50, 0, 1},
    {50, 50, 0, 1}};
  */

  pctx->pointNum = (pctx->npos
                    ? pctx->npos->axis[1].size
                    : pctx->pointNum);
  lup = pctx->npos ? nrrdDLookup[pctx->npos->type] : NULL;
  fprintf(stderr, "!%s: initilizing/seeding ... \n", me);
  /* HEY: we end up keeping a local copy of maxDet because convolution
     can produce a tensor with higher determinant than that of any
     original sample.  However, if this is going into effect,
     detReject should probably *not* be enabled... */
  maxDet = pctx->maxDet;
  for (pointIdx=0; pointIdx<pctx->pointNum; pointIdx++) {
    double detProbe;
    /*
    fprintf(stderr, "!%s: pointIdx = %u/%u\n", me, pointIdx, pctx->pointNum);
    */
    point = pushPointNew(pctx);
    if (pctx->npos) {
      ELL_3V_SET(point->pos,
                 lup(pctx->npos->data, 0 + 3*pointIdx),
                 lup(pctx->npos->data, 1 + 3*pointIdx),
                 lup(pctx->npos->data, 2 + 3*pointIdx));
      if (_pushProbe(pctx->task[0], point)) {
        sprintf(err, "%s: probing pointIdx %u of npos", me, pointIdx);
        biffAdd(PUSH, err); return 1;
      }
    } else {
      /*
      double posWorld[4];
      ELL_4MV_MUL(posWorld, pctx->gctx->shape->ItoW, posIdxHack[pointIdx]);
      ELL_34V_HOMOG(point->pos, posWorld);
      _pushProbe(pctx->task[0], point);
      */
      do {
        double posIdx[4], posWorld[4];
        posIdx[0] = AIR_AFFINE(0.0, airDrandMT(), 1.0,
                               -0.5, pctx->gctx->shape->size[0]-0.5);
        posIdx[1] = AIR_AFFINE(0.0, airDrandMT(), 1.0,
                               -0.5, pctx->gctx->shape->size[1]-0.5);
        posIdx[2] = AIR_AFFINE(0.0, airDrandMT(), 1.0,
                               -0.5, pctx->gctx->shape->size[2]-0.5);
        posIdx[3] = 1.0;
        if (2 == pctx->dimIn) {
          posIdx[pctx->sliceAxis] = 0.0;
        }
        ELL_4MV_MUL(posWorld, pctx->gctx->shape->ItoW, posIdx);
        ELL_34V_HOMOG(point->pos, posWorld);
        /*
        fprintf(stderr, "%s: posIdx = %g %g %g --> posWorld = %g %g %g "
                "--> %g %g %g\n", me,
                posIdx[0], posIdx[1], posIdx[2],
                posWorld[0], posWorld[1], posWorld[2],
                point->pos[0], point->pos[1], point->pos[2]);
        */
        if (_pushProbe(pctx->task[0], point)) {
          sprintf(err, "%s: probing pointIdx %u of world", me, pointIdx);
          biffAdd(PUSH, err); return 1;
        }
        detProbe = TEN_T_DET(point->ten);

        if (2 == pctx->dimIn) {
          /* see above HEY! HEY! */
          detProbe = (0 == pctx->sliceAxis
                      ? TEN_T_DET_YZ(point->ten)
                      : (1 == pctx->sliceAxis
                         ? TEN_T_DET_XZ(point->ten)
                         : TEN_T_DET_XY(point->ten)));
        } else {
          detProbe = TEN_T_DET(point->ten);
        }
        maxDet = AIR_MAX(maxDet, detProbe);
        /* assuming that we're not using some very blurring kernel,
           this will eventually succeed, because we previously checked
           the range of values in the mask */
        /* HEY: can't ensure that this will eventually succeed with
           seedThresh enabled! */
        /*
        fprintf(stderr, "!%s: ten[0] = %g\n", me, point->ten[0]);
        */
        /* we OR together all the tests that would
           make us REJECT this last sample */
      } while (point->ten[0] < 0.5
               || (tenGageUnknown != pctx->seedThreshItem
                   && ((pctx->seedThresh - point->seedThresh)
                       *pctx->seedThreshSign > 0)
                   )
               || (pctx->detReject 
                   && (airDrandMT() < detProbe/maxDet))
               );
    }
    if (pushBinPointAdd(pctx, point)) {
      sprintf(err, "%s: trouble binning point %u", me, point->ttaagg);
      biffAdd(PUSH, err); return 1;
    }
  }
  fprintf(stderr, "!%s: ... seeding DONE\n", me);
  return 0;
}

