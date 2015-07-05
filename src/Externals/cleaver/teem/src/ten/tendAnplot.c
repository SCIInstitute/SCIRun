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

#define INFO "Graph anisotropy metric in barycentric coords"
char *_tend_anplotInfoL =
  (INFO
   ".  The metrics all vary from 0.0 to 1.0, and will be sampled "
   "in the lower right half of the image.  The plane on which they are "
   "sampled is a surface of constant trace.  You may want to use "
   "\"unu resample -s = x0.57735 -k tent\" to transform the triangle into "
   "a 30-60-90 triangle, and \"ilk -t 1,-0.5,0,0,0.866,0 -k tent "
   "-0 u:0,1 -b pad -bg 0\" (possibly followed by "
   "teem/src/limntest/triimg) to transform the domain into an equilateral "
   "triangle.");

int
tend_anplotMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  int res, aniso, whole, nanout, hflip;
  Nrrd *nout;
  char *outS;

  hestOptAdd(&hopt, "r", "res", airTypeInt, 1, 1, &res, "256",
             "resolution of anisotropy plot");
  hestOptAdd(&hopt, "w", NULL, airTypeInt, 0, 0, &whole, NULL,
             "sample the whole triangle of constant trace, "
             "instead of just the "
             "sixth of it in which the eigenvalues have the "
             "traditional sorted order. ");
  hestOptAdd(&hopt, "hflip", NULL, airTypeInt, 0, 0, &hflip, NULL,
             "flip the two bottom corners (swapping the place of "
             "linear and planar)");
  hestOptAdd(&hopt, "nan", NULL, airTypeInt, 0, 0, &nanout, NULL,
             "set the pixel values outside the triangle to be NaN, "
             "instead of 0");
  hestOptAdd(&hopt, "a", "aniso", airTypeEnum, 1, 1, &aniso, NULL,
             "Which anisotropy metric to plot.  " TEN_ANISO_DESC,
             NULL, tenAniso);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output image (floating point)");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_anplotInfoL);
  JUSTPARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (tenAnisoPlot(nout, aniso, res, hflip, whole, nanout)) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble making plot:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  
  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
/* TEND_CMD(anplot, INFO); */
unrrduCmd tend_anplotCmd = { "anplot", INFO, tend_anplotMain };

