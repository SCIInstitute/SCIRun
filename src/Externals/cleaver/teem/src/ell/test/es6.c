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

#include "../ell.h"

char *es6Info = ("Tests ell_6ms_eigensolve_d");

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  Nrrd *nin, *_nin;
  double *in, sym[21], eval[6], evec[36];
  unsigned int rr, cc, ii, jj;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, NULL, "matrix", airTypeOther, 1, 1, &_nin, NULL,
             "6x6 matrix", NULL, NULL, nrrdHestNrrd);
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, es6Info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nin = nrrdNew();
  airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);
  
  if (!(2 == _nin->dim 
        && 6 == _nin->axis[0].size 
        && 6 == _nin->axis[1].size)) {
    fprintf(stderr, "%s: didn't get 2-D 6x6 matrix (got %u-D %ux%u)\n", me,
            _nin->dim,
            AIR_CAST(unsigned int, _nin->axis[0].size), 
            AIR_CAST(unsigned int, _nin->axis[1].size));
    airMopError(mop); return 1;
  }

  nrrdConvert(nin, _nin, nrrdTypeDouble);
  in = AIR_CAST(double *, nin->data);

  sym[ 0] = in[ 0];
  sym[ 1] = in[ 1];
  sym[ 2] = in[ 2];
  sym[ 3] = in[ 3];
  sym[ 4] = in[ 4];
  sym[ 5] = in[ 5];
  sym[ 6] = in[ 7];
  sym[ 7] = in[ 8];
  sym[ 8] = in[ 9];
  sym[ 9] = in[10];
  sym[10] = in[11];
  sym[11] = in[14];
  sym[12] = in[15];
  sym[13] = in[16];
  sym[14] = in[17];
  sym[15] = in[21];
  sym[16] = in[22];
  sym[17] = in[23];
  sym[18] = in[28];
  sym[19] = in[29];
  sym[20] = in[35];
  for (rr=1; rr<6; rr++) {
    for (cc=0; cc<rr; cc++) {
      in[cc + 6*rr] = in[rr + 6*cc];
    }
  }

  fprintf(stderr, "m = [");
  for (rr=0; rr<6; rr++) {
    for (cc=0; cc<6; cc++) {
      fprintf(stderr, "%g%s", in[cc + 6*rr], 
              cc<5 ? "," : (rr<5 ? ";" : "]"));
    }
    fprintf(stderr, "\n");
  }

  ell_6ms_eigensolve_d(eval, evec, sym, FLT_MIN/10);

  for (ii=0; ii<6; ii++) {
    fprintf(stderr, "eval[%u] = %g:\n  ", ii, eval[ii]);
    for (jj=0; jj<6; jj++) {
      fprintf(stderr, "  %g", evec[jj + 6*ii]);
    }
    fprintf(stderr, "\n");
  }

  airMopOkay(mop);
  exit(0);
}

