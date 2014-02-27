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

/*
******** gageVolumeCheck()
**
** checks whether a given volume is valid for the given kind
** and the given parameter settings in the context
**
** Note that ctx is simply passed to _gageShapeSet(), with no NULL-ity
** test- which is fine- it just means that the check is not specific
** to the parameters that can be set in the gageContext.
*/
int
gageVolumeCheck(const gageContext *ctx, const Nrrd *nin,
                const gageKind *kind) {
  char me[]="gageVolumeCheck", err[BIFF_STRLEN];
  gageShape shape;

  gageShapeReset(&shape);
  if (_gageShapeSet(ctx, &shape, nin, kind->baseDim)) {
    sprintf(err, "%s: trouble setting volume as %s kind", me, kind->name);
    biffAdd(GAGE, err); return 1;
  }
  return 0;
}

/*
******** gagePerVolumeNew()
**
** creates a new pervolume of a known kind, but nothing besides the
** answer array is allocated
**
** uses biff primarily because of the error checking in gageVolumeCheck()
*/
gagePerVolume *
gagePerVolumeNew(gageContext *ctx, const Nrrd *nin, const gageKind *kind) {
  char me[]="gagePerVolumeNew", err[BIFF_STRLEN];
  gagePerVolume *pvl;
  int ii;

  if (!( nin && kind )) {
    sprintf(err, "%s: got NULL pointer", me);
    return NULL;
  }
  if (gageVolumeCheck(ctx, nin, kind)) {
    sprintf(err, "%s: problem with given volume", me);
    biffAdd(GAGE, err); return NULL;
  }
  pvl = (gagePerVolume *)calloc(1, sizeof(gagePerVolume));
  if (!pvl) {
    sprintf(err, "%s: couldn't alloc gagePerVolume", me);
    biffAdd(GAGE, err); return NULL;
  }
  pvl->verbose = gageDefVerbose;
  pvl->kind = kind;
  GAGE_QUERY_RESET(pvl->query);
  pvl->needD[0] = pvl->needD[1] = pvl->needD[2] = AIR_FALSE;
  pvl->nin = nin;
  for (ii=gagePvlFlagUnknown+1; ii<gagePvlFlagLast; ii++) {
    pvl->flag[ii] = AIR_FALSE;
  }
  pvl->iv3 = pvl->iv2 = pvl->iv1 = NULL;
  pvl->lup = nrrdDLookup[nin->type];
  pvl->answer = (double *)calloc(gageKindTotalAnswerLength(kind),
                                 sizeof(double));
  pvl->directAnswer = (double **)calloc(kind->itemMax+1, sizeof(double*));
  if (!(pvl->answer && pvl->directAnswer)) {
    sprintf(err, "%s: couldn't alloc answer and directAnswer arrays", me);
    biffAdd(GAGE, err); return NULL;
  }
  for (ii=1; ii<=kind->itemMax; ii++) {
    pvl->directAnswer[ii] = pvl->answer + gageKindAnswerOffset(kind, ii);
  }
  pvl->flag[gagePvlFlagVolume] = AIR_TRUE;
  if (kind->pvlDataNew) {
    if (!(pvl->data = kind->pvlDataNew(kind))) {
      sprintf(err, "%s: double creating gagePerVolume data", me);
      biffAdd(GAGE, err); return NULL;
    }
  } else {
    pvl->data = NULL;
  }

  return pvl;
}

