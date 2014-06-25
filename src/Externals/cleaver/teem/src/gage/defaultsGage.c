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

const char *
gageBiffKey = "gage";

int
gageDefVerbose = 0;

double
gageDefGradMagCurvMin = 0.0;

int
gageDefRenormalize = AIR_FALSE;

int
gageDefCheckIntegrals = AIR_TRUE;

int
gageDefK3Pack = AIR_TRUE;

double
gageDefDefaultSpacing = 1.0;

int
gageDefCurvNormalSide = 1;

double
gageDefKernelIntegralNearZero = 0.0001;

int
gageDefRequireAllSpacings = AIR_TRUE;

int
gageDefRequireEqualCenters = AIR_TRUE;

int
gageDefDefaultCenter = nrrdCenterCell;

int
gageDefStackUse = AIR_FALSE; /* GLK doesn't see any reasonable circumstance
                                where this could be AIR_TRUE, since it
                                significantly changes the behavior of gage */

int
gageDefStackRenormalize = AIR_TRUE;
