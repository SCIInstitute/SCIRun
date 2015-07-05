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
#include "../privateTen.h"

/* BAD gordon */
extern double _tenQGL_Kdist(const double RThZA[3], const double RThZB[3]);
extern void _tenQGL_Klog(double klog[3],
                         const double RThZA[3], const double RThZB[3]);
extern void _tenQGL_Kexp(double RThZB[3], 
                         const double RThZA[3], const double klog[3]);

extern double _tenQGL_Rdist(const double RThPhA[3], const double RThPhB[3]);
extern void _tenQGL_Rlog(double rlog[3],
                         const double RThPhA[3], const double RThPhB[3]);
extern void _tenQGL_Rexp(double RThPhB[3], 
                         const double RThPhA[3], const double rlog[3]);

/* normalized gradients of k or r invariants, in XYZ space,
   to help determine if path is really a loxodrome */
void
kgrads(double grad[3][3], const double eval[3]) {
  double rtz[3];

  tenTripleConvertSingle_d(rtz, tenTripleTypeRThetaZ,
                           eval, tenTripleTypeEigenvalue);

  ELL_3V_SET(grad[0], cos(rtz[1]), sin(rtz[1]), 0);
  ELL_3V_SET(grad[1], -sin(rtz[1]), cos(rtz[1]), 0);
  ELL_3V_SET(grad[2], 0, 0, 1);
}

void
rgrads(double grad[3][3], const double eval[3]) {
  double rtp[3];

  tenTripleConvertSingle_d(rtp, tenTripleTypeRThetaPhi,
                           eval, tenTripleTypeEigenvalue);

  ELL_3V_SET(grad[0],
             cos(rtp[1])*sin(rtp[2]),
             sin(rtp[1])*sin(rtp[2]),
             cos(rtp[2]));
  ELL_3V_SET(grad[1], -sin(rtp[1]), cos(rtp[1]), 0);
  ELL_3V_SET(grad[2],
             cos(rtp[1])*cos(rtp[2]),
             sin(rtp[1])*cos(rtp[2]),
             -sin(rtp[2]));
}

