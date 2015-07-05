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


/*********** THIS IS A HACK !!!
 *********** THIS ISN'T REALLY A SOURCE FILE !!!
 *********** ITS JUST A MACRO (sorry) */

  /* fw? + N*?
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

#define DOT_N(ANS, a, b) \
  for (T=0.0f,i=0; i<fd; i++) { T += (a)[i]*(b)[i]; } \
  ANS = T
#define VL_N(ANS, i, axis) DOT_N(ANS, fw0 + (axis)*fd, iv##axis + i*fd)
#define D1_N(ANS, i, axis) DOT_N(ANS, fw1 + (axis)*fd, iv##axis + i*fd)
#define D2_N(ANS, i, axis) DOT_N(ANS, fw2 + (axis)*fd, iv##axis + i*fd)

  /* x0 */
  for (j=0; j<fd*fd; j++) { VL_N(ivY[j],j,X); }
  /* x0y0 */
  for (j=0; j<fd; j++) { VL_N(ivZ[j],j,Y); }
  /* x0y0z0 */
  if (doV) {
    VL_N(*val,0,Z);                           /* f */
  }

  if (!( doD1 || doD2 ))
    return;

  /* x0y0z1 */
  if (doD1) {
    D1_N(gvec[2],0,Z);                        /* g_z */
  }
  if (doD2) {
    /* x0y0z2 */
    D2_N(hess[8],0,Z);                        /* h_zz */
  }
  /* x0y1 */
  for (j=0; j<fd; j++) { D1_N(ivZ[j],j,Y); }
  /* x0y1z0 */
  if (doD1) {
    VL_N(gvec[1],0,Z);                        /* g_y */
  }
  if (doD2) {
    /* x0y1z1 */
    D1_N(hess[7],0,Z);                        /* h_yz */
    hess[5] = hess[7];
    /* x0y2 */
    for (j=0; j<fd; j++) { D2_N(ivZ[j],j,Y); }
    /* x0y2z0 */
    VL_N(hess[4],0,Z);                        /* h_yy */
  }
  /* x1 */
  for (j=0; j<fd*fd; j++) { D1_N(ivY[j],j,X); }
  /* x1y0 */
  for (j=0; j<fd; j++) { VL_N(ivZ[j],j,Y); }
  /* x1y0z0 */
  if (doD1) {
    VL_N(gvec[0],0,Z);                        /* g_x */
  }

  ell_3mv_mul_d(gvec, shape->ItoWSubInvTransp, gvec);

  if (!doD2)
    return;

  /* x1y0z1 */
  D1_N(hess[6],0,Z);                          /* h_xz */
  hess[2] = hess[6];
  /* x1y1 */
  for (j=0; j<fd; j++) { D1_N(ivZ[j],j,Y); }
  /* x1y1z0 */
  VL_N(hess[3],0,Z);                          /* h_xy */
  hess[1] = hess[3];
  /* x2 */
  for (j=0; j<fd*fd; j++) { D2_N(ivY[j],j,X); }
  /* x2y0 */
  for (j=0; j<fd; j++) { VL_N(ivZ[j],j,Y); }
  /* x2y0z0 */
  VL_N(hess[0],0,Z);                          /* h_xx */

  if (1) {
    double matA[9];
    ELL_3M_MUL(matA, shape->ItoWSubInvTransp, hess);
    ELL_3M_MUL(hess, matA, shape->ItoWSubInv);
  }

#undef DOT_N
#undef VL_N
#undef D1_N
#undef D2_N
