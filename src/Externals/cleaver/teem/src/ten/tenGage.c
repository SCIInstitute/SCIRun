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

#include "ten.h"
#include "privateTen.h"

typedef struct {
  double *buffTen, *buffWght;
  tenInterpParm *tip;  /* sneakiness: using tip->allocLen to record
                          allocation sizes of buffTen and buffWght, too */
} _tenGagePvlData;

gageItemEntry
_tenGageTable[TEN_GAGE_ITEM_MAX+1] = {
  /* enum value                  len,deriv, prereqs,                                                   parent item, parent index, needData */
  {tenGageUnknown,                 0,  0,  {0},                                                                  0,        0,     AIR_FALSE},
  {tenGageTensor,                  7,  0,  {0},                                                                  0,        0,     AIR_FALSE},
  {tenGageConfidence,              1,  0,  {tenGageTensor},                                          tenGageTensor,        0,     AIR_FALSE},

  {tenGageTrace,                   1,  0,  {tenGageTensor},                                                      0,        0,     AIR_FALSE},
  {tenGageNorm,                    1,  0,  {tenGageTensor},                                                      0,        0,     AIR_FALSE},
  {tenGageB,                       1,  0,  {tenGageTensor},                                                      0,        0,     AIR_FALSE},
  {tenGageDet,                     1,  0,  {tenGageTensor},                                                      0,        0,     AIR_FALSE},
  {tenGageS,                       1,  0,  {tenGageTensor},                                                      0,        0,     AIR_FALSE},
  {tenGageQ,                       1,  0,  {tenGageS, tenGageB},                                                 0,        0,     AIR_FALSE},
  {tenGageFA,                      1,  0,  {tenGageQ, tenGageS},                                                 0,        0,     AIR_FALSE},
  {tenGageR,                       1,  0,  {tenGageTrace, tenGageB, tenGageDet, tenGageS},                       0,        0,     AIR_FALSE},
  {tenGageMode,                    1,  0,  {tenGageR, tenGageQ},                                                 0,        0,     AIR_FALSE},
  {tenGageTheta,                   1,  0,  {tenGageMode},                                                        0,        0,     AIR_FALSE},
  {tenGageModeWarp,                1,  0,  {tenGageMode},                                                        0,        0,     AIR_FALSE},
  {tenGageOmega,                   1,  0,  {tenGageFA, tenGageMode},                                             0,        0,     AIR_FALSE},

  {tenGageEval,                    3,  0,  {tenGageTensor},                                                      0,        0,     AIR_FALSE},
  {tenGageEval0,                   1,  0,  {tenGageEval},                                              tenGageEval,        0,     AIR_FALSE},
  {tenGageEval1,                   1,  0,  {tenGageEval},                                              tenGageEval,        1,     AIR_FALSE},
  {tenGageEval2,                   1,  0,  {tenGageEval},                                              tenGageEval,        2,     AIR_FALSE},
  {tenGageEvec,                    9,  0,  {tenGageTensor},                                                      0,        0,     AIR_FALSE},
  {tenGageEvec0,                   3,  0,  {tenGageEvec},                                              tenGageEvec,        0,     AIR_FALSE},
  {tenGageEvec1,                   3,  0,  {tenGageEvec},                                              tenGageEvec,        3,     AIR_FALSE},
  {tenGageEvec2,                   3,  0,  {tenGageEvec},                                              tenGageEvec,        6,     AIR_FALSE},

  {tenGageDelNormK2,               7,  0,  {tenGageTensor},                                                      0,        0,     AIR_FALSE},
  {tenGageDelNormK3,               7,  0,  {tenGageTensor},                                                      0,        0,     AIR_FALSE},
  {tenGageDelNormR1,               7,  0,  {tenGageTensor},                                                      0,        0,     AIR_FALSE},
  {tenGageDelNormR2,               7,  0,  {tenGageTensor},                                                      0,        0,     AIR_FALSE},

  {tenGageDelNormPhi1,             7,  0,  {tenGageEvec},                                                        0,        0,     AIR_FALSE},
  {tenGageDelNormPhi2,             7,  0,  {tenGageEvec},                                                        0,        0,     AIR_FALSE},
  {tenGageDelNormPhi3,             7,  0,  {tenGageEvec},                                                        0,        0,     AIR_FALSE},

  {tenGageTensorGrad,             21,  1,  {0},                                                                  0,        0,     AIR_FALSE},
  {tenGageTensorGradMag,           3,  1,  {tenGageTensorGrad},                                                  0,        0,     AIR_FALSE},
  {tenGageTensorGradMagMag,        1,  1,  {tenGageTensorGradMag},                                               0,        0,     AIR_FALSE},

  {tenGageTraceGradVec,            3,  1,  {tenGageTensor, tenGageTensorGrad},                                   0,        0,     AIR_FALSE},
  {tenGageTraceGradMag,            1,  1,  {tenGageTraceGradVec},                                                0,        0,     AIR_FALSE},
  {tenGageTraceNormal,             3,  1,  {tenGageTraceGradVec, tenGageTraceGradMag},                           0,        0,     AIR_FALSE},

  {tenGageNormGradVec,             3,  1,  {tenGageNorm, tenGageSGradVec},                                       0,        0,     AIR_FALSE},
  {tenGageNormGradMag,             1,  1,  {tenGageNormGradVec},                                                 0,        0,     AIR_FALSE},
  {tenGageNormNormal,              3,  1,  {tenGageNormGradVec, tenGageNormGradMag},                             0,        0,     AIR_FALSE},

  {tenGageBGradVec,                3,  1,  {tenGageTensor, tenGageTensorGrad},                                   0,        0,     AIR_FALSE},
  {tenGageBGradMag,                1,  1,  {tenGageBGradVec},                                                    0,        0,     AIR_FALSE},
  {tenGageBNormal,                 3,  1,  {tenGageBGradVec, tenGageBGradMag},                                   0,        0,     AIR_FALSE},

  {tenGageDetGradVec,              3,  1,  {tenGageTensor, tenGageTensorGrad},                                   0,        0,     AIR_FALSE},
  {tenGageDetGradMag,              1,  1,  {tenGageDetGradVec},                                                  0,        0,     AIR_FALSE},
  {tenGageDetNormal,               3,  1,  {tenGageDetGradVec, tenGageDetGradMag},                               0,        0,     AIR_FALSE},

  {tenGageSGradVec,                3,  1,  {tenGageTensor, tenGageTensorGrad},                                   0,        0,     AIR_FALSE},
  {tenGageSGradMag,                1,  1,  {tenGageSGradVec},                                                    0,        0,     AIR_FALSE},
  {tenGageSNormal,                 3,  1,  {tenGageSGradVec, tenGageSGradMag},                                   0,        0,     AIR_FALSE},

  {tenGageQGradVec,                3,  1,  {tenGageSGradVec, tenGageBGradVec},                                   0,        0,     AIR_FALSE},
  {tenGageQGradMag,                1,  1,  {tenGageQGradVec},                                                    0,        0,     AIR_FALSE},
  {tenGageQNormal,                 3,  1,  {tenGageQGradVec, tenGageQGradMag},                                   0,        0,     AIR_FALSE},

  {tenGageFAGradVec,               3,  1,  {tenGageQGradVec, tenGageSGradVec, tenGageFA},                        0,        0,     AIR_FALSE},
  {tenGageFAGradMag,               1,  1,  {tenGageFAGradVec},                                                   0,        0,     AIR_FALSE},
  {tenGageFANormal,                3,  1,  {tenGageFAGradVec, tenGageFAGradMag},                                 0,        0,     AIR_FALSE},

  {tenGageRGradVec,                3,  1,  {tenGageR, tenGageTraceGradVec, tenGageBGradVec,
                                            tenGageDetGradVec, tenGageSGradVec},                                 0,        0,     AIR_FALSE},
  {tenGageRGradMag,                1,  1,  {tenGageRGradVec},                                                    0,        0,     AIR_FALSE},
  {tenGageRNormal,                 3,  1,  {tenGageRGradVec, tenGageRGradMag},                                   0,        0,     AIR_FALSE},

  {tenGageModeGradVec,             3,  1,  {tenGageRGradVec, tenGageQGradVec, tenGageMode},                      0,        0,     AIR_FALSE},
  {tenGageModeGradMag,             1,  1,  {tenGageModeGradVec},                                                 0,        0,     AIR_FALSE},
  {tenGageModeNormal,              3,  1,  {tenGageModeGradVec, tenGageModeGradMag},                             0,        0,     AIR_FALSE},
  
  {tenGageThetaGradVec,            3,  1,  {tenGageRGradVec, tenGageQGradVec, tenGageTheta},                     0,        0,     AIR_FALSE},
  {tenGageThetaGradMag,            1,  1,  {tenGageThetaGradVec},                                                0,        0,     AIR_FALSE},
  {tenGageThetaNormal,             3,  1,  {tenGageThetaGradVec, tenGageThetaGradMag},                           0,        0,     AIR_FALSE},
  
  {tenGageOmegaGradVec,            3,  1,  {tenGageFA, tenGageMode, tenGageFAGradVec, tenGageModeGradVec},       0,        0,     AIR_FALSE},
  {tenGageOmegaGradMag,            1,  1,  {tenGageOmegaGradVec},                                                0,        0,     AIR_FALSE},
  {tenGageOmegaNormal,             3,  1,  {tenGageOmegaGradVec, tenGageOmegaGradMag},                           0,        0,     AIR_FALSE},
  
  {tenGageInvarKGrads,             9,  1,  {tenGageDelNormK2, tenGageDelNormK3, tenGageTensorGrad},              0,        0,     AIR_FALSE},
  {tenGageInvarKGradMags,          3,  1,  {tenGageInvarKGrads},                                                 0,        0,     AIR_FALSE},
  {tenGageInvarRGrads,             9,  1,  {tenGageDelNormR1, tenGageDelNormR2, tenGageDelNormK3,
                                            tenGageTensorGrad},                                                  0,        0,     AIR_FALSE},
  {tenGageInvarRGradMags,          3,  1,  {tenGageInvarRGrads},                                                 0,        0,     AIR_FALSE},

  {tenGageRotTans,                 9,  1,  {tenGageDelNormPhi1, tenGageDelNormPhi2, tenGageDelNormPhi3,
                                            tenGageTensorGrad},                                                  0,        0,     AIR_FALSE},
  {tenGageRotTanMags,              3,  1,  {tenGageRotTans},                                                     0,        0,     AIR_FALSE},

  {tenGageEvalGrads,               9,  1,  {tenGageTensorGrad, tenGageEval, tenGageEvec},                        0,        0,     AIR_FALSE},

  {tenGageCl1,                     1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2},            0,        0,     AIR_FALSE},
  {tenGageCp1,                     1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2},            0,        0,     AIR_FALSE},
  {tenGageCa1,                     1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2},            0,        0,     AIR_FALSE},
  {tenGageClpmin1,                 1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2},            0,        0,     AIR_FALSE},
  {tenGageCl2,                     1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2},            0,        0,     AIR_FALSE},
  {tenGageCp2,                     1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2},            0,        0,     AIR_FALSE},
  {tenGageCa2,                     1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2},            0,        0,     AIR_FALSE},
  {tenGageClpmin2,                 1,  0,  {tenGageTensor, tenGageEval0, tenGageEval1, tenGageEval2},            0,        0,     AIR_FALSE},

  {tenGageHessian,                63,  2,  {0},                                                                  0,        0,     AIR_FALSE},
  {tenGageTraceHessian,            9,  2,  {tenGageHessian},                                                     0,        0,     AIR_FALSE},
  {tenGageBHessian,                9,  2,  {tenGageTensor, tenGageTensorGrad, tenGageHessian},                   0,        0,     AIR_FALSE},
  {tenGageDetHessian,              9,  2,  {tenGageTensor, tenGageTensorGrad, tenGageHessian},                   0,        0,     AIR_FALSE},
  {tenGageSHessian,                9,  2,  {tenGageTensor, tenGageTensorGrad, tenGageHessian},                   0,        0,     AIR_FALSE},
  {tenGageQHessian,                9,  2,  {tenGageBHessian, tenGageSHessian},                                   0,        0,     AIR_FALSE},

  {tenGageFAHessian,               9,  2,  {tenGageSHessian, tenGageQHessian,
                                            tenGageSGradVec, tenGageQGradVec, tenGageFA},                        0,        0,     AIR_FALSE},
  {tenGageFAHessianEval,           3,  2,  {tenGageFAHessian},                                                   0,        0,     AIR_FALSE},
  {tenGageFAHessianEval0,          1,  2,  {tenGageFAHessianEval},                            tenGageFAHessianEval,        0,     AIR_FALSE},
  {tenGageFAHessianEval1,          1,  2,  {tenGageFAHessianEval},                            tenGageFAHessianEval,        1,     AIR_FALSE},
  {tenGageFAHessianEval2,          1,  2,  {tenGageFAHessianEval},                            tenGageFAHessianEval,        2,     AIR_FALSE},
  {tenGageFAHessianEvec,           9,  2,  {tenGageFAHessian},                                                   0,        0,     AIR_FALSE},
  {tenGageFAHessianEvec0,          3,  2,  {tenGageFAHessianEvec},                            tenGageFAHessianEvec,        0,     AIR_FALSE},
  {tenGageFAHessianEvec1,          3,  2,  {tenGageFAHessianEvec},                            tenGageFAHessianEvec,        3,     AIR_FALSE},
  {tenGageFAHessianEvec2,          3,  2,  {tenGageFAHessianEvec},                            tenGageFAHessianEvec,        6,     AIR_FALSE},
  {tenGageFARidgeSurfaceStrength,  1,  2,  {tenGageConfidence, tenGageFAHessianEval},                            0,        0,     AIR_FALSE},
  {tenGageFAValleySurfaceStrength, 1,  2,  {tenGageConfidence, tenGageFAHessianEval},                            0,        0,     AIR_FALSE},
  {tenGageFALaplacian,             1,  2,  {tenGageFAHessian},                                                   0,        0,     AIR_FALSE},
  {tenGageFAHessianEvalMode,       1,  2,  {tenGageFAHessianEval},                                               0,        0,     AIR_FALSE},
  {tenGageFA2ndDD,                 1,  2,  {tenGageFAHessian, tenGageFANormal},                                  0,        0,     AIR_FALSE},

  {tenGageFAGeomTens,              9,  2,  {tenGageFAHessian, tenGageFAGradMag, tenGageFANormal},                0,        0,     AIR_FALSE},
  {tenGageFAKappa1,                1,  2,  {tenGageFAGeomTens},                                                  0,        0,     AIR_FALSE},
  {tenGageFAKappa2,                1,  2,  {tenGageFAGeomTens},                                                  0,        0,     AIR_FALSE},
  {tenGageFATotalCurv,             1,  2,  {tenGageFAGeomTens},                                                  0,        0,     AIR_FALSE},
  {tenGageFAShapeIndex,            1,  2,  {tenGageFAKappa1, tenGageFAKappa2},                                   0,        0,     AIR_FALSE},
  {tenGageFAMeanCurv,              1,  2,  {tenGageFAKappa1, tenGageFAKappa2},                                   0,        0,     AIR_FALSE},
  {tenGageFAGaussCurv,             1,  2,  {tenGageFAKappa1, tenGageFAKappa2},                                   0,        0,     AIR_FALSE},
  {tenGageFACurvDir1,              3,  2,  {tenGageFAGeomTens, tenGageFAKappa2},                                 0,        0,     AIR_FALSE},
  {tenGageFACurvDir2,              3,  2,  {tenGageFAGeomTens, tenGageFAKappa1},                                 0,        0,     AIR_FALSE},
  {tenGageFAFlowlineCurv,          1,  2,  {tenGageFANormal, tenGageFAHessian, tenGageFAGradMag},                0,        0,     AIR_FALSE},

  {tenGageRHessian,                9,  2,  {tenGageR, tenGageRGradVec, tenGageTraceHessian,
                                            tenGageBHessian, tenGageDetHessian, tenGageSHessian},                0,        0,     AIR_FALSE},

  {tenGageModeHessian,             9,  2,  {tenGageR, tenGageQ, tenGageRGradVec, tenGageQGradVec,
                                            tenGageRHessian, tenGageQHessian},                                   0,        0,     AIR_FALSE},
  {tenGageModeHessianEval,         3,  2,  {tenGageModeHessian},                                                 0,        0,     AIR_FALSE},
  {tenGageModeHessianEval0,        1,  2,  {tenGageModeHessianEval},                        tenGageModeHessianEval,        0,     AIR_FALSE},
  {tenGageModeHessianEval1,        1,  2,  {tenGageModeHessianEval},                        tenGageModeHessianEval,        1,     AIR_FALSE},
  {tenGageModeHessianEval2,        1,  2,  {tenGageModeHessianEval},                        tenGageModeHessianEval,        2,     AIR_FALSE},
  {tenGageModeHessianEvec,         9,  2,  {tenGageModeHessian},                                                 0,        0,     AIR_FALSE},
  {tenGageModeHessianEvec0,        3,  2,  {tenGageModeHessianEvec},                        tenGageModeHessianEvec,        0,     AIR_FALSE},
  {tenGageModeHessianEvec1,        3,  2,  {tenGageModeHessianEvec},                        tenGageModeHessianEvec,        3,     AIR_FALSE},
  {tenGageModeHessianEvec2,        3,  2,  {tenGageModeHessianEvec},                        tenGageModeHessianEvec,        6,     AIR_FALSE},

  {tenGageOmegaHessian,            9,  2,  {tenGageFA, tenGageMode, tenGageFAGradVec, tenGageModeGradVec,
                                            tenGageFAHessian, tenGageModeHessian},                               0,        0,     AIR_FALSE},
  {tenGageOmegaHessianEval,        3,  2,  {tenGageOmegaHessian},                                                0,        0,     AIR_FALSE},
  {tenGageOmegaHessianEval0,       1,  2,  {tenGageOmegaHessianEval},                      tenGageOmegaHessianEval,        0,     AIR_FALSE},
  {tenGageOmegaHessianEval1,       1,  2,  {tenGageOmegaHessianEval},                      tenGageOmegaHessianEval,        1,     AIR_FALSE},
  {tenGageOmegaHessianEval2,       1,  2,  {tenGageOmegaHessianEval},                      tenGageOmegaHessianEval,        2,     AIR_FALSE},
  {tenGageOmegaHessianEvec,        9,  2,  {tenGageOmegaHessian},                                                0,        0,     AIR_FALSE},
  {tenGageOmegaHessianEvec0,       3,  2,  {tenGageOmegaHessianEvec},                      tenGageOmegaHessianEvec,        0,     AIR_FALSE},
  {tenGageOmegaHessianEvec1,       3,  2,  {tenGageOmegaHessianEvec},                      tenGageOmegaHessianEvec,        3,     AIR_FALSE},
  {tenGageOmegaHessianEvec2,       3,  2,  {tenGageOmegaHessianEvec},                      tenGageOmegaHessianEvec,        6,     AIR_FALSE},
  {tenGageOmegaLaplacian,          1,  2,  {tenGageOmegaHessian},                                                0,        0,     AIR_FALSE},
  {tenGageOmega2ndDD,              1,  2,  {tenGageOmegaHessian, tenGageOmegaNormal},                            0,        0,     AIR_FALSE},

  {tenGageTraceGradVecDotEvec0,    1,  1,  {tenGageTraceGradVec, tenGageEvec0},                                  0,        0,     AIR_FALSE},
  {tenGageTraceDiffusionAlign,     1,  1,  {tenGageTraceNormal, tenGageEvec0},                                   0,        0,     AIR_FALSE},
  {tenGageTraceDiffusionFraction,  1,  1,  {tenGageTraceNormal, tenGageTensor},                                  0,        0,     AIR_FALSE},

  {tenGageFAGradVecDotEvec0,       1,  1,  {tenGageFAGradVec, tenGageEvec0},                                     0,        0,     AIR_FALSE},
  {tenGageFADiffusionAlign,        1,  1,  {tenGageFANormal, tenGageEvec0},                                      0,        0,     AIR_FALSE},
  {tenGageFADiffusionFraction,     1,  1,  {tenGageFANormal, tenGageTensor},                                     0,        0,     AIR_FALSE},

  {tenGageOmegaGradVecDotEvec0,    1,  1,  {tenGageOmegaGradVec, tenGageEvec0},                                  0,        0,     AIR_FALSE},
  {tenGageOmegaDiffusionAlign,     1,  1,  {tenGageOmegaNormal, tenGageEvec0},                                   0,        0,     AIR_FALSE},
  {tenGageOmegaDiffusionFraction,  1,  1,  {tenGageOmegaNormal, tenGageTensor},                                  0,        0,     AIR_FALSE},

  /* currently don't have tenGageConfGradVec */
  {tenGageConfGradVecDotEvec0,     1,  1,  {tenGageTensorGrad, tenGageEvec0},                                    0,        0,     AIR_FALSE},
  {tenGageConfDiffusionAlign,      1,  1,  {tenGageTensorGrad, tenGageEvec0},                                    0,        0,     AIR_FALSE},
  {tenGageConfDiffusionFraction,   1,  1,  {tenGageTensorGrad, tenGageTensor},                                   0,        0,     AIR_FALSE},

  {tenGageCovariance,             21,  0,  {tenGageTensor}, /* and all the values in iv3 */                      0,        0,     AIR_FALSE},
  {tenGageCovarianceRGRT,         21,  0,  {tenGageCovariance,
                                            tenGageDelNormR1, tenGageDelNormR2, tenGageDelNormK3,
                                            tenGageDelNormPhi1, tenGageDelNormPhi2, tenGageDelNormPhi3},         0,        0,     AIR_FALSE},
  {tenGageCovarianceKGRT,         21,  0,  {tenGageCovariance,
                                            tenGageDelNormK2, tenGageDelNormK3,
                                            tenGageDelNormPhi1, tenGageDelNormPhi2, tenGageDelNormPhi3},         0,        0,     AIR_FALSE},

  {tenGageTensorLogEuclidean,      7,  0,  {0},                                                                  0,        0,     AIR_FALSE},
  {tenGageTensorQuatGeoLoxK,       7,  0,  {0},                                                                  0,        0,     AIR_FALSE},
  {tenGageTensorQuatGeoLoxR,       7,  0,  {0},                                                                  0,        0,     AIR_FALSE},

  {tenGageAniso,     TEN_ANISO_MAX+1,  0,  {tenGageEval0, tenGageEval1, tenGageEval2},                           0,        0,     AIR_FALSE},

  {tenGageCl1GradVec,              3,  1,  {tenGageTrace, tenGageEval, tenGageEvalGrads},                        0,        0,     AIR_FALSE},
  {tenGageCl1GradMag,              1,  1,  {tenGageCl1GradVec},                                                  0,        0,     AIR_FALSE},
  {tenGageCp1GradVec,              3,  1,  {tenGageTrace, tenGageEval, tenGageEvalGrads},                        0,        0,     AIR_FALSE},
  {tenGageCp1GradMag,              1,  1,  {tenGageCp1GradVec},                                                  0,        0,     AIR_FALSE},
  {tenGageCa1GradVec,              3,  1,  {tenGageTrace, tenGageEval, tenGageEvalGrads},                        0,        0,     AIR_FALSE},
  {tenGageCa1GradMag,              1,  1,  {tenGageCa1GradVec},                                                  0,        0,     AIR_FALSE},
  {tenGageTensorGradRotE, 21, 1, {tenGageTensorGrad, tenGageEval, tenGageEvec}, 0, 0, AIR_FALSE}
};

