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

#include "../gage.h"

char *aaliasInfo = ("implements \"Reducing Aliasing Artifacts "
                    "in Iso-Surfaces of Binary Volumes\", "
                    "R. T. Whitaker IEEE VolVis 2000.  With optimization, "
                    "could favorably compare to "
                    "itk::AntiAliasBinaryImageFilter.");

int
main(int argc, char *argv[]) {
  char *me, *outS;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  char *err, done[13];
  Nrrd *nin, *ndist, *nmask, *nupdate, *nout;
  NrrdKernelSpec *k00, *k11, *k22;
  int E;
  unsigned int sx, sy, sz, xi, yi, zi, si, iter, maxIter;
  gageContext *ctx;
  gagePerVolume *pvl;
  double dt, *dist, *mask, *update, thresh, eps, rmsMin;
  const double *valu, *gmag, *mcrv;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "k00", "kernel", airTypeOther, 1, 1, &k00,
             "tent", "k00", NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k11", "kernel", airTypeOther, 1, 1, &k11,
             "cubicd:0,0.5", "k00", NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k22", "kernel", airTypeOther, 1, 1, &k22,
             "cubicdd:1,0", "k00", NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "dt", "step", airTypeDouble, 1, 1, &dt, "0.17",
             "time step size");
  hestOptAdd(&hopt, "th", "val", airTypeDouble, 1, 1, &thresh, "0.0",
             "the value to use for thresholding the input "
             "volume, to create the binary constraint image.");
  hestOptAdd(&hopt, "eps", "val", airTypeDouble, 1, 1, &eps, "0.05",
             "width of value bracket around threshold, to constrain the "
             "the update from letting to value, originally on either "
             "side of the threshold, from getting too close.");
  hestOptAdd(&hopt, "rms", "thresh", airTypeDouble, 1, 1, &rmsMin, "0.01",
             "RMS change below this terminates updates.");
  hestOptAdd(&hopt, "mi", "max", airTypeUInt, 1, 1, &maxIter, "100",
             "maximum # iterations");
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS, "-",
             "fixed volume output");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, aaliasInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  ndist = nrrdNew();
  nmask = nrrdNew();
  nupdate = nrrdNew();
  airMopAdd(mop, ndist, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nmask, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nupdate, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(ndist, nin, nrrdTypeDouble)
      || nrrdConvert(nmask, nin, nrrdTypeDouble)
      || nrrdConvert(nupdate, nin, nrrdTypeDouble)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't allocate buffers:\n%s", me, err);
    airMopError(mop); return 1;
  }
  dist = AIR_CAST(double *, ndist->data);
  mask = AIR_CAST(double *, nmask->data);
  update = AIR_CAST(double *, nupdate->data);

  ctx = gageContextNew();
  airMopAdd(mop, ctx, (airMopper)gageContextNix, airMopAlways);
  gageParmSet(ctx, gageParmRenormalize, AIR_TRUE);
  gageParmSet(ctx, gageParmCheckIntegrals, AIR_TRUE);
  E = 0;
  if (!E) E |= !(pvl = gagePerVolumeNew(ctx, ndist, gageKindScl));
  if (!E) E |= gagePerVolumeAttach(ctx, pvl);
  if (!E) E |= gageKernelSet(ctx, gageKernel00, k00->kernel, k00->parm);
  if (!E) E |= gageKernelSet(ctx, gageKernel11, k11->kernel, k11->parm);
  if (!E) E |= gageKernelSet(ctx, gageKernel22, k22->kernel, k22->parm);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclValue);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclGradMag);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclMeanCurv);
  if (!E) E |= gageUpdate(ctx);
  if (E) {
    airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  valu = gageAnswerPointer(ctx, pvl, gageSclValue);
  gmag = gageAnswerPointer(ctx, pvl, gageSclGradMag);
  mcrv = gageAnswerPointer(ctx, pvl, gageSclMeanCurv);

  sx = nin->axis[0].size;
  sy = nin->axis[1].size;
  sz = nin->axis[2].size;

  for (iter=0; iter<maxIter; iter++) {
    double rms;
    unsigned count;
    fprintf(stderr, "%s: iter %u:      ", me, iter);
    fflush(stderr);
    for (zi=0; zi<sz; zi++) {
      fprintf(stderr, "%s", airDoneStr(0, zi, sz-1, done));
      fflush(stderr);
      for (yi=0; yi<sy; yi++) {
        for (xi=0; xi<sx; xi++) {
          si = xi + sx*(yi + sy*zi);
          gageProbe(ctx, xi, yi, zi);
          update[si] = -dt*(*gmag)*(*mcrv);
        }
      }
    }
    rms = 0;
    count = 0;
    for (si=0; si<sx*sy*sz; si++) {
      double newval;
      if (update[si]) {
        /* NOTE: this update behavior is only slightly different than
           what's described in Equation 18 of the paper. That update
           rule ensures that the value never changes "sign" (with
           respect to the threshold value), by clamping the values
           above or below to 0.0.  But worst-case scenario is that two
           adjacent samples that were on other side of the threshold
           (i.e. 0), could then equal the threshold, which would
           confuse a marching-cubes type algorithm.  So the "eps"
           enforces a small value range around the treshold, and keeps
           the values on either side of it. */
        newval = dist[si] + update[si];
        if (mask[si] > thresh) {
          newval = AIR_MAX(newval, thresh+eps/2);
        } else {
          newval = AIR_MIN(newval, thresh-eps/2);
        }
        rms += (dist[si] - newval)*(dist[si] - newval);
        dist[si] = newval;
        count++;
      }
    }
    fprintf(stderr, "%s", airDoneStr(0, zi, sz-1, done));
    rms /= count;
    rms = sqrt(rms);
    if (rms < rmsMin) {
      fprintf(stderr, "\n%s: RMS %g below threshold %g\n", me, rms, rmsMin);
      break;
    } else {
      fprintf(stderr, " rms = %g > %g\n", rms, rmsMin);
    }
  }
  if (iter == maxIter) {
    fprintf(stderr, "%s: hit max iter %u\n", me, maxIter);
  }

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdCopy(nout, ndist)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't allocate output:\n%s", me, err);
    airMopError(mop); return 1;
  }
  
  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't save output:\n%s", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  exit(0);
}
