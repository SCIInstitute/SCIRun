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

#include "../gage.h"

/* self-contained gage usage example */

int
main(int argc, char *argv[]) {
  char *me, *err;
  airArray *mop;
  Nrrd *nin;
  gageContext *ctx;
  gagePerVolume *pvl;
  const double *grad, *norm;
  double kparm[NRRD_KERNEL_PARMS_NUM];
  int E;

  me = argv[0];
  if (2 != argc) {
    fprintf(stderr, "usage: %s <nin>\n", me);
    return 1;
  }

  mop = airMopNew();
  nin = nrrdNew();
  airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);
  
  if (nrrdLoad(nin, argv[1], NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble loading\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  if (gageKindVolumeCheck(gageKindScl, nin)) {
    airMopAdd(mop, err=biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: didn't get a %s volume:\n%s\n", me,
            gageKindScl->name, err);
    airMopError(mop); return 1;
  }

  kparm[0] = 1.0; /* scale parameter, in units of samples */
  kparm[1] = 0.0; /* B */
  kparm[2] = 0.5; /* C */

  ctx = gageContextNew();
  airMopAdd(mop, ctx, (airMopper)gageContextNix, airMopAlways);
  E = 0;
  if (!E) E |= !(pvl = gagePerVolumeNew(ctx, nin, gageKindScl));
  if (!E) E |= gagePerVolumeAttach(ctx, pvl);
  if (!E) E |= gageKernelSet(ctx, gageKernel00, nrrdKernelBCCubic, kparm);
  if (!E) E |= gageKernelSet(ctx, gageKernel11, nrrdKernelBCCubicD, kparm);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclGradVec);
  if (!E) E |= gageQueryItemOn(ctx, pvl, gageSclNormal);
  if (!E) E |= gageUpdate(ctx);
  if (E) {
    airMopAdd(mop, err=biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble setting up Gage:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  grad = gageAnswerPointer(ctx, pvl, gageSclGradVec);
  norm = gageAnswerPointer(ctx, pvl, gageSclNormal);

  if (gageProbe(ctx, 1.1, 2.3, 6.8)) {
    fprintf(stderr, "%s: trouble:\n(%d) %s\n", me, ctx->errNum, ctx->errStr);
    airMopError(mop); return 1;
  }
  printf("gradient (%g,%g,%g) --> norm (%g,%g,%g)\n",
         grad[0], grad[1], grad[2], norm[0], norm[1], norm[2]);

  airMopOkay(mop);
  return 0;
}