void
_tenGageIv3Print(FILE *file, gageContext *ctx, gagePerVolume *pvl) {
  double *iv3;
  int i, fd;

  fd = 2*ctx->radius;
  iv3 = pvl->iv3 + fd*fd*fd;
  fprintf(file, "iv3[]'s *Dxx* component:\n");
  switch(fd) {
  case 2:
    fprintf(file, "% 10.4f   % 10.4f\n", (float)iv3[6], (float)iv3[7]);
    fprintf(file, "   % 10.4f   % 10.4f\n\n", (float)iv3[4], (float)iv3[5]);
    fprintf(file, "% 10.4f   % 10.4f\n", (float)iv3[2], (float)iv3[3]);
    fprintf(file, "   % 10.4f   % 10.4f\n", (float)iv3[0], (float)iv3[1]);
    break;
  case 4:
    for (i=3; i>=0; i--) {
      fprintf(file, "% 10.4f   % 10.4f   % 10.4f   % 10.4f\n", 
              (float)iv3[12+16*i], (float)iv3[13+16*i], 
              (float)iv3[14+16*i], (float)iv3[15+16*i]);
      fprintf(file, "   % 10.4f  %c% 10.4f   % 10.4f%c   % 10.4f\n", 
              (float)iv3[ 8+16*i], (i==1||i==2)?'\\':' ',
              (float)iv3[ 9+16*i], (float)iv3[10+16*i], (i==1||i==2)?'\\':' ',
              (float)iv3[11+16*i]);
      fprintf(file, "      % 10.4f  %c% 10.4f   % 10.4f%c   % 10.4f\n", 
              (float)iv3[ 4+16*i], (i==1||i==2)?'\\':' ',
              (float)iv3[ 5+16*i], (float)iv3[ 6+16*i], (i==1||i==2)?'\\':' ',
              (float)iv3[ 7+16*i]);
      fprintf(file, "         % 10.4f   % 10.4f   % 10.4f   % 10.4f\n", 
              (float)iv3[ 0+16*i], (float)iv3[ 1+16*i],
              (float)iv3[ 2+16*i], (float)iv3[ 3+16*i]);
      if (i) fprintf(file, "\n");
    }
    break;
  default:
    for (i=0; i<fd*fd*fd; i++) {
      fprintf(file, "  iv3[% 3d,% 3d,% 3d] = % 10.4f\n",
              i%fd, (i/fd)%fd, i/(fd*fd), (float)iv3[i]);
    }
    break;
  }
  return;
}

