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

#include "unrrdu.h"
#include "privateUnrrdu.h"

#define INFO "Euclidean distance transform"
char *_unrrdu_distInfoL = 
(INFO
 ". Based on \"Distance Transforms of Sampled Functions\" by "
 "Pedro F. Felzenszwalb and Daniel P. Huttenlocher, "
 "Cornell Computing and Information Science TR2004-1963. "
 "This function first thresholds at the specified value and then "
 "does the distance transform of the resulting binary image. "
 "The signed distance (negative values inside object) is also available. "
 "Distances between non-isotropic samples are handled correctly.");

int
unrrdu_distMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  int pret;

  int E, typeOut, invert, sign;
  double thresh;
  airArray *mop;

  hestOptAdd(&opt, "th,thresh", "val", airTypeDouble, 1, 1, &thresh, NULL,
             "threshold value to separate inside from outside");
  hestOptAdd(&opt, "t,type", "type", airTypeEnum, 1, 1, &typeOut, "float",
             "type to save output in", NULL, nrrdType);
  hestOptAdd(&opt, "sgn", NULL, airTypeInt, 0, 0, &sign, NULL,
             "also compute signed (negative) distances inside objects, "
             "instead of leaving them as zero");
  hestOptAdd(&opt, "inv", NULL, airTypeInt, 0, 0, &invert, NULL,
             "values *below* threshold are considered interior to object. "
             "By default (not using this option), values above threshold "
             "are considered interior. ");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_distInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (sign) {
    E = nrrdDistanceL2Signed(nout, nin, typeOut, NULL, thresh, !invert);
  } else {
    E = nrrdDistanceL2(nout, nin, typeOut, NULL, thresh, !invert);
  }
  if (E) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error doing distance transform:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(dist, INFO);
