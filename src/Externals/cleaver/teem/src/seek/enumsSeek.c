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

#include "seek.h"

const char *
seekBiffKey = "seek";

char
_seekTypeStr[SEEK_TYPE_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_feature)",
  "isocontour",
  "ridge surface",
  "valley surface",
  "ridge line",
  "valley line",
  "minimal surface",
  "maximal surface",
};

char
_seekTypeDesc[SEEK_TYPE_MAX+1][AIR_STRLEN_MED] = {
  "unknown_feature",
  "standard marching cubes surface",
  "ridge surface",
  "valley surface",
  "ridge line",
  "valley line",
  "minimal surface",
  "maximal surface",
};

char
_seekTypeStrEqv[][AIR_STRLEN_SMALL] = {
  "isocontour",
  "ridge surface", "ridgesurface", "rs",
  "valley surface", "valleysurface", "vs",
  "ridge line", "ridgeline", "rl",
  "valley line", "valleyline", "vl",
  "minimal surface", "mins",
  "maximal surface", "maxs",
  ""
};

int
_seekTypeValEqv[] = {
  seekTypeIsocontour,
  seekTypeRidgeSurface, seekTypeRidgeSurface, seekTypeRidgeSurface,
  seekTypeValleySurface, seekTypeValleySurface, seekTypeValleySurface,
  seekTypeRidgeLine, seekTypeRidgeLine, seekTypeRidgeLine,
  seekTypeValleyLine, seekTypeValleyLine, seekTypeValleyLine,
  seekTypeMinimalSurface, seekTypeMinimalSurface,
  seekTypeMaximalSurface, seekTypeMaximalSurface,
};

airEnum
_seekType = {
  "format",
  SEEK_TYPE_MAX,
  _seekTypeStr,  NULL,
  _seekTypeDesc,
  _seekTypeStrEqv, _seekTypeValEqv,
  AIR_FALSE
};
airEnum *
seekType = &_seekType;

