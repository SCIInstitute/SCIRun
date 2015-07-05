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

/*
** The functions gageScl3PFilter2, gageScl3PFilter4, and
** gageScl3PFilterN weren't really intended to be public, but they
** have become so because they implement the brains of the lowest
** level of dot-products needed to do the convolution-based
** reconstruction, and, to do the the transforms that affect gradient
** and hessian values.  These functions are very useful for the
** implementation of other gageKinds, in which non-scalar values are
** filtered per-component.
*/

/*
** Teem trivia: "3P" stands for "3-pack", the kind of kernel set used
** (k00, k11, and k22).  6-pack filtering is still unimplemented.
*/

#define X 0
#define Y 1
#define Z 2

void
gageScl3PFilter2(gageShape *shape,
                 double *ivX, double *ivY, double *ivZ,
                 double *fw0, double *fw1, double *fw2,
                 double *val, double *gvec, double *hess,
                 int doV, int doD1, int doD2) {

  /* fw? + 2*?
       |     |  
       |     +- along which axis (0:x, 1:y, 2:z)
       |
       + what information (0:value, 1:1st deriv, 2:2nd deriv)

     ivX: 3D cube cache of original volume values
          (its scanlines are along the X axis)
     ivY: 2D square cache of intermediate filter results
          (its scanlines are along the Y axis)
     ivZ: 1D linear cache of intermediate filter results
          (it is a scanline along the Z axis)
  */

#define DOT_2(a, b) ((a)[0]*(b)[0] + (a)[1]*(b)[1])
#define VL_2(i, axis) DOT_2(fw0 + (axis)*2, iv##axis + i*2)
#define D1_2(i, axis) DOT_2(fw1 + (axis)*2, iv##axis + i*2)
#define D2_2(i, axis) DOT_2(fw2 + (axis)*2, iv##axis + i*2)

  /* x0 */
  ivY[0] = VL_2(0,X); /* interpolate values of 0th scanline along X axis */
  ivY[1] = VL_2(1,X);
  ivY[2] = VL_2(2,X);
  ivY[3] = VL_2(3,X);
  /* x0y0 */
  ivZ[0] = VL_2(0,Y);
  ivZ[1] = VL_2(1,Y);
  /* x0y0z0 */
  if (doV) {
    *val = VL_2(0,Z);                          /* f */
  }

  if (!( doD1 || doD2 ))
    return;

  /* x0y0z1 */
  if (doD1) {
    gvec[2] = D1_2(0,Z);                       /* g_z */
  }
  if (doD2) {
    /* actually, there is no possible way in which it makes sense to
       try to measure a second derivative with only two samples, so
       all this "if (doD2)" code is basically bogus, but we'll keep it
       around for generality ... */
    /* x0y0z2 */
    hess[8] = D2_2(0,Z);                       /* h_zz */
  }
  /* x0y1 */
  ivZ[0] = D1_2(0,Y);
  ivZ[1] = D1_2(1,Y);
  /* x0y1z0 */
  if (doD1) {
    gvec[1] = VL_2(0,Z);                       /* g_y */
  }
  if (doD2) {
    /* x0y1z1 */
    hess[5] = hess[7] = D1_2(0,Z);             /* h_yz */
    /* x0y2 */
    ivZ[0] = D2_2(0,Y);
    ivZ[1] = D2_2(1,Y);
    /* x0y2z0 */
    hess[4] = VL_2(0,Z);                       /* h_yy */
  }
  /* x1 */
  ivY[0] = D1_2(0,X);
  ivY[1] = D1_2(1,X);
  ivY[2] = D1_2(2,X);
  ivY[3] = D1_2(3,X);
  /* x1y0 */
  ivZ[0] = VL_2(0,Y);
  ivZ[1] = VL_2(1,Y);
  /* x1y0z0 */
  if (doD1) {
    gvec[0] = VL_2(0,Z);                       /* g_x */
  }

  ell_3mv_mul_d(gvec, shape->ItoWSubInvTransp, gvec);

  if (!doD2)
    return;

  /* x1y0z1 */
  hess[2] = hess[6] = D1_2(0,Z);               /* h_xz */
  /* x1y1 */
  ivZ[0] = D1_2(0,Y);
  ivZ[1] = D1_2(1,Y);
  /* x1y1z0 */
  hess[1] = hess[3] = VL_2(0,Z);               /* h_xy */
  /* x2 */
  ivY[0] = D2_2(0,X);
  ivY[1] = D2_2(1,X);
  ivY[2] = D2_2(2,X);
  ivY[3] = D2_2(3,X);
  /* x2y0 */
  ivZ[0] = VL_2(0,Y);
  ivZ[1] = VL_2(1,Y);
  /* x2y0z0 */
  hess[0] = VL_2(0,Z);                         /* h_xx */

  if (1) {
    double matA[9];
    ELL_3M_MUL(matA, shape->ItoWSubInvTransp, hess);
    ELL_3M_MUL(hess, matA, shape->ItoWSubInv);
  }

  return;
}

