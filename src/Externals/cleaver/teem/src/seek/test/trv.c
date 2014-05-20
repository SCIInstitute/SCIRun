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

char *info = ("test crease surface extraction.");

int
probeParseKind(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  char me[] = "probeParseKind";
  gageKind **kindP;
  
  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  kindP = (gageKind **)ptr;
  airToLower(str);
  if (!strcmp(gageKindScl->name, str)) {
    *kindP = gageKindScl;
  } else if (!strcmp(gageKindVec->name, str)) {
    *kindP = gageKindVec;
  } else {
    sprintf(err, "%s: not \"%s\" or \"%s\"", me,
            gageKindScl->name, gageKindVec->name);
    return 1;
  }

  return 0;
}

void *
probeParseKindDestroy(void *ptr) {
  gageKind *kind;
  
  if (ptr) {
    kind = AIR_CAST(gageKind *, ptr);
  }
  return NULL;
}

hestCB probeKindHestCB = {
  sizeof(gageKind *),
  "kind",
  probeParseKind,
  probeParseKindDestroy
}; 

int
main(int argc, char *argv[]) {
  char *me, *err, *outS;
  hestOpt *hopt=NULL;
  airArray *mop;

  limnPolyData *pld, *pldSub;
  gageContext *gctx=NULL;
  gagePerVolume *pvl;
  Nrrd *nin, *nmeas;
  double kparm[3], strength, scaling[3];
  seekContext *sctx;
  FILE *file;
  unsigned int ncc;
  size_t samples[3];
  gageKind *kind;
  char *itemGradS, *itemEvalS[2], *itemEvecS[2];
  int itemGrad, itemEval[2], itemEvec[2];
  int E;
  
  me = argv[0];
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume to analyze",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "k", "kind", airTypeOther, 1, 1, &kind, NULL,
             "\"kind\" of volume (\"scalar\", \"vector\", \"tensor\")",
             NULL, NULL, &probeKindHestCB);
  hestOptAdd(&hopt, "s", "strength", airTypeDouble, 1, 1, &strength, "0.01",
             "strength");
  hestOptAdd(&hopt, "gi", "grad item", airTypeString, 1, 1, &itemGradS, NULL,
             "item for gradient vector");
  hestOptAdd(&hopt, "c", "scaling", airTypeDouble, 3, 3, scaling, "1 1 1",
             "amount by which to up/down-sample on each spatial axis");
  hestOptAdd(&hopt, "n", "# CC", airTypeUInt, 1, 1, &ncc, "0",
             "if non-zero, number of CC to save");
  hestOptAdd(&hopt, "o", "output LMPD", airTypeString, 1, 1, &outS, "out.lmpd",
             "output file to save LMPD into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  itemGrad = airEnumVal(kind->enm, itemGradS);
  
  pld = limnPolyDataNew();
  airMopAdd(mop, pld, (airMopper)limnPolyDataNix, airMopAlways);
  pldSub = limnPolyDataNew();
  airMopAdd(mop, pldSub, (airMopper)limnPolyDataNix, airMopAlways);

  file = airFopen(outS, stdout, "w");
  airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);

  sctx = seekContextNew();
  airMopAdd(mop, sctx, (airMopper)seekContextNix, airMopAlways);

  gctx = gageContextNew();
  airMopAdd(mop, gctx, (airMopper)gageContextNix, airMopAlways);
  ELL_3V_SET(kparm, 1, 1.0, 0.0);
  if (!(pvl = gagePerVolumeNew(gctx, nin, kind))
      || gagePerVolumeAttach(gctx, pvl)
      || gageKernelSet(gctx, gageKernel00, nrrdKernelBCCubic, kparm)
      || gageKernelSet(gctx, gageKernel11, nrrdKernelBCCubicD, kparm)
      || gageKernelSet(gctx, gageKernel22, nrrdKernelBCCubicDD, kparm)
      || gageQueryItemOn(gctx, pvl, itemGrad)
      || gageQueryItemOn(gctx, pvl, gageSclHessEval)
      || gageQueryItemOn(gctx, pvl, gageSclHessEval2)
      || gageQueryItemOn(gctx, pvl, gageSclHessEvec)
      || gageQueryItemOn(gctx, pvl, gageSclHessEvec2)
      || gageUpdate(gctx)) {
    airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  seekVerboseSet(sctx, 10);

  E = 0;
  if (!E) E |= seekDataSet(sctx, NULL, gctx, 0);
  ELL_3V_SET(samples,
             scaling[0]*nin->axis[kind->baseDim + 0].size,
             scaling[1]*nin->axis[kind->baseDim + 1].size,
             scaling[2]*nin->axis[kind->baseDim + 2].size);
  if (!E) E |= seekSamplesSet(sctx, samples);
  if (!E) E |= seekItemGradientSet(sctx, itemGrad);
  if (!E) E |= seekItemEigensystemSet(sctx, gageSclHessEval,
                                      gageSclHessEvec);
  if (!E) E |= seekItemNormalSet(sctx, gageSclHessEvec2);
  if (!E) E |= seekStrengthUseSet(sctx, AIR_TRUE);
  if (!E) E |= seekStrengthSet(sctx, -1, strength/2, strength);
  if (!E) E |= seekItemStrengthSet(sctx, gageSclHessEval2);
  if (!E) E |= seekNormalsFindSet(sctx, AIR_TRUE);
  if (!E) E |= seekTypeSet(sctx, seekTypeRidgeSurface);
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
  if (limnPolyDataVertexWindingFix(pld, AIR_TRUE)
      || limnPolyDataVertexWindingFlip(pld)
      || limnPolyDataVertexNormals(pld)
      || limnPolyDataCCFind(pld)
      || limnPolyDataPrimitiveArea(nmeas, pld)
      || limnPolyDataPrimitiveSort(pld, nmeas)) {
    err = biffGetDone(LIMN);
    fprintf(stderr, "%s: trouble sorting:\n%s", me, err);
    free(err);
  }

  if (ncc > 1) {
    double *meas;
    unsigned int ccIdx;
    nrrdSave("meas.nrrd", nmeas, NULL);
    ncc = AIR_MIN(ncc, nmeas->axis[0].size);
    meas = AIR_CAST(double *, nmeas->data);
    for (ccIdx=ncc; ccIdx<nmeas->axis[0].size; ccIdx++) {
      meas[ccIdx] = 0.0;
    }
    if (!E) E |= limnPolyDataPrimitiveSelect(pldSub, pld, nmeas);
    if (!E) E |= limnPolyDataWriteLMPD(file, pldSub);
  } else {
    if (!E) E |= limnPolyDataWriteLMPD(file, pld);
  }
  if (E) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}