void
_tenGageFilter(gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_tenGageFilter";
  double *fw00, *fw11, *fw22, *ten, *tgrad, *thess;
  int fd;
  gageScl3PFilter_t *filter[5] = {NULL, gageScl3PFilter2, gageScl3PFilter4,
                                  gageScl3PFilter6, gageScl3PFilter8};
  unsigned int valIdx;

  fd = 2*ctx->radius;
  ten = pvl->directAnswer[tenGageTensor];
  tgrad = pvl->directAnswer[tenGageTensorGrad];
  thess = pvl->directAnswer[tenGageHessian];
  if (!ctx->parm.k3pack) {
    fprintf(stderr, "!%s: sorry, 6pack filtering not implemented\n", me);
    return;
  }
  fw00 = ctx->fw + fd*3*gageKernel00;
  fw11 = ctx->fw + fd*3*gageKernel11;
  fw22 = ctx->fw + fd*3*gageKernel22;
  /* perform the filtering */
  /* HEY: we still want trilinear interpolation of confidence, no? */
  if (fd <= 8) {
    for (valIdx=0; valIdx<7; valIdx++) {
      filter[ctx->radius](ctx->shape,
                          pvl->iv3 + valIdx*fd*fd*fd,
                          pvl->iv2 + valIdx*fd*fd,
                          pvl->iv1 + valIdx*fd,
                          fw00, fw11, fw22,
                          ten + valIdx, tgrad + valIdx*3, thess + valIdx*9,
                          pvl->needD[0], pvl->needD[1], pvl->needD[2]);
    }
  } else {
    for (valIdx=0; valIdx<7; valIdx++) {
      gageScl3PFilterN(ctx->shape, fd,
                       pvl->iv3 + valIdx*fd*fd*fd,
                       pvl->iv2 + valIdx*fd*fd,
                       pvl->iv1 + valIdx*fd,
                       fw00, fw11, fw22,
                       ten + valIdx, tgrad + valIdx*3, thess + valIdx*9,
                       pvl->needD[0], pvl->needD[1], pvl->needD[2]);
    }
  }

  return;
}

