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

/*
******** seekVerboseSet
**
*/
void
seekVerboseSet(seekContext *sctx, int verbose) {

  if (sctx) {
    sctx->verbose = verbose;
  }
  return;
}

/*
******** seekDataSet
**
** user sets EITHER: ninscl, or, gctx and pvlIdx
**
** if ninscl: this is a vanilla scalar volume, and we can do seekTypeIsocontour
** if gctx: this is a scalar or non-scalar volume, and we can do any seekType
**
** sets from input:
** ninscl, gctx, pvl
**
** So the rest of seek can use "if (sctx->ninscl)" to see if we're working
** with a vanilla scalar volume or not 
**
** invalidates:
** valItem, normItem, gradItem, evalItem, evecItem
*/
int
seekDataSet(seekContext *sctx, const Nrrd *ninscl,
            gageContext *gctx, unsigned int pvlIdx) {
  char me[]="seekDataSet", err[BIFF_STRLEN];

  if (!( sctx && (ninscl || gctx) )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(SEEK, err); return 1;
  }
  if (ninscl && gctx) {
    sprintf(err, "%s: must give ninscl or gctx, but not both", me);
    biffAdd(SEEK, err); return 1;
  }

  if (ninscl) {
    if (nrrdCheck(ninscl)) {
      sprintf(err, "%s: problem with volume", me);
      biffMove(SEEK, err, NRRD); return 1;
    }
    if (3 != ninscl->dim) {
      sprintf(err, "%s: vanilla scalar volume must be 3-D (not %d-D)",
              me, ninscl->dim);
      biffAdd(SEEK, err); return 1;
    }
    if (nrrdTypeBlock == ninscl->type) {
      sprintf(err, "%s: can't work with %s type values", me, 
              airEnumStr(nrrdType, nrrdTypeBlock));
      biffAdd(SEEK, err); return 1;
    }
    sctx->ninscl = ninscl;
    sctx->gctx = NULL;
    sctx->pvl = NULL;
  } else {
    if (!( pvlIdx < gctx->pvlNum )) {
      sprintf(err, "%s: pvlIdx %u not < pvlNum %u", me, pvlIdx, gctx->pvlNum);
      biffAdd(SEEK, err); return 1;
    }
    /* we assume that caller has done a gageUpdate(), so no other error
       checking is required (or really possible) here */
    sctx->ninscl = NULL;
    sctx->gctx = gctx;
    sctx->pvl = gctx->pvl[pvlIdx];
  }
  sctx->flag[flagData] = AIR_TRUE;

  sctx->sclvItem = -1;
  sctx->normItem = -1;
  sctx->gradItem = -1;
  sctx->evalItem = -1;
  sctx->evecItem = -1;

  return 0;
}

/*
******** seekSamplesSet
**
** sets: samples[3]
*/
int
seekSamplesSet(seekContext *sctx, size_t samples[3]) {
  char me[]="seekSamplesSet", err[BIFF_STRLEN];
  unsigned int numZero;

  if (!(sctx && samples)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(SEEK, err); return 1;
  }
  numZero = 0;
  numZero += 0 == samples[0];
  numZero += 0 == samples[1];
  numZero += 0 == samples[2];
  if (!( 0 == numZero || 3 == numZero )) {
    sprintf(err, "%s: samples (%u,%u,%u) must all be 0 or !=0 together", me,
            AIR_CAST(unsigned int, samples[0]),
            AIR_CAST(unsigned int, samples[1]),
            AIR_CAST(unsigned int, samples[2]));
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->samples[0] != samples[0]
      || sctx->samples[1] != samples[1]
      || sctx->samples[2] != samples[2]) {
    sctx->samples[0] = samples[0];
    sctx->samples[1] = samples[1];
    sctx->samples[2] = samples[2];
    sctx->flag[flagSamples] = AIR_TRUE;
  }
  return 0;
}

/*
******** seekTypeSet
**
** sets: featureType
*/
int
seekTypeSet(seekContext *sctx, int type) {
  char me[]="seekTypeSet", err[BIFF_STRLEN];

  if (!sctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(SEEK, err); return 1;
  }
  if (airEnumValCheck(seekType, type)) {
    sprintf(err, "%s: %d not a valid %s", me, type, seekType->name);
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->type != type) {
    sctx->type = type;
    sctx->flag[flagType] = AIR_TRUE;
  }
  return 0;
}

/*
********* seekLowerInsideSet
**
** sets: lowerInside
*/
int
seekLowerInsideSet(seekContext *sctx, int lowerInside) {
  char me[]="seekLowerInsideSet", err[BIFF_STRLEN];

  if (!sctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->lowerInside != lowerInside) {
    sctx->lowerInside = lowerInside;
    sctx->flag[flagLowerInside] = AIR_TRUE;
  }
  return 0;
}

/*
********* seekNormalsFindSet
**
** sets: normalsFind
*/
int
seekNormalsFindSet(seekContext *sctx, int normalsFind) {
  char me[]="seekNormalsFindSet", err[BIFF_STRLEN];

  if (!sctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->normalsFind != normalsFind) {
    sctx->normalsFind = normalsFind;
    sctx->flag[flagNormalsFind] = AIR_TRUE;
  }
  return 0;
}

int
seekStrengthUseSet(seekContext *sctx, int doit) {
  char me[]="seekStrengthUseSet", err[BIFF_STRLEN];

  if (!sctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->strengthUse != doit) {
    sctx->strengthUse = doit;
    sctx->flag[flagStrengthUse] = AIR_TRUE;
  }
  return 0;
}