void
gageScl3PFilter4(gageShape *shape,
                 double *ivX, double *ivY, double *ivZ,
                 double *fw0, double *fw1, double *fw2,
                 double *val, double *gvec, double *hess,
                 int doV, int doD1, int doD2) {

  /* fw? + 4*?
       |     |  
       |     +- along which axis (0:x, 1:y, 2:z)
       |
       + what information (0:value, 1:1st deriv, 2:2nd deriv)

     ivX: 3D cube cache of original volume values
          (its scanlines are along the X axis)
     ivY: 2D square cache of intermediate filter results
          (its scanlines are along the Y axis)
     ivZ: 1D linear cache of intermediate filter results
          (it is a scanline along the Z axis)
  */

#define DOT_4(a,b) ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2]+(a)[3]*(b)[3])
#define VL_4(i, axis) DOT_4(fw0 + (axis)*4, iv##axis + i*4)
#define D1_4(i, axis) DOT_4(fw1 + (axis)*4, iv##axis + i*4)
#define D2_4(i, axis) DOT_4(fw2 + (axis)*4, iv##axis + i*4)

  /* x0 */
  ivY[ 0] = VL_4( 0,X);
  ivY[ 1] = VL_4( 1,X);
  ivY[ 2] = VL_4( 2,X);
  ivY[ 3] = VL_4( 3,X);
  ivY[ 4] = VL_4( 4,X);
  ivY[ 5] = VL_4( 5,X);
  ivY[ 6] = VL_4( 6,X);
  ivY[ 7] = VL_4( 7,X);
  ivY[ 8] = VL_4( 8,X);
  ivY[ 9] = VL_4( 9,X);
  ivY[10] = VL_4(10,X);
  ivY[11] = VL_4(11,X);
  ivY[12] = VL_4(12,X);
  ivY[13] = VL_4(13,X);
  ivY[14] = VL_4(14,X);
  ivY[15] = VL_4(15,X);
  /*
  */
  /* x0y0 */
  ivZ[ 0] = VL_4( 0,Y);
  ivZ[ 1] = VL_4( 1,Y);
  ivZ[ 2] = VL_4( 2,Y);
  ivZ[ 3] = VL_4( 3,Y);
  /* x0y0z0 */
  if (doV) {
    *val = VL_4( 0,Z);                          /* f */
    /* What on earth was this for? 
    if (AIR_ABS(505.6159668 - *val) < 0.0001) {
      printf("ivY =   % 10.4f   % 10.4f   % 10.4f   % 10.4f\n",
             ivY[ 0], ivY[ 1], ivY[ 2], ivY[ 3]);
      printf("ivY =   % 10.4f   % 10.4f   % 10.4f   % 10.4f\n",
             ivY[ 4], ivY[ 5], ivY[ 6], ivY[ 7]);
      printf("ivY =   % 10.4f   % 10.4f   % 10.4f   % 10.4f\n",
             ivY[ 8], ivY[ 9], ivY[10], ivY[11]);
      printf("ivY =   % 10.4f   % 10.4f   % 10.4f   % 10.4f\n",
             ivY[12], ivY[13], ivY[14], ivY[15]);
      printf("--> \nivZ =   % 10.4f   % 10.4f   % 10.4f   % 10.4f\n",
             ivZ[ 0], ivZ[ 1], ivZ[ 2], ivZ[ 3]);
      printf("--> *val = % 10.4f\n", (float)(*val));
    }
    */
  }

  if (!( doD1 || doD2 ))
    return;

  /* x0y0z1 */
  if (doD1) {
    gvec[2] = D1_4( 0,Z);                       /* g_z */
  }
  if (doD2) {
    /* x0y0z2 */
    hess[8] = D2_4( 0,Z);                       /* h_zz */
  }
  /* x0y1 */
  ivZ[ 0] = D1_4( 0,Y);
  ivZ[ 1] = D1_4( 1,Y);
  ivZ[ 2] = D1_4( 2,Y);
  ivZ[ 3] = D1_4( 3,Y);
  /* x0y1z0 */
  if (doD1) {
    gvec[1] = VL_4( 0,Z);                       /* g_y */
  }
  if (doD2) {
    /* x0y1z1 */
    hess[5] = hess[7] = D1_4( 0,Z);             /* h_yz */
    /* x0y2 */
    ivZ[ 0] = D2_4( 0,Y);
    ivZ[ 1] = D2_4( 1,Y);
    ivZ[ 2] = D2_4( 2,Y);
    ivZ[ 3] = D2_4( 3,Y);
    /* x0y2z0 */
    hess[4] = VL_4( 0,Z);                       /* h_yy */
  }
  /* x1 */
  ivY[ 0] = D1_4( 0,X);
  ivY[ 1] = D1_4( 1,X);
  ivY[ 2] = D1_4( 2,X);
  ivY[ 3] = D1_4( 3,X);
  ivY[ 4] = D1_4( 4,X);
  ivY[ 5] = D1_4( 5,X);
  ivY[ 6] = D1_4( 6,X);
  ivY[ 7] = D1_4( 7,X);
  ivY[ 8] = D1_4( 8,X);
  ivY[ 9] = D1_4( 9,X);
  ivY[10] = D1_4(10,X);
  ivY[11] = D1_4(11,X);
  ivY[12] = D1_4(12,X);
  ivY[13] = D1_4(13,X);
  ivY[14] = D1_4(14,X);
  ivY[15] = D1_4(15,X);
  /* x1y0 */
  ivZ[ 0] = VL_4( 0,Y);
  ivZ[ 1] = VL_4( 1,Y);
  ivZ[ 2] = VL_4( 2,Y);
  ivZ[ 3] = VL_4( 3,Y);
  /* x1y0z0 */
  if (doD1) {
    gvec[0] = VL_4( 0,Z);                       /* g_x */
  }

  ell_3mv_mul_d(gvec, shape->ItoWSubInvTransp, gvec);

  if (!doD2)
    return;

  /* x1y0z1 */
  hess[2] = hess[6] = D1_4( 0,Z);               /* h_xz */
  /* x1y1 */
  ivZ[ 0] = D1_4( 0,Y);
  ivZ[ 1] = D1_4( 1,Y);
  ivZ[ 2] = D1_4( 2,Y);
  ivZ[ 3] = D1_4( 3,Y);
  /* x1y1z0 */
  hess[1] = hess[3] = VL_4( 0,Z);               /* h_xy */
  /* x2 */
  ivY[ 0] = D2_4( 0,X);
  ivY[ 1] = D2_4( 1,X);
  ivY[ 2] = D2_4( 2,X);
  ivY[ 3] = D2_4( 3,X);
  ivY[ 4] = D2_4( 4,X);
  ivY[ 5] = D2_4( 5,X);
  ivY[ 6] = D2_4( 6,X);
  ivY[ 7] = D2_4( 7,X);
  ivY[ 8] = D2_4( 8,X);
  ivY[ 9] = D2_4( 9,X);
  ivY[10] = D2_4(10,X);
  ivY[11] = D2_4(11,X);
  ivY[12] = D2_4(12,X);
  ivY[13] = D2_4(13,X);
  ivY[14] = D2_4(14,X);
  ivY[15] = D2_4(15,X);
  /* x2y0 */
  ivZ[ 0] = VL_4( 0,Y);
  ivZ[ 1] = VL_4( 1,Y);
  ivZ[ 2] = VL_4( 2,Y);
  ivZ[ 3] = VL_4( 3,Y);
  /* x2y0z0 */
  hess[0] = VL_4( 0,Z);                         /* h_xx */

  if (1) {
    double matA[9];
    ELL_3M_MUL(matA, shape->ItoWSubInvTransp, hess);
    ELL_3M_MUL(hess, matA, shape->ItoWSubInv);
  }

  return;
}

