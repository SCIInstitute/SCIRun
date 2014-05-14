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

/* 
** computes (r1 - r0)/(log(r1) - log(r0))
*/
double
_tenQGL_blah(double rr0, double rr1) {
  double bb, ret;

  if (rr1 > rr0) {
    /* the bb calculation below could blow up, so we recurse
       with flipped order */
    ret = _tenQGL_blah(rr1, rr0);
  } else {
    /*   rr1 <= rr0 --> rr1/rr0 <= 1 --> rr1/rr0 - 1 <=  0 --> bb <=  0 */
    /* and rr1 >= 0 --> rr1/rr0 >= 0 --> rr1/rr0 - 1 >= -1 --> bb >= -1 */
    bb = rr0 ? (rr1/rr0 - 1) : 0;
    if (bb > -0.0001) {      
      ret = rr0*(1 + bb*(0.5001249976477329
                         - bb*(7.0/6 + bb*(1.0/6 - bb/720.0))));
    } else {
      /* had trouble finding a high-quality approximation for b near -1 */
      bb = AIR_MAX(bb, -1 + 100*FLT_EPSILON);
      ret = rr0*bb/log(bb + 1);
    }
  }
  return ret;
}

#define rr0  (RThZA[0])
#define rr1  (RThZB[0])
#define rr  (oRThZ[0])
#define th0  (RThZA[1])
#define th1  (RThZB[1])
#define th  (oRThZ[1])
#define zz0  (RThZA[2])
#define zz1  (RThZB[2])
#define zz  (oRThZ[2])

void
tenQGLInterpTwoEvalK(double oeval[3],
                     const double evalA[3], const double evalB[3],
                     const double tt) {
  double RThZA[3], RThZB[3], oRThZ[3], bb;

  tenTripleConvertSingle_d(RThZA, tenTripleTypeRThetaZ,
                           evalA, tenTripleTypeEigenvalue);
  tenTripleConvertSingle_d(RThZB, tenTripleTypeRThetaZ,
                           evalB, tenTripleTypeEigenvalue);
  if (rr1 > rr0) {
    /* the bb calculation below could blow up, so we recurse
       with flipped order */
    tenQGLInterpTwoEvalK(oeval, evalB, evalA, 1-tt);
  } else {
    rr = AIR_LERP(tt, rr0, rr1);
    zz = AIR_LERP(tt, zz0, zz1);
    bb = rr0 ? (rr1/rr0 - 1) : 0;
    /* bb can't be positive, because rr1 <= rr0 enforced above, so below
       is really test for -0.001 < bb <= 0  */
    if (bb > -0.0001) {
      double dth;
      dth = th1 - th0;
      /* rr0 and rr1 are similar, use stable approximation */
      th = th0 + tt*(dth
                     + (0.5 - tt/2)*dth*bb 
                     + (-1.0/12 - tt/4 + tt*tt/3)*dth*bb*bb
                     + (1.0/24 + tt/24 + tt*tt/6 - tt*tt*tt/4)*dth*bb*bb*bb);
    } else {
      /* use original formula */
      /* have to clamp value of b so that log() values don't explode */
      bb = AIR_MAX(bb, -1 + 100*FLT_EPSILON);
      th = th0 + (th1 - th0)*log(1 + bb*tt)/log(1 + bb);
    }
    tenTripleConvertSingle_d(oeval, tenTripleTypeEigenvalue,
                             oRThZ, tenTripleTypeRThetaZ);
    /*
    fprintf(stderr, "%s: (b = %g) %g %g %g <-- %g %g %g\n", "blah", bb,
            oeval[0], oeval[1], oeval[2],
            oRThZ[0], oRThZ[1], oRThZ[2]);
    */
  }
}

double
_tenQGL_Kdist(const double RThZA[3], const double RThZB[3]) {
  double dr, dth, dz, bl, dist;

  dr = rr1 - rr0;
  bl = _tenQGL_blah(rr0, rr1);
  dth = th1  - th0;
  dz = zz1 - zz0;
  dist = sqrt(dr*dr + bl*bl*dth*dth + dz*dz);
  return dist;
}

