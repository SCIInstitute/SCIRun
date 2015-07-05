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
gageDeconvolve(Nrrd *_nout, double *lastDiffP,
               const Nrrd *nin, const gageKind *kind,
               const NrrdKernelSpec *ksp, int typeOut,
               unsigned int maxIter, int saveAnyway,
               double step, double epsilon, int verbose) {
  char me[]="gageDeconvolve", err[BIFF_STRLEN];
  gageContext *ctx[2];
  gagePerVolume *pvl[2];
  double *out[2], *val[2], alpha, (*lup)(const void *, size_t), meandiff=0;
  const double *ans[2];
  Nrrd *nout[2];
  airArray *mop;
  unsigned int sx, sy, sz, xi, yi, zi, anslen, this, last, inIdx, iter;
  int E, valItem;

  if (!(_nout && lastDiffP && nin && kind && ksp)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (!(nrrdTypeDefault == typeOut
        || !airEnumValCheck(nrrdType, typeOut))) {
    sprintf(err, "%s: typeOut %d not valid", me, typeOut);
    biffAdd(GAGE, err); return 1;
  }
  if (!( maxIter >= 1 )) {
    sprintf(err, "%s: need maxIter >= 1 (not %u)", me, maxIter);
    biffAdd(GAGE, err); return 1;
  }
  if (!( epsilon >= 0 )) {
    sprintf(err, "%s: need epsilon >= 0.0 (not %g)", me, epsilon);
    biffAdd(GAGE, err); return 1;
  }

  /* this once changed from 0 to 1, but is unlikely to change again */
  valItem = 1;

  mop = airMopNew();
  for (iter=0; iter<2; iter++) {
    nout[iter] = nrrdNew();
    airMopAdd(mop, nout[iter], (airMopper)nrrdNuke, airMopAlways);
    if (nrrdConvert(nout[iter], nin, nrrdTypeDouble)) {
      sprintf(err, "%s: couldn't allocate working buffer %u", me, iter);
      biffMove(GAGE, err, NRRD); airMopError(mop); return 1;
    }
    ctx[iter] = gageContextNew();
    airMopAdd(mop, ctx[iter], (airMopper)gageContextNix, airMopAlways);
    E = 0;
    if (!E) E |= !(pvl[iter] = gagePerVolumeNew(ctx[iter], nout[iter], kind));
    if (!E) E |= gagePerVolumeAttach(ctx[iter], pvl[iter]);
    if (!E) E |= gageKernelSet(ctx[iter], gageKernel00,
                               ksp->kernel, ksp->parm);
    if (!E) E |= gageQueryItemOn(ctx[iter], pvl[iter], valItem);
    if (!E) E |= gageUpdate(ctx[iter]);
    if (E) {
      sprintf(err, "%s: trouble setting up context %u", me, iter);
      biffAdd(GAGE, err); airMopError(mop); return 1;
    }
    out[iter] = AIR_CAST(double*, nout[iter]->data);
    ans[iter] = gageAnswerPointer(ctx[iter], pvl[iter], valItem);
  }
  
  anslen = kind->table[valItem].answerLength;
  lup = nrrdDLookup[nin->type];

  alpha = ksp->kernel->eval1_d(0.0, ksp->parm);
  sx = ctx[0]->shape->size[0];
  sy = ctx[0]->shape->size[1];
  sz = ctx[0]->shape->size[2];
  
  for (iter=0; iter<maxIter; iter++) {
    this = (iter+1) % 2;
    last = (iter+0) % 2;
    val[this] = out[this];
    val[last] = out[last];
    inIdx = 0;
    meandiff = 0;
    for (zi=0; zi<sz; zi++) {
      for (yi=0; yi<sy; yi++) {
        for (xi=0; xi<sx; xi++) {
          unsigned int ai;
          double in, aa;
          gageProbe(ctx[last], xi, yi, zi);
          for (ai=0; ai<anslen; ai++) {
            in = lup(nin->data, ai + anslen*inIdx);
            aa = ans[last][ai];
            val[this][ai] = val[last][ai] + step*(in - aa)/alpha;
            meandiff += 2*(in - aa)*(in - aa)/(DBL_EPSILON + in*in + aa*aa);
          }
          val[this] += anslen;
          val[last] += anslen;
          ++inIdx;
        }
      }
    }
    meandiff /= sx*sy*sz;
    if (verbose) {
      fprintf(stderr, "%s: iter %u meandiff = %g\n", me, iter, meandiff);
    }
    if (meandiff < epsilon) {
      /* we have indeed converged while iter < maxIter */
      break;
    }
  }
  if (iter == maxIter) {
    if (!saveAnyway) {
      sprintf(err, "%s: failed to converge in %u iterations, meandiff = %g",
              me, maxIter, meandiff);
      biffAdd(GAGE, err); airMopError(mop); return 1;
    } else {
      if (verbose) {
        fprintf(stderr, "%s: at maxIter %u; err %g still > thresh %g\n", me,
                iter, meandiff, epsilon);
      }
    }
  }

  if (nrrdClampConvert(_nout, nout[this], (nrrdTypeDefault == typeOut
                                           ? nin->type
                                           : typeOut))) {
    sprintf(err, "%s: couldn't create output", me);
    biffAdd(GAGE, err); airMopError(mop); return 1;
  }
  *lastDiffP = meandiff;

  airMopOkay(mop);
  return 0;
}
