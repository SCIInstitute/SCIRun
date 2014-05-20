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

typedef struct {
  int evti[12];  /* edge vertex index */
  double (*scllup)(const void *, size_t);
  unsigned int esIdx,  /* eigensystem index */
    zi;     /* slice index we're currently on */
  int modeSign;
  const void *scldata;
  airArray *xyzwArr, *normArr, *indxArr;
} baggage;

static baggage *
baggageNew(seekContext *sctx) {
  baggage *bag;
  unsigned int sx;

  bag = (baggage *)calloc(1, sizeof(baggage));

  /* this is basically the mapping from the 12 edges on each voxel to
     the 5 unique edges for each sample on the slab, based on the lay-out
     defined in the beginning of tables.c */
  sx = AIR_CAST(unsigned int, sctx->sx);
  /*                     X      Y */
  bag->evti[ 0] = 0 + 5*(0 + sx*0);
  bag->evti[ 1] = 1 + 5*(0 + sx*0);
  bag->evti[ 2] = 1 + 5*(1 + sx*0);
  bag->evti[ 3] = 0 + 5*(0 + sx*1);
  bag->evti[ 4] = 2 + 5*(0 + sx*0);
  bag->evti[ 5] = 2 + 5*(1 + sx*0);
  bag->evti[ 6] = 2 + 5*(0 + sx*1);
  bag->evti[ 7] = 2 + 5*(1 + sx*1);
  bag->evti[ 8] = 3 + 5*(0 + sx*0);
  bag->evti[ 9] = 4 + 5*(0 + sx*0);
  bag->evti[10] = 4 + 5*(1 + sx*0);
  bag->evti[11] = 3 + 5*(0 + sx*1);

  switch (sctx->type) {
  case seekTypeRidgeSurface:
    bag->esIdx = 2;
    bag->modeSign = -1;
    break;
  case seekTypeValleySurface:
    bag->esIdx = 0;
    bag->modeSign = +1;
    break;
  case seekTypeMaximalSurface:
    bag->esIdx = 0;
    bag->modeSign = -1;
    break;
  case seekTypeMinimalSurface:
    bag->esIdx = 2;
    bag->modeSign = +1;
    break;
  default:
    /* sprintf(err, "%s: feature type %s not handled", me,
            airEnumStr(seekType, sctx->type));
    biffAdd(SEEK, err); return 1;
    */
    /* without biff, we get as nasty as possible */
    bag->esIdx = UINT_MAX;
    bag->modeSign = 0;
    break;
  }

  if (seekTypeIsocontour == sctx->type) {
    if (sctx->ninscl) {
      bag->scllup = nrrdDLookup[sctx->ninscl->type];
      bag->scldata = sctx->ninscl->data;
    } else {
      bag->scllup = nrrdDLookup[sctx->nsclDerived->type];
      bag->scldata = sctx->nsclDerived->data;
    }
  } else {
    bag->scllup = NULL;
    bag->scldata = NULL;
  }

  bag->xyzwArr = NULL;
  bag->normArr = NULL;
  bag->indxArr = NULL;
  return bag;
}

static baggage *
baggageNix(baggage *bag) {

  if (bag) {
    airArrayNix(bag->normArr);
    airArrayNix(bag->xyzwArr);
    airArrayNix(bag->indxArr);

    airFree(bag);
  }
  return NULL;
}

