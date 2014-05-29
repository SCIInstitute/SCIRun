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

#include "gage.h"
#include "privateGage.h"

void
_gageSclAnswer (gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_gageSclAnswer";
  double gmag=0, *hess, *norm, *gvec, *gten, *k1, *k2, curv=0, 
    sHess[9]={0,0,0,0,0,0,0,0,0};
  double tmpMat[9], tmpVec[3], hevec[9], heval[3];
  double len, gp1[3], gp2[3], *nPerp, ncTen[9], nProj[9]={0,0,0,0,0,0,0,0,0};
  double alpha = 0.5;
  double beta = 0.5;
  double gamma = 5;
  double cc = 1e-6;
#define FD_MEDIAN_MAX 16
  int fd, nidx, xi, yi, zi;
  double *fw, iv3wght[2*FD_MEDIAN_MAX*FD_MEDIAN_MAX*FD_MEDIAN_MAX],
    wghtSum, wght;

  /* convenience pointers for work below */
  hess = pvl->directAnswer[gageSclHessian];
  gvec = pvl->directAnswer[gageSclGradVec];
  norm = pvl->directAnswer[gageSclNormal];
  nPerp = pvl->directAnswer[gageSclNPerp];
  gten = pvl->directAnswer[gageSclGeomTens];
  k1 = pvl->directAnswer[gageSclK1];
  k2 = pvl->directAnswer[gageSclK2];
  
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclValue)) {
    /* done if doV */
    if (ctx->verbose) {
      fprintf(stderr, "%s: val = % 15.7f\n", me, 
              (double)(pvl->directAnswer[gageSclValue][0]));
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclGradVec)) {
    /* done if doD1 */
    if (ctx->verbose) {
      fprintf(stderr, "%s: gvec = ", me);
      ell_3v_print_d(stderr, gvec);
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclGradMag)) {
    /* this is the true value of gradient magnitude */
    gmag = pvl->directAnswer[gageSclGradMag][0] = sqrt(ELL_3V_DOT(gvec, gvec));
  }

  /* NB: it would seem that gageParmGradMagMin is completely ignored ... */

  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclNormal)) {
    if (gmag) {
      ELL_3V_SCALE(norm, 1/gmag, gvec);
      /* polishing ... 
      len = sqrt(ELL_3V_DOT(norm, norm));
      ELL_3V_SCALE(norm, 1/len, norm);
      */
    } else {
      ELL_3V_COPY(norm, gageZeroNormal);
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclNPerp)) {
    /* nPerp = I - outer(norm, norm) */
    /* NB: this sets both nPerp and nProj */
    ELL_3MV_OUTER(nProj, norm, norm);
    ELL_3M_SCALE(nPerp, -1, nProj);
    nPerp[0] += 1;
    nPerp[4] += 1;
    nPerp[8] += 1;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclHessian)) {
    /* done if doD2 */
    if (ctx->verbose) {
      fprintf(stderr, "%s: hess = \n", me);
      ell_3m_print_d(stderr, hess);
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclLaplacian)) {
    pvl->directAnswer[gageSclLaplacian][0] = hess[0] + hess[4] + hess[8];
    if (ctx->verbose) {
      fprintf(stderr, "%s: lapl = %g + %g + %g  = %g\n", me,
              hess[0], hess[4], hess[8], 
              pvl->directAnswer[gageSclLaplacian][0]);
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclHessFrob)) {
    pvl->directAnswer[gageSclHessFrob][0] = ELL_3M_FROB(hess);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclHessEval)) {
    /* HEY: look at the return value for root multiplicity? */
    ell_3m_eigensolve_d(heval, hevec, hess, AIR_TRUE);
    ELL_3V_COPY(pvl->directAnswer[gageSclHessEval], heval);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclHessEvec)) {
    ELL_3M_COPY(pvl->directAnswer[gageSclHessEvec], hevec);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclHessRidgeness)) {
    double A, B, S;
    if (heval[1] >0 || heval[2]>0) {
      pvl->directAnswer[gageSclHessRidgeness][0] = 0;
    }
    else if (AIR_ABS(heval[1])<1e-10 || AIR_ABS(heval[2])<1e-10) {
      pvl->directAnswer[gageSclHessRidgeness][0] = 0;
    }
    else {
      double *ans;
      A = AIR_ABS(heval[1])/AIR_ABS(heval[2]);
      B = AIR_ABS(heval[0])/sqrt(AIR_ABS(heval[1]*heval[2]));
      S = sqrt(heval[0]*heval[0] + heval[1]*heval[1] + heval[2]*heval[2]);
      ans = pvl->directAnswer[gageSclHessRidgeness];
      ans[0] = (1-exp(-A*A/(2*alpha*alpha))) *
        exp(-B*B/(2*beta*beta)) *
        (1-exp(-S*S/(2*gamma*gamma))) *
        exp(-2*cc*cc/(AIR_ABS(heval[1])*heval[2]*heval[2]));
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclHessValleyness)) {
    double A, B, S;
    if (heval[0] <0 || heval[1]<0) {
      pvl->directAnswer[gageSclHessValleyness][0] = 0;
    }
    else if (AIR_ABS(heval[0])<1e-10 || AIR_ABS(heval[1])<1e-10) {
      pvl->directAnswer[gageSclHessValleyness][0] = 0;
    }
    else {
      double *ans;
      A = AIR_ABS(heval[1])/AIR_ABS(heval[0]);
      B = AIR_ABS(heval[2])/sqrt(AIR_ABS(heval[1]*heval[0]));
      S = sqrt(heval[0]*heval[0] + heval[1]*heval[1] + heval[2]*heval[2]);
      ans = pvl->directAnswer[gageSclHessValleyness];
      ans[0] = (1-exp(-A*A/(2*alpha*alpha))) *
        exp(-B*B/(2*beta*beta)) *
        (1-exp(-S*S/(2*gamma*gamma))) *
        exp(-2*cc*cc/(AIR_ABS(heval[1])*heval[0]*heval[0]));
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclHessMode)) {
    pvl->directAnswer[gageSclHessMode][0] = airMode3_d(heval);
  }

  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageScl2ndDD)) {
    ELL_3MV_MUL(tmpVec, hess, norm);
    pvl->directAnswer[gageScl2ndDD][0] = ELL_3V_DOT(norm, tmpVec);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclGeomTens)) {
    if (gmag > ctx->parm.gradMagCurvMin) {
      /* parm.curvNormalSide applied here to determine the sense of the
         normal when doing all curvature calculations */
      ELL_3M_SCALE(sHess, -(ctx->parm.curvNormalSide)/gmag, hess);
      
      /* gten = nPerp * sHess * nPerp */
      ELL_3M_MUL(tmpMat, sHess, nPerp);
      ELL_3M_MUL(gten, nPerp, tmpMat);

      if (ctx->verbose) {
        fprintf(stderr, "%s: gten: \n", me);
        ell_3m_print_d(stderr, gten);
        ELL_3MV_MUL(tmpVec, gten, norm);
        len = ELL_3V_LEN(tmpVec);
        fprintf(stderr, "%s: should be small: %30.15f\n", me, (double)len);
        ell_3v_perp_d(gp1, norm);
        ELL_3MV_MUL(tmpVec, gten, gp1);
        len = ELL_3V_LEN(tmpVec);
        fprintf(stderr, "%s: should be bigger: %30.15f\n", me, (double)len);
        ELL_3V_CROSS(gp2, gp1, norm);
        ELL_3MV_MUL(tmpVec, gten, gp2);
        len = ELL_3V_LEN(tmpVec);
        fprintf(stderr, "%s: should (also) be bigger: %30.15f\n",
                me, (double)len);
      }
    } else {
      ELL_3M_ZERO_SET(gten);
    }
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query,  gageSclTotalCurv)) {
    curv = pvl->directAnswer[gageSclTotalCurv][0] = ELL_3M_FROB(gten);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query,  gageSclShapeTrace)) {
    pvl->directAnswer[gageSclShapeTrace][0] = (curv
                                               ? ELL_3M_TRACE(gten)/curv
                                               : 0);
  }
  if ( (GAGE_QUERY_ITEM_TEST(pvl->query,  gageSclK1)) ||
       (GAGE_QUERY_ITEM_TEST(pvl->query,  gageSclK2)) ){
    double T, N, D;
    T = ELL_3M_TRACE(gten);
    N = curv;
    D = 2*N*N - T*T;
    /*
    if (D < -0.0000001) {
      fprintf(stderr, "%s: %g %g\n", me, T, N);
      fprintf(stderr, "%s: !!! D curv determinant % 22.10f < 0.0\n", me, D);
      fprintf(stderr, "%s: gten: \n", me);
      ell_3m_print_d(stderr, gten);
    }
    */
    D = AIR_MAX(D, 0);
    D = sqrt(D);
    k1[0] = 0.5*(T + D);
    k2[0] = 0.5*(T - D);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query,  gageSclMeanCurv)) {
    pvl->directAnswer[gageSclMeanCurv][0] = (*k1 + *k2)/2;
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query,  gageSclGaussCurv)) {
    pvl->directAnswer[gageSclGaussCurv][0] = (*k1)*(*k2);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query,  gageSclShapeIndex)) {
    pvl->directAnswer[gageSclShapeIndex][0] = 
      -(2/AIR_PI)*atan2(*k1 + *k2, *k1 - *k2);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclCurvDir1)) {
    /* HEY: this only works when K1, K2, 0 are all well mutually distinct,
       since these are the eigenvalues of the geometry tensor, and this
       code assumes that the eigenspaces are all one-dimensional */
    ELL_3M_COPY(tmpMat, gten);
    ELL_3M_DIAG_SET(tmpMat, gten[0] - *k1, gten[4]- *k1, gten[8] - *k1);
    ell_3m_1d_nullspace_d(tmpVec, tmpMat);
    ELL_3V_COPY(pvl->directAnswer[gageSclCurvDir1], tmpVec);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclCurvDir2)) {
    /* HEY: this only works when K1, K2, 0 are all well mutually distinct,
       since these are the eigenvalues of the geometry tensor, and this
       code assumes that the eigenspaces are all one-dimensional */
    ELL_3M_COPY(tmpMat, gten);
    ELL_3M_DIAG_SET(tmpMat, gten[0] - *k2, gten[4] - *k2, gten[8] - *k2);
    ell_3m_1d_nullspace_d(tmpVec, tmpMat);
    ELL_3V_COPY(pvl->directAnswer[gageSclCurvDir2], tmpVec);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclFlowlineCurv)) {
    if (gmag >= ctx->parm.gradMagCurvMin) {
      /* because of the gageSclGeomTens prerequisite, sHess, nPerp, and
         nProj are all already set */
      /* ncTen = nPerp * sHess * nProj */
      ELL_3M_MUL(tmpMat, sHess, nProj);
      ELL_3M_MUL(ncTen, nPerp, tmpMat);
    } else {
      ELL_3M_ZERO_SET(ncTen);
    }
    /* there used to be a wrong extra sqrt() here */
    pvl->directAnswer[gageSclFlowlineCurv][0] = ELL_3M_FROB(ncTen);
  }
  if (GAGE_QUERY_ITEM_TEST(pvl->query, gageSclMedian)) {
    /* this item is currently a complete oddball in that it does not
       benefit from anything done in the "filter" stage, which is in
       fact a waste of time if the query consists only  of this item */
    fd = 2*ctx->radius;
    if (fd > FD_MEDIAN_MAX) {
      fprintf(stderr, "%s: PANIC: current filter diameter = %d "
              "> FD_MEDIAN_MAX = %d\n", me, fd, FD_MEDIAN_MAX);
      exit(1);
    }
    fw = ctx->fw + fd*3*gageKernel00;
    /* HEY: this needs some optimization help */
    wghtSum = 0;
    nidx = 0;
    for (xi=0; xi<fd; xi++) {
      for (yi=0; yi<fd; yi++) {
        for (zi=0; zi<fd; zi++) {
          iv3wght[0 + 2*nidx] = pvl->iv3[nidx];
          iv3wght[1 + 2*nidx] = fw[xi + 0*fd]*fw[yi + 1*fd]*fw[zi + 2*fd];
          wghtSum += iv3wght[1 + 2*nidx];
          nidx++;
        }
      }
    }
    qsort(iv3wght, fd*fd*fd, 2*sizeof(double), nrrdValCompare[nrrdTypeDouble]);
    wght = 0;
    for (nidx=0; nidx<fd*fd*fd; nidx++) {
      wght += iv3wght[1 + 2*nidx];
      if (wght > wghtSum/2) {
        break;
      }
    }
    pvl->directAnswer[gageSclMedian][0] = iv3wght[0 + 2*nidx];
  }
  return;
}