void
_tenGageAnswer(gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_tenGageAnswer";
  double *tenAns, *evalAns, *evecAns, *vecTmp=NULL, *matTmp=NULL,
    *gradDtA=NULL, *gradDtB=NULL, *gradDtC=NULL,
    *gradDtD=NULL, *gradDtE=NULL, *gradDtF=NULL,
    *hessDtA=NULL, *hessDtB=NULL, *hessDtC=NULL,
    *hessDtD=NULL, *hessDtE=NULL, *hessDtF=NULL,
    *gradCbS=NULL, *gradCbB=NULL, *gradCbQ=NULL, *gradCbR=NULL,
    *hessCbS=NULL, *hessCbB=NULL, *hessCbQ=NULL, *hessCbR=NULL,
    gradDdXYZ[21]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  double tmp0, tmp1, tmp2, magTmp=0,
    dtA=0, dtB=0, dtC=0, dtD=0, dtE=0, dtF=0,
    cbQ=0, cbR=0, cbA=0, cbB=0, cbC=0, cbS=0,
    gradCbA[3]={0,0,0}, gradCbC[3]={0,0,0};
  double hessCbA[9]={0,0,0,0,0,0,0,0,0},
    hessCbC[9]={0,0,0,0,0,0,0,0,0};
  int ci;

  tenAns = pvl->directAnswer[tenGageTensor];
  evalAns = pvl->directAnswer[tenGageEval];
  evecAns = pvl->directAnswer[tenGageEvec];
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensor)) {
    /* done if doV */
    /* HEY: this was prohibiting a Deft-related hack 
    tenAns[0] = AIR_CLAMP(0, tenAns[0], 1);
    */
    /* HEY: and this was botching using 1-conf as potential energy for push 
    tenAns[0] = AIR_MAX(0, tenAns[0]);
    */
    dtA = tenAns[1];
    dtB = tenAns[2];
    dtC = tenAns[3];
    dtD = tenAns[4];
    dtE = tenAns[5];
    dtF = tenAns[6];
    if (ctx->verbose) {
      fprintf(stderr, "!%s: tensor = (%g) %g %g %g   %g %g   %g\n", me,
              tenAns[0], dtA, dtB, dtC, dtD, dtE, dtF);
    }
  }
  /* done if doV 
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageConfidence)) {
  }
  */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTrace)) {
    cbA = -(pvl->directAnswer[tenGageTrace][0] = dtA + dtD + dtF);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageNorm)) {
    pvl->directAnswer[tenGageNorm][0] = 
      sqrt(dtA*dtA + dtD*dtD + dtF*dtF + 2*dtB*dtB + 2*dtC*dtC + 2*dtE*dtE);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageB)) {
    cbB = pvl->directAnswer[tenGageB][0] = 
      dtA*dtD + dtA*dtF + dtD*dtF - dtB*dtB - dtC*dtC - dtE*dtE;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDet)) {
    cbC = -(pvl->directAnswer[tenGageDet][0] = 
            2*dtB*dtC*dtE + dtA*dtD*dtF 
            - dtC*dtC*dtD - dtA*dtE*dtE - dtB*dtB*dtF);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageS)) {
    cbS = (pvl->directAnswer[tenGageS][0] = 
           dtA*dtA + dtD*dtD + dtF*dtF
           + 2*dtB*dtB + 2*dtC*dtC + 2*dtE*dtE);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageQ)) {
    cbQ = pvl->directAnswer[tenGageQ][0] = (cbS - cbB)/9;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFA)) {
    tmp0 = (cbS
            ? cbQ/cbS
            : 0);
    tmp0 = AIR_MAX(0, tmp0);
    pvl->directAnswer[tenGageFA][0] = 3*sqrt(tmp0);
    /*
    if (!AIR_EXISTS(pvl->directAnswer[tenGageFA][0])) { 
      fprintf(stderr, "!%s: cbS = %g, cbQ = %g, cbQ/(epsilon + cbS) = %g\n"
              "tmp0 = max(0, cbQ/(epsilon + cbS)) = %g\n"
              "sqrt(tmp0) = %g --> %g\n", me,
              cbS, cbQ, cbQ/(epsilon + cbS),
              tmp0, sqrt(tmp0), pvl->directAnswer[tenGageFA][0]);
    }
    */
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageR)) {
    cbR = pvl->directAnswer[tenGageR][0] =
      (5*cbA*cbB - 27*cbC - 2*cbA*cbS)/54;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageMode)) {
    double cbQQQ;
    cbQQQ = 2*AIR_MAX(0, cbQ*cbQ*cbQ);
    tmp0 = 1.41421356237309504880*(cbQQQ ? cbR/(sqrt(cbQQQ)) : 0);
    pvl->directAnswer[tenGageMode][0] = AIR_CLAMP(-1, tmp0, 1);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageOmega)) {
    pvl->directAnswer[tenGageOmega][0] = 
      pvl->directAnswer[tenGageFA][0]*(1+pvl->directAnswer[tenGageMode][0])/2;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTheta)) {
    pvl->directAnswer[tenGageTheta][0] = 
      acos(-pvl->directAnswer[tenGageMode][0])/AIR_PI;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageModeWarp)) {
    pvl->directAnswer[tenGageModeWarp][0] = 
      cos((1-pvl->directAnswer[tenGageMode][0])*AIR_PI/2);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageEvec)) {
    /* we do the longer process to get eigenvectors, and in the process
       we always find the eigenvalues, whether or not they were asked for */
    tenEigensolve_d(evalAns, evecAns, tenAns);
  } else if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageEval)) {
    /* else eigenvectors are NOT needed, but eigenvalues ARE needed */
    tenEigensolve_d(evalAns, NULL, tenAns);
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDelNormK2)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDelNormK3)) {
    double tmp[7];
    tenInvariantGradientsK_d(tmp,
                             pvl->directAnswer[tenGageDelNormK2],
                             pvl->directAnswer[tenGageDelNormK3],
                             tenAns, 0.0000001);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDelNormR1)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDelNormR2)) {
    tenInvariantGradientsR_d(pvl->directAnswer[tenGageDelNormR1],
                             pvl->directAnswer[tenGageDelNormR2],
                             pvl->directAnswer[tenGageDelNormK3],
                             tenAns, 0.0000001);
  }
  
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDelNormPhi1)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDelNormPhi2)
      || GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDelNormPhi3)) {
    tenRotationTangents_d(pvl->directAnswer[tenGageDelNormPhi1],
                          pvl->directAnswer[tenGageDelNormPhi2],
                          pvl->directAnswer[tenGageDelNormPhi3],
                          evecAns);
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorGrad)) {
    /* done if doD1 */
    /* still have to set up pointer variables that item answers
       below will rely on as short-cuts */
    vecTmp = pvl->directAnswer[tenGageTensorGrad];
    gradDtA = vecTmp + 1*3;
    gradDtB = vecTmp + 2*3;
    gradDtC = vecTmp + 3*3;
    gradDtD = vecTmp + 4*3;
    gradDtE = vecTmp + 5*3;
    gradDtF = vecTmp + 6*3;
    TEN_T_SET(gradDdXYZ + 0*7, tenAns[0],
              gradDtA[0], gradDtB[0], gradDtC[0],
              gradDtD[0], gradDtE[0],
              gradDtF[0]);
    TEN_T_SET(gradDdXYZ + 1*7, tenAns[0],
              gradDtA[1], gradDtB[1], gradDtC[1],
              gradDtD[1], gradDtE[1],
              gradDtF[1]);
    TEN_T_SET(gradDdXYZ + 2*7, tenAns[0],
              gradDtA[2], gradDtB[2], gradDtC[2],
              gradDtD[2], gradDtE[2],
              gradDtF[2]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorGradMag)) {
    vecTmp = pvl->directAnswer[tenGageTensorGradMag];
    vecTmp[0] = sqrt(TEN_T_DOT(gradDdXYZ + 0*7, gradDdXYZ + 0*7));
    vecTmp[1] = sqrt(TEN_T_DOT(gradDdXYZ + 1*7, gradDdXYZ + 1*7));
    vecTmp[2] = sqrt(TEN_T_DOT(gradDdXYZ + 2*7, gradDdXYZ + 2*7));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorGradMag)) {
    pvl->directAnswer[tenGageTensorGradMagMag][0] = ELL_3V_LEN(vecTmp);
  }

  /* --- Trace --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTraceGradVec)) {
    vecTmp = pvl->directAnswer[tenGageTraceGradVec];
    ELL_3V_ADD3(vecTmp, gradDtA, gradDtD, gradDtF);
    ELL_3V_SCALE(gradCbA, -1, vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTraceGradMag)) {
    magTmp = pvl->directAnswer[tenGageTraceGradMag][0] = ELL_3V_LEN(vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTraceNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageTraceNormal],
                 magTmp ? 1/magTmp : 0, vecTmp);
  }

  /* ---- Norm stuff handled after S */

  /* --- B --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageBGradVec)) {
    gradCbB = vecTmp = pvl->directAnswer[tenGageBGradVec];
    ELL_3V_SCALE_ADD6(vecTmp, 
                      dtD + dtF, gradDtA,
                      dtA + dtF, gradDtD,
                      dtA + dtD, gradDtF,
                      -2*dtB, gradDtB,
                      -2*dtC, gradDtC,
                      -2*dtE, gradDtE);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageBGradMag)) {
    magTmp = pvl->directAnswer[tenGageBGradMag][0] = ELL_3V_LEN(vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageBNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageBNormal],
                 magTmp ? 1/magTmp : 0, vecTmp);
  }
  /* --- Det --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDetGradVec)) {
    vecTmp = pvl->directAnswer[tenGageDetGradVec];
    ELL_3V_SCALE_ADD6(vecTmp,
                      dtD*dtF - dtE*dtE, gradDtA,
                      2*(dtC*dtE - dtB*dtF), gradDtB,
                      2*(dtB*dtE - dtC*dtD), gradDtC,
                      dtA*dtF - dtC*dtC, gradDtD,
                      2*(dtB*dtC - dtA*dtE), gradDtE,
                      dtA*dtD - dtB*dtB, gradDtF);
    ELL_3V_SCALE(gradCbC, -1, vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDetGradMag)) {
    magTmp = pvl->directAnswer[tenGageDetGradMag][0] =
      AIR_CAST(float, ELL_3V_LEN(vecTmp));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDetNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageDetNormal],
                 magTmp ? 1/magTmp : 0, vecTmp);
  }
  /* --- S --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageSGradVec)) {
    gradCbS = vecTmp = pvl->directAnswer[tenGageSGradVec];
    ELL_3V_SCALE_ADD6(vecTmp,
                      2*dtA, gradDtA,
                      2*dtD, gradDtD,
                      2*dtF, gradDtF,
                      4*dtB, gradDtB,
                      4*dtC, gradDtC,
                      4*dtE, gradDtE);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageSGradMag)) {
    magTmp = pvl->directAnswer[tenGageSGradMag][0] = ELL_3V_LEN(vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageSNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageSNormal],
                 magTmp ? 1/magTmp : 0, vecTmp);
  }

  /* --- Norm --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageNormGradVec)) {
    double nslc;
    nslc = pvl->directAnswer[tenGageNorm][0];
    nslc = nslc ? 1/(2*nslc) : 0.0;
    vecTmp = pvl->directAnswer[tenGageNormGradVec];
    ELL_3V_SCALE(vecTmp, nslc, pvl->directAnswer[tenGageSGradVec]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageNormGradMag)) {
    magTmp = pvl->directAnswer[tenGageNormGradMag][0] = ELL_3V_LEN(vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageNormNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageNormNormal],
                 magTmp ? 1/magTmp : 0, vecTmp);
  }

  /* --- Q --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageQGradVec)) {
    gradCbQ = vecTmp = pvl->directAnswer[tenGageQGradVec];
    ELL_3V_SCALE_ADD2(vecTmp,
                      1.0/9, gradCbS, 
                      -1.0/9, gradCbB);

  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageQGradMag)) {
    magTmp = pvl->directAnswer[tenGageQGradMag][0] = ELL_3V_LEN(vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageQNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageQNormal],
                 magTmp ? 1/magTmp : 0, vecTmp);
  }
  /* --- FA --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAGradVec)) {
    vecTmp = pvl->directAnswer[tenGageFAGradVec];
    tmp2 = AIR_MAX(0, pvl->directAnswer[tenGageFA][0]);
    tmp0 = cbQ ? tmp2/(2*cbQ) : 0;
    tmp1 = cbS ? -tmp2/(2*cbS) : 0;
    ELL_3V_SCALE_ADD2(vecTmp,
                      tmp0, gradCbQ, 
                      tmp1, gradCbS);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAGradMag)) {
    magTmp = pvl->directAnswer[tenGageFAGradMag][0] = ELL_3V_LEN(vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFANormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageFANormal],
                 magTmp ? 1/magTmp : 0, vecTmp);
  }
  /* --- R --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageRGradVec)) {
    gradCbR = vecTmp = pvl->directAnswer[tenGageRGradVec];
    ELL_3V_SCALE_ADD4(vecTmp,
                      (5*cbB - 2*cbS)/54, gradCbA,
                      5*cbA/54, gradCbB,
                      -0.5, gradCbC,
                      -cbA/27, gradCbS);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageRGradMag)) {
    magTmp = pvl->directAnswer[tenGageRGradMag][0] = ELL_3V_LEN(vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageRNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageRNormal],
                 magTmp ? 1/magTmp : 0, vecTmp);
  }
  /* --- Mode --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageModeGradVec)) {
    vecTmp = pvl->directAnswer[tenGageModeGradVec];
    tmp1 = AIR_MAX(0, cbQ*cbQ*cbQ);
    tmp1 = tmp1 ? sqrt(1/tmp1) : 0;
    tmp0 = cbQ ? -tmp1*3*cbR/(2*cbQ) : 0;
    ELL_3V_SCALE_ADD2(vecTmp,
                      tmp0, gradCbQ,
                      tmp1, gradCbR);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageModeGradMag)) {
    magTmp = pvl->directAnswer[tenGageModeGradMag][0] = ELL_3V_LEN(vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageModeNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageModeNormal],
                 magTmp ? 1/magTmp : 0, vecTmp);
  }
  /* --- Theta --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageThetaGradVec)) {
    vecTmp = pvl->directAnswer[tenGageThetaGradVec];
    tmp1 = AIR_MAX(0, cbQ*cbQ*cbQ);
    tmp0 = tmp1 ? cbR*cbR/tmp1 : 0;
    tmp1 = sqrt(tmp1)*sqrt(1.0 - tmp0);
    tmp1 = tmp1 ? 1/(AIR_PI*tmp1) : 0.0;
    tmp0 = cbQ ? -tmp1*3*cbR/(2*cbQ) : 0.0;
    ELL_3V_SCALE_ADD2(vecTmp,
                      tmp0, gradCbQ,
                      tmp1, gradCbR);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageThetaGradMag)) {
    magTmp = pvl->directAnswer[tenGageThetaGradMag][0] = ELL_3V_LEN(vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageThetaNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageThetaNormal],
                 magTmp ? 1/magTmp : 0, vecTmp);
  }
  /* --- Omega --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageOmegaGradVec)) {
    double fa, mode, *faGrad, *modeGrad;
    vecTmp = pvl->directAnswer[tenGageOmegaGradVec];
    fa = pvl->directAnswer[tenGageFA][0];
    mode = pvl->directAnswer[tenGageMode][0];
    faGrad = pvl->directAnswer[tenGageFAGradVec];
    modeGrad = pvl->directAnswer[tenGageModeGradVec];
    ELL_3V_SCALE_ADD2(vecTmp,
                      (1+mode)/2, faGrad,
                      fa/2, modeGrad);
    ELL_3V_SCALE(vecTmp, (1+mode)/2, faGrad);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageOmegaGradMag)) {
    magTmp = pvl->directAnswer[tenGageOmegaGradMag][0] = ELL_3V_LEN(vecTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageOmegaNormal)) {
    ELL_3V_SCALE(pvl->directAnswer[tenGageOmegaNormal],
                 magTmp ? 1/magTmp : 0, vecTmp);
  }

#define SQRT_1_OVER_3 0.57735026918962576450

  /* --- Invariant gradients + rotation tangents --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageInvarKGrads)) {
    double mu1Grad[7], *mu2Grad, *skwGrad;
    
    TEN_T_SET(mu1Grad, 1,
              SQRT_1_OVER_3, 0, 0,
              SQRT_1_OVER_3, 0,
              SQRT_1_OVER_3);
    mu2Grad = pvl->directAnswer[tenGageDelNormK2];
    skwGrad = pvl->directAnswer[tenGageDelNormK3];

    ELL_3V_SET(pvl->directAnswer[tenGageInvarKGrads] + 0*3,
               TEN_T_DOT(mu1Grad, gradDdXYZ + 0*7),
               TEN_T_DOT(mu1Grad, gradDdXYZ + 1*7),
               TEN_T_DOT(mu1Grad, gradDdXYZ + 2*7));
    ELL_3V_SET(pvl->directAnswer[tenGageInvarKGrads] + 1*3,
               TEN_T_DOT(mu2Grad, gradDdXYZ + 0*7),
               TEN_T_DOT(mu2Grad, gradDdXYZ + 1*7),
               TEN_T_DOT(mu2Grad, gradDdXYZ + 2*7));
    ELL_3V_SET(pvl->directAnswer[tenGageInvarKGrads] + 2*3,
               TEN_T_DOT(skwGrad, gradDdXYZ + 0*7),
               TEN_T_DOT(skwGrad, gradDdXYZ + 1*7),
               TEN_T_DOT(skwGrad, gradDdXYZ + 2*7));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageInvarKGradMags)) {
    ELL_3V_SET(pvl->directAnswer[tenGageInvarKGradMags],
               ELL_3V_LEN(pvl->directAnswer[tenGageInvarKGrads] + 0*3),
               ELL_3V_LEN(pvl->directAnswer[tenGageInvarKGrads] + 1*3),
               ELL_3V_LEN(pvl->directAnswer[tenGageInvarKGrads] + 2*3));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageInvarRGrads)) {
    double *R1Grad, *R2Grad, *R3Grad;
    
    R1Grad = pvl->directAnswer[tenGageDelNormR1];
    R2Grad = pvl->directAnswer[tenGageDelNormR2];
    R3Grad = pvl->directAnswer[tenGageDelNormK3];

    ELL_3V_SET(pvl->directAnswer[tenGageInvarRGrads] + 0*3,
               TEN_T_DOT(R1Grad, gradDdXYZ + 0*7),
               TEN_T_DOT(R1Grad, gradDdXYZ + 1*7),
               TEN_T_DOT(R1Grad, gradDdXYZ + 2*7));
    ELL_3V_SET(pvl->directAnswer[tenGageInvarRGrads] + 1*3,
               TEN_T_DOT(R2Grad, gradDdXYZ + 0*7),
               TEN_T_DOT(R2Grad, gradDdXYZ + 1*7),
               TEN_T_DOT(R2Grad, gradDdXYZ + 2*7));
    ELL_3V_SET(pvl->directAnswer[tenGageInvarRGrads] + 2*3,
               TEN_T_DOT(R3Grad, gradDdXYZ + 0*7),
               TEN_T_DOT(R3Grad, gradDdXYZ + 1*7),
               TEN_T_DOT(R3Grad, gradDdXYZ + 2*7));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageInvarRGradMags)) {
    ELL_3V_SET(pvl->directAnswer[tenGageInvarRGradMags],
               ELL_3V_LEN(pvl->directAnswer[tenGageInvarRGrads] + 0*3),
               ELL_3V_LEN(pvl->directAnswer[tenGageInvarRGrads] + 1*3),
               ELL_3V_LEN(pvl->directAnswer[tenGageInvarRGrads] + 2*3));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageEvalGrads)) {
    double matOut[9], tenOut[9], tmpRes[9], 
      rel1, rel2, w1, w2, eps;
    unsigned int evi;
    for (evi=0; evi<=2; evi++) {
      ELL_3MV_OUTER(matOut, evecAns + evi*3, evecAns + evi*3);
      TEN_M2T(tenOut, matOut);
      ELL_3V_SET(tmpRes + evi*3,
                 TEN_T_DOT(tenOut, gradDdXYZ + 0*7),
                 TEN_T_DOT(tenOut, gradDdXYZ + 1*7),
                 TEN_T_DOT(tenOut, gradDdXYZ + 2*7));
    }

    /* Added 2008-06-27: In case there are duplicate eigenvalues,
     * average their derivatives to avoid visible artifacs in edge
     * maps. Provide a smooth transition to the ill-defined case */
    eps=0.05; /* threshold at which we start the transition */
  
    /* interpolation weights from relative eigenvalue distance */
    rel1=(evalAns[0]-evalAns[1])/(fabs(evalAns[0])+fabs(evalAns[1]));
    rel2=(evalAns[1]-evalAns[2])/(fabs(evalAns[1])+fabs(evalAns[2]));
    w1=rel1/eps-1;
    w1*=w1;
    w2=rel2/eps-1;
    w2*=w2;

    if (rel1>eps) {
      ELL_3V_COPY(pvl->directAnswer[tenGageEvalGrads], tmpRes);
    } else {
      if (rel2>eps) {
        ELL_3V_SCALE_ADD2(pvl->directAnswer[tenGageEvalGrads],
                          1-0.5*w1, tmpRes, 0.5*w1, tmpRes+3);
      } else {
        ELL_3V_SCALE_ADD3(pvl->directAnswer[tenGageEvalGrads],
                          1-0.5*w1-w1*w2/6.0, tmpRes,
                          0.5*w1-w1*w2/6.0, tmpRes+3,
                          w1*w2/3.0, tmpRes+6);
      }
    }

    if (rel2>eps) {
      ELL_3V_COPY(pvl->directAnswer[tenGageEvalGrads]+6, tmpRes+6);
    } else {
      if (rel1>eps) {
        ELL_3V_SCALE_ADD2(pvl->directAnswer[tenGageEvalGrads]+6,
                          1-0.5*w2, tmpRes+6, 0.5*w2, tmpRes+3);
      } else {
        ELL_3V_SCALE_ADD3(pvl->directAnswer[tenGageEvalGrads]+6,
                          1-0.5*w2-w1*w2/6.0, tmpRes+6,
                          0.5*w2-w1*w2/6.0, tmpRes+3,
                          w1*w2/3.0, tmpRes);
      }
    }

    ELL_3V_ADD3(pvl->directAnswer[tenGageEvalGrads]+3,
                tmpRes, tmpRes+3, tmpRes+6);
    ELL_3V_ADD2(tmpRes, pvl->directAnswer[tenGageEvalGrads],
                pvl->directAnswer[tenGageEvalGrads]+6);
    ELL_3V_SUB(pvl->directAnswer[tenGageEvalGrads]+3,
               pvl->directAnswer[tenGageEvalGrads]+3,
               tmpRes); /* l2 = trace - l1 - l3 */
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageRotTans)) {
    double phi1[7], phi2[7], phi3[7];

    tenRotationTangents_d(phi1, phi2, phi3, evecAns);
    ELL_3V_SET(pvl->directAnswer[tenGageRotTans] + 0*3,
               TEN_T_DOT(phi1, gradDdXYZ + 0*7),
               TEN_T_DOT(phi1, gradDdXYZ + 1*7),
               TEN_T_DOT(phi1, gradDdXYZ + 2*7));
    ELL_3V_SET(pvl->directAnswer[tenGageRotTans] + 1*3,
               TEN_T_DOT(phi2, gradDdXYZ + 0*7),
               TEN_T_DOT(phi2, gradDdXYZ + 1*7),
               TEN_T_DOT(phi2, gradDdXYZ + 2*7));
    ELL_3V_SET(pvl->directAnswer[tenGageRotTans] + 2*3,
               TEN_T_DOT(phi3, gradDdXYZ + 0*7),
               TEN_T_DOT(phi3, gradDdXYZ + 1*7),
               TEN_T_DOT(phi3, gradDdXYZ + 2*7));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageRotTanMags)) {
    ELL_3V_SET(pvl->directAnswer[tenGageRotTanMags],
               ELL_3V_LEN(pvl->directAnswer[tenGageRotTans] + 0*3),
               ELL_3V_LEN(pvl->directAnswer[tenGageRotTans] + 1*3),
               ELL_3V_LEN(pvl->directAnswer[tenGageRotTans] + 2*3));
  }
  /* --- C{l,p,a,lpmin}1 --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCl1)) {
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Cl1);
    pvl->directAnswer[tenGageCl1][0] = AIR_CLAMP(0, tmp0, 1);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCp1)) {
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Cp1);
    pvl->directAnswer[tenGageCp1][0] = AIR_CLAMP(0, tmp0, 1);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCa1)) {
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Ca1);
    pvl->directAnswer[tenGageCa1][0] = AIR_CLAMP(0, tmp0, 1);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageClpmin1)) {
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Clpmin1);
    pvl->directAnswer[tenGageClpmin1][0] = AIR_CLAMP(0, tmp0, 1);
  }
  /* --- C{l,p,a,lpmin}2 --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCl2)) {
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Cl2);
    pvl->directAnswer[tenGageCl2][0] = AIR_CLAMP(0, tmp0, 1);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCp2)) {
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Cp2);
    pvl->directAnswer[tenGageCp2][0] = AIR_CLAMP(0, tmp0, 1);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCa2)) {
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Ca2);
    pvl->directAnswer[tenGageCa2][0] = AIR_CLAMP(0, tmp0, 1);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageClpmin2)) {
    tmp0 = tenAnisoEval_d(evalAns, tenAniso_Clpmin2);
    pvl->directAnswer[tenGageClpmin2][0] = AIR_CLAMP(0, tmp0, 1);
  }
  /* --- Hessian madness (the derivative, not the soldier) --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageHessian)) {
    /* done if doD2; still have to set up pointers */
    matTmp = pvl->directAnswer[tenGageHessian];
    hessDtA = matTmp + 1*9;
    hessDtB = matTmp + 2*9;
    hessDtC = matTmp + 3*9;
    hessDtD = matTmp + 4*9;
    hessDtE = matTmp + 5*9;
    hessDtF = matTmp + 6*9;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTraceHessian)) {
    ELL_3M_SCALE_ADD3(pvl->directAnswer[tenGageTraceHessian],
                      1.0, hessDtA,
                      1.0, hessDtD,
                      1.0, hessDtF);
    ELL_3M_SCALE(hessCbA, -1, pvl->directAnswer[tenGageTraceHessian]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageBHessian)) {
    hessCbB = matTmp = pvl->directAnswer[tenGageBHessian];
    ELL_3M_ZERO_SET(matTmp);
    ELL_3M_SCALE_INCR(matTmp, dtB, hessDtB);
    ELL_3M_SCALE_INCR(matTmp, dtC, hessDtC);
    ELL_3M_SCALE_INCR(matTmp, dtE, hessDtE);
    ELL_3MV_OUTER_INCR(matTmp, gradDtB, gradDtB);
    ELL_3MV_OUTER_INCR(matTmp, gradDtC, gradDtC);
    ELL_3MV_OUTER_INCR(matTmp, gradDtE, gradDtE);
    ELL_3M_SCALE(matTmp, -2, matTmp);
    ELL_3MV_OUTER_INCR(matTmp, gradDtD, gradDtA);
    ELL_3MV_OUTER_INCR(matTmp, gradDtF, gradDtA);
    ELL_3MV_OUTER_INCR(matTmp, gradDtA, gradDtD);
    ELL_3MV_OUTER_INCR(matTmp, gradDtF, gradDtD);
    ELL_3MV_OUTER_INCR(matTmp, gradDtA, gradDtF);
    ELL_3MV_OUTER_INCR(matTmp, gradDtD, gradDtF);
    ELL_3M_SCALE_INCR(matTmp, dtD + dtF, hessDtA);
    ELL_3M_SCALE_INCR(matTmp, dtA + dtF, hessDtD);
    ELL_3M_SCALE_INCR(matTmp, dtA + dtD, hessDtF);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageDetHessian)) {
    double tmp[3];
    matTmp = pvl->directAnswer[tenGageDetHessian];
    ELL_3M_ZERO_SET(matTmp);
    ELL_3V_SCALE_ADD3(tmp, dtD, gradDtF,
                      dtF, gradDtD,
                      -2*dtE, gradDtE);
    ELL_3MV_OUTER_INCR(matTmp, tmp, gradDtA);
    ELL_3M_SCALE_INCR(matTmp, dtD*dtF - dtE*dtE, hessDtA);
    ELL_3V_SCALE_ADD4(tmp, 2*dtC, gradDtE,
                      2*dtE, gradDtC,
                      -2*dtB, gradDtF,
                      -2*dtF, gradDtB);
    ELL_3MV_OUTER_INCR(matTmp, tmp, gradDtB);
    ELL_3M_SCALE_INCR(matTmp, 2*(dtC*dtE - dtB*dtF), hessDtB);
    ELL_3V_SCALE_ADD4(tmp, 2*dtB, gradDtE,
                      2*dtE, gradDtB,
                      -2*dtC, gradDtD,
                      -2*dtD, gradDtC);
    ELL_3MV_OUTER_INCR(matTmp, tmp, gradDtC);
    ELL_3M_SCALE_INCR(matTmp, 2*(dtB*dtE - dtC*dtD), hessDtC);
    ELL_3V_SCALE_ADD3(tmp, dtA, gradDtF,
                      dtF, gradDtA,
                      -2*dtC, gradDtC);
    ELL_3MV_OUTER_INCR(matTmp, tmp, gradDtD);
    ELL_3M_SCALE_INCR(matTmp, dtA*dtF - dtC*dtC, hessDtD);
    ELL_3V_SCALE_ADD4(tmp, 2*dtB, gradDtC,
                      2*dtC, gradDtB,
                      -2*dtA, gradDtE,
                      -2*dtE, gradDtA);
    ELL_3MV_OUTER_INCR(matTmp, tmp, gradDtE);
    ELL_3M_SCALE_INCR(matTmp, 2*(dtB*dtC - dtA*dtE), hessDtE);
    ELL_3V_SCALE_ADD3(tmp, dtA, gradDtD,
                      dtD, gradDtA,
                      -2*dtB, gradDtB);
    ELL_3MV_OUTER_INCR(matTmp, tmp, gradDtF);
    ELL_3M_SCALE_INCR(matTmp, dtA*dtD - dtB*dtB, hessDtF);
    ELL_3M_SCALE(hessCbC, -1, pvl->directAnswer[tenGageDetHessian]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageSHessian)) {
    hessCbS = matTmp = pvl->directAnswer[tenGageSHessian];
    ELL_3M_ZERO_SET(matTmp);
    ELL_3M_SCALE_INCR(matTmp,      dtB, hessDtB);
    ELL_3MV_OUTER_INCR(matTmp, gradDtB, gradDtB);
    ELL_3M_SCALE_INCR(matTmp,      dtC, hessDtC);
    ELL_3MV_OUTER_INCR(matTmp, gradDtC, gradDtC);
    ELL_3M_SCALE_INCR(matTmp,      dtE, hessDtE);
    ELL_3MV_OUTER_INCR(matTmp, gradDtE, gradDtE);
    ELL_3M_SCALE(matTmp, 2, matTmp);
    ELL_3M_SCALE_INCR(matTmp,      dtA, hessDtA);
    ELL_3MV_OUTER_INCR(matTmp, gradDtA, gradDtA);
    ELL_3M_SCALE_INCR(matTmp,      dtD, hessDtD);
    ELL_3MV_OUTER_INCR(matTmp, gradDtD, gradDtD);
    ELL_3M_SCALE_INCR(matTmp,      dtF, hessDtF);
    ELL_3MV_OUTER_INCR(matTmp, gradDtF, gradDtF);
    ELL_3M_SCALE(matTmp, 2, matTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageQHessian)) {
    hessCbQ = pvl->directAnswer[tenGageQHessian];
    ELL_3M_SCALE_ADD2(hessCbQ,
                      1.0/9, hessCbS, 
                      -1.0/9, hessCbB);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAHessian)) {
    double tmpQ, rQ, orQ, oQ, tmpS, rS, orS, oS;
    tmpQ = AIR_MAX(0, cbQ);
    tmpS = AIR_MAX(0, cbS);
    oQ = tmpQ ? 1/tmpQ : 0;
    oS = tmpS ? 1/tmpS : 0;
    rQ = sqrt(tmpQ);
    rS = sqrt(tmpS);
    orQ = rQ ? 1/rQ : 0;
    orS = rS ? 1/rS : 0;
    matTmp = pvl->directAnswer[tenGageFAHessian];
    ELL_3M_ZERO_SET(matTmp);
    ELL_3M_SCALE_INCR(matTmp, orS*orQ, hessCbQ);
    ELL_3M_SCALE_INCR(matTmp, -rQ*orS*oS, hessCbS);
    ELL_3MV_SCALE_OUTER_INCR(matTmp, -orS*orQ*oQ/2, gradCbQ, gradCbQ);
    ELL_3MV_SCALE_OUTER_INCR(matTmp, 3*rQ*orS*oS*oS/2, gradCbS, gradCbS);
    ELL_3MV_SCALE_OUTER_INCR(matTmp, -orS*oS*orQ/2, gradCbS, gradCbQ);
    ELL_3MV_SCALE_OUTER_INCR(matTmp, -orQ*orS*oS/2, gradCbQ, gradCbS);
    ELL_3M_SCALE(matTmp, 3.0/2, matTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAHessianEvec)) {
    /* HEY: cut-and-paste from tenGageEvec, with minimal changes */
    double fakeTen[7];
    TEN_M2T(fakeTen, pvl->directAnswer[tenGageFAHessian]);
    tenEigensolve_d(pvl->directAnswer[tenGageFAHessianEval],
                    pvl->directAnswer[tenGageFAHessianEvec], fakeTen);
  } else if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAHessianEval)) {
    double fakeTen[7];
    TEN_M2T(fakeTen, pvl->directAnswer[tenGageFAHessian]);
    /* else eigenvectors are NOT needed, but eigenvalues ARE needed */
    tenEigensolve_d(pvl->directAnswer[tenGageFAHessianEval], NULL, fakeTen);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFARidgeSurfaceStrength)) {
    double ev;
    ev = -pvl->directAnswer[tenGageFAHessianEval][2];
    ev = AIR_MAX(0, ev);
    pvl->directAnswer[tenGageFARidgeSurfaceStrength][0] = tenAns[0]*ev;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAValleySurfaceStrength)) {
    double ev;
    ev = pvl->directAnswer[tenGageFAHessianEval][0];
    ev = AIR_MAX(0, ev);
    pvl->directAnswer[tenGageFAValleySurfaceStrength][0] = tenAns[0]*ev;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFALaplacian)) {
    double *hess;
    hess = pvl->directAnswer[tenGageFAHessian];
    pvl->directAnswer[tenGageFALaplacian][0] = hess[0] + hess[4] + hess[8];
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAHessianEvalMode)) {
    double *heval;
    heval = pvl->directAnswer[tenGageFAHessianEval];
    pvl->directAnswer[tenGageFAHessianEvalMode][0] = airMode3_d(heval);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFA2ndDD)) {
    double *hess, *norm, tmpv[3];
    hess = pvl->directAnswer[tenGageFAHessian];
    norm = pvl->directAnswer[tenGageFANormal];
    ELL_3MV_MUL(tmpv, hess, norm);
    pvl->directAnswer[tenGageFA2ndDD][0] = ELL_3V_DOT(norm, tmpv);
  }

  /* HEY: lots of this is copy/paste from gage/sclanswer.c */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAGeomTens)) {
    double denom, *fahess, *fagmag, tmpMat[9], *fnorm, nPerp[9], sHess[9];

    fahess = pvl->directAnswer[tenGageFAHessian];
    fagmag = pvl->directAnswer[tenGageFAGradMag];
    fnorm = pvl->directAnswer[tenGageFANormal];
    denom = (*fagmag) ? 1.0/(*fagmag) : 0.0;
    ELL_3M_SCALE(sHess, denom, fahess);
    ELL_3M_IDENTITY_SET(nPerp);
    ELL_3MV_SCALE_OUTER_INCR(nPerp, -1, fnorm, fnorm);
    /* gten = nPerp * sHess * nPerp */
    ELL_3M_MUL(tmpMat, sHess, nPerp);
    ELL_3M_MUL(pvl->directAnswer[tenGageFAGeomTens], nPerp, tmpMat);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFATotalCurv)) {
    pvl->directAnswer[tenGageFATotalCurv][0] = 
      ELL_3M_FROB(pvl->directAnswer[tenGageFAGeomTens]);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAKappa1) ||
      GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAKappa2)) {
    double *k1, *k2, T, N, D;
    k1 = pvl->directAnswer[tenGageFAKappa1];
    k2 = pvl->directAnswer[tenGageFAKappa2];
    T = ELL_3M_TRACE(pvl->directAnswer[tenGageFAGeomTens]);
    N = pvl->directAnswer[tenGageFATotalCurv][0];
    D = 2*N*N - T*T;
    D = AIR_MAX(D, 0);
    D = sqrt(D);
    k1[0] = 0.5*(T + D);
    k2[0] = 0.5*(T - D);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAMeanCurv)) {
    double k1, k2;
    k1 = pvl->directAnswer[tenGageFAKappa1][0];
    k2 = pvl->directAnswer[tenGageFAKappa2][0];
    pvl->directAnswer[tenGageFAMeanCurv][0] = (k1 + k2)/2;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAShapeIndex)) {
    double k1, k2;
    k1 = pvl->directAnswer[tenGageFAKappa1][0];
    k2 = pvl->directAnswer[tenGageFAKappa2][0];
    pvl->directAnswer[tenGageFAShapeIndex][0] = 
      -(2/AIR_PI)*atan2(k1 + k2, k1 - k2);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAGaussCurv)) {
    double k1, k2;
    k1 = pvl->directAnswer[tenGageFAKappa1][0];
    k2 = pvl->directAnswer[tenGageFAKappa2][0];
    pvl->directAnswer[tenGageFAGaussCurv][0] = k1*k2;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFACurvDir1)) {
    double kk, tmpMat[9], tmpVec[3];
    kk = pvl->directAnswer[tenGageFAKappa2][0];
    ELL_3M_COPY(tmpMat, pvl->directAnswer[tenGageFAGeomTens]);
    tmpMat[0] -= kk; tmpMat[4] -= kk; tmpMat[8] -= kk;
    ell_3m_1d_nullspace_d(tmpVec, tmpMat);
    ELL_3V_COPY(pvl->directAnswer[tenGageFACurvDir1], tmpVec);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFACurvDir2)) {
    double kk, tmpMat[9], tmpVec[3];
    kk = pvl->directAnswer[tenGageFAKappa1][0];
    ELL_3M_COPY(tmpMat, pvl->directAnswer[tenGageFAGeomTens]);
    tmpMat[0] -= kk; tmpMat[4] -= kk; tmpMat[8] -= kk;
    ell_3m_1d_nullspace_d(tmpVec, tmpMat);
    ELL_3V_COPY(pvl->directAnswer[tenGageFACurvDir1], tmpVec);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAFlowlineCurv)) {
    double *fahess, *fnorm, *fagmag, denom, nPerp[9], nProj[9],
      ncTen[9], sHess[9], tmpMat[9];
    fnorm = pvl->directAnswer[tenGageFANormal];
    fahess = pvl->directAnswer[tenGageFAHessian];
    fagmag = pvl->directAnswer[tenGageFAGradMag];
    ELL_3MV_OUTER(nProj, fnorm, fnorm);
    ELL_3M_IDENTITY_SET(nPerp);
    ELL_3M_SCALE_INCR(nPerp, -1, nProj);
    denom = (*fagmag) ? 1.0/(*fagmag) : 0.0;
    ELL_3M_SCALE(sHess, denom, fahess);
    /* ncTen = nPerp * sHess * nProj */
    ELL_3M_MUL(tmpMat, sHess, nProj);
    ELL_3M_MUL(ncTen, nPerp, tmpMat);
    pvl->directAnswer[gageSclFlowlineCurv][0] = ELL_3M_FROB(ncTen);
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageRHessian)) {
    hessCbR = matTmp = pvl->directAnswer[tenGageRHessian];
    ELL_3M_ZERO_SET(matTmp);
    ELL_3M_SCALE_INCR(matTmp, 5*cbB - 2*cbS, hessCbA);
    ELL_3MV_SCALE_OUTER_INCR(matTmp, 5, gradCbB, gradCbA);
    ELL_3MV_SCALE_OUTER_INCR(matTmp, -2, gradCbS, gradCbA);
    ELL_3M_SCALE_INCR(matTmp, 5*cbA, hessCbB);
    ELL_3MV_SCALE_OUTER_INCR(matTmp, 5, gradCbA, gradCbB);
    ELL_3M_SCALE_INCR(matTmp, -27, hessCbC);
    ELL_3M_SCALE_INCR(matTmp, -2*cbA, hessCbS);
    ELL_3MV_SCALE_OUTER_INCR(matTmp, -2, gradCbA, gradCbS);
    ELL_3M_SCALE(matTmp, 1.0/54, matTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageModeHessian)) {
    double tmpQ, oQ, rQ;
    tmpQ = AIR_MAX(0, cbQ);
    rQ = sqrt(tmpQ);
    oQ = tmpQ ? 1/tmpQ : 0;
    matTmp = pvl->directAnswer[tenGageModeHessian];
    ELL_3M_ZERO_SET(matTmp);
    ELL_3M_SCALE_INCR(matTmp, -(3.0/2)*cbR, hessCbQ);
    ELL_3MV_SCALE_OUTER_INCR(matTmp, (15.0/4)*cbR*oQ, gradCbQ, gradCbQ);
    ELL_3MV_SCALE_OUTER_INCR(matTmp, -(3.0/2), gradCbR, gradCbQ);
    ELL_3M_SCALE_INCR(matTmp, cbQ, hessCbR);
    ELL_3MV_SCALE_OUTER_INCR(matTmp, -(3.0/2), gradCbQ, gradCbR);
    tmp0 = (tmpQ && rQ) ? 1/(tmpQ*tmpQ*rQ) : 0.0;
    ELL_3M_SCALE(matTmp, tmp0, matTmp);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageModeHessianEvec)) {
    /* HEY: cut-and-paste from tenGageFAHessianEvec */
    double fakeTen[7];
    TEN_M2T(fakeTen, pvl->directAnswer[tenGageModeHessian]);
    tenEigensolve_d(pvl->directAnswer[tenGageModeHessianEval],
                    pvl->directAnswer[tenGageModeHessianEvec], fakeTen);
  } else if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageModeHessianEval)) {
    double fakeTen[7];
    TEN_M2T(fakeTen, pvl->directAnswer[tenGageModeHessian]);
    /* else eigenvectors are NOT needed, but eigenvalues ARE needed */
    tenEigensolve_d(pvl->directAnswer[tenGageModeHessianEval], NULL, fakeTen);
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageOmegaHessian)) {
    double fa, mode, *modeGrad, *faGrad, *modeHess, *faHess;
    fa = pvl->directAnswer[tenGageFA][0];
    mode = pvl->directAnswer[tenGageMode][0];
    faGrad = pvl->directAnswer[tenGageFAGradVec];
    modeGrad = pvl->directAnswer[tenGageModeGradVec];
    faHess = pvl->directAnswer[tenGageFAHessian];
    modeHess = pvl->directAnswer[tenGageModeHessian];
    matTmp = pvl->directAnswer[tenGageOmegaHessian];
    ELL_3M_ZERO_SET(matTmp);
    ELL_3M_SCALE_INCR(matTmp, (1+mode)/2, faHess);
    ELL_3M_SCALE_INCR(matTmp, fa/2, modeHess);
    ELL_3MV_OUTER_INCR(matTmp, modeGrad, faGrad);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageOmegaHessianEvec)) {
    /* HEY: cut-and-paste from tenGageFAHessianEvec */
    double fakeTen[7];
    TEN_M2T(fakeTen, pvl->directAnswer[tenGageOmegaHessian]);
    tenEigensolve_d(pvl->directAnswer[tenGageOmegaHessianEval],
                    pvl->directAnswer[tenGageOmegaHessianEvec], fakeTen);
  } else if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageOmegaHessianEval)) {
    double fakeTen[7];
    TEN_M2T(fakeTen, pvl->directAnswer[tenGageOmegaHessian]);
    /* else eigenvectors are NOT needed, but eigenvalues ARE needed */
    tenEigensolve_d(pvl->directAnswer[tenGageOmegaHessianEval], NULL, fakeTen);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageOmegaLaplacian)) {
    double *hess;
    hess = pvl->directAnswer[tenGageOmegaHessian];
    pvl->directAnswer[tenGageOmegaLaplacian][0] = hess[0] + hess[4] + hess[8];
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageOmega2ndDD)) {
    double *hess, *norm, tmpv[3];
    hess = pvl->directAnswer[tenGageOmegaHessian];
    norm = pvl->directAnswer[tenGageOmegaNormal];
    ELL_3MV_MUL(tmpv, hess, norm);
    pvl->directAnswer[tenGageOmega2ndDD][0] = ELL_3V_DOT(norm, tmpv);
  }

  /* --- evec0 dot products */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTraceGradVecDotEvec0)) {
    tmp0 = ELL_3V_DOT(evecAns + 0*3, pvl->directAnswer[tenGageTraceGradVec]);
    pvl->directAnswer[tenGageTraceGradVecDotEvec0][0] = AIR_ABS(tmp0);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTraceDiffusionAlign)) {
    tmp0 = ELL_3V_DOT(evecAns + 0*3, pvl->directAnswer[tenGageTraceNormal]);
    tmp0 = 1 - 2*acos(AIR_ABS(tmp0))/AIR_PI;
    pvl->directAnswer[tenGageTraceDiffusionAlign][0] = tmp0;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTraceDiffusionFraction)) {
    double tmpv[3];
    TEN_TV_MUL(tmpv, tenAns, pvl->directAnswer[tenGageTraceNormal]);
    tmp0 = ELL_3V_DOT(tmpv, pvl->directAnswer[tenGageTraceNormal]);
    tmp0 /= TEN_T_TRACE(tenAns) ? TEN_T_TRACE(tenAns) : 1;
    pvl->directAnswer[tenGageTraceDiffusionFraction][0] = tmp0;
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFAGradVecDotEvec0)) {
    tmp0 = ELL_3V_DOT(evecAns + 0*3, pvl->directAnswer[tenGageFAGradVec]);
    pvl->directAnswer[tenGageFAGradVecDotEvec0][0] = AIR_ABS(tmp0);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFADiffusionAlign)) {
    tmp0 = ELL_3V_DOT(evecAns + 0*3, pvl->directAnswer[tenGageFANormal]);
    tmp0 = 1 - 2*acos(AIR_ABS(tmp0))/AIR_PI;
    pvl->directAnswer[tenGageFADiffusionAlign][0] = tmp0;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageFADiffusionFraction)) {
    double tmpv[3];
    TEN_TV_MUL(tmpv, tenAns, pvl->directAnswer[tenGageFANormal]);
    tmp0 = ELL_3V_DOT(tmpv, pvl->directAnswer[tenGageFANormal]);
    tmp0 /= TEN_T_TRACE(tenAns) ? TEN_T_TRACE(tenAns) : 1;
    pvl->directAnswer[tenGageFADiffusionFraction][0] = tmp0;
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageOmegaGradVecDotEvec0)) {
    tmp0 = ELL_3V_DOT(evecAns + 0*3, pvl->directAnswer[tenGageOmegaGradVec]);
    pvl->directAnswer[tenGageOmegaGradVecDotEvec0][0] = AIR_ABS(tmp0);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageOmegaDiffusionAlign)) {
    tmp0 = ELL_3V_DOT(evecAns + 0*3, pvl->directAnswer[tenGageOmegaNormal]);
    tmp0 = 1 - 2*acos(AIR_ABS(tmp0))/AIR_PI;
    pvl->directAnswer[tenGageOmegaDiffusionAlign][0] = tmp0;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageOmegaDiffusionFraction)) {
    double tmpv[3];
    TEN_TV_MUL(tmpv, tenAns, pvl->directAnswer[tenGageOmegaNormal]);
    tmp0 = ELL_3V_DOT(tmpv, pvl->directAnswer[tenGageOmegaNormal]);
    tmp0 /= TEN_T_TRACE(tenAns) ? TEN_T_TRACE(tenAns) : 1;
    pvl->directAnswer[tenGageOmegaDiffusionFraction][0] = tmp0;
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageConfGradVecDotEvec0)) {
    double *confGrad;
    confGrad = pvl->directAnswer[tenGageTensorGrad];
    tmp0 = ELL_3V_DOT(evecAns + 0*3, confGrad);
    pvl->directAnswer[tenGageConfGradVecDotEvec0][0] = AIR_ABS(tmp0);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageConfDiffusionAlign)) {
    double *confGrad, confNorm[3], tmp;
    confGrad = pvl->directAnswer[tenGageTensorGrad];
    ELL_3V_NORM(confNorm, confGrad, tmp);
    tmp0 = ELL_3V_DOT(evecAns + 0*3, confNorm);
    tmp0 = 1 - 2*acos(AIR_ABS(tmp0))/AIR_PI;
    pvl->directAnswer[tenGageConfDiffusionAlign][0] = tmp0;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageConfDiffusionFraction)) {
    double *confGrad, confNorm[3], tmp, tmpv[3];
    confGrad = pvl->directAnswer[tenGageTensorGrad];
    ELL_3V_NORM(confNorm, confGrad, tmp);
    TEN_TV_MUL(tmpv, tenAns, confNorm);
    tmp0 = ELL_3V_DOT(tmpv, confNorm);
    tmp0 /= TEN_T_TRACE(tenAns) ? TEN_T_TRACE(tenAns) : 1;
    pvl->directAnswer[tenGageConfDiffusionFraction][0] = tmp0;
  }


  /* --- Covariance --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCovariance)) {
    unsigned int cc, tt, taa, tbb,
      vijk, vii, vjj, vkk, fd, fddd;
    double *cov, ww, wxx, wyy, wzz, ten[7];

    cov = pvl->directAnswer[tenGageCovariance];
    /* HEY: casting because radius signed (shouldn't be) */
    fd = AIR_CAST(unsigned int, 2*ctx->radius);
    fddd = fd*fd*fd;

    /* reset answer */
    for (cc=0; cc<21; cc++) {
      cov[cc] = 0;
    }

    ten[0] = 1; /* never used anyway */
    for (vijk=0; vijk<fddd; vijk++) {
      vii = vijk % fd;
      vjj = (vijk/fd) % fd;
      vkk = vijk/fd/fd;
      wxx = ctx->fw[vii + fd*(0 + 3*gageKernel00)];
      wyy = ctx->fw[vjj + fd*(1 + 3*gageKernel00)];
      wzz = ctx->fw[vkk + fd*(2 + 3*gageKernel00)];
      ww = wxx*wyy*wzz;
      for (tt=1; tt<7; tt++) {
        ten[tt] = ww*(pvl->iv3[vijk + fddd*tt] - tenAns[tt]);
      }

      cc = 0;
      for (taa=0; taa<6; taa++) {
        for (tbb=taa; tbb<6; tbb++) {
          /* HEY: do I really mean to have this factor in here? */
          /* it probably meant that the units in the IGRT TMI paper were
             wrong by this factor ... */
          cov[cc] += 100000*ten[taa+1]*ten[tbb+1];
          cc++;
        }
      }
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCovarianceRGRT)) {
    double *cov, *covr, *igrt[6];
    unsigned int taa, tbb, cc;

    cov = pvl->directAnswer[tenGageCovariance];
    covr = pvl->directAnswer[tenGageCovarianceRGRT];
    igrt[0] = pvl->directAnswer[tenGageDelNormR1];
    igrt[1] = pvl->directAnswer[tenGageDelNormR2];
    igrt[2] = pvl->directAnswer[tenGageDelNormK3];
    igrt[3] = pvl->directAnswer[tenGageDelNormPhi1];
    igrt[4] = pvl->directAnswer[tenGageDelNormPhi2];
    igrt[5] = pvl->directAnswer[tenGageDelNormPhi3];

    cc = 0;
    for (taa=0; taa<6; taa++) {
      for (tbb=taa; tbb<6; tbb++) {
        covr[cc] = tenDoubleContract_d(igrt[tbb], cov, igrt[taa]);
        cc++;
      }
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCovarianceKGRT)) {
    double *cov, *covk, *igrt[6], delnormk1[7];
    unsigned int taa, tbb, cc;

    cov = pvl->directAnswer[tenGageCovariance];
    covk = pvl->directAnswer[tenGageCovarianceKGRT];
    TEN_T_SET(delnormk1, 1, 0.57735026, 0, 0, 0.57735026, 0, 0.57735026);
    igrt[0] = delnormk1;
    igrt[1] = pvl->directAnswer[tenGageDelNormK2];
    igrt[2] = pvl->directAnswer[tenGageDelNormK3];
    igrt[3] = pvl->directAnswer[tenGageDelNormPhi1];
    igrt[4] = pvl->directAnswer[tenGageDelNormPhi2];
    igrt[5] = pvl->directAnswer[tenGageDelNormPhi3];

    cc = 0;
    for (taa=0; taa<6; taa++) {
      for (tbb=taa; tbb<6; tbb++) {
        covk[cc] = tenDoubleContract_d(igrt[tbb], cov, igrt[taa]);
        cc++;
      }
    }
  }

  /* these are items that somewhat bypass the convolution result
     (in tenGageTensor) because it has to do something else fancy
     with the constituent tensors.  This is young and hacky code;
     and it may be that facilitating this kind of processing should
     be better supported by the gage API ... */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorLogEuclidean) ||
      GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorQuatGeoLoxK) ||
      GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorQuatGeoLoxR)) {
    unsigned int vijk, vii, vjj, vkk, fd, fddd;
    _tenGagePvlData *pvlData;
    double *ans;
    int qret;

    pvlData = AIR_CAST(_tenGagePvlData *, pvl->data);
    /* HEY: casting because radius is signed (shouldn't be) */
    fd = AIR_CAST(unsigned int, 2*ctx->radius);
    fddd = fd*fd*fd;
    for (vijk=0; vijk<fddd; vijk++) {
      double wxx, wyy, wzz;
      unsigned int tt;
      vii = vijk % fd;
      vjj = (vijk/fd) % fd;
      vkk = vijk/fd/fd;
      wxx = ctx->fw[vii + fd*(0 + 3*gageKernel00)];
      wyy = ctx->fw[vjj + fd*(1 + 3*gageKernel00)];
      wzz = ctx->fw[vkk + fd*(2 + 3*gageKernel00)];
      pvlData->buffWght[vijk] = wxx*wyy*wzz;
      for (tt=0; tt<7; tt++) {
        pvlData->buffTen[tt + 7*vijk] = pvl->iv3[vijk + fddd*tt];
      }
    }
    
    qret = 0;
    if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorLogEuclidean)) {
      ans = pvl->directAnswer[tenGageTensorLogEuclidean];
      qret = tenInterpN_d(ans, pvlData->buffTen, pvlData->buffWght, fddd,
                          tenInterpTypeLogLinear, pvlData->tip);
    }
    if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorQuatGeoLoxK)) {
      ans = pvl->directAnswer[tenGageTensorQuatGeoLoxK];
      qret = tenInterpN_d(ans, pvlData->buffTen, pvlData->buffWght, fddd,
                          tenInterpTypeQuatGeoLoxK, pvlData->tip);
    }
    if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorQuatGeoLoxR)) {
      ans = pvl->directAnswer[tenGageTensorQuatGeoLoxR];
      qret= tenInterpN_d(ans, pvlData->buffTen, pvlData->buffWght, fddd,
                         tenInterpTypeQuatGeoLoxR, pvlData->tip);
    }
    if (qret) {
      char *lerr;
      fprintf(stderr, "!%s: problem!!!\n %s", me, lerr = biffGet(TEN));
      free(lerr);
    }
  }

  /* --- Aniso --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageAniso)) {
    for (ci=tenAnisoUnknown+1; ci<=TEN_ANISO_MAX; ci++) {
      pvl->directAnswer[tenGageAniso][ci] = tenAnisoEval_d(evalAns, ci);
    }
  }

  /* --- cl/cp/ca gradients --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCl1GradVec)) {
    vecTmp = pvl->directAnswer[tenGageCl1GradVec];

    ELL_3V_SET(vecTmp,
               (evalAns[0]*(-2*pvl->directAnswer[tenGageEvalGrads][3]-pvl->directAnswer[tenGageEvalGrads][6])
                +evalAns[1]*(2*pvl->directAnswer[tenGageEvalGrads][0]+pvl->directAnswer[tenGageEvalGrads][6])
                +evalAns[2]*(pvl->directAnswer[tenGageEvalGrads][0]-pvl->directAnswer[tenGageEvalGrads][3]))
               /(pvl->directAnswer[tenGageTrace][0]*pvl->directAnswer[tenGageTrace][0]),
               (evalAns[0]*(-2*pvl->directAnswer[tenGageEvalGrads][4]-pvl->directAnswer[tenGageEvalGrads][7])
                +evalAns[1]*(2*pvl->directAnswer[tenGageEvalGrads][1]+pvl->directAnswer[tenGageEvalGrads][7])
                +evalAns[2]*(pvl->directAnswer[tenGageEvalGrads][1]-pvl->directAnswer[tenGageEvalGrads][4]))
               /(pvl->directAnswer[tenGageTrace][0]*pvl->directAnswer[tenGageTrace][0]),
               (evalAns[0]*(-2*pvl->directAnswer[tenGageEvalGrads][5]-pvl->directAnswer[tenGageEvalGrads][8])
                +evalAns[1]*(2*pvl->directAnswer[tenGageEvalGrads][2]+pvl->directAnswer[tenGageEvalGrads][8])
                +evalAns[2]*(pvl->directAnswer[tenGageEvalGrads][2]-pvl->directAnswer[tenGageEvalGrads][5]))
               /(pvl->directAnswer[tenGageTrace][0]*pvl->directAnswer[tenGageTrace][0]));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCl1GradMag)) {
    pvl->directAnswer[tenGageCl1GradMag][0] = ELL_3V_LEN(vecTmp);
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCp1GradVec)) {
    vecTmp = pvl->directAnswer[tenGageCp1GradVec];

    ELL_3V_SET(vecTmp,
               2*(evalAns[0]*(pvl->directAnswer[tenGageEvalGrads][3]-pvl->directAnswer[tenGageEvalGrads][6])
                  +evalAns[1]*(-pvl->directAnswer[tenGageEvalGrads][0]-2*pvl->directAnswer[tenGageEvalGrads][6])
                  +evalAns[2]*(pvl->directAnswer[tenGageEvalGrads][0]+2*pvl->directAnswer[tenGageEvalGrads][3]))
               /(pvl->directAnswer[tenGageTrace][0]*pvl->directAnswer[tenGageTrace][0]),
               2*(evalAns[0]*(pvl->directAnswer[tenGageEvalGrads][4]-pvl->directAnswer[tenGageEvalGrads][7])
                  +evalAns[1]*(-pvl->directAnswer[tenGageEvalGrads][1]-2*pvl->directAnswer[tenGageEvalGrads][7])
                  +evalAns[2]*(pvl->directAnswer[tenGageEvalGrads][1]+2*pvl->directAnswer[tenGageEvalGrads][4]))
               /(pvl->directAnswer[tenGageTrace][0]*pvl->directAnswer[tenGageTrace][0]),
               2*(evalAns[0]*(pvl->directAnswer[tenGageEvalGrads][5]-pvl->directAnswer[tenGageEvalGrads][8])
                  +evalAns[1]*(-pvl->directAnswer[tenGageEvalGrads][2]-2*pvl->directAnswer[tenGageEvalGrads][8])
                  +evalAns[2]*(pvl->directAnswer[tenGageEvalGrads][2]+2*pvl->directAnswer[tenGageEvalGrads][5]))
               /(pvl->directAnswer[tenGageTrace][0]*pvl->directAnswer[tenGageTrace][0]));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCp1GradMag)) {
    pvl->directAnswer[tenGageCp1GradMag][0] = ELL_3V_LEN(vecTmp);
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCa1GradVec)) {
    vecTmp = pvl->directAnswer[tenGageCa1GradVec];

    ELL_3V_SET(vecTmp,
               -3*((evalAns[0]+evalAns[1])*pvl->directAnswer[tenGageEvalGrads][6]
                   -evalAns[2]*(pvl->directAnswer[tenGageEvalGrads][0]+pvl->directAnswer[tenGageEvalGrads][3]))
               /(pvl->directAnswer[tenGageTrace][0]*pvl->directAnswer[tenGageTrace][0]),
               -3*((evalAns[0]+evalAns[1])*pvl->directAnswer[tenGageEvalGrads][7]
                   -evalAns[2]*(pvl->directAnswer[tenGageEvalGrads][1]+pvl->directAnswer[tenGageEvalGrads][4]))
               /(pvl->directAnswer[tenGageTrace][0]*pvl->directAnswer[tenGageTrace][0]),
               -3*((evalAns[0]+evalAns[1])*pvl->directAnswer[tenGageEvalGrads][8]
                   -evalAns[2]*(pvl->directAnswer[tenGageEvalGrads][2]+pvl->directAnswer[tenGageEvalGrads][5]))
               /(pvl->directAnswer[tenGageTrace][0]*pvl->directAnswer[tenGageTrace][0]));
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageCa1GradMag)) {
    pvl->directAnswer[tenGageCa1GradMag][0] = ELL_3V_LEN(vecTmp);
  }

  /* --- tensor gradient, rotated into eigenframe of the tensor itself --- */
  if (GAGE_QUERY_ITEM_TEST(pvl->query, tenGageTensorGradRotE)) {
    /* confidence not affected by rotation */
    double evecsT[9], evecs[9], tmp[9], tmp2[9];
    double diff0, diff1, diff2, diffthresh;
    double rdiff0, rdiff1, rdiff2;
    unsigned int evi, tci;
    
    ELL_3V_COPY(pvl->directAnswer[tenGageTensorGradRotE],
                pvl->directAnswer[tenGageTensorGrad]);

    /* pre-compute relative eval diffs to detect ill-conditioned case */
    diff0=(evalAns[0]-evalAns[1])/(fabs(evalAns[0])+fabs(evalAns[1]));
    diff1=(evalAns[1]-evalAns[2])/(fabs(evalAns[1])+fabs(evalAns[2]));
    diff2=(evalAns[0]-evalAns[2])/(fabs(evalAns[0])+fabs(evalAns[2]));
    diffthresh=0.05;

    if (diff2>diffthresh) rdiff2=1.0; else rdiff2=diff2/diffthresh;
    if (diff1>diffthresh) rdiff1=1.0; else rdiff1=diff1/diffthresh;
    if (diff0>diffthresh) rdiff0=1.0; else rdiff0=diff0/diffthresh;

    ELL_3M_COPY(evecs,evecAns);
    ELL_3M_TRANSPOSE(evecsT,evecs);
    for (evi=0; evi<3; evi++) {
      char sign;
      /* first, simply rotate derivatives into eigenframe of value */
      TEN_T2M(tmp,gradDdXYZ + evi*7);
      ell_3m_mul_d(tmp2,tmp,evecsT);
      ell_3m_mul_d(tmp,evecs,tmp2);

      /* If necessary, perform a number of additional rotations to
       * distribute eigenvalue derivatives equally in ill-defined
       * cases. Explanation in Schultz and Seidel, "Using Eigenvalue
       * Derivatives for Edge Detection in DT-MRI Data", DAGM 2008*/
      if (rdiff0<1.0) {
        /* the goal is to find the smallest angle phi such that
         * rotation by phi around z will result in tmp[0]=tmp[4], i.e.:
         *
         * cos(phi)^2*tmp[0]-2*sin(phi)*cos(phi)*tmp[1]+sin(phi)^2*tmp[4] =
         * sin(phi)^2*tmp[0]+2*sin(phi)*cos(phi)*tmp[1]+cos(phi)^2*tmp[4]
         * =>
         * tan(2*phi)=(tmp[0]-tmp[4])/(2*tmp[1])
         *
         * we use atan2 to avoid potential problems with tmp[1]==0,
         * but manipulate the signs of the arguments s.t. the result
         * is always in [-pi/2,pi/2] (i.e., the smallest solution of
         * the above equality)
         */

        /* rotate around z axis */
        double phi, R[9], RT[9];
        sign = (tmp[0]-tmp[4])*tmp[1]>0?1:-1;
        phi=0.5*atan2(sign*fabs(tmp[0]-tmp[4]),fabs(2*tmp[1]));
        ELL_3M_ROTATE_Z_SET(R, (1.0-rdiff0)*phi);
        ELL_3M_TRANSPOSE(RT,R);
        ell_3m_mul_d(tmp2,tmp,RT);
        ell_3m_mul_d(tmp,R,tmp2);
      }
      if (rdiff1<1.0) {
        /* rotate around x axis */
        double phi, R[9], RT[9];
        sign = (tmp[4]-tmp[8])*tmp[5]>0?1:-1;
        phi=0.5*atan2(sign*fabs(tmp[4]-tmp[8]),fabs(2*tmp[5]));
        ELL_3M_ROTATE_X_SET(R, (1.0-rdiff1)*phi);
        ELL_3M_TRANSPOSE(RT,R);
        ell_3m_mul_d(tmp2,tmp,RT);
        ell_3m_mul_d(tmp,R,tmp2);
      }
      if (rdiff2<1.0) {
        double mean, submatrix[3], isoPhi, gamma, beta, A, C, R[9],RT[9];
        int axis, midaxis, smallest, sign;

        mean=(tmp[0]+tmp[4]+tmp[8])/3.0;
        /* what's the median? */
        midaxis=0;
        if ((tmp[0]>tmp[4] && tmp[4]>tmp[8])||
            (tmp[0]<tmp[4] && tmp[4]<tmp[8]))
          midaxis=1;
        else if ((tmp[4]>tmp[8] && tmp[8]>tmp[0])||
                 (tmp[4]<tmp[8] && tmp[8]<tmp[0]))
          midaxis=2;
        /* do we first rotate around smallest or largest? */
        smallest = 0;
        if (mean>tmp[4*midaxis]) {
          smallest=1;
        }
        
        sign=1;
        if ((smallest && (tmp[0]<tmp[4] && tmp[0]<tmp[8])) ||
            (!smallest && (tmp[0]>tmp[4] && tmp[0]>tmp[8]))) {
          axis=0;
          submatrix[0]=tmp[4];
          submatrix[1]=tmp[5];
          submatrix[2]=tmp[8];
          if (midaxis!=1) sign=-1;
        } else if ((smallest && (tmp[4]<tmp[0] && tmp[4]<tmp[8])) ||
                   (!smallest && (tmp[4]>tmp[0] && tmp[4]>tmp[8]))) {
          axis=1;
          submatrix[0]=tmp[8];
          submatrix[1]=tmp[2];
          submatrix[2]=tmp[0];
          if (midaxis!=2) sign=-1;
        } else {
          axis=2;
          submatrix[0]=tmp[0];
          submatrix[1]=tmp[1];
          submatrix[2]=tmp[4];
          if (midaxis!=0) sign=-1;
        }
        isoPhi=0.0f;
        gamma=sign*(submatrix[0]-submatrix[2]);
        beta=-sign*2*submatrix[1];
        A=sqrt(gamma*gamma+beta*beta);
        C=atan2(gamma,beta);
        isoPhi=0.5*(asin(2.0/A*(mean-0.5*(submatrix[0]+submatrix[2])))-C);
        /* make sure we use the minimal rotation */
        isoPhi=asin(2.0/A*(mean-0.5*(submatrix[0]+submatrix[2])));
        if (isoPhi>0) {
          if (fabs(AIR_PI-isoPhi-C)<fabs(isoPhi-C))
            isoPhi=0.5*(AIR_PI-isoPhi-C);
          else
            isoPhi=0.5*(isoPhi-C);
        } else if (isoPhi<0) {
          if (fabs(-AIR_PI-isoPhi-C)<fabs(isoPhi-C))
            isoPhi=0.5*(-AIR_PI-isoPhi-C);
          else
            isoPhi=0.5*(isoPhi-C);
        }
        
        /* perform the rotation */
        switch (axis) {
        case 0:
          ELL_3M_ROTATE_X_SET(R, (1.0-rdiff2)*isoPhi);
          break;
        case 1:
          ELL_3M_ROTATE_Y_SET(R, (1.0-rdiff2)*isoPhi);
          break;
        default:
          ELL_3M_ROTATE_Z_SET(R, (1.0-rdiff2)*isoPhi);
          break;
        }
        ELL_3M_TRANSPOSE(RT,R);
        ell_3m_mul_d(tmp2,tmp,RT);
        ell_3m_mul_d(tmp,R,tmp2);

        /* rotate around the now corrected evec */
        axis=midaxis;
        switch (midaxis) {
        case 0:
          sign = (tmp[0]-tmp[4])*tmp[1]>0?1:-1;
          ELL_3M_ROTATE_X_SET(R, (1.0-rdiff2)*0.5*atan2(sign*fabs(tmp[0]-tmp[4]),fabs(2*tmp[1])));
          break;
        case 1:
          sign = (tmp[8]-tmp[0])*tmp[2]>0?1:-1;
          ELL_3M_ROTATE_Y_SET(R, (1.0-rdiff2)*0.5*atan2(sign*fabs(tmp[8]-tmp[0]),fabs(2*tmp[2])));
          break;
        case 2:
          sign = (tmp[4]-tmp[8])*tmp[5]>0?1:-1;
          ELL_3M_ROTATE_Z_SET(R, (1.0-rdiff2)*0.5*atan2(sign*fabs(tmp[4]-tmp[8]),fabs(2*tmp[5])));
          break;
        }
        ELL_3M_TRANSPOSE(RT,R);
        ell_3m_mul_d(tmp2,tmp,RT);
        ell_3m_mul_d(tmp,R,tmp2);
      }
      /* Now, we can set the answer */
      TEN_M2T(tmp2,tmp);
      for (tci=1; tci<7; tci++) {
        pvl->directAnswer[tenGageTensorGradRotE][3*tci+evi] = tmp2[tci];
      }
    }
  }

  return;
}


