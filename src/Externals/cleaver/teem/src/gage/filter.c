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
** sets the filter sample location (fsl) array based
** on probe location (xf,yf,zf) stored in ctx->point
**
** One possible rare surpise: if a filter is not continuous with 0
** at the end of its support, and if the sample location is at the
** highest possible point (xi == N-2, xf = 1.0), then the filter
** weights may not be the desired ones.  Forward differencing (via
** nrrdKernelForwDiff) is a good example of this.
*/
void
_gageFslSet(gageContext *ctx) {
  int fr, i;
  double *fslx, *fsly, *fslz;
  double xf, yf, zf;

  fr = ctx->radius;
  fslx = ctx->fsl + 0*2*fr;
  fsly = ctx->fsl + 1*2*fr;
  fslz = ctx->fsl + 2*2*fr;
  xf = ctx->point.xf;
  yf = ctx->point.yf;
  zf = ctx->point.zf;
  switch (fr) {
  case 1:
    fslx[0] = xf; fslx[1] = xf-1;
    fsly[0] = yf; fsly[1] = yf-1;
    fslz[0] = zf; fslz[1] = zf-1;
    break;
  case 2:
    fslx[0] = xf+1; fslx[1] = xf; fslx[2] = xf-1; fslx[3] = xf-2;
    fsly[0] = yf+1; fsly[1] = yf; fsly[2] = yf-1; fsly[3] = yf-2;
    fslz[0] = zf+1; fslz[1] = zf; fslz[2] = zf-1; fslz[3] = zf-2;
    break;
  default:
    /* filter radius bigger than 2 */
    for (i=-fr+1; i<=fr; i++) {
      fslx[i+fr-1] = xf-i;
      fsly[i+fr-1] = yf-i;
      fslz[i+fr-1] = zf-i;
    }
    break;
  }
  return;
}

/*
** renormalize weights of a reconstruction kernel with
** constraint: the sum of the weights must equal the continuous
** integral of the kernel
*/
void
_gageFwValueRenormalize(gageContext *ctx, int wch) {
  double integral, sumX, sumY, sumZ, *fwX, *fwY, *fwZ;
  int i, fd;

  fd = 2*ctx->radius;
  fwX = ctx->fw + 0 + fd*(0 + 3*wch);
  fwY = ctx->fw + 0 + fd*(1 + 3*wch);
  fwZ = ctx->fw + 0 + fd*(2 + 3*wch);
  integral = ctx->ksp[wch]->kernel->integral(ctx->ksp[wch]->parm);
  sumX = sumY = sumZ = 0;
  for (i=0; i<fd; i++) {
    sumX += fwX[i];
    sumY += fwY[i];
    sumZ += fwZ[i];
  }
  for (i=0; i<fd; i++) {
    fwX[i] *= integral/sumX;
    fwY[i] *= integral/sumY;
    fwZ[i] *= integral/sumZ;
  }
  return;
}

/*
** renormalize weights of a derivative kernel with
** constraint: the sum of the weights must be zero, but
** sign of individual weights must be preserved
*/
void
_gageFwDerivRenormalize(gageContext *ctx, int wch) {
  char me[]="_gageFwDerivRenormalize";
  double negX, negY, negZ, posX, posY, posZ, fixX, fixY, fixZ,
    *fwX, *fwY, *fwZ;
  int i, fd;

  fd = 2*ctx->radius;
  fwX = ctx->fw + 0 + fd*(0 + 3*wch);
  fwY = ctx->fw + 0 + fd*(1 + 3*wch);
  fwZ = ctx->fw + 0 + fd*(2 + 3*wch);
  negX = negY = negZ = 0;
  posX = posY = posZ = 0;
  for (i=0; i<fd; i++) {
    if (fwX[i] <= 0) { negX += -fwX[i]; } else { posX += fwX[i]; }
    if (fwY[i] <= 0) { negY += -fwY[i]; } else { posY += fwY[i]; }
    if (fwZ[i] <= 0) { negZ += -fwZ[i]; } else { posZ += fwZ[i]; }
  }
  /* fix is the sqrt() of factor by which the positive values
     are too big.  negative values are scaled up by fix;
     positive values are scaled down by fix */
  fixX = sqrt(posX/negX);
  fixY = sqrt(posY/negY);
  fixZ = sqrt(posZ/negZ);
  if (ctx->verbose > 2) {
    fprintf(stderr, "%s: fixX = % 10.4f, fixY = % 10.4f, fixX = % 10.4f\n",
            me, (float)fixX, (float)fixY, (float)fixZ);
  }
  for (i=0; i<fd; i++) {
    if (fwX[i] <= 0) { fwX[i] *= fixX; } else { fwX[i] /= fixX; }
    if (fwY[i] <= 0) { fwY[i] *= fixY; } else { fwY[i] /= fixY; }
    if (fwZ[i] <= 0) { fwZ[i] *= fixZ; } else { fwZ[i] /= fixZ; }
  }
  return;
}

