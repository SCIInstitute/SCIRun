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

/*
******** gageContextNew()
**
** doesn't use biff
*/
gageContext *
gageContextNew() {
  gageContext *ctx;
  int i;
  
  ctx = (gageContext*)calloc(1, sizeof(gageContext));
  if (ctx) {
    ctx->verbose = gageDefVerbose;
    gageParmReset(&ctx->parm);
    for(i=gageKernelUnknown+1; i<gageKernelLast; i++) {
      ctx->ksp[i] = NULL;
    }
    ctx->pvl = NULL;
    ctx->pvlNum = 0;
    ctx->pvlArr = airArrayNew(AIR_CAST(void **, &(ctx->pvl)), &(ctx->pvlNum),
                              sizeof(gagePerVolume*),
                              GAGE_PERVOLUME_ARR_INCR);
    gageKernelReset(ctx); /* placed here for logic of kernel flag */
    ctx->shape = gageShapeNew();
    for (i=gageCtxFlagUnknown+1; i<gageCtxFlagLast; i++) {
      ctx->flag[i] = AIR_FALSE;
    }
    ctx->stackPos = NULL;
    ctx->stackFslw = NULL;
    ctx->needD[0] = ctx->needD[1] = ctx->needD[2] = AIR_FALSE;
    for (i=gageKernelUnknown+1; i<gageKernelLast; i++) {
      ctx->needK[i] = AIR_FALSE;
    }
    ctx->radius = 0;
    ctx->fsl = ctx->fw = NULL;
    ctx->off = NULL;
    gagePointReset(&ctx->point);
  }
  return ctx;
}

/*
******** gageContextCopy()
**
** gives you a new context, which behaves the same as the given context,
** with newly allocated pervolumes attached.  With the avoidance of
** padding to create a private copy of the volume, the gageContext is
** light-weight enough that there is no reason that this function can't
** return an independent and fully functioning copy of the context (whereas
** before you weren't allowed to do anything but gageProbe() on the on
** copied context).
*/
gageContext *
gageContextCopy(gageContext *ctx) {
  char me[]="gageContextCopy", err[BIFF_STRLEN];
  gageContext *ntx;
  unsigned int fd, pvlIdx;
  int ki;

  ntx = (gageContext*)calloc(1, sizeof(gageContext));
  if (!ntx) {
    sprintf(err, "%s: couldn't make a gageContext", me);
    biffAdd(GAGE, err); return NULL;
  }
  /* we should probably restrict ourselves to gage API calls, but given the
     constant state of gage construction, this seems much simpler.
     Pointers are fixed below */
  memcpy(ntx, ctx, sizeof(gageContext));
  for (ki=gageKernelUnknown+1; ki<gageKernelLast; ki++) {
    ntx->ksp[ki] = nrrdKernelSpecCopy(ctx->ksp[ki]);
  }
  ntx->pvlArr = airArrayNew(AIR_CAST(void **, &(ntx->pvl)), &(ntx->pvlNum),
                            sizeof(gagePerVolume*),
                            GAGE_PERVOLUME_ARR_INCR);
  airArrayLenSet(ntx->pvlArr, ctx->pvlNum);
  if (!ntx->pvl) {
    sprintf(err, "%s: couldn't allocate new pvl array", me);
    biffAdd(GAGE, err); return NULL;
  }
  for (pvlIdx=0; pvlIdx<ntx->pvlNum; pvlIdx++) {
    ntx->pvl[pvlIdx] = _gagePerVolumeCopy(ctx->pvl[pvlIdx], 2*ctx->radius);
    if (!ntx->pvl[pvlIdx]) {
      sprintf(err, "%s: trouble copying pervolume %u", me, pvlIdx);
      biffAdd(GAGE, err); return NULL;
    }
  }
  if (ctx->stackPos && ctx->stackFslw) {
    ntx->stackPos = calloc(ctx->pvlNum-1, sizeof(double));
    ntx->stackFslw = calloc(ctx->pvlNum-1, sizeof(double));
    if (!( ntx->stackPos && ntx->stackFslw )) {
      sprintf(err, "%s: couldn't allocate stackPos, stackFslw", me);
      biffAdd(GAGE, err); return NULL;
    }
    for (pvlIdx=0; pvlIdx<ntx->pvlNum-1; pvlIdx++) {
      ntx->stackPos[pvlIdx] = ctx->stackPos[pvlIdx];
      ntx->stackFslw[pvlIdx] = ctx->stackFslw[pvlIdx];
    }
  }
  ntx->shape = gageShapeCopy(ctx->shape);
  fd = 2*ntx->radius;
  ntx->fsl = (double *)calloc(fd*3, sizeof(double));
  ntx->fw = (double *)calloc(fd*3*(GAGE_KERNEL_MAX+1), sizeof(double));
  ntx->off = (unsigned int *)calloc(fd*fd*fd, sizeof(unsigned int));
  if (!( ntx->fsl && ntx->fw && ntx->off )) {
    sprintf(err, "%s: couldn't allocate new filter caches for fd=%d",
            me, fd);
    biffAdd(GAGE, err); return NULL;
  }
  /* the content of the offset array needs to be copied because
     it won't be refilled simply by calls to gageProbe() */
  memcpy(ntx->off, ctx->off, fd*fd*fd*sizeof(unsigned int));

  /* make sure gageProbe() has to refill caches */
  gagePointReset(&ntx->point);

  return ntx;
}

