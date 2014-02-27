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


#include "../push.h"

char *info = ("Tests parsing of energy, and its methods.");

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt=NULL;
  airArray *mop;
  
  pushEnergySpec *ensp;
  unsigned int pi, xi, nn;
  double xx, supp, del;

  mop = airMopNew();
  me = argv[0];
  hestOptAdd(&hopt, "energy", "spec", airTypeOther, 1, 1, &ensp, NULL,
             "specification of force function to use",
             NULL, NULL, pushHestEnergySpec);
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  fprintf(stderr, "%s: parsed energy \"%s\", with %u parms.\n", me,
          ensp->energy->name, ensp->energy->parmNum);
  for (pi=0; pi<ensp->energy->parmNum; pi++) {
    fprintf(stderr, "%u: %g\n", pi, ensp->parm[pi]);
  }
  fprintf(stderr, "\n");

  nn = 600;
  supp = ensp->energy->support(ensp->parm);
  del = AIR_DELTA(0, 2, nn, 0, supp);
  for (xi=1; xi<nn; xi++) {
    double x0, x1, ee, ff, e0, e1, dummy;
    xx = AIR_AFFINE(0, xi,   nn, 0, supp);
    x1 = AIR_AFFINE(0, xi+1, nn, 0, supp);
    x0 = AIR_AFFINE(0, xi-1, nn, 0, supp);
    ensp->energy->eval(&e1, &dummy, x1, ensp->parm);
    ensp->energy->eval(&e0, &dummy, x0, ensp->parm);
    ensp->energy->eval(&ee, &ff, xx, ensp->parm);
    printf("%g %g %g %g\n", xx, ee, ff, (e1 - e0)/del);
  }

  airMopOkay(mop);
  return 0;
}

