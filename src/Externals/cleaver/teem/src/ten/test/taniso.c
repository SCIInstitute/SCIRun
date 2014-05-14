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

char *info = ("tests anisotropy measures");

int
main(int argc, char *argv[]) {
  char *me;
  int aa, isot, equi;
  unsigned int ii, nn;
  double evalD[3], pp[3], qq[4], rot[9], mat1[9], mat2[9], tenD[7], tmp, mean;
  double aniso[4];
  float tenF[7], evalF[3];

  AIR_UNUSED(argc);
  me = argv[0];
  nn = 10000000;
  for (ii=0; ii<nn; ii++) {
    if (0 == (ii % 1000)) {
      fprintf(stderr, "."); fflush(stderr);
    }
    evalD[0] = airDrandMT();
    evalD[1] = airDrandMT();
    evalD[2] = airDrandMT();
    if (0 == airRandInt(10)) {
      evalD[1] = evalD[0];
      equi = AIR_TRUE;
      if (0 == airRandInt(2)) {
        evalD[2] = evalD[1] = evalD[0];
        isot = AIR_TRUE;
        if (0 == airRandInt(2)) {
          evalD[2] = evalD[1] = evalD[0] = 0;
        }
      } else {
        isot = AIR_FALSE;
      }
    } else {
      equi = AIR_FALSE;
      isot = AIR_FALSE;
    }
    pp[0] = AIR_AFFINE(0.0, airDrandMT(), 1.0, -1.0, 1.0);
    pp[1] = AIR_AFFINE(0.0, airDrandMT(), 1.0, -1.0, 1.0);
    pp[2] = AIR_AFFINE(0.0, airDrandMT(), 1.0, -1.0, 1.0);
    ELL_4V_SET(qq, 1, pp[0], pp[1], pp[2]);
    ELL_4V_NORM(qq, qq, tmp);
    ell_q_to_3m_d(rot, qq);
    ELL_3M_ZERO_SET(mat1);
    ELL_3M_DIAG_SET(mat1, evalD[0], evalD[1], evalD[2]);
    ell_3m_mul_d(mat2, rot, mat1);
    ELL_3M_TRANSPOSE_IP(rot, tmp);
    ell_3m_mul_d(mat1, mat2, rot);
    TEN_M2T(tenD, mat1);
    TEN_T_COPY(tenF, tenD);
    tenEigensolve_d(evalD, NULL, tenD);
    tenEigensolve_f(evalF, NULL, tenF);
    for (aa=tenAnisoUnknown+1; aa<tenAnisoLast; aa++) {
      int bogus;
      aniso[0] = tenAnisoEval_f(evalF, aa);
      aniso[1] = tenAnisoEval_d(evalD, aa);
      aniso[2] = tenAnisoTen_f(tenF, aa);
      aniso[3] = tenAnisoTen_d(tenD, aa);
      mean = (aniso[0] + aniso[1] + aniso[2] + aniso[3])/4;
      tmp = ((aniso[0]-mean)*(aniso[0]-mean)
             + (aniso[1]-mean)*(aniso[1]-mean)
             + (aniso[2]-mean)*(aniso[2]-mean)
             + (aniso[3]-mean)*(aniso[3]-mean))/4;
      bogus = (tenAniso_Mode == aa
               || tenAniso_Th == aa
               || tenAniso_Skew == aa
               || tenAniso_Ct1 == aa
               || tenAniso_Ct2 == aa);
      if (AIR_EXISTS(tmp) && bogus && isot) {
        continue;
      }
      if (AIR_EXISTS(tmp) && bogus && equi && tmp < 0.001) {
        continue;
      }
      if (!AIR_EXISTS(tmp) || tmp > 0.0000003) {
        fprintf(stderr, "\n%s: %u %d (%s) (isot %s, equi %s) tmp=%g\n",
                me, ii, aa, 
                airEnumStr(tenAniso, aa),
                isot ? "true" : "false",
                equi ? "true" : "false", tmp);
        fprintf(stderr, "  %g %g %g    %g %g    %g;\n",
                tenD[1], tenD[2], tenD[3], tenD[4], tenD[5], tenD[6]);
        fprintf(stderr, "  %f %f %f (%f %f %f)f --->\n",
                evalD[0], evalD[1], evalD[2],
                evalF[0], evalF[1], evalF[2]);
        fprintf(stderr, "  %f %f %f %f\n", 
                aniso[0], aniso[1], aniso[2], aniso[3]);
        exit(1);
      }
    }
  }

  fprintf(stderr, "\n");
  return 0;
}
