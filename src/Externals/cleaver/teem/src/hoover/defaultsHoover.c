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

#include "hoover.h"

const char *
hooverBiffKey = "hoover";
int
hooverDefVolCentering = nrrdCenterNode;
int
hooverDefImgCentering = nrrdCenterCell;

char
_hooverErrStr[HOOVER_ERR_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_err)",
  "Initialization",
  "RenderBegin",
  "ThreadCreate",
  "ThreadBegin",
  "RayBegin",
  "Sample",
  "RayEnd",
  "ThreadEnd",
  "ThreadJoin",
  "RenderEnd"
};

airEnum
_hooverErr = {
  "error",
  HOOVER_ERR_MAX,
  _hooverErrStr, NULL,
  NULL,
  NULL, NULL,
  AIR_FALSE
};
airEnum *
hooverErr = &_hooverErr;


/*
  hooverErrNone,
  hooverErrInit, 
  hooverErrRenderBegin,
  hooverErrThreadCreate,
  hooverErrThreadBegin,
  hooverErrRayBegin,
  hooverErrSample,
  hooverErrRayEnd,
  hooverErrThreadEnd,
  hooverErrThreadJoin,
  hooverErrRenderEnd,
  hooverErrLast
*/
