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

char *vhInfo = ("deepens valleys. a hack.");

int
main(int argc, char *argv[]) {
  char *me, *outS;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  char *err, done[13];
  Nrrd *nin, *nblur, *nout;
  NrrdKernelSpec *kb0, *kb1, *k00, *k11, *k22;
  NrrdResampleContext *rsmc;
  int E;
  unsigned int sx, sy, sz, xi, yi, zi, ai;
  gageContext *ctx;
  gagePerVolume *pvl;
  const double *gvec, *gmag, *evec0, *eval;
  double (*ins)(void *v, size_t I, double d);
  double (*lup)(const void *v, size_t I);
  double dotmax, dotpow, gmmax, evalshift, gmpow, _dotmax, _gmmax, scl, clamp;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "kb0", "kernel", airTypeOther, 1, 1, &kb0,
             "guass:3,5", "kernel to use for pre-process blurring",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "kb1", "kernel", airTypeOther, 1, 1, &kb1,
             "cubic:1.5,1,0", "kernel to use for pos-process blurring",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k00", "kernel", airTypeOther, 1, 1, &k00,
             "cubic:1,0", "k00", NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k11", "kernel", airTypeOther, 1, 1, &k11,
             "cubicd:1,0", "k00", NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k22", "kernel", airTypeOther, 1, 1, &k22,
             "cubicdd:1,0", "k00", NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "dotmax", "dot", airTypeDouble, 1, 1, &dotmax, "5",
             "max effective value of dot(gvec, evec0)");
  hestOptAdd(&hopt, "evs", "shift", airTypeDouble, 1, 1, &evalshift, "0",
             "negative shift to avoid changing mostly flat regions");
  hestOptAdd(&hopt, "clamp", "clamp", airTypeDouble, 1, 1, &clamp, "nan",
             "if it exists, data value can't be forced below this");
  hestOptAdd(&hopt, "dotpow", "pow", airTypeDouble, 1, 1, &dotpow, "1",
             "exponent for dot");
  hestOptAdd(&hopt, "gmmax", "dot", airTypeDouble, 1, 1, &gmmax, "2",
             "max effective value of gmag");
  hestOptAdd(&hopt, "gmpow", "pow", airTypeDouble, 1, 1, &gmpow, "1",
             "exponent for gmag");
  hestOptAdd(&hopt, "scl", "scale", airTypeDouble, 1, 1, &scl, "0.1",
             "how much to scale hack to decrease input value");
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS, "-",
             "fixed volume output");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, vhInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!( 3 == nin->dim
         && nrrdTypeBlock != nin->type )) {
    fprintf(stderr, "%s: need a 3-D scalar nrrd (not %u-D %s)", me,
            nin->dim, airEnumStr(nrrdType, nin->type));
    airMopError(mop); return 1;
  }

  nblur = nrrdNew();
  airMopAdd(mop, nblur, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdCopy(nblur, nin)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't allocate output:\n%s", me, err);
    airMopError(mop); return 1;
  }

  fprintf(stderr, "%s: pre-blurring ... ", me);
  fflush(stderr);
  rsmc = nrrdResampleContextNew();
  airMopAdd(mop, rsmc, (airMopper)nrrdResampleContextNix, airMopAlways);
  E = AIR_FALSE;
  if (!E) E |= nrrdResampleDefaultCenterSet(rsmc, nrrdCenterCell);
  if (!E) E |= nrrdResampleNrrdSet(rsmc, nin);
  for (ai=0; ai<3; ai++) {
    if (!E) E |= nrrdResampleKernelSet(rsmc, ai, kb0->kernel, kb0->parm);
    if (!E) E |= nrrdResampleSamplesSet(rsmc, ai, nin->axis[ai].size);
    if (!E) E |= nrrdResampleRangeFullSet(rsmc, ai);
  }
  if (!E) E |= nrrdResampleBoundarySet(rsmc, nrrdBoundaryBleed);
  if (!E) E |= nrrdResampleTypeOutSet(rsmc, nrrdTypeDefault);
  if (!E) E |= nrrdResampleRenormalizeSet(rsmc, AIR_TRUE);
  if (!E) E |= nrrdResampleExecute(rsmc, nblur);
  if (E) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error resampling nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }
  fprintf(stderr, "done.\n");

  ctx = gageContextNew();
  airMopAdd(mop, ctx, (airMopper)gageContextNix, airMopAlways);
  gageParmSet(ctx, gageParmRenormalize, AIR_TRUE);
  gageParmSet(ctx, gageParmCheckIntegrals, AIR_TRUE);
  E = 0;
  if (!E) E |= !(pvl = gagePerVolumeNew(ctx, nblur, gageKindScl));
  if (!E) E |= gagePerVolumeAttach(ctx, pvl);
  if (!E) E |= gageKernelSet(ctx, gageKernel00, k00->kernel, k00->parm);
  if (!E) E |= gageKernelSet(ctx, gageKernel11, k11->kernel, k11->parm);
  if (!E) E |= gageKernelSet(ctx, gageKernel22, k22->kernel, k22->parm);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclGradVec);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclGradMag);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclHessEvec0);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclHessEval);
  if (!E) E |= gageUpdate(ctx);
  if (E) {
    airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  gvec = gageAnswerPointer(ctx, pvl, gageSclGradVec);
  gmag = gageAnswerPointer(ctx, pvl, gageSclGradMag);
  evec0 = gageAnswerPointer(ctx, pvl, gageSclHessEvec0);
  eval = gageAnswerPointer(ctx, pvl, gageSclHessEval);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdCopy(nout, nin)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't allocate output:\n%s", me, err);
    airMopError(mop); return 1;
  }

  if (!(nout->type == nin->type && nblur->type == nin->type)) {
    fprintf(stderr, "%s: whoa, types (%s %s %s) not all equal\n", me,
            airEnumStr(nrrdType, nin->type),
            airEnumStr(nrrdType, nblur->type),
            airEnumStr(nrrdType, nout->type));
  }
  ins = nrrdDInsert[nout->type];
  lup = nrrdDLookup[nout->type];
  sx = nin->axis[0].size;
  sy = nin->axis[1].size;
  sz = nin->axis[2].size;

  gageProbe(ctx, 0, 0, 0);
  _dotmax = ELL_3V_DOT(gvec, evec0);
  _gmmax = *gmag;

  fprintf(stderr, "%s: hacking       ", me);
  fflush(stderr);
  for (zi=0; zi<sz; zi++) {
    fprintf(stderr, "%s", airDoneStr(0, zi, sz-1, done));
    fflush(stderr);
    for (yi=0; yi<sy; yi++) {
      for (xi=0; xi<sx; xi++) {
        size_t si;
        double dot, evl, gm, shift, in, out, mode;

        gageProbe(ctx, xi, yi, zi);
        si = xi + sx*(yi + sy*zi);

        dot = ELL_3V_DOT(gvec, evec0);
        _dotmax = AIR_MAX(_dotmax, dot);
        dot = AIR_ABS(dot);
        dot = 1 - AIR_MIN(dot, dotmax)/dotmax;
        dot = pow(dot, dotpow);

        evl = AIR_MAX(0, eval[0] - evalshift);
        mode = airMode3_d(eval);
        evl *= AIR_AFFINE(-1, mode, 1, 0, 1);

        _gmmax = AIR_MAX(_gmmax, *gmag);
        gm = 1 - AIR_MIN(*gmag, gmmax)/gmmax;
        gm = pow(gm, gmpow);

        shift = scl*gm*evl*dot;
        if (AIR_EXISTS(clamp)) {
          in = lup(nin->data, si);
          out = in - shift;
          out = AIR_MAX(out, clamp);
          shift = AIR_MAX(0, in - out);
        }

        ins(nout->data, si, shift);
      }
    }
  }
  fprintf(stderr, "\n");
  fprintf(stderr, "%s: max dot seen: %g\n", me, _dotmax);
  fprintf(stderr, "%s: max gm seen: %g\n", me, _gmmax);

  fprintf(stderr, "%s: post-blurring ... ", me);
  fflush(stderr);
  E = AIR_FALSE;
  if (!E) E |= nrrdResampleNrrdSet(rsmc, nout);
  for (ai=0; ai<3; ai++) {
    if (!E) E |= nrrdResampleKernelSet(rsmc, ai, kb1->kernel, kb1->parm);
    if (!E) E |= nrrdResampleSamplesSet(rsmc, ai, nout->axis[ai].size);
    if (!E) E |= nrrdResampleRangeFullSet(rsmc, ai);
  }
  if (!E) E |= nrrdResampleExecute(rsmc, nblur);
  if (E) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error resampling nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }
  fprintf(stderr, "done.\n");

  for (zi=0; zi<sz; zi++) {
    for (yi=0; yi<sy; yi++) {
      for (xi=0; xi<sx; xi++) {
        size_t si;
        double in, shift;

        si = xi + sx*(yi + sy*zi);
        in = lup(nin->data, si);
        shift = lup(nblur->data, si);
        ins(nout->data, si, in - shift);
      }
    }
  }

  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't save output:\n%s", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  exit(0);
}


