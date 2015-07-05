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
******** gageKindCheck
**
** some some basic checking of the gageEntryItem array (the "table") for
** the sorts of mistakes that may be introduced by its hand-coding, although
** theoretically this is good for dynamically-generated gageKinds as well.
*/
int
gageKindCheck(const gageKind *kind) {
  char me[]="gageKindCheck", err[BIFF_STRLEN];
  int pitem, pindex, alen;
  int ii, pi;
  gageItemEntry *item;

  if (!kind) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (kind->itemMax > GAGE_ITEM_MAX) {
    sprintf(err, "%s: kind \"%s\" item max %d > GAGE_ITEM_MAX %d", 
            me, kind->name, kind->itemMax, GAGE_ITEM_MAX);
    biffAdd(GAGE, err); return 1;
  }
  for (ii=1; ii<=kind->itemMax; ii++) {
    item = kind->table + ii;
    if (ii != item->enumVal) {
      sprintf(err, "%s: \"%s\"-kind \"%s\" (item %d) has enumVal %d (not %d)",
              me, kind->name, airEnumStr(kind->enm, ii),
              ii, item->enumVal, ii);
      biffAdd(GAGE, err); return 1;
    }
    alen = item->answerLength;
    if (!(1 <= alen)) {
      if (kind->dynamicAlloc) {
        sprintf(err, "%s: (dynamic) \"%s\"-kind \"%s\" (item %d) "
                "answerLength (%d) not set?",
                me, kind->name, airEnumStr(kind->enm, ii), ii, alen);
      } else {
        sprintf(err, "%s: \"%s\"-kind \"%s\" (item %d) has invalid "
                "answerLength %d",
                me, kind->name, airEnumStr(kind->enm, ii), ii, alen);
      }
      biffAdd(GAGE, err); return 1;
    }
    if (!(AIR_IN_CL(0, item->needDeriv, 2))) {
      sprintf(err, "%s: \"%s\"-kind \"%s\" (item %d) has invalid needDeriv %d",
              me, kind->name, airEnumStr(kind->enm, ii), ii, item->needDeriv);
      biffAdd(GAGE, err); return 1;
    }
    for (pi=0; pi<GAGE_ITEM_PREREQ_MAXNUM; pi++) {
      if (!( 0 <= item->prereq[pi] )) {
        if (kind->dynamicAlloc) {
          sprintf(err, "%s: (dynamic) \"%s\"-kind \"%s\" (item %d) "
                  "prereq %d (%d) not set?",
                  me, kind->name, airEnumStr(kind->enm, ii), ii,
                  pi, item->prereq[pi]);
        } else {
          sprintf(err, "%s: \"%s\"-kind \"%s\" (item %d) has invalid "
                  "prereq %d (%d)",
                  me, kind->name, airEnumStr(kind->enm, ii), ii,
                  pi, item->prereq[pi]);
        }
        biffAdd(GAGE, err); return 1;
      }
    }
    pitem = item->parentItem;
    pindex = item->parentIndex;
    if (0 != pitem) {
      if (0 == ii) {
        sprintf(err, "%s: first item (index 0) of \"%s\"-kind can't "
                "be a sub-item (wanted parent index %d)", 
                me, kind->name, pitem);
        biffAdd(GAGE, err); return 1;
      }
      if (!(AIR_IN_CL(1, pitem, kind->itemMax))) {
        sprintf(err, "%s: item %d of \"%s\"-kind wants parent item %d "
                "outside valid range [0..%d]",
                me, ii, kind->name, pitem, kind->itemMax);
        biffAdd(GAGE, err); return 1;
      }
      if (0 != kind->table[pitem].parentItem) {
        sprintf(err, "%s: item %d of \"%s\"-kind has parent %d which "
                "wants to have parent %d: can't have sub-sub-items", 
                me, ii, kind->name, pitem, kind->table[pitem].parentItem);
        biffAdd(GAGE, err); return 1;
      }
      if (!( 0 <= pindex
             && ((unsigned int)pindex + alen 
                 <= kind->table[pitem].answerLength) )) {
        sprintf(err, "%s: item %d of \"%s\"-kind wants index range [%d,%d] "
                "of parent %d, which isn't in valid range [0,%d]",
                me, ii, kind->name,
                pindex, pindex + alen - 1,
                pitem, kind->table[pitem].answerLength - 1);
        biffAdd(GAGE, err); return 1;
      }
    }
  } 
  return 0;
}

