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
#include "privateSeek.h"

seekContext *
seekContextNew(void) {
  seekContext *sctx;
  unsigned int fi;

  sctx = (seekContext *)calloc(1, sizeof(seekContext));
  if (sctx) {
    sctx->verbose = 0;
    sctx->ninscl = NULL;
    sctx->gctx = NULL;
    sctx->pvl = NULL;
    sctx->type = seekTypeUnknown;
    sctx->sclvItem = -1;
    sctx->normItem = -1;
    sctx->gradItem = -1;
    sctx->evalItem = -1;
    sctx->evecItem = -1;
    sctx->stngItem = -1;
    sctx->lowerInside = AIR_FALSE;
    sctx->normalsFind = AIR_FALSE;
    sctx->strengthUse = AIR_FALSE;
    sctx->strengthSign = 1;
    sctx->isovalue = AIR_NAN;
    sctx->strength = 0.0;
    sctx->strengthMin = 0.0;
    ELL_3V_SET(sctx->samples, 0, 0, 0);
    /* these two magic values assume a certain level of surface smoothness,
       which certainly does not apply to all cases */
    sctx->facesPerVoxel = 2.15;
    sctx->vertsPerVoxel = 1.15;
    sctx->pldArrIncr = 2048;

    sctx->nin = NULL;
    sctx->flag = AIR_CAST(int *, calloc(flagLast, sizeof(int)));
    for (fi=flagUnknown+1; fi<flagLast; fi++) {
      sctx->flag[fi] = AIR_FALSE;
    }
    sctx->baseDim = 0;
    sctx->_shape = gageShapeNew();
    sctx->shape = NULL;
    sctx->nsclDerived = nrrdNew();
    sctx->sclvAns = NULL;
    sctx->normAns = NULL;
    sctx->gradAns = NULL;
    sctx->evalAns = NULL;
    sctx->evecAns = NULL;
    sctx->stngAns = NULL;
    sctx->reverse = AIR_FALSE;
    ELL_3M_IDENTITY_SET(sctx->txfNormal);
    sctx->spanSize = 300;
    sctx->nspanHist = nrrdNew();
    sctx->range = nrrdRangeNew(AIR_NAN, AIR_NAN);
    sctx->sx = 0;
    sctx->sy = 0;
    sctx->sz = 0;
    ELL_4M_IDENTITY_SET(sctx->txfIdx);
    sctx->nvidx = nrrdNew();
    sctx->nsclv = nrrdNew();
    sctx->ngrad = nrrdNew();
    sctx->neval = nrrdNew();
    sctx->nevec = nrrdNew();
    sctx->nflip = nrrdNew();
    sctx->nstng = nrrdNew();
    sctx->vidx = NULL;
    sctx->sclv = NULL;
    sctx->grad = NULL;
    sctx->eval = NULL;
    sctx->evec = NULL;
    sctx->flip = NULL;
    sctx->stng = NULL;
    sctx->voxNum = 0;
    sctx->vertNum = 0;
    sctx->faceNum = 0;
    sctx->strengthSeenMax = AIR_NAN;
    sctx->time = AIR_NAN;
  }  
  return sctx;
}

seekContext *
seekContextNix(seekContext *sctx) {

  if (sctx) {
    sctx->flag = airFree(sctx->flag);
    sctx->_shape = gageShapeNix(sctx->_shape);
    sctx->nsclDerived = nrrdNuke(sctx->nsclDerived);
    sctx->nspanHist = nrrdNuke(sctx->nspanHist);
    sctx->range = nrrdRangeNix(sctx->range);
    sctx->nvidx = nrrdNuke(sctx->nvidx);
    sctx->nsclv = nrrdNuke(sctx->nsclv);
    sctx->ngrad = nrrdNuke(sctx->ngrad);
    sctx->neval = nrrdNuke(sctx->neval);
    sctx->nevec = nrrdNuke(sctx->nevec);
    sctx->nflip = nrrdNuke(sctx->nflip);
    sctx->nstng = nrrdNuke(sctx->nstng);
    airFree(sctx);
  }
  return NULL;
}

