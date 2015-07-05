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

#include "nrrd.h"
#include "privateNrrd.h"

int
nrrdCCValid(const Nrrd *nin) {
  char me[]="nrrdCCValid", err[BIFF_STRLEN];
  
  if (nrrdCheck(nin)) {
    sprintf(err, "%s: basic validity check failed", me);
    biffAdd(NRRD, err); return 0;
  }
  if (!( nrrdTypeIsIntegral[nin->type] )) {
    sprintf(err, "%s: need an integral type (not %s)", me,
            airEnumStr(nrrdType, nin->type));
    biffAdd(NRRD, err); return 0;
  }
  if (!( nrrdTypeSize[nin->type] <= 2 ||
         nrrdTypeInt == nin->type ||
         nrrdTypeUInt == nin->type )) {
    sprintf(err, "%s: valid connected component types are 1- and 2-byte "
            "integers, and %s and %s", me,
            airEnumStr(nrrdType, nrrdTypeInt),
            airEnumStr(nrrdType, nrrdTypeUInt));
    biffAdd(NRRD, err); return 0;
  }
  return 1;
}

/*
** things we could sensibly measure on CCs: 
** - size
** - # neighbors (needs conny argument)
** - what else?
*/

unsigned int
nrrdCCSize(Nrrd *nout, const Nrrd *nin) {
  char me[]="nrrdCCSize", func[]="ccsize", err[BIFF_STRLEN];
  unsigned int *out, maxid, (*lup)(const void *, size_t);
  size_t I, NN;

  if (!( nout && nrrdCCValid(nin) )) {
    sprintf(err, "%s: invalid args", me);
    biffAdd(NRRD, err); return 1;
  }
  maxid = nrrdCCMax(nin);
  if (nrrdMaybeAlloc_va(nout, nrrdTypeUInt, 1,
                        AIR_CAST(size_t, maxid+1))) {
    sprintf(err, "%s: can't allocate output", me);
    biffAdd(NRRD, err); return 1;
  }
  out = (unsigned int *)(nout->data);
  lup = nrrdUILookup[nin->type];
  NN = nrrdElementNumber(nin);
  for (I=0; I<NN; I++) {
    out[lup(nin->data, I)] += 1;
  }
  if (nrrdContentSet_va(nout, func, nin, "")) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  
  return 0;
}

/*
******** nrrdCCMax
**
** returns the highest CC ID, or 0 if there were problems
**
** does NOT use biff
*/
unsigned int
nrrdCCMax(const Nrrd *nin) {
  unsigned int (*lup)(const void *, size_t), id, max;
  size_t I, NN;

  if (!nrrdCCValid(nin)) {
    return 0;
  }
  lup = nrrdUILookup[nin->type];
  NN = nrrdElementNumber(nin);
  max = 0;
  for (I=0; I<NN; I++) {
    id = lup(nin->data, I);
    max = AIR_MAX(max, id);
  }
  return max;
}

/*
******** nrrdCCNum
**
** returns the number of connected components (the # of CC IDs assigned)
** a return of 0 indicates an error
*/
unsigned int
nrrdCCNum(const Nrrd *nin) {
  unsigned int (*lup)(const void *, size_t), num;
  size_t I, max, NN;
  unsigned char *hist;
  
  if (!nrrdCCValid(nin)) {
    return 0;
  }
  lup = nrrdUILookup[nin->type];
  NN = nrrdElementNumber(nin);
  max = nrrdCCMax(nin);
  hist = (unsigned char *)calloc(max+1, sizeof(unsigned char));
  if (!hist) {
    return 0;
  }
  for (I=0; I<NN; I++) {
    hist[lup(nin->data, I)] = 1;
  }
  num = 0;
  for (I=0; I<=max; I++) {
    num += hist[I];
  }
  free(hist);
  return num;
}
