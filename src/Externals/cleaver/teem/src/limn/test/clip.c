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

#include "teem/limn.h"

char *info = ("test limnPolyDataClip");

int
main(int argc, char *argv[]) {
  char *me, *err, *outS;
  hestOpt *hopt=NULL;
  airArray *mop;

  limnPolyData *pld;
  FILE *file;
  Nrrd *nin;
  double thresh;
  int bitflag;
  
  me = argv[0];
  hestOptAdd(&hopt, "vi", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input values",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "pi", "lpld", airTypeOther, 1, 1, &pld, NULL,
             "input polydata",
             NULL, NULL, limnHestPolyDataLMPD);
  hestOptAdd(&hopt, "th", "thresh", airTypeDouble, 1, 1, &thresh, NULL,
             "threshold value");
  hestOptAdd(&hopt, "o", "output LMPD", airTypeString, 1, 1, &outS, "out.lmpd",
             "output file to save LMPD into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  bitflag = limnPolyDataInfoBitFlag(pld);
  fprintf(stderr, "!%s: bitflag = %d\n", me, bitflag);
  fprintf(stderr, "!%s: rgba %d,  norm %d,   tex2 %d\n", me,
          (1 << limnPolyDataInfoRGBA) & bitflag,
          (1 << limnPolyDataInfoNorm) & bitflag,
          (1 << limnPolyDataInfoTex2) & bitflag);

  file = airFopen(outS, stdout, "w");
  if (!file) {
    fprintf(stderr, "%s: couldn't open \"%s\" for writing", me, outS);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);

  if (limnPolyDataClip(pld, nin, thresh)
      || limnPolyDataWriteLMPD(file, pld)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}
