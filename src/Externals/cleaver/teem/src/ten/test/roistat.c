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


#include "../ten.h"

char *info = ("tensor ROI information.");

int
main(int argc, char *argv[]) {
  char *me, *err;
  hestOpt *hopt=NULL;
  airArray *mop;
  
  unsigned int sx, sy, sz, ss, ii, anisoTypeNum, anisoTypeIdx,
    roiVoxNum, roiVoxIdx, statNum, statIdx;
  float *ten, *roi, *aniso, eval[3], *stat;
  Nrrd *nten, *_nroi, *nroi, *naniso, *nstat;
  int *anisoType, *measr;
  size_t anisoSize[2];
  mop = airMopNew();
  
  me = argv[0];
  hestOptAdd(&hopt, "r,roi", "roi", airTypeOther, 1, 1, &_nroi, NULL,
             "ROI volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "i,input", "volume", airTypeOther, 1, 1, &nten, "-",
             "tensor volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "a,aniso", "aniso", airTypeEnum, 1, -1, &anisoType, NULL,
             "which anisotropy measures to measure",
             &anisoTypeNum, tenAniso);
  hestOptAdd(&hopt, "m,measr", "measr", airTypeEnum, 1, -1, &measr, NULL,
             "which measures/statistics to calculate",
             &statNum, nrrdMeasure);
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (tenTensorCheck(nten, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
    airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: didn't get tensor input:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }

  nroi = nrrdNew();
  airMopAdd(mop, nroi, AIR_CAST(airMopper, nrrdNuke), airMopAlways);
  if (nrrdConvert(nroi, _nroi, nrrdTypeFloat)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't convert ROI to float:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }

  sx = nten->axis[1].size;
  sy = nten->axis[2].size;
  sz = nten->axis[3].size;
  if (!(3 == nroi->dim 
        && sx == nroi->axis[0].size
        && sy == nroi->axis[1].size
        && sz == nroi->axis[2].size)) {
    fprintf(stderr, "%s: ROI dimension or axis sizes don't match volume", me);
    airMopError(mop); 
    return 1;
  }
  ss = sx*sy*sz;

  ten = AIR_CAST(float*, nten->data);
  roi = AIR_CAST(float*, nroi->data);
  
  /* NOTE: for time being the statistics are not weighted, because
     nrrdMeasureLine[]() can't take a weight vector... */

  /* find number of voxels in ROI */
  roiVoxNum = 0;
  for (ii=0; ii<ss; ii++) {
    roiVoxNum += (roi[ii] > 0);
  }
  /* fprintf(stderr, "%s: # voxels in ROI == %u\n", me, roiVoxNum); */

  /* allocate anisotropy buffers */
  naniso = nrrdNew();
  airMopAdd(mop, naniso, AIR_CAST(airMopper, nrrdNuke), airMopAlways);
  anisoSize[0] = roiVoxNum;
  anisoSize[1] = anisoTypeNum;
  if (nrrdMaybeAlloc_nva(naniso, nrrdTypeFloat, 2, anisoSize)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't allocate aniso:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  aniso = AIR_CAST(float *, naniso->data);

  /* store all anisotropies in all ROI voxels */
  roiVoxIdx = 0;
  for (ii=0; ii<ss; ii++) {
    if (roi[ii] > 0) {
      tenEigensolve_f(eval, NULL, ten + 7*ii);
      for (anisoTypeIdx=0; anisoTypeIdx<anisoTypeNum; anisoTypeIdx++) {
        aniso[roiVoxIdx + roiVoxNum*anisoTypeIdx] 
          = tenAnisoEval_f(eval, anisoType[anisoTypeIdx]);
      }
      roiVoxIdx++;
    }
  }

  printf("statistic:");
  for (anisoTypeIdx=0; anisoTypeIdx<anisoTypeNum; anisoTypeIdx++) {
    printf(" %s", airEnumStr(tenAniso, anisoType[anisoTypeIdx]));
  }
  printf("\n");

  /* do per-anisotropy statistics */
  nstat = nrrdNew();
  airMopAdd(mop, nstat, AIR_CAST(airMopper, nrrdNuke), airMopAlways);
  for (statIdx=0; statIdx<statNum; statIdx++) {
    printf("%s:", airEnumStr(nrrdMeasure, measr[statIdx]));

    if (nrrdProject(nstat, naniso, 0, measr[statIdx], nrrdTypeFloat)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: couldn't measure:\n%s\n", me, err);
      airMopError(mop); 
      return 1;
    }
    stat = AIR_CAST(float *, nstat->data);
    for (anisoTypeIdx=0; anisoTypeIdx<anisoTypeNum; anisoTypeIdx++) {
      printf(" %g", stat[anisoTypeIdx]);
    }

    printf("\n");
  }
  
  airMopOkay(mop);
  return 0;
}
