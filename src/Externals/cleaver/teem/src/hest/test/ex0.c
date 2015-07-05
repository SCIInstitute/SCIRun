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


#include "../hest.h"

int
main(int argc, char **argv) {
  static int res[2], v, numIn;
  static char **in, *out;
  hestOpt opt[] = {
    {"res",   "sx sy", airTypeInt,    2,  2,   res,  NULL, 
     "image resolution"},
    {"v",     "level", airTypeInt,    1,  1,   &v,   "0",
     "verbosity level"},
    {"out",   "file",  airTypeString, 1,  1,   &out, "output.ppm",
     "PPM image output"},
    {NULL,    "input",  airTypeString, 1, -1,   &in,  NULL,
     "input image file(s)", &numIn},
    {NULL, NULL, 0}
  };
  char *err = NULL;

  if (hestOptCheck(opt, &err)) {
    fprintf(stderr, "ERROR: %s\n", err); free(err); 
    exit(1);
  }
  printf("hestOpt array looks fine.\n");
  exit(0);
}