/*
** _gagePerVolumeCopy()
**
** copies a pervolume for use in a copied context, and probably
** should only be called by gageContextCopy()
*/
gagePerVolume *
_gagePerVolumeCopy(gagePerVolume *pvl, unsigned int fd) {
  char me[]="gagePerVolumeCopy", err[BIFF_STRLEN];
  gagePerVolume *nvl;
  int ii;
  
  nvl = (gagePerVolume *)calloc(1, sizeof(gagePerVolume));
  if (!nvl) {
    sprintf(err, "%s: couldn't create new pervolume", me);
    biffAdd(GAGE, err); return NULL;
  }
  /* we should probably restrict ourselves to gage API calls, but given the
     constant state of gage construction, this seems much simpler.
     Pointers to per-pervolume-allocated arrays are fixed below */
  memcpy(nvl, pvl, sizeof(gagePerVolume));
  nvl->iv3 = (double *)calloc(fd*fd*fd*nvl->kind->valLen, sizeof(double));
  nvl->iv2 = (double *)calloc(fd*fd*nvl->kind->valLen, sizeof(double));
  nvl->iv1 = (double *)calloc(fd*nvl->kind->valLen, sizeof(double));
  nvl->answer = (double *)calloc(gageKindTotalAnswerLength(nvl->kind),
                                 sizeof(double));
  nvl->directAnswer = (double **)calloc(nvl->kind->itemMax+1,
                                        sizeof(double*));
  if (!( nvl->iv3 && nvl->iv2 && nvl->iv1
         && nvl->answer && nvl->directAnswer )) {
    sprintf(err, "%s: couldn't allocate all caches "
            "(fd=%u, valLen=%u, totAnsLen=%u, itemMax=%u)", me,
            fd, nvl->kind->valLen, gageKindTotalAnswerLength(nvl->kind),
            nvl->kind->itemMax);
    biffAdd(GAGE, err); return NULL;
  }
  for (ii=1; ii<=pvl->kind->itemMax; ii++) {
    nvl->directAnswer[ii] = nvl->answer + gageKindAnswerOffset(pvl->kind, ii);
  }
  if (pvl->kind->pvlDataCopy) {
    if (!(nvl->data = pvl->kind->pvlDataCopy(pvl->kind, pvl->data))) {
      sprintf(err, "%s: double copying gagePerVolume data", me);
      biffAdd(GAGE, err); return NULL;
    }
  } else {
    nvl->data = NULL;
  }
  
  return nvl;
}

/*
******** gagePerVolumeNix()
**
** frees all dynamically allocated memory assocated with a pervolume
**
** does not use biff
*/
gagePerVolume *
gagePerVolumeNix(gagePerVolume *pvl) {

  if (pvl) {
    if (pvl->kind->pvlDataNix) {
      pvl->data = pvl->kind->pvlDataNix(pvl->kind, pvl->data);
    }
    pvl->iv3 = (double *)airFree(pvl->iv3);
    pvl->iv2 = (double *)airFree(pvl->iv2);
    pvl->iv1 = (double *)airFree(pvl->iv1);
    pvl->answer = (double *)airFree(pvl->answer);
    pvl->directAnswer = (double **)airFree(pvl->directAnswer);
    airFree(pvl);
  }
  return NULL;
}

/*
******** gageAnswerPointer()
**
** way of getting a pointer to a specific answer in a pervolume's ans array
**
*/
const double *
gageAnswerPointer(const gageContext *ctx, const gagePerVolume *pvl, int item) {
  const double *ret;

  AIR_UNUSED(ctx);
  if (pvl && !airEnumValCheck(pvl->kind->enm, item)) {
    ret = pvl->answer + gageKindAnswerOffset(pvl->kind, item);
  } else {
    ret = NULL;
  }
  return ret;
}

/* non-const version of the above */
double *
_gageAnswerPointer(const gageContext *ctx, gagePerVolume *pvl, int item) {
  double *ret;

  AIR_UNUSED(ctx);
  if (pvl && !airEnumValCheck(pvl->kind->enm, item)) {
    ret = pvl->answer + gageKindAnswerOffset(pvl->kind, item);
  } else {
    ret = NULL;
  }
  return ret;
}

unsigned int
gageAnswerLength(const gageContext *ctx, const gagePerVolume *pvl, int item) {
  unsigned int ret;
  
  AIR_UNUSED(ctx);
  if (pvl && !airEnumValCheck(pvl->kind->enm, item)) {
    ret = gageKindAnswerLength(pvl->kind, item);
  } else {
    ret = 0;
  }
  return ret;
}

