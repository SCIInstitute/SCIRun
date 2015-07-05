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

#include "gage.h"
#include "privateGage.h"

int
_gagePvlFlagCheck(gageContext *ctx, int pvlFlag) {
  int ret;
  unsigned int pvlIdx;
  
  ret = AIR_FALSE;
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    ret |= ctx->pvl[pvlIdx]->flag[pvlFlag];
  }
  return ret;
}

void
_gagePvlFlagDown(gageContext *ctx, int pvlFlag) {
  unsigned int pvlIdx;
  
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    ctx->pvl[pvlIdx]->flag[pvlFlag] = AIR_FALSE;
  }
}

/* 
** One could go from all the pvls' queries to the context's needD in
** one shot, but doing it in two steps (as below) seems a little clearer,
** and it means that pvl->needD isn't needlessly re-computed for 
** pvl's whose query hasn't changed.
*/

/*
** for each pvl: pvl's query --> pvl's needD
*/
void
_gagePvlNeedDUpdate(gageContext *ctx) {
  char me[]="_gagePvlNeedDUpdate";
  gagePerVolume *pvl;
  int que, needD[3];
  unsigned int pvlIdx;

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    pvl = ctx->pvl[pvlIdx];
    if (pvl->flag[gagePvlFlagQuery]) {
      ELL_3V_SET(needD, 0, 0, 0);
      que = pvl->kind->itemMax+1;
      do {
        que--;
        if (GAGE_QUERY_ITEM_TEST(pvl->query, que)) {
          needD[pvl->kind->table[que].needDeriv] = 1;
        }
      } while (que);
      if (!ELL_3V_EQUAL(needD, pvl->needD)) {
        if (ctx->verbose) {
          fprintf(stderr, "%s: updating pvl[%d]'s needD to (%d,%d,%d)\n",
                  me, pvlIdx, needD[0], needD[1], needD[2]);
        }
        ELL_3V_COPY(pvl->needD, needD);
        pvl->flag[gagePvlFlagNeedD] = AIR_TRUE;
      }
    }
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  return;
}

/*
** all pvls' needD --> ctx's needD
*/
void
_gageNeedDUpdate(gageContext *ctx) {
  char me[]="_gageNeedDUpdate";
  gagePerVolume *pvl;
  int needD[3];
  unsigned int pvlIdx;

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  ELL_3V_SET(needD, 0, 0, 0);
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    pvl = ctx->pvl[pvlIdx];
    needD[0] |= pvl->needD[0];
    needD[1] |= pvl->needD[1];
    needD[2] |= pvl->needD[2];
  }
  if (!ELL_3V_EQUAL(needD, ctx->needD)) {
    if (ctx->verbose) {
      fprintf(stderr, "%s: updating ctx's needD to (%d,%d,%d)\n",
              me, needD[0], needD[1], needD[2]);
    }
    ELL_3V_COPY(ctx->needD, needD);
    ctx->flag[gageCtxFlagNeedD] = AIR_TRUE;
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  return;
}

