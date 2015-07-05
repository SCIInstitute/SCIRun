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

pullTask *
_pullTaskNew(pullContext *pctx, int threadIdx) {
  char me[]="_pullTaskNew", err[BIFF_STRLEN];
  pullTask *task;
  unsigned int ii, offset;

  task = (pullTask *)calloc(1, sizeof(pullTask));
  if (!task) {
    sprintf(err, "%s: couldn't allocate task", me);
    biffAdd(PULL, err); return NULL;
  }    

  task->pctx = pctx;
  for (ii=0; ii<pctx->volNum; ii++) {
    if (!(task->vol[ii] = _pullVolumeCopy(pctx->vol[ii]))) {
      sprintf(err, "%s: trouble copying vol %u/%u", me, ii, pctx->volNum);
      biffAdd(PULL, err); return NULL;
    }
  }
  if (0) {
    gagePerVolume *pvl;
    const double *ans;
    double pos[3];
    int gret;
    for (ii=0; ii<pctx->volNum; ii++) {
      pvl = task->vol[ii]->gctx->pvl[0];
      fprintf(stderr, "!%s: vol[%u] query:\n", me, ii);
      gageQueryPrint(stderr, pvl->kind, pvl->query);
      ans = gageAnswerPointer(task->vol[ii]->gctx, pvl, gageSclValue);
      ELL_3V_SET(pos, 0.6, 0.6, 0.3);
      gret = gageProbeSpace(task->vol[ii]->gctx, pos[0], pos[1], pos[2],
                            AIR_FALSE, AIR_TRUE);
      fprintf(stderr, "!%s: (%d) val(%g,%g,%g) = %g\n", me, gret,
              pos[0], pos[1], pos[2], *ans);
      ELL_3V_SET(pos, 0.5, 0.0, 0.0);
      gret = gageProbeSpace(task->vol[ii]->gctx, pos[0], pos[1], pos[2],
                            AIR_FALSE, AIR_TRUE);
      fprintf(stderr, "!%s: (%d) val(%g,%g,%g) = %g\n", me, gret,
              pos[0], pos[1], pos[2], *ans);
    }
  }
  offset = 0;
  for (ii=0; ii<=PULL_INFO_MAX; ii++) {
    unsigned int volIdx;
    if (pctx->ispec[ii]) {
      volIdx = pctx->ispec[ii]->volIdx;
      task->ans[ii] = gageAnswerPointer(task->vol[volIdx]->gctx,
                                        task->vol[volIdx]->gpvl,
                                        pctx->ispec[ii]->item);
      fprintf(stderr, "!%s: task->ans[%u] = %p\n", me, ii, task->ans[ii]);
    } else {
      task->ans[ii] = NULL;
    }
  }
  if (pctx->threadNum > 1) {
    task->thread = airThreadNew();
  }
  task->threadIdx = threadIdx;
  task->rng = airRandMTStateNew(pctx->rngSeed + threadIdx);
  task->pointBuffer = pullPointNew(pctx);
  pctx->idtagNext = 0; /* because pullPointNew incremented it */
  task->neighPoint = AIR_CAST(pullPoint **, calloc(_PULL_NEIGH_MAXNUM,
                                                   sizeof(pullPoint*)));
  task->addPoint = NULL;
  /*
  task->addPointArr = airArrayNew(AIR_CAST(void **, &(task->addPoint)),
                                  NULL, sizeof(pullPoint*),
                                  unsigned int incr);
  */
  task->returnPtr = NULL;
  task->stuckNum = 0;
  return task;
}

pullTask *
_pullTaskNix(pullTask *task) {
  unsigned int ii;

  if (task) {
    for (ii=0; ii<task->pctx->volNum; ii++) {
      task->vol[ii] = pullVolumeNix(task->vol[ii]);
    }
    if (task->pctx->threadNum > 1) {
      task->thread = airThreadNix(task->thread);
    }
    task->rng = airRandMTStateNix(task->rng);
    task->pointBuffer = pullPointNix(task->pointBuffer);
    task->neighPoint = airFree(task->neighPoint);
    airFree(task);
  }
  return NULL;
}

/*
** _pullTaskSetup sets:
**** pctx->task
**** pctx->task[]
*/
int
_pullTaskSetup(pullContext *pctx) {
  char me[]="_pullTaskSetup", err[BIFF_STRLEN];
  unsigned int tidx;

  pctx->task = (pullTask **)calloc(pctx->threadNum, sizeof(pullTask *));
  if (!(pctx->task)) {
    sprintf(err, "%s: couldn't allocate array of tasks", me);
    biffAdd(PULL, err); return 1;
  }
  for (tidx=0; tidx<pctx->threadNum; tidx++) {
    if (pctx->verbose) {
      fprintf(stderr, "%s: creating task %u/%u\n", me, tidx, pctx->threadNum);
    }
    pctx->task[tidx] = _pullTaskNew(pctx, tidx);
    if (!(pctx->task[tidx])) {
      sprintf(err, "%s: couldn't allocate task %d", me, tidx);
      biffAdd(PULL, err); return 1;
    }
  }
  return 0;
}

void
_pullTaskFinish(pullContext *pctx) {
  unsigned int tidx;
  
  for (tidx=0; tidx<pctx->threadNum; tidx++) {
    pctx->task[tidx] = _pullTaskNix(pctx->task[tidx]);
  }
  pctx->task = airFree(pctx->task);
  return;
}