/*
******** gageContextNix()
**
** responsible for freeing and clearing up everything hanging off a 
** context so that things can be returned to the way they were prior
** to gageContextNew().
**
** does not use biff
*/
gageContext *
gageContextNix(gageContext *ctx) {
  /* char me[]="gageContextNix"; */
  unsigned int pvlIdx;

  if (ctx) {
    gageKernelReset(ctx);
    for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
      gagePerVolumeNix(ctx->pvl[pvlIdx]);
      /* no point in doing a detach, the whole context is going bye-bye */
    }
    ctx->shape = gageShapeNix(ctx->shape);
    ctx->fw = (double *)airFree(ctx->fw);
    ctx->fsl = (double *)airFree(ctx->fsl);
    ctx->off = (unsigned int *)airFree(ctx->off);
  }
  airFree(ctx);
  return NULL;
}

/*
******** gageKernelSet()
**
** sets one kernel in a gageContext; but the value of this function
** is all the error checking it does.
**
** Refers to ctx->checkIntegrals and acts appropriately.
**
** Does use biff.
*/
int
gageKernelSet(gageContext *ctx, 
              int which, const NrrdKernel *k, const double *kparm) {
  char me[]="gageKernelSet", err[BIFF_STRLEN];
  int numParm;
  double support, integral;

  if (!(ctx && k && kparm)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (airEnumValCheck(gageKernel, which)) {
    sprintf(err, "%s: \"which\" (%d) not in range [%d,%d]", me,
            which, gageKernelUnknown+1, gageKernelLast-1);
    biffAdd(GAGE, err); return 1;
  }
  if (ctx->verbose) {
    fprintf(stderr, "%s: which = %d -> %s\n", me, which,
            airEnumStr(gageKernel, which));
  }
  numParm = k->numParm;
  if (!(AIR_IN_CL(0, numParm, NRRD_KERNEL_PARMS_NUM))) {
    sprintf(err, "%s: kernel's numParm (%d) not in range [%d,%d]",
            me, numParm, 0, NRRD_KERNEL_PARMS_NUM);
    biffAdd(GAGE, err); return 1;
  }
  support = k->support(kparm);
  if (!( support > 0 )) {
    sprintf(err, "%s: kernel's support (%g) not > 0", me, support);
    biffAdd(GAGE, err); return 1;
  }
  if (ctx->parm.checkIntegrals) {
    integral = k->integral(kparm);
    if (gageKernel00 == which ||
        gageKernel10 == which ||
        gageKernel20 == which ||
        gageKernelStack == which) {
      if (!( integral > 0 )) {
        sprintf(err, "%s: reconstruction kernel's integral (%g) not > 0.0",
                me, integral);
        biffAdd(GAGE, err); return 1;
      }
    } else {
      /* its a derivative, so integral must be near zero */
      if (!( AIR_ABS(integral) <= ctx->parm.kernelIntegralNearZero )) {
        sprintf(err, "%s: derivative kernel's integral (%g) not within "
                "%g of 0.0",
                me, integral, ctx->parm.kernelIntegralNearZero);
        biffAdd(GAGE, err); return 1;
      }
    }
  }

  /* okay, enough enough, go set the kernel */
  if (!ctx->ksp[which]) {
    ctx->ksp[which] = nrrdKernelSpecNew();
  }
  nrrdKernelSpecSet(ctx->ksp[which], k, kparm);
  ctx->flag[gageCtxFlagKernel] = AIR_TRUE;

  return 0;
}

/*
******** gageKernelReset()
**
** reset kernels (including the stack ksp) and parameters.
*/
void
gageKernelReset(gageContext *ctx) {
  /* char me[]="gageKernelReset"; */
  int i;

  if (ctx) {
    for(i=gageKernelUnknown+1; i<gageKernelLast; i++) {
      ctx->ksp[i] = nrrdKernelSpecNix(ctx->ksp[i]);
    }
    ctx->flag[gageCtxFlagKernel] = AIR_TRUE;
  }
  return;
}

/*
******** gageParmSet()
**
** for setting the boolean-ish flags in the context in a safe and
** intelligent manner, since changing some of them can have many
** consequences
*/
void
gageParmSet(gageContext *ctx, int which, double val) {
  char me[]="gageParmSet";
  unsigned int pvlIdx;
  
  switch (which) {
  case gageParmVerbose:
    ctx->verbose = AIR_CAST(int, val);
    for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
      ctx->pvl[pvlIdx]->verbose = AIR_CAST(int, val);
    }
    break;
  case gageParmRenormalize:
    ctx->parm.renormalize = val ? AIR_TRUE : AIR_FALSE;
    /* we have to make sure that any existing filter weights
       are not re-used; because gageUpdage() is not called mid-probing,
       we don't use the flag machinery.  Instead we just invalidate
       the last known fractional probe locations */
    gagePointReset(&ctx->point);
    break;
  case gageParmCheckIntegrals:
    ctx->parm.checkIntegrals = val ? AIR_TRUE : AIR_FALSE;
    /* no flags to set, simply affects future calls to gageKernelSet() */
    break;
  case gageParmK3Pack:
    ctx->parm.k3pack = val ? AIR_TRUE : AIR_FALSE;
    ctx->flag[gageCtxFlagK3Pack] = AIR_TRUE;
    break;
  case gageParmGradMagCurvMin:
    ctx->parm.gradMagCurvMin = val;
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmDefaultSpacing:
    ctx->parm.defaultSpacing = val;
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmCurvNormalSide:
    ctx->parm.curvNormalSide = AIR_CAST(int, val);
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmKernelIntegralNearZero:
    ctx->parm.kernelIntegralNearZero = val;
    /* no flag to set, simply affects future calls to gageKernelSet() */
    break;
  case gageParmRequireAllSpacings:
    ctx->parm.requireAllSpacings = AIR_CAST(int, val);
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmRequireEqualCenters:
    ctx->parm.requireEqualCenters = AIR_CAST(int, val);
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmDefaultCenter:
    ctx->parm.defaultCenter = AIR_CAST(int, val);
    /* no flag to set, I guess, although the value here effects the 
       action of _gageShapeSet when called by gagePerVolumeAttach ... */
    break;
  case gageParmStackUse:
    ctx->parm.stackUse = AIR_CAST(int, val);
    /* no flag to set, right? simply affects future calls to gageProbe()? */
    /* HEY: no? because if you're turning on the stack behavior, you now
       should be doing the error checking to make sure that all the pvls
       have the same kind!  This should be caught by gageUpdate(), which is
       supposed to be called after changing anything, prior to gageProbe() */
    break;
  case gageParmStackRenormalize:
    ctx->parm.stackRenormalize = AIR_CAST(int, val);
    break;
  default:
    fprintf(stderr, "\n%s: which = %d not valid!!\n\n", me, which);
    break;
  }
  return;
}

