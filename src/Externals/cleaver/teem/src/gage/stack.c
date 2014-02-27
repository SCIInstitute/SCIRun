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

/* these functions don't necessarily belong in gage, but we're putting
   them here for the time being.  Being in gage means that vprobe and
   pprobe don't need extra libraries to find them. */

#define BT 2.526917043979558
#define AA 0.629078014852877

double
gageTauOfTee(double tee) {
  double tau;

  tau = (tee < BT ? AA*sqrt(tee) : 0.5365 + log(tee)/2);
  return tau;
}

double
gageTeeOfTau(double tau) {
  double tee;

  /* is it surprising that the transition is at tau = 1 ? */
  tee = (tau < 1 ? tau*tau/(AA*AA) : exp(2*(tau - 0.5365)));
  return tee;
}

#undef BT
#undef AA

double
gageSigOfTau(double tau) {
  
  return sqrt(gageTeeOfTau(tau));
}

double
gageTauOfSig(double sig) {

  return gageTauOfTee(sig*sig);
}

/*
** little helper function to do pre-blurring of a given nrrd 
** of the sort that might be useful for scale-space gage use
**
** nblur has to already be allocated for "blnum" Nrrd*s, AND, they all
** have to point to valid (possibly empty) Nrrds, so they can hold the
** results of blurring. "scale" is filled with the result of
** scaleCB(d_i), for "dom" evenly-spaced samples d_i between
** scldomMin and scldomMax
*/
int
gageStackBlur(Nrrd *const nblur[], const double *scale,
              unsigned int blnum,
              const Nrrd *nin, unsigned int baseDim,
              const NrrdKernelSpec *_kspec,
              int boundary, int renormalize, int verbose) {
  char me[]="gageStackBlur", err[BIFF_STRLEN], val[AIR_STRLEN_LARGE],
    keyscl[]="scale", keykern[]="kernel";
  unsigned int blidx, axi;
  NrrdResampleContext *rsmc;
  NrrdKernelSpec *kspec;
  airArray *mop;
  int E;

  if (!(nblur && scale && nin && _kspec)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (!( blnum >= 2)) {
    sprintf(err, "%s: need blnum > 2, not %u", me, blnum);
    biffAdd(GAGE, err); return 1;
  }
  for (blidx=0; blidx<blnum; blidx++) {
    if (!AIR_EXISTS(scale[blidx])) {
      fprintf(stderr, "%s: scale[%u] = %g doesn't exist", me, blidx, 
              scale[blidx]);
      biffAdd(GAGE, err); return 1;
    }
    if (blidx) {
      if (!( scale[blidx-1] < scale[blidx] )) {
        fprintf(stderr, "%s: scale[%u] = %g not < scale[%u] = %g", me,
                blidx, scale[blidx-1], blidx+1, scale[blidx]);
        biffAdd(GAGE, err); return 1;
      }
    }
  }
  if (3 + baseDim != nin->dim) {
    sprintf(err, "%s: need nin->dim %u (not %u) with baseDim %u", me,
            3 + baseDim, nin->dim, baseDim);
    biffAdd(GAGE, err); return 1;
  }
  if (airEnumValCheck(nrrdBoundary, boundary)) {
    sprintf(err, "%s: %d not a valid %s value", me,
            boundary, nrrdBoundary->name);
    biffAdd(GAGE, err); return 1;
  }
  
  mop = airMopNew();
  kspec = nrrdKernelSpecCopy(_kspec);
  if (!kspec) {
    sprintf(err, "%s: problem copying kernel spec", me);
    biffAdd(GAGE, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, kspec, (airMopper)nrrdKernelSpecNix, airMopAlways);
  /* pre-allocate output Nrrds in case not already there */
  for (blidx=0; blidx<blnum; blidx++) {
    if (!nblur[blidx]) {
      sprintf(err, "%s: got NULL nblur[%u]", me, blidx);
      biffAdd(GAGE, err); airMopError(mop); return 1;
    }
  }
  rsmc = nrrdResampleContextNew();
  airMopAdd(mop, rsmc, (airMopper)nrrdResampleContextNix, airMopAlways);

  E = 0;
  if (!E) E |= nrrdResampleDefaultCenterSet(rsmc, nrrdDefaultCenter);
  if (!E) E |= nrrdResampleNrrdSet(rsmc, nin);
  if (baseDim) {
    unsigned int bai;
    for (bai=0; bai<baseDim; bai++) {
      if (!E) E |= nrrdResampleKernelSet(rsmc, bai, NULL, NULL);
    }
  }
  for (axi=0; axi<3; axi++) {
    if (!E) E |= nrrdResampleSamplesSet(rsmc, baseDim + axi,
                                        nin->axis[baseDim + axi].size);
    if (!E) E |= nrrdResampleRangeFullSet(rsmc, baseDim + axi);
  }
  if (!E) E |= nrrdResampleBoundarySet(rsmc, boundary);
  if (!E) E |= nrrdResampleTypeOutSet(rsmc, nrrdTypeDefault);
  if (!E) E |= nrrdResampleRenormalizeSet(rsmc, renormalize);
  if (E) {
    fprintf(stderr, "%s: trouble setting up resampling\n", me);
    biffAdd(GAGE, err); airMopError(mop); return 1;
  }
  for (blidx=0; blidx<blnum; blidx++) {
    kspec->parm[0] = scale[blidx];
    for (axi=0; axi<3; axi++) {
      if (!E) E |= nrrdResampleKernelSet(rsmc, baseDim + axi,
                                         kspec->kernel, kspec->parm);
    }
    if (verbose) {
      fprintf(stderr, "%s: resampling %u of %u (scale %g) ... ", me, blidx,
              blnum, scale[blidx]);
      fflush(stderr);
    }
    if (!E) E |= nrrdResampleExecute(rsmc, nblur[blidx]);
    if (!E) nrrdKeyValueAdd(nblur[blidx], me, "true");
    sprintf(val, "%g", scale[blidx]);
    if (!E) nrrdKeyValueAdd(nblur[blidx], keyscl, val);
    nrrdKernelSpecSprint(val, kspec);
    if (!E) nrrdKeyValueAdd(nblur[blidx], keykern, val);
    if (E) {
      if (verbose) {
        fprintf(stderr, "problem!\n");
      }
      sprintf(err, "%s: trouble resampling %u of %u (scale %g)",
              me, blidx, blnum, scale[blidx]);
      biffAdd(GAGE, err); airMopError(mop); return 1;
    }
    if (verbose) {
      fprintf(stderr, "done.\n");
    }
  }

  airMopOkay(mop);
  return 0;
}

/*
** this is a little messy: the pvl array is allocated and filled here
** because that's the most idiomatic extension of the way that 
** gagePerVolumeNew() is both the allocator and the initializer. sigh.
*/
int
gageStackPerVolumeNew(gageContext *ctx,
                      gagePerVolume ***pvlP,
                      const Nrrd *const *nblur, unsigned int blnum,
                      const gageKind *kind) {
  char me[]="gageStackPerVolumeNew", err[BIFF_STRLEN];
  gagePerVolume **pvl;
  airArray *mop;
  unsigned int blidx;

  if (!( ctx && pvlP && nblur && kind )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (!blnum) {
    sprintf(err, "%s: need non-zero num", me);
    biffAdd(GAGE, err); return 1;
  }

  mop = airMopNew();
  pvl = *pvlP = AIR_CAST(gagePerVolume **,
                         calloc(blnum, sizeof(gagePerVolume *)));
  if (!pvl) {
    sprintf(err, "%s: couldn't allocate %u pvl pointers", me, blnum);
    biffAdd(GAGE, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, pvlP, (airMopper)airSetNull, airMopOnError);
  airMopAdd(mop, pvl, (airMopper)airFree, airMopOnError);
  for (blidx=0; blidx<blnum; blidx++) {
    if (!( pvl[blidx] = gagePerVolumeNew(ctx, nblur[blidx], kind) )) {
      sprintf(err, "%s: on pvl %u of %u", me, blidx, blnum);
      biffAdd(GAGE, err); airMopError(mop); return 1;
    }
  }

  airMopOkay(mop);
  return 0;
}

/*
** the "base" pvl is the LAST pvl, ctx->pvl[pvlNum-1]
*/
int
gageStackPerVolumeAttach(gageContext *ctx, gagePerVolume *pvlBase,
                         gagePerVolume **pvlStack, const double *stackPos,
                         unsigned int blnum) {
  char me[]="gageStackPerVolumeAttach", err[BIFF_STRLEN];
  unsigned int blidx;

  if (!(ctx && pvlBase && pvlStack && stackPos)) { 
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (!( blnum >= 2 )) {
    /* this constriant is important for the logic of stack reconstruction:
       minimum number of node-centered samples is 2 */
    sprintf(err, "%s: need at least two samples along stack", me);
    biffAdd(GAGE, err); return 1;
  }
  if (ctx->pvlNum) {
    sprintf(err, "%s: can't have pre-existing volumes (%u) "
            "prior to stack attachment", me, ctx->pvlNum);
    biffAdd(GAGE, err); return 1;
  }
  for (blidx=0; blidx<blnum; blidx++) {
    if (!AIR_EXISTS(stackPos[blidx])) {
      fprintf(stderr, "%s: stackPos[%u] = %g doesn't exist", me, blidx, 
              stackPos[blidx]);
      biffAdd(GAGE, err); return 1;
    }
    if (blidx < blnum-1) {
      if (!( stackPos[blidx] < stackPos[blidx+1] )) {
        fprintf(stderr, "%s: stackPos[%u] = %g not < stackPos[%u] = %g", me,
                blidx, stackPos[blidx], blidx+1, stackPos[blidx+1]);
        biffAdd(GAGE, err); return 1;
      }
    }
  }

  /* the base volume is LAST, after all the stack samples */
  for (blidx=0; blidx<blnum; blidx++) {
    if (gagePerVolumeAttach(ctx, pvlStack[blidx])) {
      sprintf(err, "%s: on pvl %u of %u", me, blidx, blnum);
      biffAdd(GAGE, err); return 1;
    }
  }
  if (gagePerVolumeAttach(ctx, pvlBase)) {
    sprintf(err, "%s: on base pvl", me);
    biffAdd(GAGE, err); return 1;
  }
  
  airFree(ctx->stackPos);
  airFree(ctx->stackFslw);
  ctx->stackPos = calloc(blnum, sizeof(double));
  ctx->stackFslw = calloc(blnum, sizeof(double));
  if (!( ctx->stackPos && ctx->stackFslw )) {
    sprintf(err, "%s: couldn't allocate stack buffers (%p %p)", me,
            AIR_CAST(void *, ctx->stackPos),
            AIR_CAST(void *, ctx->stackFslw));
    biffAdd(GAGE, err); return 1;
  }
  for (blidx=0; blidx<blnum; blidx++) {
    ctx->stackPos[blidx] = stackPos[blidx];
  }

  return 0;
}

/*
** _gageStackIv3Fill
**
** after the individual iv3's in the stack have been filled, 
** this does the across-stack filtering to fill pvl[pvlNum-1]'s iv3
*/
int
_gageStackIv3Fill(gageContext *ctx) {
  char me[]="_gageStackIv3Fill";
  unsigned int fd, pvlIdx, cacheIdx, cacheLen, baseIdx, valLen;

  fd = 2*ctx->radius;
  baseIdx = ctx->pvlNum - 1;
  cacheLen = fd*fd*fd*ctx->pvl[0]->kind->valLen;
  if (ctx->verbose > 2) {
    fprintf(stderr, "%s: cacheLen = %u\n", me, cacheLen);
  }
  if (nrrdKernelHermiteFlag == ctx->ksp[gageKernelStack]->kernel) {
    unsigned int xi, yi, zi, blurIdx, valIdx, fdd;
    double xx, *iv30, *iv31, sigma0, sigma1;
    
    fdd = fd*fd;
    /* initialize the output iv3 to all zeros, since we won't be
       usefully setting the values on the boundary, and we can't have
       any non-existant values creeping in.  We shouldn't need to do
       any kind of nrrdBoundaryBleed thing here, because the kernel
       weights really should be zero on the boundary. */
    for (cacheIdx=0; cacheIdx<cacheLen; cacheIdx++) {
      ctx->pvl[baseIdx]->iv3[cacheIdx] = 0;
    }

    /* find the interval in the pre-blurred volumes containing the
       desired scale location */
    for (pvlIdx=0; pvlIdx<ctx->pvlNum-1; pvlIdx++) {
      if (ctx->stackFslw[pvlIdx]) {
        /* has to be non-zero somewhere, since _gageLocationSet()
           gives an error if the integral of stackFslw[] is zero */
        break;
      }
    }
    /* so no way that pvlIdx == pvlNum-1 */
    if (pvlIdx == ctx->pvlNum-2) {
      /* pvlNum-2 is pvl index of last pre-blurred volume */
      /* gageStackPerVolumeAttach() enforces getting at least two 
         pre-blurred volumes --> pvlNum >= 3 --> blurIdx >= 0 */
      blurIdx = pvlIdx-1;
      xx = 1;
    } else {
      blurIdx = pvlIdx;
      /* by design, the hermite non-kernel generates the same values as
         the tent kernel (with scale forced == 1), so we can use those
         to control the interpolation */
      xx = 1 - ctx->stackFslw[pvlIdx];
    }
    iv30 = ctx->pvl[blurIdx]->iv3;
    iv31 = ctx->pvl[blurIdx+1]->iv3;
    sigma0 = ctx->stackPos[blurIdx];
    sigma1 = ctx->stackPos[blurIdx+1];
    valLen = ctx->pvl[baseIdx]->kind->valLen;
    for (valIdx=0; valIdx<valLen; valIdx++) {
      unsigned iii;
      double val0, val1, drv0, drv1, lapl0, lapl1, aa, bb, cc, dd;
      for (zi=1; zi<fd-1; zi++) {
        for (yi=1; yi<fd-1; yi++) {
          for (xi=1; xi<fd-1; xi++) {
            iii = xi + fd*(yi + fd*zi);
            val0 = iv30[iii];
            val1 = iv31[iii];
            lapl0 = (iv30[iii + 1]   + iv30[iii - 1] +
                     iv30[iii + fd]  + iv30[iii - fd] +
                     iv30[iii + fdd] + iv30[iii - fdd] - 6*val0);
            lapl1 = (iv31[iii + 1]   + iv31[iii - 1] +
                     iv31[iii + fd]  + iv31[iii - fd] +
                     iv31[iii + fdd] + iv31[iii - fdd] - 6*val1);
            /* the (sigma1 - sigma0) factor is needed to convert the
               derivative with respect to sigma (sigma*lapl) into the
               derivative with respect to xx */
            drv0 = sigma0*lapl0*(sigma1 - sigma0);
            drv1 = sigma1*lapl1*(sigma1 - sigma0);
            /* Hermite spline coefficients, thanks Mathematica */
            aa = drv0 + drv1 + 2*val0 - 2*val1;
            bb = -2*drv0 - drv1 - 3*val0 + 3*val1;
            cc = drv0;
            dd = val0;
            ctx->pvl[baseIdx]->iv3[iii] = dd + xx*(cc + xx*(bb + aa*xx));
          }
        }
      }
    }
  } else {
    /* we're doing simple convolution-based recon on the stack */
    /* NOTE we are treating the 4D fd*fd*fd*valLen iv3 as a big 1-D array */
    double wght, val;
    for (cacheIdx=0; cacheIdx<cacheLen; cacheIdx++) {
      val = 0;
      for (pvlIdx=0; pvlIdx<ctx->pvlNum-1; pvlIdx++) {
        wght = ctx->stackFslw[pvlIdx];
        val += (wght
                ? wght*ctx->pvl[pvlIdx]->iv3[cacheIdx]
                : 0);
      }
      ctx->pvl[baseIdx]->iv3[cacheIdx] = val;
    }
  }
  return 0;
}

/*
******** gageStackProbe()
*/
int
gageStackProbe(gageContext *ctx,
               double xi, double yi, double zi, double stackIdx) {
  char me[]="gageStackProbe";

  if (!ctx) {
    return 1;
  }
  if (!ctx->parm.stackUse) {
    sprintf(ctx->errStr, "%s: can't probe stack without parm.stackUse", me);
    ctx->errNum = 1;
    return 1;
  }
  return _gageProbe(ctx, xi, yi, zi, stackIdx);
}

int
gageStackProbeSpace(gageContext *ctx,
                    double xx, double yy, double zz, double ss,
                    int indexSpace, int clamp) {
  char me[]="gageStackProbeSpace";

  if (!ctx) {
    return 1;
  }
  if (!ctx->parm.stackUse) {
    sprintf(ctx->errStr, "%s: can't probe stack without parm.stackUse", me);
    ctx->errNum = 1;
    return 1;
  }
  return _gageProbeSpace(ctx, xx, yy, zz, ss, indexSpace, clamp);;
}
