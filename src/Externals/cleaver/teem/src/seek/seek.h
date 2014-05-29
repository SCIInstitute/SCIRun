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

#ifndef SEEK_HAS_BEEN_INCLUDED
#define SEEK_HAS_BEEN_INCLUDED

#include <stdlib.h>

#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/hest.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/gage.h>
#include <teem/limn.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(seek_EXPORTS) || defined(teem_EXPORTS)
#    define SEEK_EXPORT extern __declspec(dllexport)
#  else
#    define SEEK_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define SEEK_EXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SEEK seekBiffKey

/*
******** seekType* enum
**
** the kinds of features that can be extracted with seek3DExtract
*/
enum {
  seekTypeUnknown,        /* 0: nobody knows */
  seekTypeIsocontour,     /* 1: standard marching-cubes */
  seekTypeRidgeSurface,   /* 2: */
  seekTypeValleySurface,  /* 3: */
  seekTypeRidgeLine,      /* 4: */
  seekTypeValleyLine,     /* 5: */
  seekTypeMinimalSurface, /* 6: */
  seekTypeMaximalSurface, /* 7: */
  seekTypeLast
};
#define SEEK_TYPE_MAX        7

typedef struct {
  /* ------- input ------- */
  int verbose;
  const Nrrd *ninscl;           /* a vanilla scalar (only) volume */
  gageContext *gctx;            /* for handling non-vanilla scalar volumes */
  gagePerVolume *pvl;           /* for handling non-vanilla scalar volumes */
  int type;                     /* from seekType* enum */
  int sclvItem,
    gradItem, normItem,
    evalItem, evecItem,
    stngItem;                   /* "stng" == strength */
  int lowerInside,              /* lower values are logically inside
                                   isosurfaces, not outside */
    normalsFind,                /* find normals for isosurface vertices, either
                                   with forward and central differencing on 
                                   values, or via the given normItem. **NOTE**
                                   simplifying assumption: if gctx, and
                                   if normalsFind, then normItem must be set
                                   (i.e. will not find normals via differencing
                                   when we have a gctx) */
    strengthUse,                /* do reject contributions to the feature for
                                   which the strength measurement falls below
                                   the required threshold */
    strengthSign;               /* the sign (either +1 of -1) of the strength
                                   quantity when it is considered to have high
                                   magnitude. E.g. for ridge surfaces,
                                   hesseval[2] should be very negative so 
                                   strengthSign == -1 */
  double isovalue,              /* for seekTypeIsocontour */
    strength,                   /* if strengthUse, feature needs to satisfy
                                   strengthAns*strengthSign > strength */
    strengthMin;
  size_t samples[3];            /* user-requested dimensions of feature grid */
  double facesPerVoxel,         /* approximate; for pre-allocating geometry */
    vertsPerVoxel;              /* approximate; for pre-allocating geometry */
  unsigned int pldArrIncr;      /* increment for airArrays used during the
                                   creation of geometry */
  /* ------ internal ----- */
  int *flag;                    /* for controlling updates of internal state */
  const Nrrd *nin;              /* either ninscl or gctx->pvl->nin */
  unsigned int baseDim;         /* 0 for scalars, or something else */
  gageShape *_shape,            /* local shape, always owned */
    *shape;                     /* not owned */
  Nrrd *nsclDerived;            /* for seekTypeIsocontour: volume of computed
                                   scalars, as measured by gage */
  const double *sclvAns,
    *gradAns, *normAns,
    *evalAns, *evecAns,
    *stngAns;
  int reverse;                  /* for seekTypeIsocontour: need to reverse
                                   sign of scalar field normal to get the 
                                   "right" isocontour normal */
  double txfNormal[9];          /* for seekTypeIsocontour: how to
                                   transform normals */
  size_t spanSize;              /* for seekTypeIsocontour: resolution of
                                   span space along edge */
  Nrrd *nspanHist;              /* for seekTypeIsocontour: span space
                                   histogram */
  NrrdRange *range;             /* for seekTypeIsocontour: range of scalars */
  size_t sx, sy, sz;            /* actual dimensions of feature grid */
  double txfIdx[16];            /* transforms from the index space of the 
                                   feature sampling grid to the index space
                                   of the underlying volume */
  int *vidx;                    /* 5 * sx * sy array of vertex index
                                   offsets, to support re-using of vertices
                                   across voxels and slices. Yes, this means
                                   there is allocation for edges in the voxels
                                   beyond the positive X and Y edges */
  double *sclv,                 /* 4 * (sx+2) * (sy+2) scalar value cache,
                                   with Z as fastest axis, and one sample
                                   of padding on all sides, as needed for
                                   central-difference-based gradients */
    *grad,                      /* 3 * 2 * sx * sy array of gradients, for
                                   crease feature extraction; axis ordering
                                   (vec,z,x,y) */
    *eval,                      /* 3 * 2 * sx * sy array of eigenvalues */
    *evec;                      /* 9 * 2 * sx * sy array of eigenvectors */
  signed char *flip;            /* 2 * 5 * sx * sy record of how eigenvector(s)
                                   of interest flip along the 5 voxel edges
                                   that are unique to each voxel.  Fastest axis
                                   is the two eigensystem indices that are
                                   tracked, in the case of crease lines */
  double *stng;                 /* 2 * sx * sy array of strength */
  Nrrd *nvidx, *nsclv,          /* nrrd wrappers around arrays above */
    *ngrad, *neval,
    *nevec, *nflip,
    *nstng;
  /* ------ output ----- */
  unsigned int
    voxNum, vertNum, faceNum;   /* number of voxels contributing to latest
                                   isosurface, and number of vertices and
                                   faces in that isosurface */
  double strengthSeenMax;       /* in case strength was used, the maximum
                                   vertex strength seen (from probing slabs) */
  double time;                  /* time for extraction */
} seekContext;

