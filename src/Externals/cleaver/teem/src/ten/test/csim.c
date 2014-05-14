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

static int
csimDo(double tm[7], double tcov[21], double rm[3], double rv[3],
       Nrrd *ntbuff, tenEstimateContext *tec, double *dwibuff, double sigma,
       double bvalue, double B0, unsigned int NN, int randrot,
       double _tenOrig[7]) {
  char me[]="csimDo", err[BIFF_STRLEN];
  double *tbuff;
  unsigned int II, taa, tbb, cc;

  if (!(ntbuff
        && ntbuff->data
        && 2 == ntbuff->dim
        && 7 == ntbuff->axis[0].size
        && NN == ntbuff->axis[1].size)) {
    sprintf(err, "%s: ntbuff not allocated for 2-by-%u array of %s", me,
            NN, airEnumStr(nrrdType, nrrdTypeDouble));
    biffAdd(TEN, err); return 1;
  }

  /* find all tensors from simulated DWIs */
  tbuff = AIR_CAST(double *, ntbuff->data);
  for (II=0; II<NN; II++) {
    double tenOrig[7], rotf[9], rotb[9], matA[9], matB[9], qq[4], tmp;
    ELL_3M_IDENTITY_SET(rotf); /* sssh warnings */
    ELL_3M_IDENTITY_SET(rotb); /* sssh warnings */

    if (randrot) {
      if (1) {
        double eval[3], evec[9], eps, ma[9], mb[9], rf[9], rb[9];
        tenEigensolve_d(eval, evec, _tenOrig);
        airNormalRand(&eps, NULL);
        ell_aa_to_3m_d(rf, 0*eps/20, evec + 0);
        TEN_T_SCALE_INCR(_tenOrig, 0*eps/30, _tenOrig);
        TEN_T2M(ma, _tenOrig);
        ELL_3M_TRANSPOSE(rb, rf);
        ELL_3M_MUL(mb, ma, rf);
        ELL_3M_MUL(ma, rb, mb);
        TEN_M2T(_tenOrig, ma);
      }
      TEN_T2M(matA, _tenOrig);
      airNormalRand(qq+0, qq+1);
      airNormalRand(qq+2, qq+3);
      ELL_4V_NORM(qq, qq, tmp);
      ell_q_to_3m_d(rotf, qq);
      ELL_3M_TRANSPOSE(rotb, rotf);
      ELL_3M_MUL(matB, matA, rotf);
      ELL_3M_MUL(matA, rotb, matB);
      TEN_M2T(tenOrig, matA);
    } else {
      TEN_T_COPY(tenOrig, _tenOrig);
    }
    if (tenEstimate1TensorSimulateSingle_d(tec, dwibuff, sigma,
                                           bvalue, B0, tenOrig)
        || tenEstimate1TensorSingle_d(tec, tbuff, dwibuff)) {
      sprintf(err, "%s: trouble on exp %u/%u", me, II, NN);
      biffAdd(TEN, err); return 1;
    }
    if (randrot) {
      TEN_T2M(matA, tbuff);
      ELL_3M_MUL(matB, matA, rotb);
      ELL_3M_MUL(matA, rotf, matB);
      TEN_M2T(tbuff, matA);
    } /* else we leave tbuff as it is */
    /*
    if (_tenOrig[0] > 0.5) {
      double tdiff[7];
      TEN_T_SUB(tdiff, _tenOrig, tbuff);
      fprintf(stderr, "!%s: %g\n"
              "         (%g) %g,%g,%g  %g,%g  %g\n"
              "         (%g) %g,%g,%g  %g,%g  %g\n", 
              me, TEN_T_NORM(tdiff),
              _tenOrig[0], _tenOrig[1], _tenOrig[2], _tenOrig[3], _tenOrig[4], 
              _tenOrig[5], _tenOrig[6], 
              tbuff[0], tbuff[1], tbuff[2], tbuff[3], tbuff[4],
              tbuff[5], tbuff[6]);
    }
    */
    tbuff += 7;
  }

  /* find mean tensor, and mean R_i */
  tbuff = AIR_CAST(double *, ntbuff->data);
  TEN_T_SET(tm, 0, 0, 0, 0, 0, 0, 0);
  ELL_3V_SET(rm, 0, 0, 0);
  for (II=0; II<NN; II++) {
    TEN_T_INCR(tm, tbuff);
    rm[0] += sqrt(_tenAnisoTen_d[tenAniso_S](tbuff));
    rm[1] += _tenAnisoTen_d[tenAniso_FA](tbuff);
    rm[2] += _tenAnisoTen_d[tenAniso_Mode](tbuff);
    tbuff += 7;
  }
  rm[0] /= NN;
  rm[1] /= NN;
  rm[2] /= NN;
  TEN_T_SCALE(tm, 1.0/NN, tm);

  /* accumulate covariance tensor, and R_i variances */
  for (cc=0; cc<21; cc++) {
    tcov[cc] = 0;
  }
  ELL_3V_SET(rv, 0, 0, 0);
  tbuff = AIR_CAST(double *, ntbuff->data);
  for (II=0; II<NN; II++) {
    double r[3];
    r[0] = sqrt(_tenAnisoTen_d[tenAniso_S](tbuff));
    r[1] = _tenAnisoTen_d[tenAniso_FA](tbuff);
    r[2] = _tenAnisoTen_d[tenAniso_Mode](tbuff);
    cc = 0;
    rv[0] += (r[0] - rm[0])*(r[0] - rm[0])/(NN-1);
    rv[1] += (r[1] - rm[1])*(r[1] - rm[1])/(NN-1);
    rv[2] += (r[2] - rm[2])*(r[2] - rm[2])/(NN-1);
    for (taa=0; taa<6; taa++) {
      for (tbb=taa; tbb<6; tbb++) {
        tcov[cc] += (10000*(tbuff[taa+1]-tm[taa+1])
                     *10000*(tbuff[tbb+1]-tm[tbb+1])/(NN-1));
        cc++;
      }
    }
    tbuff += 7;
  }

  return 0;
}