static int
outputInit(seekContext *sctx, baggage *bag, limnPolyData *lpld) {
  char me[]="outputInit", err[BIFF_STRLEN];
  unsigned int estVertNum, estFaceNum, minI, maxI, valI, *spanHist;
  int E;

  if (seekTypeIsocontour == sctx->type
      && AIR_IN_OP(sctx->range->min, sctx->isovalue, sctx->range->max)) {
    unsigned int estVoxNum=0;
    /* estimate number of voxels, faces, and vertices involved */
    spanHist = AIR_CAST(unsigned int*, sctx->nspanHist->data);
    valI = airIndex(sctx->range->min, sctx->isovalue, sctx->range->max,
                    sctx->spanSize);
    for (minI=0; minI<=valI; minI++) {
      for (maxI=valI; maxI<sctx->spanSize; maxI++) {
        estVoxNum += spanHist[minI + sctx->spanSize*maxI];
      }
    }
    estVertNum = AIR_CAST(unsigned int, estVoxNum*(sctx->vertsPerVoxel));
    estFaceNum = AIR_CAST(unsigned int, estVoxNum*(sctx->facesPerVoxel));
    if (sctx->verbose) {
      fprintf(stderr, "%s: estimated vox --> vert, face: %u --> %u, %u\n", me,
              estVoxNum, estVertNum, estFaceNum);
    }
  } else {
    estVertNum = 0;
    estFaceNum = 0;
  }
  /* need something non-zero so that pre-allocations below aren't no-ops */
  estVertNum = AIR_MAX(1, estVertNum);
  estFaceNum = AIR_MAX(1, estFaceNum);
  
  /* initialize limnPolyData with estimated # faces and vertices */
  /* we will manage the innards of the limnPolyData entirely ourselves */
  if (limnPolyDataAlloc(lpld, 0, 0, 0, 0)) {
    sprintf(err, "%s: trouble emptying given polydata", me);
    biffAdd(SEEK, err); return 1;
  }
  bag->xyzwArr = airArrayNew((void**)&(lpld->xyzw), &(lpld->xyzwNum),
                             4*sizeof(float), sctx->pldArrIncr);
  if (sctx->normalsFind) {
    bag->normArr = airArrayNew((void**)&(lpld->norm), &(lpld->normNum),
                               3*sizeof(float), sctx->pldArrIncr);
  } else {
    bag->normArr = NULL;
  }
  bag->indxArr = airArrayNew((void**)&(lpld->indx), &(lpld->indxNum),
                             sizeof(unsigned int), sctx->pldArrIncr);
  lpld->primNum = 1;  /* for now, its just triangle soup */
  lpld->type = (unsigned char *)calloc(lpld->primNum, sizeof(unsigned char));
  lpld->icnt = (unsigned int *)calloc(lpld->primNum, sizeof(unsigned int));
  lpld->type[0] = limnPrimitiveTriangles;
  lpld->icnt[0] = 0;  /* incremented below */
  
  E = 0;
  airArrayLenPreSet(bag->xyzwArr, estVertNum);
  E |= !(bag->xyzwArr->data);
  if (sctx->normalsFind) {
    airArrayLenPreSet(bag->normArr, estVertNum);
    E |= !(bag->normArr->data);
  }
  airArrayLenPreSet(bag->indxArr, 3*estFaceNum);
  E |= !(bag->indxArr->data);
  if (E) {
    sprintf(err, "%s: couldn't pre-allocate contour geometry (%p %p %p)", me,
            bag->xyzwArr->data,
            (sctx->normalsFind ? bag->normArr->data : NULL),
            bag->indxArr->data);
    biffAdd(SEEK, err); return 1;
  }

  /* initialize output summary info */
  sctx->voxNum = 0;
  sctx->vertNum = 0;
  sctx->faceNum = 0;

  return 0;
}

static double
sclGet(seekContext *sctx, baggage *bag,
       unsigned int xi, unsigned int yi, unsigned int zi) {

  zi = AIR_MIN(sctx->sz-1, zi);
  return bag->scllup(bag->scldata, xi + sctx->sx*(yi + sctx->sy*zi));
}

static void
idxProbe(seekContext *sctx, baggage *bag, double xi, double yi, double zi) {
  double idxOut[4], idxIn[4];
  AIR_UNUSED(bag);

  ELL_4V_SET(idxOut, xi, yi, zi, 1);
  ELL_4MV_MUL(idxIn, sctx->txfIdx, idxOut);
  ELL_4V_HOMOG(idxIn, idxIn);
  gageProbe(sctx->gctx, idxIn[0], idxIn[1], idxIn[2]);
  return;
}