/* enumsSeek.c */
SEEK_EXPORT const char *seekBiffKey;
SEEK_EXPORT airEnum *seekType;

/* tables.c */
SEEK_EXPORT const int seekContour3DTopoHackEdge[256];
SEEK_EXPORT const int seekContour3DTopoHackTriangle[256][19];

/* methodsSeek.c */
SEEK_EXPORT seekContext *seekContextNew(void);
SEEK_EXPORT seekContext *seekContextNix(seekContext *sctx);

/* setSeek.c */
SEEK_EXPORT void seekVerboseSet(seekContext *sctx, int verbose);
SEEK_EXPORT int seekDataSet(seekContext *sctx, const Nrrd *ninscl,
                            gageContext *gctx, unsigned int pvlIdx);
SEEK_EXPORT int seekNormalsFindSet(seekContext *sctx, int doit);
SEEK_EXPORT int seekStrengthUseSet(seekContext *sctx, int doit);
SEEK_EXPORT int seekStrengthSet(seekContext *sctx, int strengthSign,
                                double strengthMin, double strength);
SEEK_EXPORT int seekSamplesSet(seekContext *sctx, size_t samples[3]);
SEEK_EXPORT int seekTypeSet(seekContext *sctx, int type);
SEEK_EXPORT int seekLowerInsideSet(seekContext *sctx, int lowerInside);
SEEK_EXPORT int seekItemScalarSet(seekContext *sctx, int item);
SEEK_EXPORT int seekItemStrengthSet(seekContext *sctx, int item);
SEEK_EXPORT int seekItemNormalSet(seekContext *sctx, int item);
SEEK_EXPORT int seekItemGradientSet(seekContext *sctx, int item);
SEEK_EXPORT int seekItemEigensystemSet(seekContext *sctx,
                                       int evalItem, int evecItem);
SEEK_EXPORT int seekIsovalueSet(seekContext *sctx, double isovalue);

/* updateSeek */
SEEK_EXPORT int seekUpdate(seekContext *sctx);

/* extract.c */
SEEK_EXPORT int seekExtract(seekContext *sctx, limnPolyData *lpld);

#ifdef __cplusplus
}
#endif

#endif /* SEEK_HAS_BEEN_INCLUDED */
