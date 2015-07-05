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

char *indxInfo = ("for debugging part of _gageLocationSet");

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  double *val, top, min, max;
  unsigned ii, valLen, xi, size;
  int center;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, NULL, "val0 val1", airTypeDouble, 1, -1, &val, NULL,
             "input values", &valLen, NULL);
  hestOptAdd(&hopt, "c,center", "center", airTypeEnum, 1, 1, &center, NULL,
             "centering (cell or node)", NULL, nrrdCenter);
  hestOptAdd(&hopt, "s", "size", airTypeUInt, 1, 1, &size, NULL,
             "number of samples");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, indxInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  fprintf(stderr, "%s: using %s centering\n", me,
          airEnumStr(nrrdCenter, center));

  top = size-1;
  if (nrrdCenterNode == center) {
    min = 0;
    max = top;
  } else {
    min = -0.5;
    max = top + 0.5;
  }

  for (ii=0; ii<valLen; ii++) {
    double _xi, xf;
    _xi = val[ii];
    fprintf(stderr, "%u \t%g \t", ii, _xi);
    if (!( AIR_IN_CL(min, _xi, max) )) {
      fprintf(stderr, "OUTSIDE\n");
    } else {
      if (0 && nrrdCenterCell == center) {
        xi = AIR_CAST(unsigned int, _xi+1) - 1; 
      } else {
        xi = AIR_CAST(unsigned int, _xi);
      }
      xi -= (xi == max);
      xf = _xi - xi;
      fprintf(stderr, "%u + %g\n", xi, xf);
    }
  }

  airMopOkay(mop);
  exit(0);
}