void
gageScl3PFilter6(gageShape *shape, 
                 double *ivX, double *ivY, double *ivZ,
                 double *fw0, double *fw1, double *fw2,
                 double *val, double *gvec, double *hess,
                 int doV, int doD1, int doD2) {
  int i, j;
  double T;

#define fd 6
#include "scl3pfilterbody.c"
#undef fd

  return;
}

void
gageScl3PFilter8(gageShape *shape, 
                 double *ivX, double *ivY, double *ivZ,
                 double *fw0, double *fw1, double *fw2,
                 double *val, double *gvec, double *hess,
                 int doV, int doD1, int doD2) {
  int i, j;
  double T;

#define fd 8
#include "scl3pfilterbody.c"
#undef fd

  return;
}

void
gageScl3PFilterN(gageShape *shape, int fd,
                 double *ivX, double *ivY, double *ivZ,
                 double *fw0, double *fw1, double *fw2,
                 double *val, double *gvec, double *hess,
                 int doV, int doD1, int doD2) {
  int i, j;
  double T;

#include "scl3pfilterbody.c"

  return;
}

void
_gageSclFilter(gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_gageSclFilter";
  int fd;
  double *fw00, *fw11, *fw22;
  gageScl3PFilter_t *filter[5] = {NULL, gageScl3PFilter2, gageScl3PFilter4,
                                  gageScl3PFilter6, gageScl3PFilter8};

  fd = 2*ctx->radius;
  if (!ctx->parm.k3pack) {
    fprintf(stderr, "!%s: sorry, 6-pack filtering not implemented\n", me);
    return;
  }
  fw00 = ctx->fw + fd*3*gageKernel00;
  fw11 = ctx->fw + fd*3*gageKernel11;
  fw22 = ctx->fw + fd*3*gageKernel22;
  /* perform the filtering */
  if (fd <= 8) {
    filter[ctx->radius](ctx->shape, pvl->iv3, pvl->iv2, pvl->iv1, 
                        fw00, fw11, fw22,
                        pvl->directAnswer[gageSclValue],
                        pvl->directAnswer[gageSclGradVec],
                        pvl->directAnswer[gageSclHessian],
                        pvl->needD[0], pvl->needD[1], pvl->needD[2]);
  } else {
    gageScl3PFilterN(ctx->shape, fd,
                     pvl->iv3, pvl->iv2, pvl->iv1, 
                     fw00, fw11, fw22,
                     pvl->directAnswer[gageSclValue],
                     pvl->directAnswer[gageSclGradVec],
                     pvl->directAnswer[gageSclHessian],
                     pvl->needD[0], pvl->needD[1], pvl->needD[2]);
  }

  return;
}

#undef X
#undef Y
#undef Z