/*
** this is one of the few things that has to operate on more than one
** zi plane at once, and it is honestly probably the primary motivation
** for putting zi into the baggage.
**
** NOTE: this is doing some bounds (on the positive x, y, z edges of the 
** volume) that probably should be done closer to the caller
*/
static int
evecFlipProbe(seekContext *sctx, baggage *bag,
              signed char *flip,  /* OUTPUT HERE */
              unsigned int xi, unsigned int yi, unsigned int ziOff,
              unsigned int dx, unsigned int dy, unsigned int dz) {
  char me[]="evecFlipProbe", err[BIFF_STRLEN];
  unsigned int sx, sy, sz, si;
  double stngA, stngB, u, du, dot, strength, wantDot, minDu, mode;
  double current[3], next[3], posNext[3], posA[3], posB[3], evecA[3], evecB[3];

  sx = AIR_CAST(unsigned int, sctx->sx);
  sy = AIR_CAST(unsigned int, sctx->sy);
  sz = AIR_CAST(unsigned int, sctx->sz);
  si = xi + sx*yi;

  if (!(xi + dx < sx
        && yi + dy < sy
        && bag->zi + ziOff + dz < sz)) {
    /* the edge we're being asked about is outside the volume */
    *flip = 0;
    return 0;
  }

  if (sctx->strengthUse) {
    stngA = sctx->stng[ziOff    + 2*(xi    + sx*yi)];
    stngB = sctx->stng[ziOff+dz + 2*(xi+dx + sx*(yi+dy))];
    if (!( stngA > sctx->strengthMin && stngB > sctx->strengthMin )) {
      *flip = 0;
      return 0;
    }
  } else {
    stngA = 0;
    stngB = 0;
  }

  /* this edge is in bounds, and the endpoints meet strengthMin (if we care) */

  ELL_3V_SET(posA, xi, yi, bag->zi+ziOff);
  ELL_3V_SET(posB, xi+dx, yi+dy, bag->zi+ziOff+dz);
  ELL_3V_COPY(evecA, sctx->evec 
              + 3*(bag->esIdx + 3*(ziOff    + 2*(xi    + sx*yi))));
  ELL_3V_COPY(evecB, sctx->evec
              + 3*(bag->esIdx + 3*(ziOff+dz + 2*(xi+dx + sx*(yi+dy)))));

#define SETNEXT(uu) \
  ELL_3V_SCALE_ADD2(posNext, 1.0-(uu), posA, (uu), posB); \
  idxProbe(sctx, bag, posNext[0], posNext[1], posNext[2]); \
  ELL_3V_COPY(next, sctx->evecAns + 3*bag->esIdx); \
  if (ELL_3V_DOT(current, next) < 0) { \
    ELL_3V_SCALE(next, -1, next); \
  } \
  dot = ELL_3V_DOT(current, next); \
  mode = bag->modeSign*airMode3_d(sctx->evalAns); \
  if (sctx->strengthUse) { \
    strength = sctx->strengthSign*sctx->stngAns[0]; \
  } else { \
    strength = 0; \
  }

  ELL_3V_COPY(current, evecA);
  u = 0;
  du = 0.49999;
  wantDot = 0.9; /* between cos(pi/6) and cos(pi/8) */
  minDu = 0.0002;
  while (u + du < 1.0) {
    SETNEXT(u+du);
    if (sctx->strengthUse && strength < sctx->strengthMin) {
      /* sorry, strength got too weak along this edge */
      *flip = 0;
      return 0;
    }
    if (mode < -0.8) {
      /* sorry, eigenvalue mode got too close to 2nd order isotropy */
      *flip = 0;
      return 0;
    }
    while (dot < wantDot) {
      /* angle between current and next is too big; reduce step */
      du /= 2;
      if (du < minDu) {
        sprintf(err, "%s: evector wild @ u=%g: du=%g < minDu=%g; "
                "dot=%g; stngA,B = %g,%g; strength = %g; mode = %g; "
                "(xi,yi,zi)=(%u,%u,%u+%u); (dx,dy,dz)=(%u,%u,%u) ",
                me, u, du, minDu,
                dot, stngA, stngB, strength, mode,
                xi, yi, bag->zi, ziOff, dx, dy, dz);
        /* biffAdd(SEEK, err); return 1; */
        fprintf(stderr, "%s\n", err);
        *flip = 0;
        return 0;
      }
      SETNEXT(u+du);
    }
    /* current and next have a small angle between them */
    ELL_3V_COPY(current, next);
    u += du;
  }
  /* last fake iteration, to check endpoint explicitly */
  u = 1.0;
  SETNEXT(u);
  if (dot < wantDot) {
    sprintf(err, "%s: confused at end of edge", me);
    biffAdd(SEEK, err); return 1;
  }
  ELL_3V_COPY(current, next);

#undef SETNEXT
  
  /* we have now tracked the eigenvector along the edge */
  dot = ELL_3V_DOT(current, evecB);
  *flip = (dot > 0
           ? +1
           : -1);
  return 0;
}

/*
** !!! this has to be done as a separate second pass because of how
** !!! the flip quantities correspond to the edges between voxels
**
** I backed out of an effort to track two eigenvectors at once...
*/
static int
evecFlipShuffleProbe(seekContext *sctx, baggage *bag) {
  char me[]="evecFlipShuffleProbe", err[BIFF_STRLEN];
  unsigned int xi, yi, sx, sy, si;
  signed char flipA, flipB, flipC;

  sx = AIR_CAST(unsigned int, sctx->sx);
  sy = AIR_CAST(unsigned int, sctx->sy);

  /* NOTE: these have to go all the way to sy-1 and sx-1, instead of
     sy-2 and sx-2 (like shuffleProbe() below) because of the need to
     set the flips on the edges at the positive X and Y volume
     boundary.  The necessary bounds checking happens in
     evecFlipProbe(): messy, I know */
  for (yi=0; yi<sy; yi++) {
    for (xi=0; xi<sx; xi++) {
      si = xi + sx*yi;
      /* ================================================= */
      if (!bag->zi) {
        /* ----------------- set/probe top of initial slab */
        if (evecFlipProbe(sctx, bag,    &flipA, xi, yi, 0, 1, 0, 0)
            || evecFlipProbe(sctx, bag, &flipB, xi, yi, 0, 0, 1, 0)) {
          sprintf(err, "%s: problem at (xi,yi) = (%u,%u), zi=0", me, xi, yi);
          biffAdd(SEEK, err); return 1;
        }
        sctx->flip[0 + 5*si] = flipA;
        sctx->flip[1 + 5*si] = flipB;
      } else {
        /* ------------------- shuffle to top from bottom of slab */
        sctx->flip[0 + 5*si] = sctx->flip[3 + 5*si];
        sctx->flip[1 + 5*si] = sctx->flip[4 + 5*si];
      }
      /* ----------------------- set/probe bottom of slab */
      if (evecFlipProbe(sctx, bag,    &flipA, xi, yi, 0, 0, 0, 1)
          || evecFlipProbe(sctx, bag, &flipB, xi, yi, 1, 1, 0, 0)
          || evecFlipProbe(sctx, bag, &flipC, xi, yi, 1, 0, 1, 0)) {
        sprintf(err, "%s: problem at (xi,yi,zi) = (%u,%u,%u)", me,
                xi, yi, bag->zi);
        biffAdd(SEEK, err); return 1;
      }
      sctx->flip[2 + 5*si] = flipA;
      sctx->flip[3 + 5*si] = flipB;
      sctx->flip[4 + 5*si] = flipC;
      /* ================================================= */
    }
  }

  return 0;
}

