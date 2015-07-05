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

#define INFO "Generate twisting helical tensor field"
char *_tend_helixInfoL =
  (INFO
   ". The main utility of such a field is to debug handling of coordinate "
   "systems in tensor field visualization.  The \"space directions\" and "
   "\"space origin\" fields of the NRRD header determines the mapping from "
   "coordinates in the index space of the image to coordinates in the "
   "world space in which the image is "
   "sampled.  The \"measurement frame\" field determines the mapping from "
   "the coordinates of the tensor itself, to coordinates of the world space. "
   "When these are correctly handled, the "
   "region of high anisotropy is a right-handed helix (same as DNA). "
   "Using differing axes sizes (via \"-s\") helps make sure that the "
   "raster ordering of axes is correct.  In addition, the tensors twist "
   "relative to the helix, which exposes handling of the measurement frame. "
   "If you trace paths guided by the principal eigenvector of the tensors, "
   "along the surface of the helical cylinder, you get another "
   "right-handed helix, as if the the tensor field is modeling the result "
   "if twisting a set of fibers into single-stranded helical bundle. ");

void
tend_helixDoit(Nrrd *nout, double bnd,
               double orig[3], double i2w[9], double mf[9],
               double r, double R, double S, double angle, int incrtwist,
               double ev[3], double bgEval) {
  int sx, sy, sz, xi, yi, zi;
  double th, t0, t1, t2, t3, v1, v2,
    wpos[3], vpos[3], mfT[9],
    W2H[9], H2W[9], H2C[9], C2H[9], fv[3], rv[3], uv[3], mA[9], mB[9], inside,
    tmp[3], len;
  float *out;

  sx = nout->axis[1].size;
  sy = nout->axis[2].size;
  sz = nout->axis[3].size;
  out = (float*)nout->data;
  ELL_3M_TRANSPOSE(mfT, mf);
  for (zi=0; zi<sz; zi++) {
    fprintf(stderr, "zi = %d/%d\n", zi, sz);
    for (yi=0; yi<sy; yi++) {
      for (xi=0; xi<sx; xi++) {
        ELL_3V_SET(tmp, xi, yi, zi);
        ELL_3MV_MUL(vpos, i2w, tmp);
        ELL_3V_INCR(vpos, orig);

#define WPOS(pos, th) ELL_3V_SET((pos),R*cos(th), R*sin(th), S*(th)/(2*AIR_PI))
#define VAL(th) (WPOS(wpos, th), ELL_3V_DIST(wpos, vpos))
#define RR 0.61803399
#define CC (1.0-RR)
#define SHIFT3(a,b,c,d) (a)=(b); (b)=(c); (c)=(d)
#define SHIFT2(a,b,c)   (a)=(b); (b)=(c)
        
        th = atan2(vpos[1], vpos[0]);
        th += 2*AIR_PI*floor(0.5 + vpos[2]/S - th/(2*AIR_PI));
        if (S*th/(2*AIR_PI) > vpos[2]) {
          t0 = th - AIR_PI; t3 = th;
        } else {
          t0 = th; t3 = th + AIR_PI;
        }
        t1 = RR*t0 + CC*t3;
        t2 = CC*t0 + RR*t3;
        v1 = VAL(t1);
        v2 = VAL(t2);
        while ( t3-t0 > 0.000001*(AIR_ABS(t1)+AIR_ABS(t2)) ) {
          if (v1 < v2) {
            SHIFT3(t3, t2, t1, CC*t0 + RR*t2);
            SHIFT2(v2, v1, VAL(t1));
          } else {
            SHIFT3(t0, t1, t2, RR*t1 + CC*t3);
            SHIFT2(v1, v2, VAL(t2));
          }
        }
        /* t1 (and t2) are now the th for which the point on the helix
           (R*cos(th), R*sin(th), S*(th)/(2*AIR_PI)) is closest to vpos */

        WPOS(wpos, t1);
        ELL_3V_SUB(wpos, vpos, wpos);
        ELL_3V_SET(fv, -R*sin(t1), R*cos(t1), S/AIR_PI);  /* helix tangent */
        ELL_3V_NORM(fv, fv, len);
        ELL_3V_COPY(rv, wpos);
        ELL_3V_NORM(rv, rv, len);
        len = ELL_3V_DOT(rv, fv);
        ELL_3V_SCALE(tmp, -len, fv);
        ELL_3V_ADD2(rv, rv, tmp);
        ELL_3V_NORM(rv, rv, len);  /* rv now normal to helix, closest to 
                                      pointing to vpos */
        ELL_3V_CROSS(uv, rv, fv);
        ELL_3V_NORM(uv, uv, len);  /* (rv,fv,uv) now right-handed frame */
        ELL_3MV_ROW0_SET(W2H, uv); /* as is (uv,rv,fv) */
        ELL_3MV_ROW1_SET(W2H, rv);
        ELL_3MV_ROW2_SET(W2H, fv);
        ELL_3M_TRANSPOSE(H2W, W2H);
        inside = 0.5 - 0.5*airErf((ELL_3V_LEN(wpos)-r)/(bnd + 0.0001));
        if (incrtwist) {
          th = angle*ELL_3V_LEN(wpos)/r;
        } else {
          th = angle;
        }
        ELL_3M_ROTATE_Y_SET(H2C, th);
        ELL_3M_TRANSPOSE(C2H, H2C);
        ELL_3M_SCALE_SET(mA,
                         AIR_LERP(inside, bgEval, ev[1]),
                         AIR_LERP(inside, bgEval, ev[2]),
                         AIR_LERP(inside, bgEval, ev[0]));
        ELL_3M_MUL(mB, mA, H2C);
        ELL_3M_MUL(mA, mB, W2H);
        ELL_3M_MUL(mB, mA, mf);
        ELL_3M_MUL(mA, C2H, mB);
        ELL_3M_MUL(mB, H2W, mA);
        ELL_3M_MUL(mA, mfT, mB);
        
        TEN_M2T_TT(out, float, mA);
        out[0] = 1.0;
        out += 7;
      }
    }
  }
  return;
}

