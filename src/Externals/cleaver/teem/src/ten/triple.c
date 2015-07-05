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

typedef void (*tenTripleConverter)(double dst[3], const double src[3]);

#define SQRT6 2.449489742783178098197284
#define SQRT2 1.414213562373095048801689
#define SQRT3 1.732050807568877293527446

#define J1 (j[0])
#define J2 (j[1])
#define J3 (j[2])

#define K1 (k[0])
#define K2 (k[1])
#define K3 (k[2])

#define R1 (r[0])
#define R2 (r[1])
#define R3 (r[2])

#define MU1 (mu[0])
#define MU2 (mu[1])
#define MU3 (mu[2])

static void 
_iden(double dst[3], const double src[3]) {
  ELL_3V_COPY(dst, src);
  return;
}

/* in the function names below, the format is _<dst>_<src>() */
static void 
_mu_ev(double mu[3], const double ev[3]) {
  double mm;
  
  mm = mu[0] = (ev[0] + ev[1] + ev[2])/3;
  mu[1] = ((ev[0] - mm)*(ev[0] - mm) +
           (ev[1] - mm)*(ev[1] - mm) +
           (ev[2] - mm)*(ev[2] - mm))/3;
  mu[2] = ((ev[0] - mm)*(ev[0] - mm)*(ev[0] - mm) +
           (ev[1] - mm)*(ev[1] - mm)*(ev[1] - mm) +
           (ev[2] - mm)*(ev[2] - mm)*(ev[2] - mm))/3;
}

static double 
_xyzmat[] = {2/SQRT6, -1/SQRT6, -1/SQRT6,
             0,        1/SQRT2, -1/SQRT2,
             1/SQRT3,  1/SQRT3,  1/SQRT3};

static void
_xyz_ev(double xyz[3], const double _ev[3]) {
  double ev[3], tmp;
  
  ELL_3V_COPY(ev, _ev);
  ELL_SORT3(ev[0], ev[1], ev[2], tmp);
  ELL_3MV_MUL(xyz, _xyzmat, ev);
}

static void
_ev_xyz(double ev[3], const double xyz[3]) {
  
  ELL_3MV_TMUL(ev, _xyzmat, xyz);
}

static void
_j_ev(double j[3], const double ev[3]) {

  J1 = ev[0] + ev[1] + ev[2];
  J2 = ev[0]*ev[1] + ev[0]*ev[2] + ev[1]*ev[2];
  J3 = ev[0]*ev[1]*ev[2];
}

static void
_k_mu(double k[3], const double mu[3]) {
  double stdv;

  K1 = 3*MU1;
  stdv = sqrt(MU2);
  K2 = SQRT3*stdv;
  K3 = stdv ? SQRT2*MU3/(stdv*stdv*stdv) : 0;
}

static void
_r_ev(double r[3], const double ev[3]) {
  double mu[3], stdv;

  _mu_ev(mu, ev);
  R1 = sqrt(ev[0]*ev[0] + ev[1]*ev[1] + ev[2]*ev[2]);
  stdv = sqrt(MU2);
  R2 = R1 ? (3/SQRT2)*stdv/R1 : 0;
  R3 = stdv ? SQRT2*MU3/(stdv*stdv*stdv) : 0;
}

static void
_r_mu(double r[3], const double mu[3]) {
  double stdv;

  R1 = sqrt(3*(MU1*MU1 + MU2));
  stdv = sqrt(MU2);
  R2 = R1 ? (3/SQRT2)*stdv/R1 : 0;
  R3 = stdv ? SQRT2*MU3/(stdv*stdv*stdv) : 0;
}

static void
_ev_wp(double ev[3], const double wp[3]) {

  ev[0] = wp[0] + wp[1]*cos(wp[2]);
  ev[1] = wp[0] + wp[1]*cos(wp[2] - 2*AIR_PI/3);
  ev[2] = wp[0] + wp[1]*cos(wp[2] + 2*AIR_PI/3);
}

static void
_wp_mu(double wp[3], const double mu[3]) {
  double stdv, mode;
  
  wp[0] = MU1;
  stdv = sqrt(MU2);
  wp[1] = SQRT2*stdv;
  mode = stdv ? SQRT2*MU3/(stdv*stdv*stdv) : 0;
  mode = AIR_CLAMP(-1, mode, 1);
  wp[2] = acos(AIR_CLAMP(-1, mode, 1))/3;
}

static void
_mu_j(double mu[3], const double j[3]) {

  MU1 = J1/3;
  MU2 = 2*(J1*J1 - 3*J2)/9;
  MU3 = 2*J1*J1*J1/27 - J1*J2/3 + J3;
}