static int
shuffleProbe(seekContext *sctx, baggage *bag) {
  char me[]="shuffleProbe", err[BIFF_STRLEN];
  unsigned int xi, yi, sx, sy, si, spi;

  sx = AIR_CAST(unsigned int, sctx->sx);
  sy = AIR_CAST(unsigned int, sctx->sy);

  for (yi=0; yi<sy; yi++) {
    for (xi=0; xi<sx; xi++) {
      si = xi + sx*yi;
      spi = (xi+1) + (sx+2)*(yi+1);
      /* ================================================= */
      if (!bag->zi) {
        /* ----------------- set/probe top of initial slab */
        sctx->vidx[0 + 5*si] = -1;
        sctx->vidx[1 + 5*si] = -1;
        if (sctx->gctx) { /* HEY: need this check, what's the right way? */
          idxProbe(sctx, bag, xi, yi, 0);
        }
        if (sctx->strengthUse) {
          sctx->stng[0 + 2*si] = sctx->strengthSign*sctx->stngAns[0];
          if (!si) {
            sctx->strengthSeenMax = sctx->stng[0 + 2*si];
          }
          sctx->strengthSeenMax = AIR_MAX(sctx->strengthSeenMax,
                                          sctx->stng[0 + 2*si]);
        }
        switch (sctx->type) {
        case seekTypeIsocontour:
          sctx->sclv[0 + 4*spi] = (sclGet(sctx, bag, xi, yi, 0)
                                   - sctx->isovalue);
          sctx->sclv[1 + 4*spi] = (sclGet(sctx, bag, xi, yi, 0)
                                   - sctx->isovalue);
          sctx->sclv[2 + 4*spi] = (sclGet(sctx, bag, xi, yi, 1)
                                   - sctx->isovalue);
          break;
        case seekTypeRidgeSurface:
        case seekTypeValleySurface:
        case seekTypeMaximalSurface:
        case seekTypeMinimalSurface:
          ELL_3V_COPY(sctx->grad + 3*(0 + 2*si), sctx->gradAns);
          ELL_3V_COPY(sctx->eval + 3*(0 + 2*si), sctx->evalAns);
          ELL_3M_COPY(sctx->evec + 9*(0 + 2*si), sctx->evecAns);
          break;
        }
      } else {
        /* ------------------- shuffle to top from bottom of slab */
        sctx->vidx[0 + 5*si] = sctx->vidx[3 + 5*si];
        sctx->vidx[1 + 5*si] = sctx->vidx[4 + 5*si];
        if (sctx->strengthUse) {
          sctx->stng[0 + 2*si] = sctx->stng[1 + 2*si];
        }
        switch (sctx->type) {
        case seekTypeIsocontour:
          sctx->sclv[0 + 4*spi] = sctx->sclv[1 + 4*spi];
          sctx->sclv[1 + 4*spi] = sctx->sclv[2 + 4*spi];
          sctx->sclv[2 + 4*spi] = sctx->sclv[3 + 4*spi];
          break;
        case seekTypeRidgeSurface:
        case seekTypeValleySurface:
        case seekTypeMaximalSurface:
        case seekTypeMinimalSurface:
          ELL_3V_COPY(sctx->grad + 3*(0 + 2*si), sctx->grad + 3*(1 + 2*si));
          ELL_3V_COPY(sctx->eval + 3*(0 + 2*si), sctx->eval + 3*(1 + 2*si));
          ELL_3M_COPY(sctx->evec + 9*(0 + 2*si), sctx->evec + 9*(1 + 2*si));
          break;
        }
      }
      /* ----------------------- set/probe bottom of slab */
      sctx->vidx[2 + 5*si] = -1;
      sctx->vidx[3 + 5*si] = -1;
      sctx->vidx[4 + 5*si] = -1;
      if (sctx->gctx) { /* HEY: need this check, what's the right way? */
        idxProbe(sctx, bag, xi, yi, bag->zi+1);
      }
      if (sctx->strengthUse) {
        sctx->stng[1 + 2*si] = sctx->strengthSign*sctx->stngAns[0];
        sctx->strengthSeenMax = AIR_MAX(sctx->strengthSeenMax,
                                        sctx->stng[1 + 2*si]);
      }
      switch (sctx->type) {
      case seekTypeIsocontour:
        sctx->sclv[3 + 4*spi] = (sclGet(sctx, bag, xi, yi, bag->zi+2)
                                 - sctx->isovalue);
        break;
      case seekTypeRidgeSurface:
      case seekTypeValleySurface:
      case seekTypeMaximalSurface:
      case seekTypeMinimalSurface:
        ELL_3V_COPY(sctx->grad + 3*(1 + 2*si), sctx->gradAns);
        ELL_3V_COPY(sctx->eval + 3*(1 + 2*si), sctx->evalAns);
        ELL_3M_COPY(sctx->evec + 9*(1 + 2*si), sctx->evecAns);
        break;
      }
      /* ================================================= */
    }
    /* copy ends of this scanline left/right to margin */
    if (seekTypeIsocontour == sctx->type) {
      ELL_4V_COPY(sctx->sclv + 4*(0    + (sx+2)*(yi+1)),
                  sctx->sclv + 4*(1    + (sx+2)*(yi+1)));
      ELL_4V_COPY(sctx->sclv + 4*(sx+1 + (sx+2)*(yi+1)),
                  sctx->sclv + 4*(sx   + (sx+2)*(yi+1)));
    }
  }
  /* copy top and bottom scanline up/down to margin */
  if (seekTypeIsocontour == sctx->type) {
    for (xi=0; xi<sx+2; xi++) {
      ELL_4V_COPY(sctx->sclv + 4*(xi + (sx+2)*0),
                  sctx->sclv + 4*(xi + (sx+2)*1));
      ELL_4V_COPY(sctx->sclv + 4*(xi + (sx+2)*(sy+1)),
                  sctx->sclv + 4*(xi + (sx+2)*sy));
    }
  }

  /* this is done as a separate pass because it looks at values between
     voxels (so its indexing is not trivial to fold into loops above) */
  if (seekTypeRidgeSurface == sctx->type
      || seekTypeValleySurface == sctx->type
      || seekTypeMaximalSurface == sctx->type
      || seekTypeMinimalSurface == sctx->type) {
    if (evecFlipShuffleProbe(sctx, bag)) {
      sprintf(err, "%s: trouble at zi=%u\n", me, bag->zi);
      biffAdd(SEEK, err); return 1;
    }
  }

  return 0;
}