int
gageQueryReset(gageContext *ctx, gagePerVolume *pvl) {
  char me[]="gageQueryReset", err[BIFF_STRLEN];

  AIR_UNUSED(ctx);
  if (!( pvl )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }

  GAGE_QUERY_RESET(pvl->query);

  return 0;
}


/*
******** gageQuerySet()
**
** sets a query in a pervolume.  Does recursive expansion of query
** to cover all prerequisite measures.  
**
** Sets: pvl->query
**
** the gageContext is not actually used here, but I'm cautiously
** including it in case its used in the future.
*/
int
gageQuerySet(gageContext *ctx, gagePerVolume *pvl, gageQuery query) {
  char me[]="gageQuerySet", err[BIFF_STRLEN];
  gageQuery lastQuery;
  int pi, ii;
  
  AIR_UNUSED(ctx);
  if (!( pvl )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  GAGE_QUERY_COPY(pvl->query, query);
  if (pvl->verbose) {
    fprintf(stderr, "%s: original ", me);
    gageQueryPrint(stderr, pvl->kind, pvl->query);
  }
  /* recursive expansion of prerequisites */
  do {
    GAGE_QUERY_COPY(lastQuery, pvl->query);
    ii = pvl->kind->itemMax+1;
    do {
      ii--;
      if (GAGE_QUERY_ITEM_TEST(pvl->query, ii)) {
        for (pi=0; pi<GAGE_ITEM_PREREQ_MAXNUM; pi++) {
          if (0 != pvl->kind->table[ii].prereq[pi]) {
            GAGE_QUERY_ITEM_ON(pvl->query, pvl->kind->table[ii].prereq[pi]);
          }
        }
      }
    } while (ii);
  } while (!GAGE_QUERY_EQUAL(pvl->query, lastQuery));
  if (pvl->verbose) {
    fprintf(stderr, "%s: expanded ", me);
    gageQueryPrint(stderr, pvl->kind, pvl->query);
  }

  /* doing this kind of error checking here is not really
     the way gage should work-- it should be done at the 
     time of gageUpdate()-- but the novelty of pvl->data
     encourages putting new smarts at superficial levels
     instead of deeper levels */
  if (!pvl->data) {
    for (ii=1; ii<=pvl->kind->itemMax; ii++) {
      if (GAGE_QUERY_ITEM_TEST(pvl->query, ii)
          && pvl->kind->table[ii].needData) {
        sprintf(err, "%s: item %d (%s) needs data, but pvl->data is NULL", 
                me, ii, airEnumStr(pvl->kind->enm, ii));
        biffAdd(GAGE, err); return 1;
      }
    }
  }

  pvl->flag[gagePvlFlagQuery] = AIR_TRUE;

  return 0;
}

int
gageQueryAdd(gageContext *ctx, gagePerVolume *pvl, gageQuery query) {
  char me[]="gageQueryAdd", err[BIFF_STRLEN];

  if (!( pvl )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }

  GAGE_QUERY_ADD(pvl->query, query);
  if (gageQuerySet(ctx, pvl, pvl->query)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(GAGE, err); return 1;
  }

  return 0;
}

int
gageQueryItemOn(gageContext *ctx, gagePerVolume *pvl, int item) {
  char me[]="gageQueryItemOn", err[BIFF_STRLEN];

  if (!( pvl )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }

  if (airEnumValCheck(pvl->kind->enm, item)) {
    sprintf(err, "%s: %d not a valid %s value", me,
            item, pvl->kind->enm->name);
    biffAdd(GAGE, err); return 1;
  }
  GAGE_QUERY_ITEM_ON(pvl->query, item);
  if (gageQuerySet(ctx, pvl, pvl->query)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(GAGE, err); return 1;
  }

  return 0;
}

