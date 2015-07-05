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

#include "../seek.h"

char *info = ("test stupid cubes isosurfaces.");

int
main(int argc, char *argv[]) {
  char *me, *err, *outS;
  hestOpt *hopt=NULL;
  airArray *mop;
  limnPolyData *pld;
  gageContext *gctx=NULL;
  gagePerVolume *pvl;
  Nrrd *nin, *nmeas;
  double isoval, kparm[3];
  seekContext *sctx;
  FILE *file;
  int usegage, E, hack;
  size_t samples[3];
  
  me = argv[0];
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume to surface",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "v", "iso", airTypeDouble, 1, 1, &isoval, NULL,
             "isovalue");
  hestOptAdd(&hopt, "g", NULL, airTypeInt, 0, 0, &usegage, NULL,
             "use gage too");
  hestOptAdd(&hopt, "hack", NULL, airTypeInt, 0, 0, &hack, NULL, "hack");
  hestOptAdd(&hopt, "o", "output LMPD", airTypeString, 1, 1, &outS, "out.lmpd",
             "output file to save LMPD into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  pld = limnPolyDataNew();
  airMopAdd(mop, pld, (airMopper)limnPolyDataNix, airMopAlways);

  file = airFopen(outS, stdout, "w");
  airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);

  sctx = seekContextNew();
  airMopAdd(mop, sctx, (airMopper)seekContextNix, airMopAlways);

  if (usegage) {
    gctx = gageContextNew();
    airMopAdd(mop, gctx, (airMopper)gageContextNix, airMopAlways);
    ELL_3V_SET(kparm, 2, 1.0, 0.0);
    if (!(pvl = gagePerVolumeNew(gctx, nin, gageKindScl))
        || gagePerVolumeAttach(gctx, pvl)
        || gageKernelSet(gctx, gageKernel00, nrrdKernelBCCubic, kparm)
        || gageKernelSet(gctx, gageKernel11, nrrdKernelBCCubicD, kparm)
        || gageKernelSet(gctx, gageKernel22, nrrdKernelBCCubicDD, kparm)
        || gageQueryItemOn(gctx, pvl, gageSclValue)
        || gageQueryItemOn(gctx, pvl, gageSclNormal)
        || (usegage
            && (gageQueryItemOn(gctx, pvl, gageSclGradVec)
                || gageQueryItemOn(gctx, pvl, gageSclHessEval)
                || gageQueryItemOn(gctx, pvl, gageSclHessEvec)
                || gageQueryItemOn(gctx, pvl, gageSclHessEvec2)))
        || gageUpdate(gctx)) {
      airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  }

  seekVerboseSet(sctx, 10);

  E = 0;
  if (usegage) {
    if (!E) E |= seekDataSet(sctx, NULL, gctx, 0);
    if (hack) {
      ELL_3V_SET(samples, 5, 5, 5);
      if (!E) E |= seekSamplesSet(sctx, samples);
      if (!E) E |= seekItemGradientSet(sctx, gageSclGradVec);
      if (!E) E |= seekItemEigensystemSet(sctx, gageSclHessEval,
                                          gageSclHessEvec);
      if (!E) E |= seekItemNormalSet(sctx, gageSclHessEvec2);
    } else {
      if (!E) E |= seekItemScalarSet(sctx, gageSclValue);
      if (!E) E |= seekItemNormalSet(sctx, gageSclNormal);
    }
  } else {
    if (!E) E |= seekDataSet(sctx, nin, NULL, 0);
  }
  if (!E) E |= seekNormalsFindSet(sctx, AIR_TRUE);
  if (hack) {
    if (!E) E |= seekTypeSet(sctx, seekTypeRidgeSurface);
  } else {
    if (!E) E |= seekTypeSet(sctx, seekTypeIsocontour);
    if (!E) E |= seekIsovalueSet(sctx, isoval);
  }
  if (!E) E |= seekUpdate(sctx);
  if (!E) E |= seekExtract(sctx, pld);
  if (E) {
    airMopAdd(mop, err = biffGetDone(SEEK), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  fprintf(stderr, "%s: extraction time = %g\n", me, sctx->time);

  nmeas = nrrdNew();
  airMopAdd(mop, nmeas, (airMopper)nrrdNuke, airMopAlways);
  if (limnPolyDataCCFind(pld)
      || limnPolyDataPrimitiveArea(nmeas, pld)
      || limnPolyDataPrimitiveSort(pld, nmeas)) {
    err = biffGetDone(LIMN);
    fprintf(stderr, "%s: trouble sorting:\n%s", me, err);
    free(err);
  }

  if (limnPolyDataWriteLMPD(file, pld)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}