int
seekStrengthSet(seekContext *sctx, int strengthSign,
                double strengthMin, double strength) {
  char me[]="seekStrengthSet", err[BIFF_STRLEN];

  if (!sctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(SEEK, err); return 1;
  }
  if (!(1 == strengthSign || -1 == strengthSign)) {
    sprintf(err, "%s: strengthSign (%d) not +1 or -1", me, strengthSign);
    biffAdd(SEEK, err); return 1;
  }
  if (!(AIR_EXISTS(strengthMin) && AIR_EXISTS(strength))) {
    sprintf(err, "%s: strength %g or %g doesn't exist", me,
            strengthMin, strength);
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->strengthSign != strengthSign) {
    sctx->strengthSign = strengthSign;
    sctx->flag[flagStrength] = AIR_TRUE;
  }
  if (sctx->strengthMin != strengthMin) {
    sctx->strengthMin = strengthMin;
    sctx->flag[flagStrength] = AIR_TRUE;
  }
  if (sctx->strength != strength) {
    sctx->strength = strength;
    sctx->flag[flagStrength] = AIR_TRUE;
  }
  return 0;
}

static int
itemCheck(seekContext *sctx, int item, unsigned int wantLen) {
  char me[]="itemCheck", err[BIFF_STRLEN];

  if (!sctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(SEEK, err); return 1;
  }
  if (!(sctx->gctx && sctx->pvl)) {
    sprintf(err, "%s: don't have a gage context", me);
    biffAdd(SEEK, err); return 1;
  }
  if (airEnumValCheck(sctx->pvl->kind->enm, item)) {
    sprintf(err, "%s: %d not valid %s item", me, item,
            sctx->pvl->kind->enm->name);
    biffAdd(SEEK, err); return 1;
  }
  if (!GAGE_QUERY_ITEM_TEST(sctx->pvl->query, item)) {
    sprintf(err, "%s: item \"%s\" (%d) not set in query", me,
            airEnumStr(sctx->pvl->kind->enm, item), item);
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->pvl->kind->table[item].answerLength != wantLen) {
    sprintf(err, "%s: item %s has length %u, not wanted %u", me,
            airEnumStr(sctx->pvl->kind->enm, item),
            sctx->pvl->kind->table[item].answerLength, wantLen);
    biffAdd(SEEK, err); return 1;
  }
  return 0;
}

/*
******** seekItemScalarSet
**
** sets: sclvItem
*/
int
seekItemScalarSet(seekContext *sctx, int item) {
  char me[]="seekItemScalarSet", err[BIFF_STRLEN];

  if (itemCheck(sctx, item, 1)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->sclvItem != item) {
    sctx->sclvItem = item;
    sctx->flag[flagItemValue] = AIR_TRUE;
  }
  return 0;
}

/*
******** seekItemStrengthSet
**
*/
int
seekItemStrengthSet(seekContext *sctx, int item) {
  char me[]="seekItemStrengthSet", err[BIFF_STRLEN];

  if (itemCheck(sctx, item, 1)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->stngItem != item) {
    sctx->stngItem = item;
    sctx->flag[flagItemStrength] = AIR_TRUE;
  }
  return 0;
}

/*
******** seekItemGradientSet
**
** sets: gradItem
*/
int
seekItemGradientSet(seekContext *sctx, int item) {
  char me[]="seekItemGradientSet", err[BIFF_STRLEN];

  if (itemCheck(sctx, item, 3)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->gradItem != item) {
    sctx->gradItem = item;
    sctx->flag[flagItemGradient] = AIR_TRUE;
  }
  /* sctx->gradAns = gageAnswerPointer(sctx->gctx, sctx->pvl, item); */
  return 0;
}

/*
******** seekItemNormalSet
**
** sets: normItem
*/
int
seekItemNormalSet(seekContext *sctx, int item) {
  char me[]="seekItemNormalSet", err[BIFF_STRLEN];

  if (itemCheck(sctx, item, 3)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->normItem != item) {
    sctx->normItem = item;
    sctx->flag[flagItemNormal] = AIR_TRUE;
  }
  /* sctx->normAns = gageAnswerPointer(sctx->gctx, sctx->pvl, item); */
  return 0;
}

/*
******** seekItemEigensystemSet
**
** sets: evalItem, evecItem
*/
int
seekItemEigensystemSet(seekContext *sctx, int evalItem, int evecItem) {
  char me[]="seekItemEigenvectorSet", err[BIFF_STRLEN];

  if (itemCheck(sctx, evalItem, 3)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(SEEK, err); return 1;
  }
  if (itemCheck(sctx, evecItem, 9)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->evalItem != evalItem
      || sctx->evecItem != evecItem) {
    sctx->evalItem = evalItem;
    sctx->evecItem = evecItem;
    sctx->flag[flagItemEigensystem] = AIR_TRUE;
  }
  /*
  sctx->evalAns = gageAnswerPointer(sctx->gctx, sctx->pvl, sctx->evalItem);
  sctx->evecAns = gageAnswerPointer(sctx->gctx, sctx->pvl, sctx->evecItem);
  */
  return 0;
}

/*
******** seekIsovalueSet
**
** sets: isovalue
*/
int
seekIsovalueSet(seekContext *sctx, double isovalue) {
  char me[]="seekIsovalueSet", err[BIFF_STRLEN];

  if (!sctx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(SEEK, err); return 1;
  }
  if (!AIR_EXISTS(isovalue)) {
    sprintf(err, "%s: given isovalue %g doesn't exit", me, isovalue);
    biffAdd(SEEK, err); return 1;
  }
  if (sctx->isovalue != isovalue) {
    sctx->isovalue = isovalue;
    sctx->flag[flagIsovalue] = AIR_TRUE;
  }
  return 0;
}
