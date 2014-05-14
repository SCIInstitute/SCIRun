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

#define INFO "Find connected components (CCs)"
static char *myinfo =
(INFO
 ", and then sorts primitive according to area.");

int
limnpu_ccfindMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *hopt = NULL;
  char *err, *perr;
  airArray *mop;
  int pret;

  limnPolyData *pld;
  Nrrd *nmeas;
  char *out;

  hestOptAdd(&hopt, NULL, "input", airTypeOther, 1, 1, &pld, NULL,
             "input polydata filename",
             NULL, NULL, limnHestPolyDataLMPD);
  hestOptAdd(&hopt, NULL, "output", airTypeString, 1, 1, &out, NULL,
             "output polydata filename");
  
  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);

  USAGE(myinfo);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nmeas = nrrdNew();
  airMopAdd(mop, nmeas, (airMopper)nrrdNuke, airMopAlways);

  if (limnPolyDataCCFind(pld)
      || limnPolyDataPrimitiveArea(nmeas, pld)
      || limnPolyDataPrimitiveSort(pld, nmeas)
      || limnPolyDataSave(out, pld)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s", me, err);
    airMopError(mop);
    return 1;
  }
  
  airMopOkay(mop);
  return 0;
}

unrrduCmd limnpu_ccfindCmd = { "ccfind", INFO, limnpu_ccfindMain };