#define VAL(xx, yy, zz)  (val[4*( (xx) + (yy)*(sx+2) + spi) + (zz+1)])
void
voxelGrads(double vgrad[8][3], double *val, int sx, int spi) {
  ELL_3V_SET(vgrad[0],
             VAL( 1,  0,  0) - VAL(-1,  0,  0),
             VAL( 0,  1,  0) - VAL( 0, -1,  0),
             VAL( 0,  0,  1) - VAL( 0,  0, -1));
  ELL_3V_SET(vgrad[1],
             VAL( 2,  0,  0) - VAL( 0,  0,  0),
             VAL( 1,  1,  0) - VAL( 1, -1,  0),
             VAL( 1,  0,  1) - VAL( 1,  0, -1));
  ELL_3V_SET(vgrad[2],
             VAL( 1,  1,  0) - VAL(-1,  1,  0),
             VAL( 0,  2,  0) - VAL( 0,  0,  0),
             VAL( 0,  1,  1) - VAL( 0,  1, -1));
  ELL_3V_SET(vgrad[3],
             VAL( 2,  1,  0) - VAL( 0,  1,  0),
             VAL( 1,  2,  0) - VAL( 1,  0,  0),
             VAL( 1,  1,  1) - VAL( 1,  1, -1));
  ELL_3V_SET(vgrad[4],
             VAL( 1,  0,  1) - VAL(-1,  0,  1),
             VAL( 0,  1,  1) - VAL( 0, -1,  1),
             VAL( 0,  0,  2) - VAL( 0,  0,  0));
  ELL_3V_SET(vgrad[5],
             VAL( 2,  0,  1) - VAL( 0,  0,  1),
             VAL( 1,  1,  1) - VAL( 1, -1,  1),
             VAL( 1,  0,  2) - VAL( 1,  0,  0));
  ELL_3V_SET(vgrad[6],
             VAL( 1,  1,  1) - VAL(-1,  1,  1),
             VAL( 0,  2,  1) - VAL( 0,  0,  1),
             VAL( 0,  1,  2) - VAL( 0,  1,  0));
  ELL_3V_SET(vgrad[7],
             VAL( 2,  1,  1) - VAL( 0,  1,  1),
             VAL( 1,  2,  1) - VAL( 1,  0,  1),
             VAL( 1,  1,  2) - VAL( 1,  1,  0));
}
#undef VAL