void
_gageFwSet(gageContext *ctx) {
  char me[]="_gageFwSet";
  int kidx, fd;
  
  fd = 2*ctx->radius;
  for (kidx=gageKernelUnknown+1; kidx<gageKernelLast; kidx++) {
    if (!ctx->needK[kidx] || kidx==gageKernelStack) {
      continue;
    }
    /* we evaluate weights for all three axes with one call */
    ctx->ksp[kidx]->kernel->evalN_d(ctx->fw + fd*3*kidx, ctx->fsl,
                                    fd*3, ctx->ksp[kidx]->parm);
  }
  
  if (ctx->verbose > 2) {
    fprintf(stderr, "%s: filter weights after kernel evaluation:\n", me);
    _gagePrint_fslw(stderr, ctx);
  }
  if (ctx->parm.renormalize) {
    for (kidx=gageKernelUnknown+1; kidx<gageKernelLast; kidx++) {
      if (!ctx->needK[kidx] || kidx==gageKernelStack) {
        continue;
      }
      switch (kidx) {
      case gageKernel00:
      case gageKernel10:
      case gageKernel20:
        _gageFwValueRenormalize(ctx, kidx);
        break;
      default:
        _gageFwDerivRenormalize(ctx, kidx);
        break;
      }
    }
    if (ctx->verbose > 2) {
      fprintf(stderr, "%s: filter weights after renormalization:\n", me);
      _gagePrint_fslw(stderr, ctx);
    }
  }
  
  return;
}