/*
******** gagePerVolumeIsAttached()
**
*/
int
gagePerVolumeIsAttached(const gageContext *ctx, const gagePerVolume *pvl) {
  int ret;
  unsigned int pvlIdx;

  ret = AIR_FALSE;
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    if (pvl == ctx->pvl[pvlIdx]) {
      ret = AIR_TRUE;
    }
  }
  return ret;
}

/*
******** gagePerVolumeAttach()
**
** attaches a pervolume to a context, which actually involves 
** very little work
*/
int
gagePerVolumeAttach(gageContext *ctx, gagePerVolume *pvl) {
  char me[]="gagePerVolumeAttach", err[BIFF_STRLEN];
  gageShape *shape;
  unsigned int newidx;

  if (!( ctx && pvl )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (gagePerVolumeIsAttached(ctx, pvl)) {
    sprintf(err, "%s: given pervolume already attached", me);
    biffAdd(GAGE, err); return 1;
  }

  if (0 == ctx->pvlNum) {
    /* the volume "shape" is context state that we set now, because unlike 
       everything else (handled by gageUpdate()), it does not effect
       the kind or amount of padding done */
    if (_gageShapeSet(ctx, ctx->shape, pvl->nin, pvl->kind->baseDim)) {
      sprintf(err, "%s: trouble", me); 
      biffAdd(GAGE, err); return 1;
    }
    ctx->flag[gageCtxFlagShape] = AIR_TRUE;
  } else {
    /* have to check to that new pvl matches first one.  Since all
       attached pvls were at some point the "new" one, they all
       should match each other */
    shape = gageShapeNew();
    if (_gageShapeSet(ctx, shape, pvl->nin, pvl->kind->baseDim)) {
      sprintf(err, "%s: trouble", me); 
      biffAdd(GAGE, err); return 1;
    }
    if (!gageShapeEqual(ctx->shape, "existing context", shape, "new volume")) {
      sprintf(err, "%s: trouble", me);
      biffAdd(GAGE, err); gageShapeNix(shape); return 1;
    }
    gageShapeNix(shape); 
  }
  /* here we go */
  newidx = airArrayLenIncr(ctx->pvlArr, 1);
  if (!ctx->pvl) {
    sprintf(err, "%s: couldn't increase length of pvl", me);
    biffAdd(GAGE, err); return 1;
  }
  ctx->pvl[newidx] = pvl;
  pvl->verbose = ctx->verbose;

  return 0;
}

/*
******** gagePerVolumeDetach()
**
** detaches a pervolume from a context, but does nothing else
** with the pervolume; caller may want to call gagePerVolumeNix
** if this pervolume will no longer be used
*/
int
gagePerVolumeDetach(gageContext *ctx, gagePerVolume *pvl) {
  char me[]="gagePerVolumeDetach", err[BIFF_STRLEN];
  unsigned int pvlIdx, foundIdx=0;

  if (!( ctx && pvl )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (!gagePerVolumeIsAttached(ctx, pvl)) {
    sprintf(err, "%s: given pervolume not currently attached", me);
    biffAdd(GAGE, err); return 1;
  }
  for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
    if (pvl == ctx->pvl[pvlIdx]) {
      foundIdx = pvlIdx;
    }
  }
  for (pvlIdx=foundIdx+1; pvlIdx<ctx->pvlNum; pvlIdx++) {
    ctx->pvl[pvlIdx-1] = ctx->pvl[pvlIdx];
  }
  ctx->pvl[ctx->pvlNum-1] = NULL;
  airArrayLenIncr(ctx->pvlArr, -1);
  if (0 == ctx->pvlNum) {
    /* leave things the way that they started */
    gageShapeReset(ctx->shape);
    ctx->flag[gageCtxFlagShape] = AIR_TRUE;
  }
  return 0;
}

/*
** gageIv3Fill()
**
** based on ctx's shape and radius, and the (xi,yi,zi) determined from
** the probe location, fills the iv3 cache in the given pervolume
**
** This function is a bottleneck for so many things, so forcing off
** the verbose comments seems like one way of trying to speed it up.
** However, temporarily if-def'ing out unused branches of the
** "switch(pvl->kind->valLen)", and #define'ing fddd to 64 (for 4x4x4
** neighborhoods) did not noticeably speed anything up.
**
*/
void
gageIv3Fill(gageContext *ctx, gagePerVolume *pvl) {
  char me[]="gageIv3Fill";
  int lx, ly, lz, hx, hy, hz, _xx, _yy, _zz;
  unsigned int xx, yy, zz, 
    fr, cacheIdx, dataIdx, fddd;
  unsigned int sx, sy, sz;
  char *data, *here;
  unsigned int tup;

  if (ctx->verbose > 1) {
    fprintf(stderr, "%s: hello\n", me);
  }
  sx = ctx->shape->size[0];
  sy = ctx->shape->size[1];
  sz = ctx->shape->size[2];
  fr = ctx->radius;
  lx = ctx->point.xi - (fr - 1);
  ly = ctx->point.yi - (fr - 1);
  lz = ctx->point.zi - (fr - 1);
  hx = lx + 2*fr - 1;
  hy = ly + 2*fr - 1;
  hz = lz + 2*fr - 1;
  fddd = 2*fr*2*fr*2*fr;
  data = (char*)pvl->nin->data;
  if (lx >= 0 && ly >= 0 && lz >= 0 
      && hx < AIR_CAST(int, sx)
      && hy < AIR_CAST(int, sy)
      && hz < AIR_CAST(int, sz)) {
    /* all the samples we need are inside the existing volume */
    dataIdx = lx + sx*(ly + sy*(lz));
    if (ctx->verbose > 1) {
      fprintf(stderr, "%s: hello, valLen = %d, pvl->nin = %p, data = %p\n",
              me, pvl->kind->valLen,
              AIR_CAST(void*, pvl->nin), pvl->nin->data);
    }
    here = data + dataIdx*pvl->kind->valLen*nrrdTypeSize[pvl->nin->type];
    if (ctx->verbose > 1) {
      fprintf(stderr, "%s: size = (%u,%u,%u);\n"
              "  fd = %d; coord = (%d,%d,%d) --> dataIdx = %d\n",
              me, sx, sy, sz, 2*fr,
              ctx->point.xi, ctx->point.yi, ctx->point.zi,
              dataIdx);
      fprintf(stderr, "%s: here = %p; iv3 = %p; off[0,1,2,3,4,5,6,7] = "
              "%d,%d,%d,%d,%d,%d,%d,%d\n",
              me, here, AIR_CAST(void*, pvl->iv3),
              ctx->off[0], ctx->off[1], ctx->off[2], ctx->off[3],
              ctx->off[4], ctx->off[5], ctx->off[6], ctx->off[7]);
    }
    switch(pvl->kind->valLen) {
    case 1:
      for (cacheIdx=0; cacheIdx<fddd; cacheIdx++) {
        pvl->iv3[cacheIdx] = pvl->lup(here, ctx->off[cacheIdx]);
      }
      break;
      /* NOTE: the tuple axis is being shifted from the fastest to
         the slowest axis, to anticipate component-wise filtering
         operations */
    case 3:
      for (cacheIdx=0; cacheIdx<fddd; cacheIdx++) {
        pvl->iv3[cacheIdx + fddd*0] = pvl->lup(here, 0 + 3*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*1] = pvl->lup(here, 1 + 3*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*2] = pvl->lup(here, 2 + 3*ctx->off[cacheIdx]);
      }
      break;
    case 7:
      /* this might come in handy for tenGage ... */
      for (cacheIdx=0; cacheIdx<fddd; cacheIdx++) {
        pvl->iv3[cacheIdx + fddd*0] = pvl->lup(here, 0 + 7*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*1] = pvl->lup(here, 1 + 7*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*2] = pvl->lup(here, 2 + 7*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*3] = pvl->lup(here, 3 + 7*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*4] = pvl->lup(here, 4 + 7*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*5] = pvl->lup(here, 5 + 7*ctx->off[cacheIdx]);
        pvl->iv3[cacheIdx + fddd*6] = pvl->lup(here, 6 + 7*ctx->off[cacheIdx]);
      }
      break;
    default:
      for (cacheIdx=0; cacheIdx<fddd; cacheIdx++) {
        for (tup=0; tup<pvl->kind->valLen; tup++) {
          pvl->iv3[cacheIdx + fddd*tup] = 
            pvl->lup(here, tup + pvl->kind->valLen*ctx->off[cacheIdx]);
        }
      }
      break;
    }
  } else {
    /* the query requires samples which don't actually lie 
       within the volume- more care has to be taken */
    cacheIdx = 0;
    for (_zz=lz; _zz<=hz; _zz++) {
      zz = AIR_CLAMP(0, _zz, AIR_CAST(int, sz-1));
      for (_yy=ly; _yy<=hy; _yy++) {
        yy = AIR_CLAMP(0, _yy, AIR_CAST(int, sy-1));
        for (_xx=lx; _xx<=hx; _xx++) {
          xx = AIR_CLAMP(0, _xx, AIR_CAST(int, sx-1));
          dataIdx = xx + sx*(yy + sy*zz);
          here = data + dataIdx*pvl->kind->valLen*nrrdTypeSize[pvl->nin->type];
          for (tup=0; tup<pvl->kind->valLen; tup++) {
            pvl->iv3[cacheIdx + fddd*tup] = pvl->lup(here, tup);
          }
          cacheIdx++;
        }
      }
    }
  }
  if (ctx->verbose > 1) {
    fprintf(stderr, "%s: bye\n", me);
  }
  return;
}

/*
** _gageProbe
**
** how to do probing.  (x,y,z) position is *index space* position.
** Note, however, that derivatives (gradients and hessians) will
** effectively be computed in *world space*.
**
** doesn't actually do much more than call callbacks in the gageKind
** structs of the attached pervolumes
**
** NOTE: the stack filter weights are (like the spatial filter weights)
** computed inside _gageLocationSet()
*/
int
_gageProbe(gageContext *ctx, double _xi, double _yi, double _zi,
           double stackIdx) {
  char me[]="_gageProbe";
  unsigned int xi, yi, zi;
  unsigned int pvlIdx;
  
  if (!ctx) {
    return 1;
  }
  xi = ctx->point.xi;
  yi = ctx->point.yi;
  zi = ctx->point.zi;
  if (_gageLocationSet(ctx, _xi, _yi, _zi, stackIdx)) {
    /* we're outside the volume; leave gageErrStr and gageErrNum set;
       as they have just been set by _gageLocationSet() */
    return 1;
  }
  
  /* fprintf(stderr, "##%s: bingo 1\n", me); */
  /* if necessary, refill the iv3 cache */
  if (!( xi == ctx->point.xi &&
         yi == ctx->point.yi &&
         zi == ctx->point.zi )) {
    if (ctx->parm.stackUse) {
      for (pvlIdx=0; pvlIdx<ctx->pvlNum-1; pvlIdx++) {
        /* note that we only fill the cache for the stack samples
           that have a non-zero weight */
        if (ctx->stackFslw[pvlIdx]) {
          gageIv3Fill(ctx, ctx->pvl[pvlIdx]);
        }
      }
    } else {
      for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
        gageIv3Fill(ctx, ctx->pvl[pvlIdx]);
      }
    }
  }
  /* fprintf(stderr, "##%s: bingo 2\n", me); */
  if (ctx->parm.stackUse) {
    unsigned int baseIdx, vi;
    baseIdx = ctx->pvlNum - 1;
    if (ctx->verbose > 2) {
      for (vi=0; vi<baseIdx; vi++) {
        fprintf(stderr, "%s: pvl[%u]'s value cache at "
                "coords = %d,%d,%d:\n", me, vi,
                ctx->point.xi, ctx->point.yi, ctx->point.zi);
        ctx->pvl[vi]->kind->iv3Print(stderr, ctx, ctx->pvl[vi]);
      }
    }
    _gageStackIv3Fill(ctx);
    if (ctx->verbose > 1) {
      fprintf(stderr, "%s: base pvl's value cache at "
              "coords = %d,%d,%d:\n", me, 
              ctx->point.xi, ctx->point.yi, ctx->point.zi);
      ctx->pvl[baseIdx]->kind->iv3Print(stderr, ctx, ctx->pvl[baseIdx]);
    }
    ctx->pvl[baseIdx]->kind->filter(ctx, ctx->pvl[baseIdx]);
    ctx->pvl[baseIdx]->kind->answer(ctx, ctx->pvl[baseIdx]);
  } else {
    for (pvlIdx=0; pvlIdx<ctx->pvlNum; pvlIdx++) {
      if (ctx->verbose > 1) {
        fprintf(stderr, "%s: pvl[%u]'s value cache at "
                "coords = %d,%d,%d:\n", me, pvlIdx,
                ctx->point.xi, ctx->point.yi, ctx->point.zi);
        ctx->pvl[pvlIdx]->kind->iv3Print(stderr, ctx, ctx->pvl[pvlIdx]);
      }
      ctx->pvl[pvlIdx]->kind->filter(ctx, ctx->pvl[pvlIdx]);
      ctx->pvl[pvlIdx]->kind->answer(ctx, ctx->pvl[pvlIdx]);
    }
  }
  
  /* fprintf(stderr, "##%s: bingo 5\n", me); */
  return 0;
}