void
_tenQGL_Klog(double klog[3],
             const double RThZA[3], const double RThZB[3]) {
  double dr, bl, dth, dz;

  dr = rr1 - rr0;
  bl = _tenQGL_blah(rr0, rr1);
  dth = th1  - th0;
  dz = zz1 - zz0;
  ELL_3V_SET(klog, dr, bl*dth, dz);
  return;
}

void
_tenQGL_Kexp(double RThZB[3], 
             const double RThZA[3], const double klog[3]) {
  double bl;
  
  rr1 = rr0 + klog[0];
  bl = _tenQGL_blah(rr0, rr1);
  th1 = th0 + (bl ? klog[1]/bl : 0);
  zz1 = zz0 + klog[2];
  return;
}

#undef rr0
#undef rr1
#undef rr
#undef th0
#undef th1
#undef th
#undef zz0
#undef zz1
#undef zz

/*
** stable computation of (ph1 - ph0)/(log(tan(ph1/2)) - log(tan(ph0/2)))
*/
double
_tenQGL_fooo(double ph1, double ph0) {
  double ret;

  if (ph0 > ph1) {
    ret = _tenQGL_fooo(ph0, ph1);
  } else if (0 == ph0/2) {
    ret = 0;
  } else {
    /* ph1 >= ph0 > 0 */
    if (ph1 - ph0 < 0.0001) {
      double dph, ss, cc;
      dph = ph1 - ph0;
      ss = sin(ph1);
      cc = cos(ph1);
      ret = (ss
             + cc*dph/2
             + ((cos(2*ph1) - 3)/ss)*dph*dph/24
             + (cc/(ss*ss))*dph*dph*dph/24);
    } else {
      ret = (ph1 - ph0)/(log(tan(ph1/2)) - log(tan(ph0/2)));
    }
  }
  return ret;
}

#define rr0  (RThPhA[0])
#define rr1  (RThPhB[0])
#define rr  (oRThPh[0])
#define th0  (RThPhA[1])
#define th1  (RThPhB[1])
#define th  (oRThPh[1])
#define ph0  (RThPhA[2])
#define ph1  (RThPhB[2])
#define ph  (oRThPh[2])

void
_tenQGL_Rlog(double rlog[3],
             const double RThPhA[3], const double RThPhB[3]) {
  double dr, dth, dph, bl, fo;

  dr = rr1 - rr0;
  dth = th1 - th0;
  dph = ph1 - ph0;
  bl = _tenQGL_blah(rr0, rr1);
  fo = _tenQGL_fooo(ph0, ph1);
  /*             rlog[0]  rlog[1]   rlog[2] */
  ELL_3V_SET(rlog,  dr,  bl*dth*fo, dph*bl);
}

void
_tenQGL_Rexp(double RThPhB[3], 
             const double RThPhA[3], const double rlog[3]) {
  double bl, fo;
  
  rr1 = rr0 + rlog[0];
  bl = _tenQGL_blah(rr0, rr1);
  ph1 = ph0 + (bl ? rlog[2]/bl : 0);
  fo = _tenQGL_fooo(ph0, ph1);
  th1 = th0 + (bl*fo ? rlog[1]/(bl*fo) : 0);
  return;
}

/* unlike with the K stuff, with the R stuff I seemed to have more luck 
   implementing pair-wise interpolation in terms of log and exp
*/
void
tenQGLInterpTwoEvalR(double oeval[3],
                     const double evalA[3], const double evalB[3],
                     const double tt) {
  double RThPhA[3], RThPhB[3], rlog[3], oRThPh[3];

  tenTripleConvertSingle_d(RThPhA, tenTripleTypeRThetaPhi,
                           evalA, tenTripleTypeEigenvalue);
  tenTripleConvertSingle_d(RThPhB, tenTripleTypeRThetaPhi,
                           evalB, tenTripleTypeEigenvalue);
  _tenQGL_Rlog(rlog, RThPhA, RThPhB);
  ELL_3V_SCALE(rlog, tt, rlog);
  _tenQGL_Rexp(oRThPh, RThPhA, rlog);
  tenTripleConvertSingle_d(oeval, tenTripleTypeEigenvalue,
                           oRThPh, tenTripleTypeRThetaPhi);
  return;
}