char *info = ("quaternion geo-lox hacking.  Actually all this does is "
              "help debug the analytic loxodrome part, not the quaternion "
              "geodesic part.");

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt=NULL;
  airArray *mop;

  double tripA[3], tripB[3], evalA[3], evalB[3],
    rt_A[3], rt_B[3], trip[3], eval[3], lasteval[3], lastxyz[3],
    logAB[3], ndist;
  int ittype, ottype, ptype, rttype;
  unsigned int NN, ii;
  tenInterpParm *tip;

  void (*interp)(double oeval[3], const double evalA[3],
                 const double evalB[3], const double tt);
  double (*qdist)(const double RTh_A[3], const double RTh_B[3]);
  void (*qlog)(double klog[3],
               const double RThZA[3], const double RThZB[3]);
  void (*qexp)(double RThZB[3], 
               const double RThZA[3], const double klog[3]);
  void (*grads)(double grad[3][3], const double eval[3]);

  me = argv[0];
  mop = airMopNew();
  tip = tenInterpParmNew();
  airMopAdd(mop, tip, (airMopper)tenInterpParmNix, airMopAlways);

  hestOptAdd(&hopt, "a", "start", airTypeDouble, 3, 3, tripA, NULL,
             "start triple of values");
  hestOptAdd(&hopt, "b", "end", airTypeDouble, 3, 3, tripB, NULL,
             "end triple of values");
  hestOptAdd(&hopt, "it", "type", airTypeEnum, 1, 1, &ittype, NULL,
             "type of given start and end triples", NULL, tenTripleType);
  hestOptAdd(&hopt, "ot", "type", airTypeEnum, 1, 1, &ottype, NULL,
             "type of triples for output", NULL, tenTripleType);
  hestOptAdd(&hopt, "p", "type", airTypeEnum, 1, 1, &ptype, NULL,
             "type of path interpolation", NULL, tenInterpType);
  hestOptAdd(&hopt, "n", "# steps", airTypeUInt, 1, 1, &NN, "100",
             "number of steps along path");

  hestOptAdd(&hopt, "v", "verbosity", airTypeInt, 1, 1,
             &(tip->verbose), "0", "verbosity");
  hestOptAdd(&hopt, "s", "stepsize", airTypeDouble, 1, 1,
             &(tip->convStep), "1", "step size in update");
  hestOptAdd(&hopt, "r", "recurse", airTypeInt, 0, 0,
             &(tip->enableRecurse), NULL,
             "enable recursive solution, when useful");
  hestOptAdd(&hopt, "mn", "minnorm", airTypeDouble, 1, 1,
             &(tip->minNorm), "0.000001",
             "minnorm of something");
  hestOptAdd(&hopt, "mi", "maxiter", airTypeUInt, 1, 1,
             &(tip->maxIter), "0",
             "if non-zero, max # iterations for computation");
  hestOptAdd(&hopt, "c", "conv", airTypeDouble, 1, 1,
             &(tip->convEps), "0.0001",
             "convergence threshold of length fraction");

  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!( tenInterpTypeQuatGeoLoxK == ptype
         || tenInterpTypeQuatGeoLoxR == ptype )) {
    fprintf(stderr, "%s: need type %s or %s, not %s\n", me,
            airEnumStr(tenInterpType, tenInterpTypeQuatGeoLoxK),
            airEnumStr(tenInterpType, tenInterpTypeQuatGeoLoxR),
            airEnumStr(tenInterpType, ptype));
    airMopError(mop); 
    return 1;
  }

  if (tenInterpTypeQuatGeoLoxK == ptype) {
    interp = tenQGLInterpTwoEvalK;
    qdist = _tenQGL_Kdist;
    qlog = _tenQGL_Klog;
    qexp = _tenQGL_Kexp;
    grads = kgrads;
    rttype = tenTripleTypeRThetaZ;
  } else {
    interp = tenQGLInterpTwoEvalR;
    qdist = _tenQGL_Rdist;
    qlog = _tenQGL_Rlog;
    qexp = _tenQGL_Rexp;
    grads = rgrads;
    rttype = tenTripleTypeRThetaPhi;
  }

  fprintf(stderr, "%s: (%s) %f %f %f \n--%s--> %f %f %f\n", me,
          airEnumStr(tenTripleType, ittype),
          tripA[0], tripA[1], tripA[2],
          airEnumStr(tenInterpType, ptype),
          tripB[0], tripB[1], tripB[2]);

  tenTripleConvertSingle_d(evalA, tenTripleTypeEigenvalue, tripA, ittype);
  tenTripleConvertSingle_d(evalB, tenTripleTypeEigenvalue, tripB, ittype);
  tenTripleConvertSingle_d(rt_A, rttype, tripA, ittype);
  tenTripleConvertSingle_d(rt_B, rttype, tripB, ittype);

  ndist = 0;
  ELL_3V_SET(lasteval, AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_3V_SET(lastxyz, AIR_NAN, AIR_NAN, AIR_NAN);
  qlog(logAB, rt_A, rt_B);
  fprintf(stderr, "%s: log = %g %g %g (%g)\n", me, 
          logAB[0], logAB[1], logAB[2], ELL_3V_LEN(logAB));
  for (ii=0; ii<NN; ii++) {
    double tt, xyz[3], dot[3], ll[3], prayRT[3], prayO[3];
    tt = AIR_AFFINE(0, ii, NN-1, 0.0, 1.0);
    interp(eval, evalA, evalB, tt);
    tenTripleConvertSingle_d(trip, ottype,
                             eval, tenTripleTypeEigenvalue);
    tenTripleConvertSingle_d(xyz, tenTripleTypeXYZ,
                             eval, tenTripleTypeEigenvalue);
    ELL_3V_SCALE(ll, tt, logAB);
    qexp(prayRT, rt_A, ll);
    tenTripleConvertSingle_d(prayO, ottype, prayRT, rttype);
    if (ii) {
      double diff[3], gr[3][3];
      ELL_3V_SUB(diff, lasteval, eval);
      ndist += ELL_3V_LEN(diff);
      ELL_3V_SUB(diff, lastxyz, xyz);
      grads(gr, eval);
      dot[0] = ELL_3V_DOT(diff, gr[0]);
      dot[1] = ELL_3V_DOT(diff, gr[1]);
      dot[2] = ELL_3V_DOT(diff, gr[2]);
    } else {
      ELL_3V_SET(dot, 0, 0, 0);
    }
    printf("%03u %g %g   %g %g    %g %g   00   %g %g %g\n", ii,
           trip[0], prayO[0], 
           trip[1], prayO[1], 
           trip[2], prayO[2],
           dot[0], dot[1], dot[2]);
    ELL_3V_COPY(lasteval, eval);
    ELL_3V_COPY(lastxyz, xyz);
  }

  fprintf(stderr, "%s: dist %g =?= %g\n", me,
          qdist(rt_A, rt_B), ndist);

  airMopOkay(mop);
  return 0;
}
