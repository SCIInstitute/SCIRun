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

#include "limn.h"
#include "privateLimn.h"

#define INFO "Select some subset of primitives"
static char *myinfo =
(INFO
 ". Can either specify a range, or a list, or not, until implemented.");

int
limnpu_pselMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *hopt = NULL;
  char *err, *perr;
  airArray *mop;
  int pret;

  limnPolyData *pldIn, *pldOut;
  unsigned int prange[2], pi;
  Nrrd *nsel;
  double *sel;
  char *out;
  size_t size[NRRD_DIM_MAX];

  hestOptAdd(&hopt, "r", "range", airTypeUInt, 2, 2, prange, NULL,
             "range of indices of primitives to select");
  hestOptAdd(&hopt, NULL, "input", airTypeOther, 1, 1, &pldIn, NULL,
             "input polydata filename",
             NULL, NULL, limnHestPolyDataLMPD);
  hestOptAdd(&hopt, NULL, "output", airTypeString, 1, 1, &out, NULL,
             "output polydata filename");
  
  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);

  USAGE(myinfo);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!( prange[0] <= pldIn->primNum-1 &&
         prange[1] <= pldIn->primNum-1 )) {
    fprintf(stderr, "%s: prange[0] %u or [1] %u outside range [0,%u]", me,
            prange[0], prange[1], pldIn->primNum-1);
    airMopError(mop);
    return 1;
  }
  if (!( prange[0] <= prange[1] )) {
    fprintf(stderr, "%s: need prange[0] %u <= [1] %u", me,
            prange[0], prange[1]);
    airMopError(mop);
    return 1;
  }

  nsel = nrrdNew();
  airMopAdd(mop, nsel, (airMopper)nrrdNuke, airMopAlways);
  size[0] = pldIn->primNum;
  if (nrrdMaybeAlloc_nva(nsel, nrrdTypeDouble, 1, size)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble allocating buffer:%s", me, err);
    airMopError(mop);
    return 1;
  }

  sel = AIR_CAST(double *, nsel->data);
  for (pi=prange[0]; pi<=prange[1]; pi++) {
    sel[pi] = 1;
  }

  pldOut = limnPolyDataNew();
  airMopAdd(mop, pldOut, (airMopper)limnPolyDataNix, airMopAlways);
  if (limnPolyDataPrimitiveSelect(pldOut, pldIn, nsel)
      || limnPolyDataSave(out, pldOut)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:%s", me, err);
    airMopError(mop);
    return 1;
  }
  
  airMopOkay(mop);
  return 0;
}

unrrduCmd limnpu_pselCmd = { "psel", INFO, limnpu_pselMain };