static void
vvalIsoSet(seekContext *sctx, baggage *bag, double vval[8],
           unsigned int xi, unsigned int yi) {
  unsigned int sx, sy, si, spi, vi;

  AIR_UNUSED(bag);
  sx = AIR_CAST(unsigned int, sctx->sx);
  sy = AIR_CAST(unsigned int, sctx->sy);
  si = xi + sx*yi;
  spi = (xi+1) + (sx+2)*(yi+1);

  /* learn voxel values */
  /*                      X   Y                 Z */
  vval[0] = sctx->sclv[4*(0 + 0*(sx+2) + spi) + 1];
  vval[1] = sctx->sclv[4*(1 + 0*(sx+2) + spi) + 1];
  vval[2] = sctx->sclv[4*(0 + 1*(sx+2) + spi) + 1];
  vval[3] = sctx->sclv[4*(1 + 1*(sx+2) + spi) + 1];
  vval[4] = sctx->sclv[4*(0 + 0*(sx+2) + spi) + 2];
  vval[5] = sctx->sclv[4*(1 + 0*(sx+2) + spi) + 2];
  vval[6] = sctx->sclv[4*(0 + 1*(sx+2) + spi) + 2];
  vval[7] = sctx->sclv[4*(1 + 1*(sx+2) + spi) + 2];
  if (sctx->strengthUse) {
    double s, w, ssum, wsum;
    /*                 Z      X   Y   */
    ssum = wsum = 0;
#define ACCUM(vi) w = AIR_ABS(1.0/vval[vi]); ssum += w*s; wsum += w
    s = sctx->stng[0 + 2*(0 + 0*sx + si)]; ACCUM(0);
    s = sctx->stng[0 + 2*(1 + 0*sx + si)]; ACCUM(1);
    s = sctx->stng[0 + 2*(0 + 1*sx + si)]; ACCUM(2);
    s = sctx->stng[0 + 2*(1 + 1*sx + si)]; ACCUM(3);
    s = sctx->stng[1 + 2*(0 + 0*sx + si)]; ACCUM(4);
    s = sctx->stng[1 + 2*(1 + 0*sx + si)]; ACCUM(5);
    s = sctx->stng[1 + 2*(0 + 1*sx + si)]; ACCUM(6);
    s = sctx->stng[1 + 2*(1 + 1*sx + si)]; ACCUM(7);
#undef ACCUM
    if (ssum/wsum < sctx->strengthMin) {
      for (vi=0; vi<8; vi++) {
        vval[vi] = 0;
      }
    }
  }
  return;
}


static void
vvalSurfSet(seekContext *sctx, baggage *bag, double vval[8],
            unsigned int xi, unsigned int yi) {
  /* char me[]="vvalSurfSet"; */
  double eval[8][3], evec[8][3], grad[8][3], stng[8], minStrength=0;
  signed char flip[12], flipProd;
  unsigned int sx, sy, si, vi, ei, vrti[8];

  sx = AIR_CAST(unsigned int, sctx->sx);
  sy = AIR_CAST(unsigned int, sctx->sy);
  si = xi + sx*yi;
  vrti[0] = 0 + 2*(xi + 0 + sx*(yi + 0));
  vrti[1] = 0 + 2*(xi + 1 + sx*(yi + 0));
  vrti[2] = 0 + 2*(xi + 0 + sx*(yi + 1));
  vrti[3] = 0 + 2*(xi + 1 + sx*(yi + 1));
  vrti[4] = 1 + 2*(xi + 0 + sx*(yi + 0));
  vrti[5] = 1 + 2*(xi + 1 + sx*(yi + 0));
  vrti[6] = 1 + 2*(xi + 0 + sx*(yi + 1));
  vrti[7] = 1 + 2*(xi + 1 + sx*(yi + 1));

  for (vi=0; vi<8; vi++) {
    ELL_3V_COPY(grad[vi], sctx->grad + 3*vrti[vi]);
    ELL_3V_COPY(evec[vi], sctx->evec + 3*(bag->esIdx + 3*vrti[vi]));
    ELL_3V_COPY(eval[vi], sctx->eval + 3*vrti[vi]);
    if (sctx->strengthUse) {
      stng[vi] = sctx->stng[vrti[vi]];
      if (!vi) {
        minStrength = stng[vi];
      } else {
        minStrength = AIR_MIN(minStrength, stng[vi]);
      }
    }
  }
  flipProd = 1;
  for (ei=0; ei<12; ei++) {
    flip[ei] = sctx->flip[bag->evti[ei] + 5*si];
    flipProd *= flip[ei];
  }

  if ((sctx->strengthUse && minStrength < sctx->strengthMin)
      || !flipProd) {
    /* either the corners this voxel don't meet strength,
       or something else is funky */
    for (vi=0; vi<8; vi++) {
      vval[vi] = 0;
    }
  } else {
    ELL_3V_SCALE(evec[1], flip[0],                  evec[1]);
    ELL_3V_SCALE(evec[2], flip[1],                  evec[2]);
    ELL_3V_SCALE(evec[3], flip[0]*flip[2],          evec[3]);
    ELL_3V_SCALE(evec[4], flip[4],                  evec[4]);
    ELL_3V_SCALE(evec[5], flip[4]*flip[8],          evec[5]);
    ELL_3V_SCALE(evec[6], flip[4]*flip[9],          evec[6]);
    ELL_3V_SCALE(evec[7], flip[4]*flip[8]*flip[10], evec[7]);
    for (vi=0; vi<8; vi++) {
      vval[vi] = ELL_3V_DOT(grad[vi], evec[vi]);
    }
  }
  return;
}