char *info = ("does something");

int
main(int argc, char *argv[]) {
  char *me, *err;
  hestOpt *hopt=NULL;
  airArray *mop;

  char *outTenS, *outCovarS, *outRmvS;
  int seed, E;
  unsigned int NN;
  Nrrd *_ninTen, *ninTen, *ngrad, *_ninB0, *ninB0, *nmask,
    *noutCovar, *noutTen, *noutRmv, *ntbuff;
  float sigma, bval;
  size_t sizeX, sizeY, sizeZ;
  tenEstimateContext *tec;
  int axmap[NRRD_DIM_MAX], randrot;

  mop = airMopNew();
  me = argv[0];
  hestOptAdd(&hopt, "i", "ten", airTypeOther, 1, 1, &_ninTen, NULL,
             "input tensor volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "n", "#sim", airTypeUInt, 1, 1, &NN, "100",
             "number of simulations to run");
  hestOptAdd(&hopt, "seed", "seed", airTypeInt, 1, 1, &seed, "42",
             "seed value for RNG which creates noise");
  hestOptAdd(&hopt, "r", "reference field", airTypeOther, 1, 1, &_ninB0, NULL,
             "reference anatomical scan, with no diffusion weighting",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "rr", NULL, airTypeOther, 0, 0, &randrot, NULL,
             "randomize gradient set orientation");
  hestOptAdd(&hopt, "g", "grad list", airTypeOther, 1, 1, &ngrad, "",
             "gradient list, one row per diffusion-weighted image", 
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "b", "b", airTypeFloat, 1, 1, &bval, "1000",
             "b value for simulated scan");
  hestOptAdd(&hopt, "sigma", "sigma", airTypeFloat, 1, 1, &sigma, "0.0",
             "Rician noise parameter");
  hestOptAdd(&hopt, "ot", "filename", airTypeString, 1, 1, &outTenS, 
             "tout.nrrd", "file to write output tensor nrrd to");
  hestOptAdd(&hopt, "oc", "filename", airTypeString, 1, 1, &outCovarS, 
             "cout.nrrd", "file to write output covariance nrrd to");
  hestOptAdd(&hopt, "or", "filename", airTypeString, 1, 1, &outRmvS, 
             "rout.nrrd", "file to write output R_i means, variances to");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (tenGradientCheck(ngrad, nrrdTypeDefault, 7)) {
    airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: problem with gradient list:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  if (tenTensorCheck(_ninTen, nrrdTypeDefault, AIR_TRUE, AIR_TRUE)) {
    airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: didn't like input:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  sizeX = _ninTen->axis[1].size;
  sizeY = _ninTen->axis[2].size;
  sizeZ = _ninTen->axis[3].size;
  if (!(3 == _ninB0->dim &&
        sizeX == _ninB0->axis[0].size &&
        sizeY == _ninB0->axis[1].size &&
        sizeZ == _ninB0->axis[2].size)) {
    fprintf(stderr, "%s: given B0 (%u-D) volume not 3-D " _AIR_SIZE_T_CNV
            "x" _AIR_SIZE_T_CNV "x" _AIR_SIZE_T_CNV, me, _ninB0->dim,
            sizeX, sizeY, sizeZ);
    airMopError(mop); 
    return 1;
  }

  ninTen = nrrdNew();
  airMopAdd(mop, ninTen, (airMopper)nrrdNuke, airMopOnError);
  nmask = nrrdNew();
  airMopAdd(mop, nmask, (airMopper)nrrdNuke, airMopOnError);
  ninB0 = nrrdNew();
  airMopAdd(mop, ninB0, (airMopper)nrrdNuke, airMopOnError);
  noutCovar = nrrdNew();
  airMopAdd(mop, noutCovar, (airMopper)nrrdNuke, airMopOnError);
  noutTen = nrrdNew();
  airMopAdd(mop, noutTen, (airMopper)nrrdNuke, airMopOnError);
  noutRmv = nrrdNew();
  airMopAdd(mop, noutRmv, (airMopper)nrrdNuke, airMopOnError);
  ntbuff = nrrdNew();
  airMopAdd(mop, ntbuff, (airMopper)nrrdNuke, airMopOnError);

  if (nrrdConvert(ninTen, _ninTen, nrrdTypeDouble)
      || nrrdSlice(nmask, ninTen, 0, 0)
      || nrrdConvert(ninB0, _ninB0, nrrdTypeDouble)
      || nrrdMaybeAlloc_va(noutTen, nrrdTypeDouble, 4, 
                           AIR_CAST(size_t, 7), sizeX, sizeY, sizeZ)
      || nrrdMaybeAlloc_va(noutCovar, nrrdTypeDouble, 4, 
                           AIR_CAST(size_t, 21), sizeX, sizeY, sizeZ)
      || nrrdMaybeAlloc_va(noutRmv, nrrdTypeDouble, 4, 
                           AIR_CAST(size_t, 6), sizeX, sizeY, sizeZ)
      || nrrdMaybeAlloc_va(ntbuff, nrrdTypeDouble, 2,
                           AIR_CAST(size_t, 7), NN)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble setting up tec:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  tec = tenEstimateContextNew();
  airMopAdd(mop, tec, (airMopper)tenEstimateContextNix, airMopAlways);

  E = 0;
  if (!E) E |= tenEstimateMethodSet(tec, tenEstimate1MethodLLS);
  if (!E) E |= tenEstimateValueMinSet(tec, 0.000000001);
  if (!E) E |= tenEstimateGradientsSet(tec, ngrad, bval, AIR_TRUE);
  if (!E) E |= tenEstimateThresholdSet(tec, 0, 0);
  if (!E) E |= tenEstimateUpdate(tec);
  if (E) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble setting up tec:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  airSrandMT(seed);

  fprintf(stderr, "!%s: randrot = %d\n", me, randrot);
  if (1) {
    unsigned int II;
    unsigned int nsamp;
    double *inTen, *outTen, *outCovar, *outRmv, 
      *dwibuff, (*lup)(const void *, size_t);
    char doneStr[AIR_STRLEN_SMALL];

    dwibuff = AIR_CAST(double *, calloc(ngrad->axis[1].size, sizeof(double)));
    airMopAdd(mop, dwibuff, airFree, airMopAlways);
    nsamp = sizeX*sizeY*sizeZ;
    inTen = AIR_CAST(double *, ninTen->data);
    lup  = nrrdDLookup[nrrdTypeDouble];
    outTen = AIR_CAST(double *, noutTen->data);
    outCovar = AIR_CAST(double *, noutCovar->data);
    outRmv = AIR_CAST(double *, noutRmv->data);
    fprintf(stderr, "!%s: simulating ...       ", me);
    fflush(stderr);
    for (II=0; II<nsamp; II++) {
      if (!(II % sizeX)) {
        fprintf(stderr, "%s", airDoneStr(0, II, nsamp, doneStr));
        fflush(stderr);
      }
      if (csimDo(outTen, outCovar, outRmv + 0, outRmv + 3, ntbuff,
                 tec, dwibuff, sigma,
                 bval, lup(ninB0->data, II), NN, randrot, inTen)) {
        airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble:\n%s\n", me, err);
        airMopError(mop);
        return 1;
      }
      inTen += 7;
      outTen += 7;
      outCovar += 21;
      outRmv += 6;
    }
    fprintf(stderr, "%s\n", airDoneStr(0, II, nsamp, doneStr));
  }

  axmap[0] = -1;
  axmap[1] = 1;
  axmap[2] = 2;
  axmap[3] = 3;
  if (nrrdSplice(noutTen, noutTen, nmask, 0, 0)
      || nrrdAxisInfoCopy(noutTen, ninTen, axmap, NRRD_AXIS_INFO_SIZE_BIT)
      || nrrdAxisInfoCopy(noutCovar, ninTen, axmap, NRRD_AXIS_INFO_SIZE_BIT)
      || nrrdAxisInfoCopy(noutRmv, ninTen, axmap, NRRD_AXIS_INFO_SIZE_BIT)
      || nrrdBasicInfoCopy(noutTen, ninTen,
                           NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)
      || nrrdBasicInfoCopy(noutCovar, ninTen,
                           NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)
      || nrrdBasicInfoCopy(noutRmv, ninTen,
                           NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)
      || nrrdSave(outTenS, noutTen, NULL)
      || nrrdSave(outCovarS, noutCovar, NULL)
      || nrrdSave(outRmvS, noutRmv, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving output:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }

  airMopOkay(mop);
  return 0;
}