/*
** ctx's needD & k3pack --> needK
*/
void
_gageNeedKUpdate(gageContext *ctx) {
  char me[]="_gageNeedKUpdate";
  int kernIdx, needK[GAGE_KERNEL_MAX+1], change;
  
  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  for (kernIdx=gageKernelUnknown+1; kernIdx<gageKernelLast; kernIdx++) {
    needK[kernIdx] = AIR_FALSE;
  }
  if (ctx->needD[0]) {
    needK[gageKernel00] = AIR_TRUE;
  }
  if (ctx->needD[1]) {
    needK[gageKernel11] = AIR_TRUE;
    if (ctx->parm.k3pack) {
      needK[gageKernel00] = AIR_TRUE;
    } else {
      needK[gageKernel10] = AIR_TRUE;
    }  
  }
  if (ctx->needD[2]) {
    needK[gageKernel22] = AIR_TRUE;
    if (ctx->parm.k3pack) {
      needK[gageKernel00] = AIR_TRUE;
      needK[gageKernel11] = AIR_TRUE;
    } else {
      needK[gageKernel20] = AIR_TRUE;
      needK[gageKernel21] = AIR_TRUE;
    }  
  }
  change = AIR_FALSE;
  for (kernIdx=gageKernelUnknown+1; kernIdx<gageKernelLast; kernIdx++) {
    change |= (needK[kernIdx] != ctx->needK[kernIdx]);
  }
  if (change) {
    if (ctx->verbose) {
      fprintf(stderr, "%s: changing needK to (%d,%d,%d,%d,%d,%d)\n",
              me, needK[1], needK[2], needK[3], needK[4], needK[5], needK[6]);
    }
    for (kernIdx=gageKernelUnknown+1; kernIdx<gageKernelLast; kernIdx++) {
      ctx->needK[kernIdx] = needK[kernIdx];
    }
    ctx->flag[gageCtxFlagNeedK] = AIR_TRUE;
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  return;
}

/*
** ctx's ksp[] & needK --> radius
**
*/
int
_gageRadiusUpdate(gageContext *ctx) {
  char me[]="_gageRadiusUpdate", err[BIFF_STRLEN];
  unsigned int kernIdx, radius;
  double maxRad, rad;
  NrrdKernelSpec *ksp;

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  maxRad = 0;
  for (kernIdx=gageKernelUnknown+1; kernIdx<gageKernelLast; kernIdx++) {
    if (ctx->needK[kernIdx]) {
      ksp = ctx->ksp[kernIdx];
      if (!ksp) {
        sprintf(err, "%s: need kernel %s but it hasn't been set", 
                me, airEnumStr(gageKernel, kernIdx));
        biffAdd(GAGE, err); return 1;
      }
      rad = ksp->kernel->support(ksp->parm);
      maxRad = AIR_MAX(maxRad, rad);
      if (ctx->verbose) {
        fprintf(stderr, "%s: k[%s]=%s -> rad = %g -> maxRad = %g\n", me,
                airEnumStr(gageKernel, kernIdx), ksp->kernel->name,
                rad, maxRad);
      }
    }
  }
  radius = AIR_ROUNDUP(maxRad);
  /* In case either kernels have tiny supports (less than 0.5), or if
     we in fact don't need any kernels, then we need to do this to 
     ensure that we generate a valid radius */
  radius = AIR_MAX(radius, 1);
  if (ctx->parm.stackUse
      && nrrdKernelHermiteFlag == ctx->ksp[gageKernelStack]->kernel) {
    if (ctx->verbose) {
      fprintf(stderr, "%s: hermite on stack: bumping radius %d --> %d\n",
              me, radius, radius+1);
    }
    radius += 1;
  }
  if (radius != ctx->radius) {
    if (ctx->verbose) {
      fprintf(stderr, "%s: changing radius from %d to %d\n",
              me, ctx->radius, radius);
    }
    ctx->radius = radius;
    ctx->flag[gageCtxFlagRadius] = AIR_TRUE;
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  return 0;
}

int
_gageCacheSizeUpdate(gageContext *ctx) {
  char me[]="_gageCacheSizeUpdate", err[BIFF_STRLEN];
  int fd;
  gagePerVolume *pvl;
  unsigned int pvlIdx;

  if (ctx->verbose) fprintf(stderr, "%s: hello (radius = %d)\n", me,
                            ctx->radius);
  if (!( ctx->radius > 0 )) {
    sprintf(err, "%s: have bad radius %d", me, ctx->radius);
    biffAdd(GAGE, err); return 1;
  }
  fd = 2*ctx->radius;
  ctx->fsl = (double *)airFree(ctx->fsl);
  ctx->fw = (double *)airFree(ctx->fw);
  ctx->off = (unsigned int *)airFree(ctx->off);
  ctx->fsl = (double *)calloc(fd*3, sizeof(double));
  ctx->fw = (double *)calloc(fd*3*(GAGE_KERNEL_MAX+1), sizeof(double));
  ctx->off = (unsigned int *)calloc(fd*fd*fd, sizeof(unsigned int));
  if (!(ctx->fsl && ctx->fw && ctx->off)) {
    sprintf(err, "%s: couldn't allocate filter caches for fd=%d", me, fd);
    biffAdd(GAGE, err); return 1;
  }
  /* note that all pvls get caches allocated for the same size, even if
     their queries don't involve the largest-size kernels. This is actually
     the feature that enabled the stack functionality to be easily added. */
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    pvl = ctx->pvl[pvlIdx];
    pvl->iv3 = (double *)airFree(pvl->iv3);
    pvl->iv2 = (double *)airFree(pvl->iv2);
    pvl->iv1 = (double *)airFree(pvl->iv1);
    pvl->iv3 = (double *)calloc(fd*fd*fd*pvl->kind->valLen, sizeof(double));
    pvl->iv2 = (double *)calloc(fd*fd*pvl->kind->valLen, sizeof(double));
    pvl->iv1 = (double *)calloc(fd*pvl->kind->valLen, sizeof(double));
    if (!(pvl->iv3 && pvl->iv2 && pvl->iv1)) {
      sprintf(err, "%s: couldn't allocate pvl[%d]'s value caches for fd=%d",
              me, pvlIdx, fd);
      biffAdd(GAGE, err); return 1;
    }
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);
  
  return 0;
}

void
_gageOffValueUpdate(gageContext *ctx) {
  char me[]="_gageOffValueUpdate";
  int fd, i, j, k;
  unsigned int sx, sy;

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);

  sx = ctx->shape->size[0];
  sy = ctx->shape->size[1];
  fd = 2*ctx->radius;
  /* HEY: look into special casing this for small fd */
  for (k=0; k<fd; k++) {
    for (j=0; j<fd; j++) {
      for (i=0; i<fd; i++) {
        ctx->off[i + fd*(j + fd*k)] = i + sx*(j + sy*k);
      }
    }
  }
  /* no flags to set for further action */
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);
  
  return;
}