static int
triangulate(seekContext *sctx, baggage *bag, limnPolyData *lpld) {
  /* char me[]="triangulate"; */
  unsigned xi, yi, sx, sy, si, spi;
  /* ========================================================== */
  /* NOTE: these things must agree with information in tables.c */
  int e2v[12][2] = {        /* maps edge index to corner vertex indices */
    {0, 1},  /*  0 */
    {0, 2},  /*  1 */
    {1, 3},  /*  2 */
    {2, 3},  /*  3 */
    {0, 4},  /*  4 */
    {1, 5},  /*  5 */
    {2, 6},  /*  6 */
    {3, 7},  /*  7 */
    {4, 5},  /*  8 */
    {4, 6},  /*  9 */
    {5, 7},  /* 10 */
    {6, 7}   /* 11 */
  };
  double vccoord[8][3] = {  /* vertex corner coordinates */
    {0, 0, 0},  /* 0 */
    {1, 0, 0},  /* 1 */
    {0, 1, 0},  /* 2 */
    {1, 1, 0},  /* 3 */
    {0, 0, 1},  /* 4 */
    {1, 0, 1},  /* 5 */
    {0, 1, 1},  /* 6 */
    {1, 1, 1}   /* 7 */
  };
  /* ========================================================== */

  sx = AIR_CAST(unsigned int, sctx->sx);
  sy = AIR_CAST(unsigned int, sctx->sy);

  for (yi=0; yi<sy-1; yi++) {
    double vval[8], vgrad[8][3], vert[3], tvertA[4], tvertB[4], ww;
    unsigned char vcase;
    int ti, vi, ei, vi0, vi1, ecase;
    const int *tcase;
    unsigned int vii[3];
    for (xi=0; xi<sx-1; xi++) {
      si = xi + sx*yi;
      spi = (xi+1) + (sx+2)*(yi+1);
      switch (sctx->type) {
      case seekTypeIsocontour:
        vvalIsoSet(sctx, bag, vval, xi, yi);
        break;
      case seekTypeRidgeSurface:
      case seekTypeValleySurface:
      case seekTypeMaximalSurface:
      case seekTypeMinimalSurface:
        vvalSurfSet(sctx, bag, vval, xi, yi);
        break;
      }
      /* determine voxel and edge case */
      vcase = 0;
      for (vi=0; vi<8; vi++) {
        vcase |= (vval[vi] > 0) << vi;
      }
      if (0 == vcase || 255 == vcase) {
        /* no triangles added here */
        continue;
      }
      /* set voxel corner gradients */
      if (seekTypeIsocontour == sctx->type 
          && sctx->normalsFind 
          && !sctx->normAns) {
        voxelGrads(vgrad, sctx->sclv, sx, spi);
      }
      sctx->voxNum++;
      ecase = seekContour3DTopoHackEdge[vcase];
      /* create new vertices as needed */
      for (ei=0; ei<12; ei++) {
        if ((ecase & (1 << ei))
            && -1 == sctx->vidx[bag->evti[ei] + 5*si]) {
          int ovi;
          double tvec[3], grad[3], tlen;
          /* this edge is needed for triangulation,
             and, we haven't already created a vertex for it */
          vi0 = e2v[ei][0];
          vi1 = e2v[ei][1];
          ww = vval[vi0]/(vval[vi0] - vval[vi1]);
          ELL_3V_LERP(vert, ww, vccoord[vi0], vccoord[vi1]);
          ELL_4V_SET(tvertA, vert[0] + xi, vert[1] + yi, vert[2] + bag->zi, 1);
          ELL_4MV_MUL(tvertB, sctx->txfIdx, tvertA);
          /* tvertB is now in input index space */
          ELL_4MV_MUL(tvertA, sctx->shape->ItoW, tvertB);
          /* tvertA is now in input world space */
          ELL_4V_HOMOG(tvertA, tvertA);
          ELL_4V_HOMOG(tvertB, tvertB);
          ovi = sctx->vidx[bag->evti[ei] + 5*si] = 
            airArrayLenIncr(bag->xyzwArr, 1);
          ELL_4V_SET_TT(lpld->xyzw + 4*ovi, float,
                        tvertA[0], tvertA[1], tvertA[2], 1.0);
          /*
          fprintf(stderr, "!%s: vert %u: %g %g %g\n", me, ovi,
                  tvertA[0], tvertA[1], tvertA[2]);
          */
          if (sctx->normalsFind) {
            airArrayLenIncr(bag->normArr, 1);
            if (sctx->normAns) {
              gageProbe(sctx->gctx, tvertB[0], tvertB[1], tvertB[2]);
              ELL_3V_SCALE_TT(lpld->norm + 3*ovi, float, -1, sctx->normAns);
              if (sctx->reverse) {
                ELL_3V_SCALE(lpld->norm + 3*ovi, -1, lpld->norm + 3*ovi);
              }
            } else {
              ELL_3V_LERP(grad, ww, vgrad[vi0], vgrad[vi1]);
              ELL_3MV_MUL(tvec, sctx->txfNormal, grad);
              ELL_3V_NORM_TT(lpld->norm + 3*ovi, float, tvec, tlen);
            }
          }
          sctx->vertNum++;
          /*
            fprintf(stderr, "%s: vert %d (edge %d) of (%d,%d,%d) "
            "at %g %g %g\n",
            me, sctx->vidx[bag->evti[ei] + 5*si], ei, xi, yi, zi,
            vert[0] + xi, vert[1] + yi, vert[2] + bag->zi);
          */
        }
      }
      /* add triangles */
      ti = 0;
      tcase = seekContour3DTopoHackTriangle[vcase];
      while (-1 != tcase[0 + 3*ti]) {
        unsigned iii;
        ELL_3V_SET(vii,
                   sctx->vidx[bag->evti[tcase[0 + 3*ti]] + 5*si],
                   sctx->vidx[bag->evti[tcase[1 + 3*ti]] + 5*si],
                   sctx->vidx[bag->evti[tcase[2 + 3*ti]] + 5*si]);
        if (sctx->reverse) {
          int tmpi;
          tmpi = vii[1]; vii[1] = vii[2]; vii[2] = tmpi;
        }
        iii = airArrayLenIncr(bag->indxArr, 3);
        ELL_3V_COPY(lpld->indx + iii, vii);
        /*
        fprintf(stderr, "!%s: tri %u: %u %u %u\n",
                me, iii/3, vii[0], vii[1], vii[2]);
        */
        lpld->icnt[0] += 3;
        sctx->faceNum++;
        ti++;
      }
    }
  }
  return 0;
}