int
gageKindTotalAnswerLength(const gageKind *kind) {
  char me[]="gageKindTotalAnswerLength", *err;
  int ii, alen;

  if (gageKindCheck(kind)) {
    err = biffGetDone(GAGE); 
    fprintf(stderr, "%s: PANIC:\n %s", me, err);
    free(err); exit(1);
  }
  alen = 0;
  for (ii=1; ii<=kind->itemMax; ii++) {
    alen += (0 == kind->table[ii].parentItem
             ? kind->table[ii].answerLength
             : 0);
  }
  return alen;
}

/*
** _gageKindAnswerOffset
**
** return the location of the item in the master answer array 
**
** I don't think this will work if there are sub-sub-items
*/
int
_gageKindAnswerOffset(const gageKind *kind, int item) {
  int parent, ii;

  if (1 >= item) {
    /* the first item always has zero offset */
    return 0;
  }

  /* else we're not the first */
  parent = kind->table[item].parentItem;
  if (0 != parent) {
    /* we're a sub-item */
    return (kind->table[item].parentIndex 
            + _gageKindAnswerOffset(kind, parent));
  }

  /* else we're not a sub-item: find the first previous non-sub-item */
  ii = item-1;
  while (0 != kind->table[ii].parentItem) {
    /* gageKindCheck ensures that item 1 is not a sub-item */
    ii--;
  }
  return (kind->table[ii].answerLength
          + _gageKindAnswerOffset(kind, ii));
}

unsigned int
gageKindAnswerLength(const gageKind *kind, int item) {
  char me[]="gageKindAnswerLength", *err;

  if (gageKindCheck(kind)) {
    err = biffGetDone(GAGE); 
    fprintf(stderr, "%s: PANIC:\n %s", me, err);
    free(err); exit(1);
  }

  return (!airEnumValCheck(kind->enm, item)
          ? kind->table[item].answerLength
          : 0);
}

int
gageKindAnswerOffset(const gageKind *kind, int item) {
  char me[]="gageKindAnswerOffset", *err;  
  
  if (gageKindCheck(kind)) {
    err = biffGetDone(GAGE); 
    fprintf(stderr, "%s: PANIC:\n %s", me, err);
    free(err); exit(1);
  }

  return _gageKindAnswerOffset(kind, item);
}

/*
** so that you can see if a given volume will work as the given kind
*/
int
gageKindVolumeCheck(const gageKind *kind, const Nrrd *nrrd) {
  char me[]="gageKindVolumeCheck", err[BIFF_STRLEN];

  if (!(kind && nrrd)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (nrrdCheck(nrrd)) {
    sprintf(err, "%s: problem with nrrd", me);
    biffMove(GAGE, err, NRRD); return 1;
  }
  if (!(nrrd->dim == 3 + kind->baseDim)) {
    sprintf(err, "%s: nrrd should be %u-D, not %u-D",
            me, 3 + kind->baseDim, nrrd->dim);
    biffAdd(GAGE, err); return 1;
  }
  if (nrrdTypeBlock == nrrd->type) {
    sprintf(err, "%s: can't handle %s-type volumes", me,
            airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(GAGE, err); return 1;
  }
  if (1 == kind->baseDim && (kind->valLen != nrrd->axis[0].size)) {
    sprintf(err, "%s: kind requires %u axis 0 values, not " 
            _AIR_SIZE_T_CNV, me, kind->valLen, nrrd->axis[0].size);
    biffAdd(GAGE, err); return 1;
  }
  /* this eventually calls _gageShapeSet(), which, for purely historical
     reasons, does the brunt of the error checking, some of which is almost
     certainly redundant with checks above ... */
  if (gageVolumeCheck(NULL, nrrd, kind)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(GAGE, err); return 1;
  }
  return 0;
}