/*
** _gageLocationSet
**
** updates probe location in general context, and things which
** depend on it:
** fsl, fw
**
** (_xi,_yi,_zi) is *index* space position in the volume
** _si is the index-space position in the stack, the value is ignored
** if there is no stack behavior 
**
** does NOT use biff, but returns 1 on error and 0 if all okay
** Currently only error is probing outside volume, which sets
** ctx->errNum and sprints message into ctx->errStr.
*/
int
_gageLocationSet(gageContext *ctx, double _xi, double _yi, double _zi,
                 double stackIdx) {
  char me[]="_gageProbeLocationSet";
  unsigned int top[3];  /* "top" x, y, z: highest valid index in volume */
  int xi, yi, zi;       /* computed integral positions in volume */
  double xf, yf, zf, min, max[3];

  top[0] = ctx->shape->size[0] - 1;
  top[1] = ctx->shape->size[1] - 1;
  top[2] = ctx->shape->size[2] - 1;
  if (nrrdCenterNode == ctx->shape->center) {
    min = 0;
    max[0] = top[0];
    max[1] = top[1];
    max[2] = top[2];
  } else {
    min = -0.5;
    max[0] = top[0] + 0.5;
    max[1] = top[1] + 0.5;
    max[2] = top[2] + 0.5;
  }
  if (!( AIR_IN_CL(min, _xi, max[0]) && 
         AIR_IN_CL(min, _yi, max[1]) && 
         AIR_IN_CL(min, _zi, max[2]) )) {
    sprintf(ctx->errStr, "%s: position (%g,%g,%g) outside (%s-centered) "
            "bounds [%g,%g]x[%g,%g]x[%g,%g]",
            me, _xi, _yi, _zi,
            airEnumStr(nrrdCenter, ctx->shape->center),
            min, max[0], min, max[1], min, max[2]);
    ctx->errNum = gageErrBoundsSpace;
    return 1;
  }
  if (ctx->parm.stackUse) {
    if (!( AIR_IN_CL(0, stackIdx, ctx->pvlNum-2) )) {
      sprintf(ctx->errStr, "%s: stack position %g outside (%s-centered) "
              "bounds [0,%u]", me,
              stackIdx, airEnumStr(nrrdCenter, nrrdCenterNode), ctx->pvlNum-2);
      ctx->errNum = gageErrBoundsStack;
      return 1;
    }
  }
  /* even after all these years, GLK is still tweaking this stuff ...
  if (nrrdCenterCell == ctx->shape->center) {
    xi = AIR_CAST(unsigned int, _xi+1) - 1; 
    yi = AIR_CAST(unsigned int, _yi+1) - 1;
    zi = AIR_CAST(unsigned int, _zi+1) - 1;
    xi -= (xi == max[0]);
    yi -= (yi == max[1]);
    zi -= (zi == max[2]);
  } else {
    xi = AIR_CAST(unsigned int, _xi);
    yi = AIR_CAST(unsigned int, _yi);
    zi = AIR_CAST(unsigned int, _zi);
  }
  */
  xi = AIR_CAST(unsigned int, _xi); /* for cell-centered, [-0.5,0] --> 0 */
  yi = AIR_CAST(unsigned int, _yi);
  zi = AIR_CAST(unsigned int, _zi);
  xi -= (xi == max[0]);  /* only can kick in for node-centered */
  yi -= (yi == max[1]);
  zi -= (zi == max[2]);
  xf = _xi - xi;
  yf = _yi - yi;
  zf = _zi - zi;
  ctx->point.xi = xi;
  ctx->point.yi = yi;
  ctx->point.zi = zi;
  if (ctx->verbose > 2) {
    fprintf(stderr, "%s: \n"
            "        pos (% 15.7f,% 15.7f,% 15.7f) \n"
            "        -> i(%5d,%5d,%5d) \n"
            "         + f(% 15.7f,% 15.7f,% 15.7f) \n",
            me, _xi, _yi, _zi, xi, yi, zi, xf, yf, zf);
  }

  /* HEY: the position optimization has to be turned off in the context
     of stacks; Raul found this bug using vprobe */
  if (ctx->parm.stackUse
      || !( ctx->point.xf == xf &&
            ctx->point.yf == yf &&
            ctx->point.zf == zf )) {
    ctx->point.xf = xf;
    ctx->point.yf = yf;
    ctx->point.zf = zf;
    /* these may take some time (especially if using renormalization),
       hence the conditional above */
    _gageFslSet(ctx);
    _gageFwSet(ctx);
  }

  if (ctx->parm.stackUse) {
    double sum;
    unsigned int ii;
    NrrdKernelSpec *sksp;

    /* node-centered sampling of stack indices from 0 to ctx->pvlNum-2 */
    for (ii=0; ii<ctx->pvlNum-1; ii++) {
      ctx->stackFslw[ii] = stackIdx - ii;
      if (ctx->verbose > 2) {
        fprintf(stderr, "%s: ctx->stackFslw[%u] (fsl) = %g\n", 
                me, ii, ctx->stackFslw[ii]);
      }
    }
    sksp = ctx->ksp[gageKernelStack];
    sksp->kernel->evalN_d(ctx->stackFslw, ctx->stackFslw,
                          ctx->pvlNum-1, sksp->parm);
    if (ctx->verbose > 2) {
      for (ii=0; ii<ctx->pvlNum-1; ii++) {
        fprintf(stderr, "%s: ctx->stackFslw[%u] (fw) = %g\n", 
                me, ii, ctx->stackFslw[ii]);
      }
    }
  
    /* HEY: we really are quite far from implementing arbitrary
       nrrdBoundary behaviors here!!!! */
    /* HEY: surprise: gageParmStackRenormalize does NOT control this,
       since that refers to *derivative* normalization */
    /* renormalize weights */
    sum = 0;
    for (ii=0; ii<ctx->pvlNum-1; ii++) {
      sum += ctx->stackFslw[ii];
    }
    if (!sum) {
      sprintf(ctx->errStr, "%s: integral of stackFslw[] is zero, "
              "can't do stack reconstruction", me);
      ctx->errNum = gageErrStackIntegral;
      return 1;
    }
    for (ii=0; ii<ctx->pvlNum-1; ii++) {
      ctx->stackFslw[ii] /= sum;
    }
    if (ctx->verbose > 2) {
      for (ii=0; ii<ctx->pvlNum-1; ii++) {
        fprintf(stderr, "%s: ctx->stackFslw[%u] (fw) = %g\n", 
                me, ii, ctx->stackFslw[ii]);
      }
    }

    /* fix derivative kernel weights for stack. Have to reconstruct
       the world-space stack position from stackFrac and stackBaseIdx */
    if (ctx->parm.stackRenormalize) {
      unsigned int kidx, fd, j, stackBaseIdx;
      double stackFrac, scl, *fwX, *fwY, *fwZ;

      stackBaseIdx = AIR_CAST(unsigned int, stackIdx);
      stackBaseIdx -= (stackBaseIdx == ctx->pvlNum-1);
      stackFrac = stackIdx - stackBaseIdx;
      scl = AIR_AFFINE(0.0, stackFrac, 1.0,
                       ctx->stackPos[stackBaseIdx],
                       ctx->stackPos[stackBaseIdx+1]);
      fd = 2*ctx->radius;
      kidx = gageKernel11;
      fwX = ctx->fw + 0 + fd*(0 + 3*kidx);
      fwY = ctx->fw + 0 + fd*(1 + 3*kidx);
      fwZ = ctx->fw + 0 + fd*(2 + 3*kidx);
      for (j=0; j<fd; j++) {
        fwX[j] *= scl;
        fwY[j] *= scl;
        fwZ[j] *= scl;
      }
      kidx = gageKernel22;
      fwX = ctx->fw + 0 + fd*(0 + 3*kidx);
      fwY = ctx->fw + 0 + fd*(1 + 3*kidx);
      fwZ = ctx->fw + 0 + fd*(2 + 3*kidx);
      for (j=0; j<fd; j++) {
        fwX[j] *= scl*scl;
        fwY[j] *= scl*scl;
        fwZ[j] *= scl*scl;
      }
    }
  }
  
  return 0;
}