static void
_r_j(double r[3], const double j[3]) {
  double mu[3], stdv;

  R1 = sqrt(J1*J1 - 2*J2);
  R2 = sqrt(J1*J1 - 3*J2)/R1;
  _mu_j(mu, j);
  stdv = sqrt(MU2);
  R3 = stdv ? SQRT2*MU3/(stdv*stdv*stdv) : 0;
}

static void
_k_r(double k[3], const double r[3]) {
  
  K1 = R1*sqrt(3 - 2*R2*R2);
  K2 = (SQRT2/SQRT3)*R1*R2;
  K3 = R3;
}

/*
_j_r(double j[3], const double r[3]) {
  double ss, nmu3;
  
  J1 = R1*sqrt(3 - 2*R2*R2);
  J2 = R1*R1*(1 - R2*R2);
  ss = R1*R2;
  nmu3 = 2*R3*ss*ss*ss;
  J3 = 
}
*/

static void
_wp_k(double wp[3], const double k[3]) {

  wp[0] = K1/3;
  wp[1] = (SQRT2/SQRT3)*K2;
  wp[2] = acos(AIR_CLAMP(-1, K3, 1))/3;
}

static void
_k_wp(double k[3], const double wp[3]) {

  K1 = 3*wp[0];
  K2 = (SQRT3/SQRT2)*wp[1];
  K3 = cos(3*wp[2]);
}

static void
_rtz_xyz(double rThZ[3], const double XYZ[3]) {

  rThZ[0] = sqrt(XYZ[0]*XYZ[0] + XYZ[1]*XYZ[1]);
  rThZ[1] = atan2(XYZ[1], XYZ[0]);
  rThZ[2] = XYZ[2];
}

static void
_rtp_xyz(double RThPh[3], const double XYZ[3]) {

  RThPh[0] = sqrt(XYZ[0]*XYZ[0] + XYZ[1]*XYZ[1] + XYZ[2]*XYZ[2]);
  RThPh[1] = atan2(XYZ[1], XYZ[0]);
  RThPh[2] = atan2(sqrt(XYZ[0]*XYZ[0] + XYZ[1]*XYZ[1]), XYZ[2]);
}

static void
_xyz_rtz(double XYZ[3], const double rThZ[3]) {
  
  XYZ[0] = rThZ[0]*cos(rThZ[1]);
  XYZ[1] = rThZ[0]*sin(rThZ[1]);
  XYZ[2] = rThZ[2];
}

static void
_xyz_rtp(double XYZ[3], const double RThPh[3]) {

  XYZ[0] = RThPh[0]*cos(RThPh[1])*sin(RThPh[2]);
  XYZ[1] = RThPh[0]*sin(RThPh[1])*sin(RThPh[2]);
  XYZ[2] = RThPh[0]*cos(RThPh[2]);
}

static void
_rtz_k(double rThZ[3], const double k[3]) {

  rThZ[0] = K2;
  rThZ[1] = acos(AIR_CLAMP(-1, K3, 1))/3;
  rThZ[2] = K1/SQRT3;
}

static void
_k_rtz(double k[3], const double rThZ[3]) {

  K1 = SQRT3*rThZ[2];
  K2 = rThZ[0];
  K3 = cos(3*rThZ[1]);
}

static void
_rtp_r(double RThPh[3], const double r[3]) {

  RThPh[0] = R1;
  RThPh[1] = acos(AIR_CLAMP(-1, R3, 1))/3;
  RThPh[2] = asin(AIR_CLAMP(-1, (SQRT2/SQRT3)*R2, 1));
}

static void
_r_rtp(double r[3], const double RThPh[3]) {

  R1 = RThPh[0];
  R2 = sin(RThPh[2])*SQRT3/SQRT2;
  R3 = cos(3*RThPh[1]);
}

static void
_wp_rtz(double wp[3], const double rThZ[3]) {
  
  wp[0] = rThZ[2]/SQRT3;
  wp[1] = (SQRT2/SQRT3)*rThZ[0];
  wp[2] = rThZ[1];
}

static void
_rtz_wp(double rThZ[3], const double wp[3]) {
  
  rThZ[0] = (SQRT3/SQRT2)*wp[1];
  rThZ[1] = wp[2];
  rThZ[2] = SQRT3*wp[0];
}

#define CONVERT1(dst, mid, src) \
static void \
_##dst##_##src(double dst[3], const double src[3]) { \
  double mid[3]; \
  _##mid##_##src(mid, src); \
  _##dst##_##mid(dst, mid); \
}

#define CONVERT2(dst, mdB, mdA, src) \
static void \
_##dst##_##src(double dst[3], const double src[3]) { \
  double mdA[3], mdB[3];  \
  _##mdA##_##src(mdA, src); \
  _##mdB##_##mdA(mdB, mdA); \
  _##dst##_##mdB(dst, mdB); \
}