/*
******** gageUpdate()
**
** call just before probing begins.
*/
int
gageUpdate(gageContext *ctx) {
  char me[]="gageUpdate", err[BIFF_STRLEN];
  unsigned int pi;
  int i, haveQuery;

  if (!( ctx )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (0 == ctx->pvlNum) {
    sprintf(err, "%s: context has no attached pervolumes", me);
    biffAdd(GAGE, err); return 1;
  }
  haveQuery = AIR_FALSE;
  for (pi=0; pi<ctx->pvlNum; pi++) {
    haveQuery |= GAGE_QUERY_NONZERO(ctx->pvl[pi]->query);
  }
  if (!haveQuery) {
    sprintf(err, "%s: all pervolumes have no query items set", me);
    biffAdd(GAGE, err); return 1;
  }

  /* HEY: shouldn't there be some more logic/state for this? */
  if (ctx->parm.stackUse) {
    if (!ctx->ksp[gageKernelStack]) {
      sprintf(err, "%s: can't do stack without ksp[%s]", me,
              airEnumStr(gageKernel, gageKernelStack));
      biffAdd(GAGE, err); return 1;
    }
    if (!( 2 <= ctx->pvlNum )) {
      sprintf(err, "%s: need at least 2 pervolumes for stack", me);
      biffAdd(GAGE, err); return 1;
    }
    for (pi=1; pi<ctx->pvlNum; pi++) {
      if (!( ctx->pvl[0]->kind == ctx->pvl[pi]->kind )) {
        sprintf(err, "%s: pvl[%u] kind (%s) != pvl[0] kind (%s)", me,
                pi, ctx->pvl[pi]->kind->name, ctx->pvl[0]->kind->name);
        biffAdd(GAGE, err); return 1;
      }
    }
  }

  /* start traversing the whole update graph ... */
  if (ctx->verbose) {
    fprintf(stderr, "%s: hello ____________________ \n", me);
    fprintf(stderr, "    context flags:");
    for (i=gageCtxFlagUnknown+1; i<gageCtxFlagLast; i++) {
      fprintf(stderr, " %d=%d", i, ctx->flag[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "    pvl flags:");
    for (i=gagePvlFlagUnknown+1; i<gagePvlFlagLast; i++) {
      fprintf(stderr, " %d=%d", i, _gagePvlFlagCheck(ctx, i));
    }
    fprintf(stderr, "\n");
  }
  if (_gagePvlFlagCheck(ctx, gagePvlFlagQuery)) {
    _gagePvlNeedDUpdate(ctx);
    _gagePvlFlagDown(ctx, gagePvlFlagQuery);
  }
  if (_gagePvlFlagCheck(ctx, gagePvlFlagNeedD)) {
    _gageNeedDUpdate(ctx);
    _gagePvlFlagDown(ctx, gagePvlFlagNeedD);
  }
  if (ctx->flag[gageCtxFlagNeedD] || ctx->flag[gageCtxFlagK3Pack]) {
    _gageNeedKUpdate(ctx);
    ctx->flag[gageCtxFlagNeedD] = AIR_FALSE;
    ctx->flag[gageCtxFlagK3Pack] = AIR_FALSE;
  }
  if (ctx->flag[gageCtxFlagKernel] || ctx->flag[gageCtxFlagNeedK]) {
    if (_gageRadiusUpdate(ctx)) {
      sprintf(err, "%s: trouble", me); biffAdd(GAGE, err); return 1;
    }
    ctx->flag[gageCtxFlagKernel] = AIR_FALSE;
    ctx->flag[gageCtxFlagNeedK] = AIR_FALSE;
  }
  if (ctx->flag[gageCtxFlagRadius]
      /* HEY HEY HEY: this is a total hack: right now its possible for a 
         new pvl to have unallocated iv3,iv2,iv1, if it was attached to a
         context which had already been probing, as was the case with
         _tenRegisterDoit.  So, with this hack we reallocate ALL caches
         just because a new pervolume was attached ... */
      || _gagePvlFlagCheck(ctx, gagePvlFlagVolume)) {
    if (_gageCacheSizeUpdate(ctx)) {
      sprintf(err, "%s: trouble", me); biffAdd(GAGE, err); return 1;
    }
  }
  if (ctx->flag[gageCtxFlagRadius]
      || ctx->flag[gageCtxFlagShape]
      /* see above; following flags that triggered _gageCacheSizeUpdate(ctx) */
      || _gagePvlFlagCheck(ctx, gagePvlFlagVolume)) {
    _gageOffValueUpdate(ctx);
    ctx->flag[gageCtxFlagShape] = AIR_FALSE;
  }
  ctx->flag[gageCtxFlagRadius] = AIR_FALSE;
    
  /* chances are, something above has invalidated the state maintained
     during successive calls to gageProbe() */
  gagePointReset(&ctx->point);

  for (pi=0; pi<ctx->pvlNum; pi++) {
    if (ctx->pvl[pi]->kind->pvlDataUpdate) {
      if (ctx->pvl[pi]->kind->pvlDataUpdate(ctx->pvl[pi]->kind,
                                            ctx,
                                            ctx->pvl[pi],
                                            ctx->pvl[pi]->data)) {
        sprintf(err, "%s: pvlDataUpdate(pvl[%u]) failed", me, pi);
        biffAdd(GAGE, err); return 1;
      }
    }
  }

  if (ctx->verbose) fprintf(stderr, "%s: bye ^^^^^^^^^^^^^^^^^^^ \n", me);

  return 0;
}