void *
_tenGagePvlDataNew(const struct gageKind_t *kind) {
  _tenGagePvlData *pvlData;

  AIR_UNUSED(kind);
  pvlData = AIR_CAST(_tenGagePvlData *, calloc(1, sizeof(_tenGagePvlData)));
  if (pvlData) {
    pvlData->buffTen = NULL;
    pvlData->buffWght = NULL;
    pvlData->tip = tenInterpParmNew();
  }
  return pvlData;
}

void *
_tenGagePvlDataCopy(const struct gageKind_t *kind,
                    const void *_pvlDataOld) {
  _tenGagePvlData *pvlDataNew, *pvlDataOld;
  unsigned int num;

  AIR_UNUSED(kind);
  pvlDataOld = AIR_CAST(_tenGagePvlData *, _pvlDataOld);
  num = pvlDataOld->tip->allocLen;
  pvlDataNew = AIR_CAST(_tenGagePvlData *, calloc(1, sizeof(_tenGagePvlData)));
  if (pvlDataNew) {
    pvlDataNew->buffTen = AIR_CAST(double *, calloc(7*num, sizeof(double)));
    pvlDataNew->buffWght = AIR_CAST(double *, calloc(num, sizeof(double)));
    pvlDataNew->tip = tenInterpParmCopy(pvlDataOld->tip);
  }
  return pvlDataNew;
}

