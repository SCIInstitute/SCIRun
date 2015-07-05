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

char *info = ("does stupid geodesics");

int
main(int argc, char *argv[]) {
  char *me, *err;
  hestOpt *hopt=NULL;
  airArray *mop;

  char *outS;
  double _tA[6], tA[7], _tB[6], tB[7], time0, time1, conv, confThresh,
    pA[3], pB[3], qA[4], qB[4], rA[9], rB[9], mat1[9], mat2[9], tmp,
    stepSize, minNorm, sclA, sclB;
  unsigned int NN, maxiter, refIdx[3];
  int recurse, ptype, verb;
  Nrrd *_nin, *nin, *nout;
  tenInterpParm *tip;

  mop = airMopNew();
  me = argv[0];
  hestOptAdd(&hopt, "a", "tensor", airTypeDouble, 6, 6, _tA, "1 0 0 1 0 1",
             "first tensor");
  hestOptAdd(&hopt, "pa", "qq", airTypeDouble, 3, 3, pA, "0 0 0",
             "rotation of first tensor");
  hestOptAdd(&hopt, "sa", "scl", airTypeDouble, 1, 1, &sclA, "1.0",
             "scaling of first tensor");
  hestOptAdd(&hopt, "b", "tensor", airTypeDouble, 6, 6, _tB, "1 0 0 1 0 1",
             "second tensor");
  hestOptAdd(&hopt, "pb", "qq", airTypeDouble, 3, 3, pB, "0 0 0",
             "rotation of second tensor");
  hestOptAdd(&hopt, "sb", "scl", airTypeDouble, 1, 1, &sclB, "1.0",
             "scaling of second tensor");
  hestOptAdd(&hopt, "i", "nten", airTypeOther, 1, 1, &_nin, "",
             "input tensor volume (makes previous options moot)",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "ri", "x y z", airTypeUInt, 3, 3, refIdx, "0 0 0",
             "index of reference tensor in input tensor volume");
  hestOptAdd(&hopt, "th", "thresh", airTypeDouble, 1, 1, &confThresh, "0.5",
             "conf mask threshold on \"-i\"");
  hestOptAdd(&hopt, "n", "# steps", airTypeUInt, 1, 1, &NN, "100",
             "number of steps in between two tensors");
  hestOptAdd(&hopt, "s", "stepsize", airTypeDouble, 1, 1, &stepSize, "1",
             "step size in update");
  hestOptAdd(&hopt, "mn", "minnorm", airTypeDouble, 1, 1, &minNorm, "0.000001",
             "minnorm of something");
  hestOptAdd(&hopt, "c", "conv", airTypeDouble, 1, 1, &conv, "0.0001",
             "convergence threshold of length fraction");
  hestOptAdd(&hopt, "mi", "maxiter", airTypeUInt, 1, 1, &maxiter, "0",
             "if non-zero, max # iterations for computation");
  hestOptAdd(&hopt, "r", "recurse", airTypeInt, 0, 0, &recurse, NULL,
             "enable recursive solution, when useful");
  hestOptAdd(&hopt, "t", "path type", airTypeEnum, 1, 1, &ptype, "lerp",
             "what type of path to compute", NULL, tenInterpType);
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS, "-",
             "file to write output nrrd to");
  hestOptAdd(&hopt, "v", "verbosity", airTypeInt, 1, 1, &verb, "0",
             "verbosity");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  tip = tenInterpParmNew();
  airMopAdd(mop, tip, (airMopper)tenInterpParmNix, airMopAlways);
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  tip->verbose = verb;
  tip->convStep = stepSize;
  tip->enableRecurse = recurse;
  tip->minNorm = minNorm;
  tip->maxIter = maxiter;
  tip->convEps = conv;
  if (_nin) {
    double refTen[7], inTen[7], *in, *out;
    unsigned int xi, yi, zi, sx, sy, sz, dimOut;
    int axmap[NRRD_DIM_MAX], numerical;
    size_t size[NRRD_DIM_MAX];

    if (tenTensorCheck(_nin, nrrdTypeDefault, AIR_TRUE, AIR_TRUE)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: input volume not valid:\n%s\n",
              me, err);
      airMopError(mop); 
      return 1;
    }
    sx = AIR_CAST(unsigned int, _nin->axis[1].size);
    sy = AIR_CAST(unsigned int, _nin->axis[2].size);
    sz = AIR_CAST(unsigned int, _nin->axis[3].size);
    if (!( refIdx[0] < sx 
           && refIdx[1] < sy
           && refIdx[2] < sz )) {
      fprintf(stderr, "%s: index (%u,%u,%u) out of bounds (%u,%u,%u)\n", me,
              refIdx[0], refIdx[1], refIdx[2], sx, sy, sz);
      airMopError(mop);
      return 1;
    }
    nin = nrrdNew();
    airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);
    numerical = (ptype == tenInterpTypeGeoLoxK
                 || ptype == tenInterpTypeGeoLoxR
                 || ptype == tenInterpTypeLoxK
                 || ptype == tenInterpTypeLoxR
                 || ptype == tenInterpTypeQuatGeoLoxK
                 || ptype == tenInterpTypeQuatGeoLoxR);
    if (numerical) {
      tip->lengthFancy = AIR_TRUE;
      dimOut = 4;
      size[0] = 3;
      size[1] = _nin->axis[1].size;
      size[2] = _nin->axis[2].size;
      size[3] = _nin->axis[3].size;
      axmap[0] = -1;
      axmap[1] = 1;
      axmap[2] = 2;
      axmap[3] = 3;
    } else {
      dimOut = 3;
      size[0] = _nin->axis[1].size;
      size[1] = _nin->axis[2].size;
      size[2] = _nin->axis[3].size;
      axmap[0] = 1;
      axmap[1] = 2;
      axmap[2] = 3;
    }      
    if (nrrdConvert(nin, _nin, nrrdTypeDouble)
        || nrrdMaybeAlloc_nva(nout, nrrdTypeDouble, dimOut, size)
        || nrrdAxisInfoCopy(nout, nin, axmap, 
                            NRRD_AXIS_INFO_SIZE_BIT)
        || nrrdBasicInfoCopy(nout, nin, 
                             (NRRD_BASIC_INFO_DATA_BIT
                              | NRRD_BASIC_INFO_TYPE_BIT
                              | NRRD_BASIC_INFO_DIMENSION_BIT
                              | NRRD_BASIC_INFO_CONTENT_BIT
                              | NRRD_BASIC_INFO_SAMPLEUNITS_BIT))) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble:\n%s\n", me, err);
      airMopError(mop); 
      return 1;
    }
    in = AIR_CAST(double *, nin->data);
    out = AIR_CAST(double *, nout->data);
    TEN_T_COPY(refTen, in + 7*(refIdx[0] + sx*(refIdx[1] + sy*refIdx[2])));
    fprintf(stderr, "!%s: reference tensor = (%g) %g %g %g   %g %g    %g\n",
            me, refTen[0], refTen[1], refTen[2], refTen[3],
            refTen[4], refTen[5], refTen[6]);
    for (zi=0; zi<sz; zi++) {
      for (yi=0; yi<sy; yi++) {
        for (xi=0; xi<sx; xi++) {
          TEN_T_COPY(inTen, in + 7*(xi + sx*(yi + sy*zi)));
          if (numerical) {
            fprintf(stderr, "!%s: %u %u %u \n", me, xi, yi, zi);
            if (inTen[0] < confThresh) {
              out[0] = AIR_NAN;
              out[1] = AIR_NAN;
              out[2] = AIR_NAN;
            } else {
              tip->verbose = 10*(xi == refIdx[0]
                                 && yi == refIdx[1]
                                 && zi == refIdx[2]);
              out[0] =  tenInterpDistanceTwo_d(inTen, refTen, ptype, tip);
              out[1] =  tip->lengthShape;
              out[2] =  tip->lengthOrient;
            }
            out += 3;
          } else {
            if (inTen[0] < confThresh) {
              *out = AIR_NAN;
            } else {
              *out =  tenInterpDistanceTwo_d(inTen, refTen, ptype, tip);
            }
            out += 1;
          }
        }
        if (numerical) {
          if (nrrdSave(outS, nout, NULL)) {
            airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
            fprintf(stderr, "%s: trouble saving output:\n%s\n", me, err);
            airMopError(mop); 
            return 1;
          }
        }
      }
    }
  } else {
    /* only doing the path between two specified tensors */
    ELL_6V_COPY(tA + 1, _tA);
    tA[0] = 1.0;
    TEN_T_SCALE(tA, sclA, tA);
    ELL_6V_COPY(tB + 1, _tB);
    tB[0] = 1.0;
    TEN_T_SCALE(tB, sclB, tB);
    
    ELL_4V_SET(qA, 1, pA[0], pA[1], pA[2]);
    ELL_4V_NORM(qA, qA, tmp);
    ELL_4V_SET(qB, 1, pB[0], pB[1], pB[2]);
    ELL_4V_NORM(qB, qB, tmp);
    ell_q_to_3m_d(rA, qA);
    ell_q_to_3m_d(rB, qB);
    
    TEN_T2M(mat1, tA);
    ell_3m_mul_d(mat2, rA, mat1);
    ELL_3M_TRANSPOSE_IP(rA, tmp);
    ell_3m_mul_d(mat1, mat2, rA);
    TEN_M2T(tA, mat1);
    
    TEN_T2M(mat1, tB);
    ell_3m_mul_d(mat2, rB, mat1);
    ELL_3M_TRANSPOSE_IP(rB, tmp);
    ell_3m_mul_d(mat1, mat2, rB);
    TEN_M2T(tB, mat1);
    /*
      fprintf(stderr, "!%s: tA = (%g) %g %g %g\n    %g %g\n    %g\n", me,
      tA[0], tA[1], tA[2], tA[3], tA[4], tA[5], tA[6]);
      fprintf(stderr, "!%s: tB = (%g) %g %g %g\n    %g %g\n    %g\n", me,
      tB[0], tB[1], tB[2], tB[3], tB[4], tB[5], tB[6]);
    */
    
    time0 = airTime();
    if (tenInterpTwoDiscrete_d(nout, tA, tB, ptype, NN, tip)) {
      airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble computing path:\n%s\n",
              me, err);
      airMopError(mop); 
      return 1;
    }
    fprintf(stderr, "!%s: ------- # iter = %u, conv = %g\n", me,
            tip->numIter, tip->convFinal);
    time1 = airTime();
    fprintf(stderr, "%s: path length = %g; time = %g\n",
            me, tenInterpPathLength(nout, AIR_FALSE, AIR_FALSE, AIR_FALSE),
            time1 - time0);
    
    if (1) {
      double *geod, eval0[3], eval[3], evec0[9], evec[9], rot[9], diff[7],
        nrm, tmp, axis[3], angle;
      unsigned int ii, NN;
      
      NN = AIR_CAST(unsigned int, nout->axis[1].size);
      geod = AIR_CAST(double *, nout->data);
      geod += 7;
      for (ii=1; ii<NN; ii++) {
        double igrad[3][7];
        
        tenEigensolve_d(eval0, evec0, geod-7);
        ELL_3M_TRANSPOSE_IP(evec0, tmp);
        tenEigensolve_d(eval, evec, geod);
        ELL_3M_MUL(rot, evec0, evec);
        angle = ell_3m_to_aa_d(axis, rot);
        TEN_T_SUB(diff, geod, geod-7);
        tenInvariantGradientsK_d(igrad[0], igrad[1], igrad[2], geod, 0);
        nrm = TEN_T_NORM(diff);
        TEN_T_SCALE(diff, 1.0/nrm, diff);
        fprintf(stderr, "%2u %9.6f (%9.6f %9.6f %9.6f) : %9.6f %9.6f %9.6f "
                ": (%9.6f,%9.6f,%9.6f) %g %g %g\n",
                ii, angle, axis[0], axis[1], axis[2],
                TEN_T_DOT(igrad[0], diff),
                TEN_T_DOT(igrad[1], diff),
                TEN_T_DOT(igrad[2], diff),
                nrm, TEN_T_NORM(diff), TEN_T_NORM(igrad[2]),
                eval[0], eval[1], eval[2]);
        geod += 7;
      }
    }

    if (0) {
      double eval[3], evec[9], rot[9], tt[7], qB[4];
      double unitq[8][4] = {{+1, 0, 0, 0},
                            {-1, 0, 0, 0},
                            {0, +1, 0, 0},
                            {0, -1, 0, 0},
                            {0, 0, +1, 0},
                            {0, 0, -1, 0},
                            {0, 0, 0, +1},
                            {0, 0, 0, -1}};
      unsigned qi;
      tenEigensolve_d(eval, evec, tB);
      ELL_3M_TRANSPOSE(rot, evec);
      ell_3m_to_q_d(qB, evec);
      fprintf(stderr, "%s:    tB: (%g) %f %f %f, %f %f, %f; qB = %f %f %f %f\n", me,
              tB[0], tB[1], tB[2], tB[3], tB[4], tB[5], tB[6],
              qB[0], qB[1], qB[2], qB[3]);
      for (qi=0; qi<8; qi++) {
        double qm[4];
        ell_q_mul_d(qm, qB, unitq[qi]);
        ell_q_to_3m_d(rot, qm);
        ELL_3M_TRANSPOSE(evec, rot);
        tenMakeSingle_d(tt, tB[0], eval, evec);
        fprintf(stderr, "%s: tt[%u]: (%g) %f %f %f, %f %f, %f; qm = %f %f %f %f\n", me, qi,
                tt[0], tt[1], tt[2], tt[3], tt[4], tt[5], tt[6],
                qm[0], qm[1], qm[2], qm[3]);
      }
    }
  }

  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving output:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }

  airMopOkay(mop);
  return 0;
}