/*
******** gageProbe()
**
** bummer: the non-stack gageProbe function is now just a wrapper
** around the stack-based gageProbe
*/
int
gageProbe(gageContext *ctx, double xi, double yi, double zi) {

  return _gageProbe(ctx, xi, yi, zi, 0.0);
}

int
_gageProbeSpace(gageContext *ctx, double xx, double yy, double zz, double ss,
               int indexSpace, int clamp) {
  char me[]="_gageProbeSpace";
  int ret;
  unsigned int *size;
  double xi, yi, zi, si;

  size = ctx->shape->size;
  si = 0.0;
  if (indexSpace) {
    xi = xx;
    yi = yy;
    zi = zz;
    if (ctx->parm.stackUse) {
      si = ss;
    } else {
      si = AIR_NAN;
    }
  } else {
    /* have to convert from world to index */
    /* HEY: this has to be tested/debugged */
    double icoord[4]; double wcoord[4];
    ELL_4V_SET(wcoord, xx, yy, zz, 1);
    ELL_4MV_MUL(icoord, ctx->shape->WtoI, wcoord);
    ELL_4V_HOMOG(icoord, icoord);
    xi = icoord[0];
    yi = icoord[1];
    zi = icoord[2];
    if (ctx->parm.stackUse) {
      unsigned int sidx;
      /* HEY: this linear search is just for initial testing, 
         will be made more efficient later */
      if (ss < ctx->stackPos[0]) {
        si = -1;
      } else if (ss > ctx->stackPos[ctx->pvlNum-2]) {
        si = ctx->pvlNum-1; /* which is out of bounds */
      } else {
        for (sidx=0; sidx<ctx->pvlNum-2; sidx++) {
          if (AIR_IN_CL(ctx->stackPos[sidx], ss, ctx->stackPos[sidx+1])) {
            break;
          }
        }
        if (sidx == ctx->pvlNum-2) {
          fprintf(stderr, "!%s: search failure for ss = %g\n", me, ss);
        }
        si = AIR_AFFINE(ctx->stackPos[sidx], ss, ctx->stackPos[sidx+1],
                        0.0, 1.0);
      }
    }
    /*
    fprintf(stderr, "%s: wpos (%g,%g,%g) --> ipos (%g,%g,%g)\n", me,
            xx, yy, zz, xi, yi, zi);
    */
  }
  if (clamp) {
    if (nrrdCenterNode == ctx->shape->center) {
      xi = AIR_CLAMP(0, xi, size[0]-1);
      yi = AIR_CLAMP(0, yi, size[1]-1);
      zi = AIR_CLAMP(0, zi, size[2]-1);
    } else {
      xi = AIR_CLAMP(-0.5, xi, size[0]-0.5);
      yi = AIR_CLAMP(-0.5, yi, size[1]-0.5);
      zi = AIR_CLAMP(-0.5, zi, size[2]-0.5);
    }
    if (ctx->parm.stackUse) {
      si = AIR_CLAMP(0, si, ctx->pvlNum-2);
    }
  }
  ret = _gageProbe(ctx, xi, yi, zi, si);
  return ret;
}

int
gageProbeSpace(gageContext *ctx, double xx, double yy, double zz,
               int indexSpace, int clamp) {
  
  return _gageProbeSpace(ctx, xx, yy, zz, 0.0, indexSpace, clamp);
}
