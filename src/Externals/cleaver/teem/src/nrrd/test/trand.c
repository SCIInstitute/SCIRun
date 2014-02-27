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

#define BINS 1024
#define HGHT 800

int
main(int argc, char **argv) {
  Nrrd *nval, *nhist, *npgm;
  double *val;
  int i;

  AIR_UNUSED(argc);
  AIR_UNUSED(argv);
  nrrdAlloc_va(nval=nrrdNew(), nrrdTypeDouble, 1,
               AIR_CAST(size_t, BINS*BINS));
  val = (double *)nval->data;

  airSrandMT((int)airTime());
  for (i=0; i<BINS*BINS; i++) {
    val[i] = airDrandMT();
  }
  
  nrrdHisto(nhist=nrrdNew(), nval, NULL, NULL, BINS, nrrdTypeInt);
  nrrdHistoDraw(npgm=nrrdNew(), nhist, HGHT, AIR_FALSE, 0.0);
  nrrdSave("hist.pgm", npgm, NULL);
  
  nrrdNuke(nval);
  nrrdNuke(nhist);
  nrrdNuke(npgm);

  return 0;
}
