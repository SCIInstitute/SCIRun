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


#include "../ten.h"

char *info = ("tests invariant grads and rotation tangents.");

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt=NULL;
  airArray *mop;

  double _ten[6], ten[7], minnorm, igrt[6][7], eval[3], evec[9],
    pp[3], qq[4], rot[9], matA[9], matB[9], tmp;
  int doK, ret, ii, jj;
  mop = airMopNew();
  
  me = argv[0];
  hestOptAdd(&hopt, NULL, "tensor", airTypeDouble, 6, 6, _ten, NULL,
             "tensor value");
  hestOptAdd(&hopt, "mn", "minnorm", airTypeDouble, 1, 1, &minnorm,
             "0.00001",
             "minimum norm before special handling");
  hestOptAdd(&hopt, "k", NULL, airTypeInt, 0, 0, &doK, NULL,
             "Use K invariants, instead of R (the default)");
  hestOptAdd(&hopt, "p", "x y z", airTypeDouble, 3, 3, pp, "0 0 0",
             "location in quaternion quotient space");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  ELL_6V_COPY(ten+1, _ten);
  ten[0] = 1.0;

  fprintf(stderr, "input tensor = %f %f %f    %f %f     %f\n", 
          ten[1], ten[2], ten[3], ten[4], ten[5], ten[6]);

  ELL_4V_SET(qq, 1, pp[0], pp[1], pp[2]);
  ELL_4V_NORM(qq, qq, tmp);
  ell_q_to_3m_d(rot, qq);
  TEN_T2M(matA, ten);
  ELL_3M_MUL(matB, rot, matA);
  ELL_3M_TRANSPOSE_IP(rot, tmp);
  ELL_3M_MUL(matA, matB, rot);
  TEN_M2T(ten, matA);

  fprintf(stderr, "rotated tensor = %f %f %f    %f %f     %f\n", 
          ten[1], ten[2], ten[3], ten[4], ten[5], ten[6]);

  ret = tenEigensolve_d(eval, evec, ten);
  fprintf(stderr, "eigensystem: %s: %g %g %g\n", 
          airEnumDesc(ell_cubic_root, ret),
          eval[0], eval[1], eval[2]);

  if (doK) {
    tenInvariantGradientsK_d(igrt[0], igrt[1], igrt[2], ten, minnorm);
  } else {
    tenInvariantGradientsR_d(igrt[0], igrt[1], igrt[2], ten, minnorm);
  }
  tenRotationTangents_d(igrt[3], igrt[4], igrt[5], evec);

  fprintf(stderr, "invariant gradients and rotation tangents:\n");
  for (ii=0; ii<=2; ii++) {
    fprintf(stderr, "  %s_%d: (norm=%g) %f %f %f     %f %f     %f\n", 
            doK ? "K" : "R", ii+1,
            TEN_T_NORM(igrt[ii]),
            igrt[ii][1], igrt[ii][2], igrt[ii][3],
            igrt[ii][4], igrt[ii][5],
            igrt[ii][6]);
  }
  for (ii=3; ii<=5; ii++) {
    fprintf(stderr, "phi_%d: (norm=%g) %f %f %f     %f %f     %f\n", 
            ii-2,
            TEN_T_NORM(igrt[ii]),
            igrt[ii][1], igrt[ii][2], igrt[ii][3],
            igrt[ii][4], igrt[ii][5],
            igrt[ii][6]);
  }

  fprintf(stderr, "dot products:\n");
  for (ii=0; ii<=5; ii++) {
    for (jj=ii+1; jj<=5; jj++) {
      fprintf(stderr, "%d,%d==%f  ", ii, jj, TEN_T_DOT(igrt[ii], igrt[jj]));
    }
    fprintf(stderr, "\n");
  }

  
  airMopOkay(mop);
  return 0;
}