CONVERT1(ev, xyz, rtz)    /* _ev_rtz */
CONVERT1(rtz, xyz, ev)    /* _rtz_ev */
CONVERT1(ev, xyz, rtp)    /* _ev_rtp */
CONVERT1(rtp, xyz, ev)    /* _rtp_ev */

CONVERT1(k, mu, ev)       /* _k_ev */
CONVERT1(wp, mu, ev)      /* _wp_ev */

CONVERT1(ev, wp, mu)      /* _ev_mu */

CONVERT2(ev, wp, mu, j)   /* _ev_j */
CONVERT1(ev, wp, k)       /* _ev_k */

CONVERT2(ev, xyz, rtp, r) /* _ev_r */

static tenTripleConverter
_convert[TEN_TRIPLE_TYPE_MAX+1][TEN_TRIPLE_TYPE_MAX+1] = {
  {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
  /* DEST:    SRC:  ev      mu      xyz       rtz       rtp       J      K       R       WP */
  /* ev */  {NULL, _iden,   _ev_mu, _ev_xyz,  _ev_rtz,  _ev_rtp,  _ev_j, _ev_k,  _ev_r,  _ev_wp},
  /* mu */  {NULL, _mu_ev,  _iden,  NULL,     NULL,     NULL,     _mu_j, NULL,   NULL,   NULL},
  /* xyz */ {NULL, _xyz_ev, NULL,   _iden,    _xyz_rtz, _xyz_rtp, NULL,  NULL,   NULL,   NULL},
  /* rtz */ {NULL, _rtz_ev, NULL,   _rtz_xyz, _iden,    NULL,     NULL,  _rtz_k, NULL,   _rtz_wp},
  /* rtp */ {NULL, _rtp_ev, NULL,   _rtp_xyz, NULL,     _iden,    NULL,  NULL,   _rtp_r, NULL},
  /* J */   {NULL, _j_ev,   NULL,   NULL,     NULL,     NULL,     _iden, NULL,   NULL,   NULL},
  /* K */   {NULL, _k_ev,   _k_mu,  NULL,     _k_rtz,   NULL,     NULL,  _iden,  _k_r,   _k_wp},
  /* R */   {NULL, _r_ev,   _r_mu,  NULL,     NULL,     _r_rtp,   _r_j,  NULL,   _iden,  NULL},
  /* WP */  {NULL, _wp_ev,  _wp_mu, NULL,     _wp_rtz,  NULL,     NULL,  _wp_k,   NULL,  _iden}};

void
tenTripleConvertSingle_d(double dst[3], int dstType,
                         const double src[3], const int srcType) {
  char me[]="tenTripleConvertSingle_d";
  int direct;

  if (airEnumValCheck(tenTripleType, dstType)
      || airEnumValCheck(tenTripleType, srcType)) {
    /* got invalid source or destination type */
    ELL_3V_SET(dst, AIR_NAN, AIR_NAN, AIR_NAN);
    return;
  }

  if (_convert[dstType][srcType]) {
    /* we have a direct converter */
    _convert[dstType][srcType](dst, src);
    direct = AIR_TRUE;
  } else {
    double eval[3];
    /* else, for lack of anything clever, we convert via evals */
    _convert[tenTripleTypeEigenvalue][srcType](eval, src);
    _convert[dstType][tenTripleTypeEigenvalue](dst, eval);
    direct = AIR_FALSE;
  }

  /* warn if conversion created non-existant values from
     existant input */
  if (ELL_3V_EXISTS(src) && !ELL_3V_EXISTS(dst)) {
    fprintf(stderr, "%s: problem? (%s) %g %g %g <-%s- (%s) %g %g %g\n", me,
            airEnumStr(tenTripleType, dstType),
            dst[0], dst[1], dst[2], 
            direct ? "-" : "...",
            airEnumStr(tenTripleType, srcType),
            src[0], src[1], src[2]);
  }
  
  return;
}

void
tenTripleConvertSingle_f(float _dst[3], int dstType,
                         const float _src[3], const int srcType) {
  double dst[3], src[3];

  ELL_3V_COPY(src, _src);
  tenTripleConvertSingle_d(dst, dstType, src, srcType);
  ELL_3V_COPY_TT(_dst, float, dst);
}

void
tenTripleCalcSingle_d(double dst[3], int ttype, double ten[7]) {
  double eval[3];

  /* in time this can become more efficient ... */
  switch (ttype) {
  case tenTripleTypeEigenvalue:
    tenEigensolve_d(dst, NULL, ten);
    break;
  case tenTripleTypeMoment:
  case tenTripleTypeXYZ:
  case tenTripleTypeRThetaZ:
  case tenTripleTypeRThetaPhi:
  case tenTripleTypeK:
  case tenTripleTypeJ:
  case tenTripleTypeWheelParm:
    tenEigensolve_d(eval, NULL, ten);
    tenTripleConvertSingle_d(dst, ttype, eval, tenTripleTypeEigenvalue);
    break;
  case tenTripleTypeR:
    dst[0] = sqrt(_tenAnisoTen_d[tenAniso_S](ten));
    dst[1] = _tenAnisoTen_d[tenAniso_FA](ten);
    dst[2] = _tenAnisoTen_d[tenAniso_Mode](ten);
    break;
  default:
    /* what on earth? */
    ELL_3V_SET(dst, AIR_NAN, AIR_NAN, AIR_NAN);
  }
  return;
}

void
tenTripleCalcSingle_f(float dst[3], int ttype, float ten[7]) {
  double dst_d[3], ten_d[7];

  TEN_T_COPY(ten_d, ten);
  tenTripleCalcSingle_d(dst_d, ttype, ten_d);
  ELL_3V_COPY_TT(dst, float, dst_d);
  return;
}

int
tenTripleCalc(Nrrd *nout, int ttype, const Nrrd *nten) {
  char me[]="tenTripleCalc", err[BIFF_STRLEN];
  size_t II, NN, size[NRRD_DIM_MAX];
  double (*ins)(void *, size_t, double), (*lup)(const void *, size_t);

  if (!( nout && nten )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenTripleType, ttype)) {
    sprintf(err, "%s: got invalid %s (%d)", me,
            tenTripleType->name, ttype);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nten, nrrdTypeDefault, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a valid DT array", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( nrrdTypeFloat == nten->type ||
         nrrdTypeDouble == nten->type )) {
    sprintf(err, "%s: need input type %s or %s, not %s\n", me, 
            airEnumStr(nrrdType, nrrdTypeFloat),
            airEnumStr(nrrdType, nrrdTypeFloat),
            airEnumStr(nrrdType, nten->type));
  }
  
  nrrdAxisInfoGet_nva(nten, nrrdAxisInfoSize, size);
  size[0] = 3;
  if (nrrdMaybeAlloc_nva(nout, nten->type, nten->dim, size)) {
    sprintf(err, "%s: couldn't alloc output", me);
    biffMove(TEN, err, NRRD); return 1;
  }

  NN = nrrdElementNumber(nten)/7;
  lup = nrrdDLookup[nten->type];
  ins = nrrdDInsert[nten->type];
  for (II=0; II<NN; II++) {
    double ten[7], trip[3];
    unsigned int vv;
    for (vv=0; vv<7; vv++) {
      ten[vv] = lup(nten->data, vv + 7*II);
    }
    tenTripleCalcSingle_d(trip, ttype, ten);
    for (vv=0; vv<3; vv++) {
      ins(nout->data, vv + 3*II, trip[vv]);
    }
  }
  if (nrrdAxisInfoCopy(nout, nten, NULL, (NRRD_AXIS_INFO_SIZE_BIT))) {
    sprintf(err, "%s: couldn't copy axis info", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  nout->axis[0].kind = nrrdKindUnknown;
  if (nrrdBasicInfoCopy(nout, nten,
                        NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }

  return 0;
}

int
tenTripleConvert(Nrrd *nout, int dstType,
                 const Nrrd *nin, int srcType) {
  char me[]="tenTripleConvert", err[BIFF_STRLEN];
  size_t II, NN;
  double (*ins)(void *, size_t, double), (*lup)(const void *, size_t);

  if (!( nout && nin )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if ( airEnumValCheck(tenTripleType, dstType) ||
       airEnumValCheck(tenTripleType, srcType) ) {
    sprintf(err, "%s: got invalid %s dst (%d) or src (%d)", me, 
            tenTripleType->name, dstType, srcType);
    biffAdd(TEN, err); return 1;
  }
  if (3 != nin->axis[0].size) {
    sprintf(err, "%s: need axis[0].size 3, not " _AIR_SIZE_T_CNV, me,
            nin->axis[0].size);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    sprintf(err, "%s: input has non-scalar %s type",
            me, airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(TEN, err); return 1;
  }

  if (nrrdCopy(nout, nin)) {
    sprintf(err, "%s: couldn't initialize output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  lup = nrrdDLookup[nin->type];
  ins = nrrdDInsert[nout->type];
  NN = nrrdElementNumber(nin)/3;
  for (II=0; II<NN; II++) {
    double src[3], dst[3];
    src[0] = lup(nin->data, 0 + 3*II);
    src[1] = lup(nin->data, 1 + 3*II);
    src[2] = lup(nin->data, 2 + 3*II);
    tenTripleConvertSingle_d(dst, dstType, src, srcType);
    ins(nout->data, 0 + 3*II, dst[0]);
    ins(nout->data, 1 + 3*II, dst[1]);
    ins(nout->data, 2 + 3*II, dst[2]);
  }

  return 0;
}