double
_tenQGL_Rdist(const double RThPhA[3], const double RThPhB[3]) {
  double dr, dth, dph, bl, fo;

  dr = rr1 - rr0;
  dth = th1 - th0;
  dph = ph1 - ph0;
  bl = _tenQGL_blah(rr0, rr1);
  fo = _tenQGL_fooo(ph0, ph1);
  return sqrt(dr*dr + bl*bl*(dth*dth*fo*fo + dph*dph));
}

#undef rr0
#undef rr1
#undef rr
#undef th0
#undef th1
#undef th
#undef ph0
#undef ph1
#undef ph

/* returns the index into unitq[] of the quaternion that led to the
   right alignment.  If it was already aligned, this will be 0, 
   because unitq[0] is the identity quaternion */
int
_tenQGL_q_align(double qOut[4], const double qRef[4], const double qIn[4]) {
  unsigned int ii, maxDotIdx;
  double unitq[8][4] = {{+1, 0, 0, 0},
                        {-1, 0, 0, 0},
                        {0, +1, 0, 0},
                        {0, -1, 0, 0},
                        {0, 0, +1, 0},
                        {0, 0, -1, 0},
                        {0, 0, 0, +1},
                        {0, 0, 0, -1}};
  double dot[8], qInMul[8][4], maxDot;
  
  for (ii=0; ii<8; ii++) {
    ell_q_mul_d(qInMul[ii], qIn, unitq[ii]);
    dot[ii] = ELL_4V_DOT(qRef, qInMul[ii]);
  }
  maxDotIdx = 0;
  maxDot = dot[maxDotIdx];
  for (ii=1; ii<8; ii++) {
    if (dot[ii] > maxDot) {
      maxDotIdx = ii;
      maxDot = dot[maxDotIdx];
    }
  }
  ELL_4V_COPY(qOut, qInMul[maxDotIdx]);
  return maxDotIdx;
}

void
tenQGLInterpTwoEvec(double oevec[9],
                    const double evecA[9], const double evecB[9],
                    double tt) {
  double rotA[9], rotB[9], orot[9],
    oq[4], qA[4], qB[4], _qB[4], qdiv[4], angle, axis[3], qq[4];

  ELL_3M_TRANSPOSE(rotA, evecA);
  ELL_3M_TRANSPOSE(rotB, evecB);
  ell_3m_to_q_d(qA, rotA);
  ell_3m_to_q_d(_qB, rotB);
  _tenQGL_q_align(qB, qA, _qB);
  /* there's probably a faster way to do this slerp qA --> qB */
  ell_q_div_d(qdiv, qA, qB); /* div = A^-1 * B */
  angle = ell_q_to_aa_d(axis, qdiv);
  ell_aa_to_q_d(qq, angle*tt, axis);
  ell_q_mul_d(oq, qA, qq);
  ell_q_to_3m_d(orot, oq);
  ELL_3M_TRANSPOSE(oevec, orot);
}

void
tenQGLInterpTwo(double oten[7],
                const double tenA[7], const double tenB[7],
                int ptype, double tt, tenInterpParm *tip) {
  double oeval[3], evalA[3], evalB[3], oevec[9], evecA[9], evecB[9], cc;

  AIR_UNUSED(tip);
  tenEigensolve_d(evalA, evecA, tenA);
  tenEigensolve_d(evalB, evecB, tenB);
  cc = AIR_LERP(tt, tenA[0], tenB[0]);

  if (tenInterpTypeQuatGeoLoxK == ptype) {
    tenQGLInterpTwoEvalK(oeval, evalA, evalB, tt);
  } else {
    tenQGLInterpTwoEvalR(oeval, evalA, evalB, tt);
  }
  tenQGLInterpTwoEvec(oevec, evecA, evecB, tt);
  tenMakeSingle_d(oten, cc, oeval, oevec);

  return;
}

