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

#define INFO "Rasterize polydata"
static char *myinfo =
(INFO
 ". ");

int
limnpu_rastMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *hopt = NULL;
  char *err, *perr;
  airArray *mop;
  int pret;

  limnPolyData *pld;
  double min[3], max[3];
  Nrrd *nout;
  char *out;
  int type;
  size_t size[NRRD_DIM_MAX];

  hestOptAdd(&hopt, "min", "min", airTypeDouble, 3, 3, min, NULL,
             "bottom corner");
  hestOptAdd(&hopt, "max", "max", airTypeDouble, 3, 3, max, NULL,
             "top corner");
  hestOptAdd(&hopt, "s", "size", airTypeSize_t, 3, 3, size, NULL,
             "number of samples along each axis");
  hestOptAdd(&hopt, "t", "type", airTypeEnum, 1, 1, &type, "uchar",
             "type of output nrrd", 
             NULL, nrrdType);
  hestOptAdd(&hopt, NULL, "input", airTypeOther, 1, 1, &pld, NULL,
             "input polydata filename",
             NULL, NULL, limnHestPolyDataLMPD);
  hestOptAdd(&hopt, NULL, "output", airTypeString, 1, 1, &out, NULL,
             "output nrrd filename");
  
  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);

  USAGE(myinfo);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (limnPolyDataRasterize(nout, pld, min, max, size, type)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:%s", me, err);
    airMopError(mop);
    return 1;
  }
  if (nrrdSave(out, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:%s", me, err);
    airMopError(mop);
    return 1;
  }
  
  airMopOkay(mop);
  return 0;
}

unrrduCmd limnpu_rastCmd = { "rast", INFO, limnpu_rastMain };