void *
_tenGagePvlDataNix(const struct gageKind_t *kind,
                   void *_pvlData) {
  _tenGagePvlData *pvlData;

  AIR_UNUSED(kind);
  pvlData = AIR_CAST(_tenGagePvlData *, _pvlData);
  airFree(pvlData->buffTen);
  airFree(pvlData->buffWght);
  tenInterpParmNix(pvlData->tip);
  airFree(pvlData);
  return NULL;
}

int 
_tenGagePvlDataUpdate(const struct gageKind_t *kind,
                      const gageContext *ctx, const gagePerVolume *pvl,
                      const void *_pvlData) {
  _tenGagePvlData *pvlData;
  unsigned int fd, num;

  AIR_UNUSED(kind);
  AIR_UNUSED(pvl);
  pvlData = AIR_CAST(_tenGagePvlData *, _pvlData);
  fd = AIR_CAST(unsigned int, 2*ctx->radius);
  num = fd*fd*fd;
  if (num != pvlData->tip->allocLen) {
    /* HEY: no error checking */
    pvlData->buffTen = airFree(pvlData->buffTen);
    pvlData->buffWght = airFree(pvlData->buffWght);
    pvlData->buffTen = AIR_CAST(double *, calloc(7*num, sizeof(double)));
    pvlData->buffWght = AIR_CAST(double *, calloc(num, sizeof(double)));
    tenInterpParmBufferAlloc(pvlData->tip, num);  
  }
  return 0;
}


gageKind
_tenGageKind = {
  AIR_FALSE, /* statically allocated */
  "tensor",
  &_tenGage,
  1,
  7,
  TEN_GAGE_ITEM_MAX,
  _tenGageTable,
  _tenGageIv3Print,
  _tenGageFilter,
  _tenGageAnswer,
  _tenGagePvlDataNew,
  _tenGagePvlDataCopy,
  _tenGagePvlDataNix,
  _tenGagePvlDataUpdate,
  NULL
};
gageKind *
tenGageKind = &_tenGageKind;
