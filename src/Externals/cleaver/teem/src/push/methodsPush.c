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
** the reason to take the pushContext *pctx argument is to allow
** doling out the ttaagg ID
*/
pushPoint *
pushPointNew(pushContext *pctx) {
  pushPoint *pnt;

  if (pctx) {
    pnt = AIR_CAST(pushPoint *, calloc(1, sizeof(pushPoint)));
    if (pnt) {
      pnt->ttaagg = pctx->ttaagg++;
      ELL_3V_SET(pnt->pos, AIR_NAN, AIR_NAN, AIR_NAN);
      ELL_3V_SET(pnt->frc, AIR_NAN, AIR_NAN, AIR_NAN);
      TEN_T_SET(pnt->ten, AIR_NAN, AIR_NAN, AIR_NAN,
                AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN);
      TEN_T_SET(pnt->inv, AIR_NAN, AIR_NAN, AIR_NAN,
                AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN);
      ELL_3V_SET(pnt->cnt, AIR_NAN, AIR_NAN, AIR_NAN);
      pnt->grav = AIR_NAN;
      ELL_3V_SET(pnt->gravGrad, AIR_NAN, AIR_NAN, AIR_NAN);
      pnt->seedThresh = AIR_NAN;
      pnt->enr = DBL_MAX;  /* any finite quantity will be less than this */

      pnt->neighArr = airArrayNew((void**)&(pnt->neigh), &(pnt->neighNum),
                                  sizeof(pushPoint *), 10);
    }
  } else {
    pnt = NULL;
  }
  return pnt;
}

pushPoint *
pushPointNix(pushPoint *pnt) {

  airFree(pnt);
  return NULL;
}


pushContext *
pushContextNew(void) {
  pushContext *pctx;

  pctx = (pushContext *)calloc(1, sizeof(pushContext));
  if (pctx) {
    pctx->pointNum = 0;

    pctx->nin = NULL;
    pctx->npos = NULL;

    pctx->stepInitial = 1;
    pctx->scale = 0.2;
    pctx->wall = 0.1;
    pctx->cntScl = 0.0;
    pctx->deltaLimit = 0.3;
    pctx->deltaFracMin = 0.2;
    pctx->energyStepFrac = 0.9;
    pctx->deltaFracStepFrac = 0.5;
    pctx->neighborTrueProb = 0.3;
    pctx->probeProb = 0.5;
    pctx->energyImprovMin = 0.01;

    pctx->detReject = AIR_FALSE;
    pctx->midPntSmp = AIR_FALSE;
    pctx->verbose = 0;

    pctx->seedRNG = 42;
    pctx->threadNum = 1;
    pctx->maxIter = 0;
    pctx->snap = 0;

    pctx->gravItem = tenGageUnknown;
    pctx->gravGradItem = tenGageUnknown;
    pctx->gravScl = AIR_NAN;
    pctx->gravZero = AIR_NAN;

    pctx->seedThreshItem  = tenGageUnknown;
    pctx->seedThreshSign = +1;
    pctx->seedThresh = 0.0;

    pctx->ensp = pushEnergySpecNew();

    pctx->binSingle = AIR_FALSE;
    pctx->binIncr = 512;

    pctx->ksp00 = nrrdKernelSpecNew();
    pctx->ksp11 = nrrdKernelSpecNew();
    pctx->ksp22 = nrrdKernelSpecNew();

    pctx->ttaagg = 0;
    pctx->nten = NULL;
    pctx->ninv = NULL;
    pctx->nmask = NULL;
    pctx->gctx = NULL;
    pctx->tpvl = NULL;
    pctx->ipvl = NULL;
    pctx->finished = AIR_FALSE;
    pctx->dimIn = 0;
    pctx->sliceAxis = 42;  /* an invalid value */

    pctx->bin = NULL;
    ELL_3V_SET(pctx->binsEdge, 0, 0, 0);
    pctx->binNum = 0;
    pctx->binIdx = 0;
    pctx->binMutex = NULL;

    pctx->step = AIR_NAN;
    pctx->maxDist = AIR_NAN;
    pctx->maxEval = AIR_NAN;
    pctx->meanEval = AIR_NAN;
    pctx->maxDet = AIR_NAN;
    pctx->energySum = 0;

    pctx->task = NULL;

    pctx->iterBarrierA = NULL;
    pctx->iterBarrierB = NULL;

    pctx->deltaFrac = AIR_NAN;

    pctx->timeIteration = 0;
    pctx->timeRun = 0;
    pctx->iter = 0;
    pctx->noutPos = nrrdNew();
    pctx->noutTen = nrrdNew();
  }
  return pctx;
}

/*
** this should only nix things created by pushContextNew
*/
pushContext *
pushContextNix(pushContext *pctx) {
  
  if (pctx) {
    pctx->ensp = pushEnergySpecNix(pctx->ensp);
    pctx->ksp00 = nrrdKernelSpecNix(pctx->ksp00);
    pctx->ksp11 = nrrdKernelSpecNix(pctx->ksp11);
    pctx->ksp22 = nrrdKernelSpecNix(pctx->ksp22);
    pctx->noutPos = nrrdNuke(pctx->noutPos);
    pctx->noutTen = nrrdNuke(pctx->noutTen);
    airFree(pctx);
  }
  return NULL;
}