static int
surfaceExtract(seekContext *sctx, limnPolyData *lpld) {
  char me[]="surfaceExtract", err[BIFF_STRLEN], done[AIR_STRLEN_SMALL];
  unsigned int zi, sz;
  baggage *bag;

  bag = baggageNew(sctx);
  sz = AIR_CAST(unsigned int, sctx->sz);

  /* this creates the airArrays in bag */
  if (outputInit(sctx, bag, lpld)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(SEEK, err); return 1;
  }

  if (sctx->verbose > 2) {
    fprintf(stderr, "%s: extracting ...       ", me);
  }
  for (zi=0; zi<sz-1; zi++) {
    if (sctx->verbose > 2) {
      fprintf(stderr, "%s", airDoneStr(0, zi, sz-2, done));
      fflush(stderr);
    }
    bag->zi = zi;
    if (shuffleProbe(sctx, bag)
        || triangulate(sctx, bag, lpld)) {
      sprintf(err, "%s: trouble on zi = %u", me, zi);
      biffAdd(SEEK, err); return 1;
    }
  }
  if (sctx->verbose > 2) {
    fprintf(stderr, "%s\n", airDoneStr(0, zi, sz-2, done));
  }

  /* this cleans up the airArrays in bag */
  baggageNix(bag);

  return 0;
}

int
seekExtract(seekContext *sctx, limnPolyData *lpld) {
  char me[]="seekExtract", err[BIFF_STRLEN];
  double time0;
  int E;

  if (!( sctx && lpld )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(SEEK, err); return 1;
  }

  if (seekTypeIsocontour == sctx->type) {
    if (!AIR_EXISTS(sctx->isovalue)) {
      sprintf(err, "%s: didn't seem to ever set isovalue (now %g)", me,
              sctx->isovalue);
      biffAdd(SEEK, err); return 1;
    }
  }

  if (sctx->verbose) {
    fprintf(stderr, "%s: --------------------\n", me);
    fprintf(stderr, "%s: flagResult = %d\n", me,
            sctx->flag[flagResult]);
  }

  /* reset max strength seen */
  sctx->strengthSeenMax = AIR_NAN;

  /* start time */
  time0 = airTime();

  switch(sctx->type) {
  case seekTypeIsocontour:
  case seekTypeRidgeSurface:
  case seekTypeValleySurface:
  case seekTypeMinimalSurface:
  case seekTypeMaximalSurface:
    E = surfaceExtract(sctx, lpld);
    break;
  default:
    sprintf(err, "%s: sorry, %s extraction not implemented", me,
            airEnumStr(seekType, sctx->type));
    biffAdd(SEEK, err); return 1;
    break;
  }
  if (E) {
    sprintf(err, "%s: trouble", me);
    biffAdd(SEEK, err); return 1;
  }

  /* end time */
  sctx->time = airTime() - time0;

  sctx->flag[flagResult] = AIR_FALSE;

  return 0;
}
