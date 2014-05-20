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

#include "../nrrd.h"

char *otsuInfo = ("demonstrates nrrd's Otsu thresholding");

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  char *err;
  Nrrd *nhist;
  double thresh, expo;
  
  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "i", "nhist", airTypeOther, 1, 1, &nhist, NULL,
             "input histogram", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "e", "expo", airTypeDouble, 1, 1, &expo, "2.0",
             "exponent to use for variance calculation");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, otsuInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (nrrdHistoThresholdOtsu(&thresh, nhist, expo)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s", me, err);
    airMopError(mop); return 1;
  }

  fprintf(stderr, "%s: threshold = %g\n", me, thresh);

  airMopOkay(mop);
  exit(0);
}