/*
** This does (non-optionally) use biff, to report convergence failures
** 
** we do in fact require non-NULL tip, because it holds the buffers we need
*/
int
_tenQGLInterpNEval(double evalOut[3],
                   const double *evalIn, /* size 3 -by- NN */
                   const double *wght,   /* size NN */
                   unsigned int NN,
                   int ptype, tenInterpParm *tip) {
  char me[]="_tenQGLInterpNEval", err[BIFF_STRLEN];
  double RTh_Out[3], elen;
  unsigned int ii, iter;
  int rttype;
  void (*llog)(double lg[3], const double RTh_A[3], const double RTh_B[3]);
  void (*lexp)(double RTh_B[3], const double RTh_A[3], const double lg[3]);

  if (!(evalOut && evalIn && tip)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  /* convert to (R,Th,_) and initialize RTh_Out */
  if (tenInterpTypeQuatGeoLoxK == ptype) {
    rttype = tenTripleTypeRThetaZ;
    llog = _tenQGL_Klog;
    lexp = _tenQGL_Kexp;
  } else {
    rttype = tenTripleTypeRThetaPhi;
    llog = _tenQGL_Rlog;
    lexp = _tenQGL_Rexp;
  }
  ELL_3V_SET(RTh_Out, 0, 0, 0);
  for (ii=0; ii<NN; ii++) {
    double ww;
    tenTripleConvertSingle_d(tip->rtIn + 3*ii, rttype,
                             evalIn + 3*ii, tenTripleTypeEigenvalue);
    ww = wght ? wght[ii] : 1.0/NN;
    ELL_3V_SCALE_INCR(RTh_Out, ww, tip->rtIn + 3*ii);
  }

  /* compute iterated weighted mean, stored in RTh_Out */
  iter = 0;
  do {
    double logavg[3];
    /* take log of everyone */
    for (ii=0; ii<NN; ii++) {
      llog(tip->rtLog + 3*ii, RTh_Out, tip->rtIn + 3*ii);
    }
    /* average, and find length */
    ELL_3V_SET(logavg, 0, 0, 0);
    for (ii=0; ii<NN; ii++) {
      double ww;
      ww = wght ? wght[ii] : 1.0/NN;
      ELL_3V_SCALE_INCR(logavg, ww, tip->rtLog + 3*ii);
    }
    elen = ELL_3V_LEN(logavg);
    lexp(RTh_Out, RTh_Out, logavg);
    iter++;
  } while ((!tip->maxIter || iter < tip->maxIter) && elen > tip->convEps);

  if (elen > tip->convEps) {
    sprintf(err, "%s: still have error %g (> eps %g) after max %d iters", me,
            elen, tip->convEps, tip->maxIter);
    ELL_3V_SET(evalOut, AIR_NAN, AIR_NAN, AIR_NAN);
    biffAdd(TEN, err); return 1;
  }

  /* finish, convert to eval */
  tenTripleConvertSingle_d(evalOut, tenTripleTypeEigenvalue,
                           RTh_Out, rttype);

  return 0;
}

double
_tenQGL_q_interdot(unsigned int *centerIdxP,
                   double *qq, double *inter, unsigned int NN) {
  unsigned int ii, jj;
  double sum, dot, max;

  for (jj=0; jj<NN; jj++) {
    for (ii=0; ii<NN; ii++) {
      inter[ii + NN*jj] = 0;
    }
  }
  sum = 0;
  for (jj=0; jj<NN; jj++) {
    inter[jj + NN*jj] = 1.0;
    for (ii=jj+1; ii<NN; ii++) {
      dot = ELL_4V_DOT(qq + 4*ii, qq + 4*jj);
      inter[ii + NN*jj] = dot;
      inter[jj + NN*ii] = dot;
      sum += dot;
    }
  }
  for (jj=0; jj<NN; jj++) {
    for (ii=1; ii<NN; ii++) {
      inter[0 + NN*jj] += inter[ii + NN*jj];
    }
  }
  *centerIdxP = 0;
  max = inter[0 + NN*(*centerIdxP)];
  for (jj=1; jj<NN; jj++) {
    if (inter[0 + NN*jj] > max) {
      *centerIdxP = jj;
      max = inter[0 + NN*(*centerIdxP)];
    }
  }
  return sum;
}

/*
** This does (non-optionally) use biff, to report convergence failures
** 
** we do in fact require non-NULL tip, because it holds the buffers we need
*/
int
_tenQGLInterpNEvec(double evecOut[9],
                   const double *evecIn, /* size 9 -by- NN */
                   const double *wght,   /* size NN */
                   unsigned int NN,
                   tenInterpParm *tip) {
  char me[]="_tenQGLInterpNEvec", err[BIFF_STRLEN];
  double qOut[4], maxWght, len, odsum, dsum, rot[9];
  unsigned int ii, centerIdx=0, fix, qiter;

  if (!( evecOut && evecIn && tip )) {
    sprintf(err, "%s: got NULL pointer", me); 
    biffAdd(TEN, err); return 1;
  }
  /* convert to quaternions */
  for (ii=0; ii<NN; ii++) {
    ELL_3M_TRANSPOSE(rot, evecIn + 9*ii);
    ell_3m_to_q_d(tip->qIn + 4*ii, rot);
  }
  odsum = _tenQGL_q_interdot(&centerIdx, tip->qIn, tip->qInter, NN);

  /* find quaternion with maximal weight, use it as is (decree that
     its the right representative), and then align rest with that.
     This is actually principled; symmetry allows it */
  centerIdx = 0;
  if (wght) {
    maxWght = wght[centerIdx];
    for (ii=1; ii<NN; ii++) {
      if (wght[ii] > maxWght) {
        centerIdx = ii;
        maxWght = wght[centerIdx];
      }
    }
  }
  for (ii=0; ii<NN; ii++) {
    if (ii == centerIdx) {
      continue;
    }
    _tenQGL_q_align(tip->qIn + 4*ii, tip->qIn + 4*centerIdx, tip->qIn + 4*ii);
  }
  dsum = _tenQGL_q_interdot(&centerIdx, tip->qIn, tip->qInter, NN);

  /* try to settle on tightest set of representatives */
  qiter = 0;
  do {
    fix = 0;
    for (ii=0; ii<NN; ii++) {
      unsigned int ff;
      if (ii == centerIdx) {
        continue;
      }
      ff = _tenQGL_q_align(tip->qIn + 4*ii, tip->qIn + 4*centerIdx,
                           tip->qIn + 4*ii);
      fix = AIR_MAX(fix, ff);
    }
    dsum = _tenQGL_q_interdot(&centerIdx, tip->qIn, tip->qInter, NN);
    if (tip->maxIter && qiter > tip->maxIter) {
      sprintf(err, "%s: q tightening unconverged after %u iters; "
              "interdot = %g -> maxfix = %u; center = %u\n", 
              me, tip->maxIter, dsum, fix, centerIdx);
      biffAdd(TEN, err); return 1;
    }
    qiter++;
  } while (fix);
  /*
  fprintf(stderr, "!%s: dsum %g --%u--> %g\n", me, odsum, qiter, dsum);
  */
  /* make sure they're normalized */
  for (ii=0; ii<NN; ii++) {
    ELL_4V_NORM(tip->qIn + 4*ii, tip->qIn + 4*ii, len);
  }

  /* compute iterated weighted mean, stored in qOut */
  if (ell_q_avgN_d(qOut, &qiter, tip->qIn, tip->qBuff, wght,
                   NN, tip->convEps, tip->maxIter)) {
    sprintf(err, "%s: problem doing quaternion mean", me);
    biffMove(TEN, err, ELL); return 1;
  }
  /*
  fprintf(stderr, "!%s: q avg converged in %u\n", me, qiter);
  */
  
  /* finish, convert back to evec */
  ell_q_to_3m_d(rot, qOut);
  ELL_3M_TRANSPOSE(evecOut, rot);

  return 0;
}