int
tend_helixMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  int size[3], nit;
  Nrrd *nout;
  double R, r, S, bnd, angle, ev[3], ip[3], iq[4], mp[3], mq[4], tmp[9],
    orig[3], i2w[9], rot[9], mf[9], spd[4][3], bge;
  char *outS;

  hestOptAdd(&hopt, "s", "size", airTypeInt, 3, 3, size, NULL, 
             "sizes along fast, medium, and slow axes of the sampled volume, "
             "often called \"X\", \"Y\", and \"Z\".  It is best to use "
             "slightly different sizes here, to expose errors in interpreting "
             "axis ordering (e.g. \"-s 39 40 41\")");
  hestOptAdd(&hopt, "ip", "image orientation", airTypeDouble, 3, 3, ip,
             "0 0 0",
             "quaternion quotient space orientation of image");
  hestOptAdd(&hopt, "mp", "measurement orientation", airTypeDouble, 3, 3, mp,
             "0 0 0",
             "quaternion quotient space orientation of measurement frame");
  hestOptAdd(&hopt, "b", "boundary", airTypeDouble, 1, 1, &bnd, "10",
             "parameter governing how fuzzy the boundary between high and "
             "low anisotropy is. Use \"-b 0\" for no fuzziness");
  hestOptAdd(&hopt, "r", "little radius", airTypeDouble, 1, 1, &r, "30",
             "(minor) radius of cylinder tracing helix");
  hestOptAdd(&hopt, "R", "big radius", airTypeDouble, 1, 1, &R, "50",
             "(major) radius of helical turns");
  hestOptAdd(&hopt, "S", "spacing", airTypeDouble, 1, 1, &S, "100",
             "spacing between turns of helix (along its axis)");
  hestOptAdd(&hopt, "a", "angle", airTypeDouble, 1, 1, &angle, "60",
             "maximal angle of twist of tensors along path.  There is no "
             "twist at helical core of path, and twist increases linearly "
             "with radius around this path.  Positive twist angle with "
             "positive spacing resulting in a right-handed twist around a "
             "right-handed helix. ");
  hestOptAdd(&hopt, "nit", NULL, airTypeInt, 0, 0, &nit, NULL,
             "changes behavior of twist angle as function of distance from "
             "center of helical core: instead of increasing linearly as "
             "describe above, be at a constant angle");
  hestOptAdd(&hopt, "ev", "eigenvalues", airTypeDouble, 3, 3, ev,
             "0.006 0.002 0.001",
             "eigenvalues of tensors (in order) along direction of coil, "
             "circumferential around coil, and radial around coil. ");
  hestOptAdd(&hopt, "bg", "background", airTypeDouble, 1, 1, &bge, "0.5",
             "eigenvalue of isotropic background");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output file");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_helixInfoL);
  JUSTPARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 4,
                        AIR_CAST(size_t, 7),
                        AIR_CAST(size_t, size[0]),
                        AIR_CAST(size_t, size[1]),
                        AIR_CAST(size_t, size[2]))) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble allocating output:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  ELL_4V_SET(iq, 1.0, ip[0], ip[1], ip[2]);
  ell_q_to_3m_d(rot, iq);
  ELL_3V_SET(orig,
             -2*R + 2*R/size[0],
             -2*R + 2*R/size[1],
             -2*R + 2*R/size[2]);
  ELL_3M_ZERO_SET(i2w);
  ELL_3M_DIAG_SET(i2w, 4*R/size[0], 4*R/size[1], 4*R/size[2]);
  ELL_3MV_MUL(tmp, rot, orig);
  ELL_3V_COPY(orig, tmp);
  ELL_3M_MUL(tmp, rot, i2w);
  ELL_3M_COPY(i2w, tmp);
  ELL_4V_SET(mq, 1.0, mp[0], mp[1], mp[2]);
  ell_q_to_3m_d(mf, mq);
  tend_helixDoit(nout, bnd,
                 orig, i2w, mf,
                 r, R, S, angle*AIR_PI/180, !nit, ev, bge);
  nrrdSpaceSet(nout, nrrdSpaceRightAnteriorSuperior);
  nrrdSpaceOriginSet(nout, orig);
  ELL_3V_SET(spd[0], AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_3MV_COL0_GET(spd[1], i2w);
  ELL_3MV_COL1_GET(spd[2], i2w);
  ELL_3MV_COL2_GET(spd[3], i2w);
  nrrdAxisInfoSet_va(nout, nrrdAxisInfoSpaceDirection,
                     spd[0], spd[1], spd[2], spd[3]);
  nrrdAxisInfoSet_va(nout, nrrdAxisInfoCenter,
                     nrrdCenterUnknown, nrrdCenterCell,
                     nrrdCenterCell, nrrdCenterCell);
  nrrdAxisInfoSet_va(nout, nrrdAxisInfoKind,
                     nrrdKind3DMaskedSymMatrix, nrrdKindSpace,
                     nrrdKindSpace, nrrdKindSpace);
  nout->measurementFrame[0][0] = mf[0];
  nout->measurementFrame[1][0] = mf[1];
  nout->measurementFrame[2][0] = mf[2];
  nout->measurementFrame[0][1] = mf[3];
  nout->measurementFrame[1][1] = mf[4];
  nout->measurementFrame[2][1] = mf[5];
  nout->measurementFrame[0][2] = mf[6];
  nout->measurementFrame[1][2] = mf[7];
  nout->measurementFrame[2][2] = mf[8];

  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
TEND_CMD(helix, INFO);
