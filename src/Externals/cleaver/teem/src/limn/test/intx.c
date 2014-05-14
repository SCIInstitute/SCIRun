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


#include "../limn.h"

char *info = ("test intersection of two lists.");

static unsigned int
flipListIntx(unsigned int *dstC,
             const unsigned int *_srcA, const unsigned int *_srcB) {
  const unsigned int *srcA, *srcB;
  unsigned int numA, numB, numC, idxA, idxB;

  numA = _srcA[0];
  srcA = _srcA + 1;
  numB = _srcB[0];
  srcB = _srcB + 1;
  numC = 0;
  for (idxA=0; idxA<numA; idxA++) {
    for (idxB=0; idxB<numB; idxB++) {
      if (srcA[idxA] == srcB[idxB]) {
        dstC[numC++] = srcA[idxA];
      }
    }
  }
  return numC;
}

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt=NULL;
  airArray *mop;

  unsigned int *srcA, *srcB, *dstC, *_srcA, *_srcB, numA, numB, numC, idx;
  
  me = argv[0];
  hestOptAdd(&hopt, "a", "vals", airTypeUInt, 1, -1, &srcA, NULL,
             "list of values", &numA);
  hestOptAdd(&hopt, "b", "vals", airTypeUInt, 1, -1, &srcB, NULL,
             "list of values", &numB);
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  fprintf(stderr, "a =");
  for (idx=0; idx<numA; idx++) {
    fprintf(stderr, " %u", srcA[idx]);
  }
  fprintf(stderr, "\n");
  fprintf(stderr, "b =");
  for (idx=0; idx<numB; idx++) {
    fprintf(stderr, " %u", srcB[idx]);
  }
  fprintf(stderr, "\n");

  _srcA = AIR_CAST(unsigned int*, calloc(1+numA, sizeof(unsigned int)));
  airMopAdd(mop, _srcA, airFree, airMopAlways);
  _srcB = AIR_CAST(unsigned int*, calloc(1+numB, sizeof(unsigned int)));
  airMopAdd(mop, _srcB, airFree, airMopAlways);
  dstC = AIR_CAST(unsigned int*, calloc(1+AIR_MAX(numA,numB),
                                        sizeof(unsigned int)));
  airMopAdd(mop, dstC, airFree, airMopAlways);

  _srcA[0] = numA;
  for (idx=0; idx<numA; idx++) {
    _srcA[1+idx] = srcA[idx];
  }
  _srcB[0] = numB;
  for (idx=0; idx<numB; idx++) {
    _srcB[1+idx] = srcB[idx];
  }

  numC = flipListIntx(dstC, _srcA, _srcB);
  fprintf(stderr, "intx(a,b) =");
  for (idx=0; idx<numC; idx++) {
    fprintf(stderr, " %u", dstC[idx]);
  }
  fprintf(stderr, "\n");
  
  airMopOkay(mop);
  return 0;
}

