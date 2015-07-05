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

char
gageErrStr[AIR_STRLEN_LARGE]="";

int
gageErrNum=-1;

/*
******** gageZeroNormal[]
**
** this is the vector to supply when someone wants the normalized
** version of a vector with zero length.  We could be nasty and
** set this to {AIR_NAN, AIR_NAN, AIR_NAN}, but simply passing
** NANs around can make things fantastically slow ...
*/
double
gageZeroNormal[3] = {0,0,0};

char
_gageKernelStr[][AIR_STRLEN_SMALL] = {
  "(unknown_kernel)",
  "00",
  "10",
  "11",
  "20",
  "21",
  "22",
  "stack"
};

char
_gageKernelDesc[][AIR_STRLEN_MED] = {
  "unknown kernel",
  "kernel for reconstructing values",
  "kernel for reconstruction values when doing 1st derivatives",
  "kernel for measuring 1st derivatives when doing 1st derivatives",
  "kernel for reconstruction values when doing 2nd derivatives",
  "kernel for measuring 1st derivatives when doing 2nd derivatives",
  "kernel for measuring 2nd derivatives when doing 2nd derivatives",
  "kernel for reconstruction across a stack"
};

char
_gageKernelStrEqv[][AIR_STRLEN_SMALL] = {
  "00", "k00",
  "10", "k10",
  "11", "k11",
  "20", "k20",
  "21", "k21",
  "22", "k22",
  "stack", "ss", "kss",
  ""
};

int
_gageKernelValEqv[] = {
  gageKernel00, gageKernel00,
  gageKernel10, gageKernel10,
  gageKernel11, gageKernel11,
  gageKernel20, gageKernel20,
  gageKernel21, gageKernel21,
  gageKernel22, gageKernel22,
  gageKernelStack, gageKernelStack, gageKernelStack
};

airEnum
_gageKernel_enum = {
  "kernel",
  GAGE_KERNEL_MAX,
  _gageKernelStr, NULL,
  _gageKernelDesc,
  _gageKernelStrEqv, _gageKernelValEqv,
  AIR_FALSE
};
airEnum *
gageKernel = &_gageKernel_enum;

void
gageParmReset(gageParm *parm) {

  if (parm) {
    parm->renormalize = gageDefRenormalize;
    parm->checkIntegrals = gageDefCheckIntegrals;
    parm->k3pack = gageDefK3Pack;
    parm->gradMagCurvMin = gageDefGradMagCurvMin;
    parm->defaultSpacing = gageDefDefaultSpacing;
    parm->curvNormalSide = gageDefCurvNormalSide;
    parm->kernelIntegralNearZero = gageDefKernelIntegralNearZero;
    parm->requireAllSpacings = gageDefRequireAllSpacings;
    parm->requireEqualCenters = gageDefRequireEqualCenters;
    parm->defaultCenter = gageDefDefaultCenter;
    parm->stackUse = gageDefStackUse;
    parm->stackRenormalize = gageDefStackRenormalize;
  }
  return;
}

void
gagePointReset(gagePoint *point) {

  if (point) {
    /* learned: can't initialize the floating point to AIR_NAN, 
       non-dot-net windows compilers proclaim that QNAN == x
       for any existant x!!!  For some reason though, infinity
       is handled correctly */
    point->xf = point->yf = point->zf = AIR_POS_INF;
    point->xi = point->yi = point->zi = -1;
  }
  return;
}

void
gageItemSpecInit(gageItemSpec *isp) {

  if (isp) {
    isp->item = -1;
    isp->kind = NULL;
  }
  return;
}

gageItemSpec *
gageItemSpecNew(void) {
  gageItemSpec *isp;

  isp = (gageItemSpec *)calloc(1, sizeof(gageItemSpec));
  gageItemSpecInit(isp);
  return isp;
}

gageItemSpec *
gageItemSpecNix(gageItemSpec *isp) {

  if (isp) {
    airFree(isp);
  }
  return NULL;
}

char
_gageErrStr[GAGE_ERR_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown gageErr)",
  "none",
  "space bounds",
  "stack bounds",
  "stack integral"
};

airEnum
_gageErr = {
  "gageErr",
  GAGE_ERR_MAX,
  _gageErrStr, NULL,
  NULL,
  NULL, NULL,
  AIR_FALSE
};
airEnum *
gageErr = &_gageErr;
