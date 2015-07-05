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

#include "bane.h"

const char *
baneBiffKey = "bane";

int
baneDefVerbose = 0;

int
baneDefMakeMeasrVol = AIR_TRUE;

double
baneDefIncLimit = 0.80;  /* throwing away more than 20% is too much */

int
baneDefRenormalize = AIR_TRUE;

int
baneDefPercHistBins = 1024;

int
baneStateHistEqBins = 4096;

int
baneStateHistEqSmart = 2;

int
baneHack = 0;

const char
baneDefLHKernel00[] = "cubic:0.333,0.333";

const char
baneDefLHKernel11[] = "cubicd:0.333,0.333";
