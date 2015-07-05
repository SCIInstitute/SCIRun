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

#ifndef GAGE_HAS_BEEN_INCLUDED
#define GAGE_HAS_BEEN_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(gage_EXPORTS) || defined(teem_EXPORTS)
#    define GAGE_EXPORT extern __declspec(dllexport)
#  else
#    define GAGE_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define GAGE_EXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define GAGE gageBiffKey

/*
** the only extent to which gage treats different axes differently is
** the spacing between samples along the axis.  To have different
** filters for the same function, but along different axes, would be
** too messy.  [Sun Mar 9 13:32:22 EDT 2008: Actually, doing per-axis
** kernels is looking more and more sensible all the time...] Thus,
** gage is not very useful as the engine for downsampling: it can't
** tell that along one axis samples should be blurred while they
** should be interpolated along another.  Rather, it assumes that the
** main task of probing is *reconstruction*: of values, of
** derivatives, or lots of different quantities
*/

/*
** terminology: gage is intended to measure multiple things at one
** point in a volume.  The SET of ALL the things that you want
** gage to measure is called the "QUERY".  Each of the many quantities
** comprising the query are called "ITEM"s.  
*/

/*
******** gageParm.. enum
**
** these are passed to gageSet.  Look for like-wise named field of
** gageParm for documentation on what these mean.
**
** The following things have to agree:
** - gageParm* enum
** - fields of gageParm struct
** - analagous gageDef* defaults (their declaration and setting)
** - action of gageParmSet
** - action of gageParmReset
*/
enum {
  gageParmUnknown,
  gageParmVerbose,                /* non-boolean int */
  gageParmRenormalize,            /* int */
  gageParmCheckIntegrals,         /* int */
  gageParmK3Pack,                 /* int */
  gageParmGradMagCurvMin,         /* double */
  gageParmDefaultSpacing,         /* double */
  gageParmCurvNormalSide,         /* int */
  gageParmKernelIntegralNearZero, /* double */
  gageParmRequireAllSpacings,     /* int */
  gageParmRequireEqualCenters,    /* int */
  gageParmDefaultCenter,          /* int */
  gageParmStackUse,               /* int */
  gageParmStackRenormalize,       /* int (does not imply gageParmStackUse) */
  gageParmLast
};

enum {
  gageErrUnknown,            /* 0: nobody knows */
  gageErrNone,               /* 1: no error, actually, all's well */
  gageErrBoundsSpace,        /* 2: out of 3-D (index-space) bounds */
  gageErrBoundsStack,        /* 3: out of 1-D bounds of stack */
  gageErrStackIntegral,      /* 4: stack recon coeff's sum to 0 */
  gageErrLast
};
#define GAGE_ERR_MAX            4

/*
******** gage{Ctx,Pvl}Flag.. enum
**
** organizes all the dependendies within a context and between a
** context and pervolumes.  This logic is to determine the support
** required for a query: different queries need different kernels,
** different kernels have different supports.  The user should not
** have to be concerned about any of this; it should be useful only
** to gageUpdate().
*/
enum {
  gageCtxFlagUnknown,
  gageCtxFlagNeedD,      /*  1: derivatives required for query */
  gageCtxFlagK3Pack,     /*  2: whether to use 3 or 6 kernels */
  gageCtxFlagNeedK,      /*  3: which of the kernels will actually be used */
  gageCtxFlagKernel,     /*  4: any one of the kernels or its parameters */
  gageCtxFlagRadius,     /*  5: radius of support for kernels with query */
  gageCtxFlagShape,      /*  6: a new pervolume shape was set */
  gageCtxFlagLast
};
#define GAGE_CTX_FLAG_MAX    6

enum {
  gagePvlFlagUnknown,
  gagePvlFlagVolume,     /*  1: got a new volume */
  gagePvlFlagQuery,      /*  2: what do you really care about */
  gagePvlFlagNeedD,      /*  3: derivatives required for query */
  gagePvlFlagLast
};
#define GAGE_PVL_FLAG_MAX    3
  

/*
******** gageKernel.. enum
**
** these are the different kernels that might be used in gage, regardless
** of what kind of volume is being probed.
*/
enum {
  gageKernelUnknown,    /*  0: nobody knows */
  gageKernel00,         /*  1: measuring values */
  gageKernel10,         /*  2: reconstructing 1st derivatives */
  gageKernel11,         /*  3: measuring 1st derivatives */
  gageKernel20,         /*  4: reconstructing 1st partials and 2nd deriv.s */
  gageKernel21,         /*  5: measuring 1st partials for a 2nd derivative */
  gageKernel22,         /*  6: measuring 2nd derivatives */
  gageKernelStack,      /*  7: for reconstructing across a stack */
  gageKernelLast
};
#define GAGE_KERNEL_MAX     7

/* 
******** GAGE_ITEM_PREREQ_MAXNUM
**
** Max number of prerequisites for any item in *any* kind.
**
** This value has gotten bumped periodically, which used to mean
** that *all* item tables had to be updated, when "-1" was used
** represent the unknown item.  But now that 0 represents the 
** unknown item, and because struct elements are implicitly 
** initialized to zero, this is no longer the case.
**
** Wed Nov  8 14:12:44 EST 2006 pre-emptively upping this from 6
*/
#define GAGE_ITEM_PREREQ_MAXNUM 8

/*
******** gageItemEntry struct
**
** necessary information about each item supported by the kind, which 
** is defined at compile-time in a per-kind table (at least it is for
** the scalar, vector, and tensor kinds)
**
** NOTE!!! YOU CAN NOT re-arrange these variables, because of all the
** compile-time definitions that are done to define the gageKind->table
** for all current kinds.
*/
typedef struct {
  int enumVal;          /* the item's enum value */
  unsigned int 
    answerLength;       /* how many double's are needed to store the answer,
                           or (for non-zero items), 0 to represent 
                           "the length will be learned later at runtime" */
  int needDeriv,        /* what kind of derivative info is immediately needed
                           for this item (not recursively expanded). This is
                           NO LONGER a bitvector: values are 0, 1, 2, .. */
    prereq[GAGE_ITEM_PREREQ_MAXNUM],
                        /* what are the other items this item depends on
                           (not recusively expanded), you can list up to
                           GAGE_ITEM_PREREQ_MAXNUM of them, and use 0
                           (the unknown item) to fill out the list.
                           _OR_ -1 if this is a dynamic kind and the prereqs
                           will not be known until later in runtime */
    parentItem,         /* the enum value of an item (answerLength > 1)
                           containing the smaller value for which we are
                           merely an alias
                           _OR_ 0 if there's no parent */
    parentIndex,        /* where our value starts in parents value 
                           _OR_ 0 if there's no parent */
    needData;           /* whether non-NULL gagePerVolume->data is needed
                           for answering this item */
} gageItemEntry;
  
/*
** modifying the enums below (scalar, vector, etc query quantities)
** necesitates modifying:
** - the central item table 
** - the associated arrays in arrays.c
** - the arrays in enums.c, 
** - the "answer" method itself.
*/

/*
******** gageScl* enum
**
** all the "items" that gage can measure in a scalar volume.
**
** NOTE: although it is currently listed that way, it is not necessary
** that prerequisite measurements are listed before the other measurements
** which need them (that is represented by _gageSclPrereq)
**
** The description for each enum value starts with the numerical value
** followed by a string which identifies the value in the gageScl airEnum.
** The "[N]" indicates how many doubles are used for storing the quantity.
*/
enum {
  gageSclUnknown,      /*  0: nobody knows */
  gageSclValue,        /*  1: "v", data value: [1] */
  gageSclGradVec,      /*  2: "grad", gradient vector, un-normalized: [3] */
  gageSclGradMag,      /*  3: "gm", gradient magnitude: [1] */
  gageSclNormal,       /*  4: "n", gradient vector, normalized: [3] */
  gageSclNPerp,        /*  5: "np", projection onto tangent plane: [9] */
  gageSclHessian,      /*  6: "h", Hessian: [9] (column-order) */
  gageSclLaplacian,    /*  7: "l", Laplacian: Dxx + Dyy + Dzz: [1] */
  gageSclHessFrob,     /*  8: "hf", Frobenius normal of Hessian: [1] */
  gageSclHessEval,     /*  9: "heval", Hessian's eigenvalues: [3] */
  gageSclHessEval0,    /* 10: "heval0", Hessian's 1st eigenvalue: [1] */
  gageSclHessEval1,    /* 11: "heval1", Hessian's 2nd eigenvalue: [1] */
  gageSclHessEval2,    /* 12: "heval2", Hessian's 3rd eigenvalue: [1] */
  gageSclHessEvec,     /* 13: "hevec", Hessian's eigenvectors: [9] */
  gageSclHessEvec0,    /* 14: "hevec0", Hessian's 1st eigenvector: [3] */
  gageSclHessEvec1,    /* 15: "hevec1", Hessian's 2nd eigenvector: [3] */
  gageSclHessEvec2,    /* 16: "hevec2", Hessian's 3rd eigenvector: [3] */
  gageScl2ndDD,        /* 17: "2d", 2nd dir.deriv. along gradient: [1] */
  gageSclGeomTens,     /* 18: "gten", sym. matx w/ evals {0, K1, K2} and
                              evecs {grad, cdir0, cdir1}: [9] */
  gageSclK1,           /* 19: "k1", 1st principle curvature: [1] */
  gageSclK2,           /* 20: "k2", 2nd principle curvature (k2 <= k1): [1] */
  gageSclTotalCurv,    /* 21: "tc", L2 norm(K1,K2) (not Koen.'s "C"): [1] */
  gageSclShapeTrace,   /* 22, "st", (K1+K2)/Curvedness: [1] */
  gageSclShapeIndex,   /* 23: "si", Koen.'s shape index, ("S"): [1] */
  gageSclMeanCurv,     /* 24: "mc", mean curvature (K1 + K2)/2: [1] */
  gageSclGaussCurv,    /* 25: "gc", gaussian curvature K1*K2: [1] */
  gageSclCurvDir1,     /* 26: "cdir1", 1st principle curv direction: [3] */
  gageSclCurvDir2,     /* 27: "cdir2", 2nd principle curv direction: [3] */
  gageSclFlowlineCurv, /* 28: "fc", curvature of normal streamline: [1] */
  gageSclMedian,       /* 29: "med", median filter */
  gageSclHessValleyness,   /* 30: "valley measure", vallyness measure: [1] */
  gageSclHessRidgeness,    /* 31: "ridge measure", ridgeness measure: [1] */
  gageSclHessMode,     /* 32: "mode hessian", Hessian's mode: [1] */
  gageSclLast
};
#define GAGE_SCL_ITEM_MAX  32

/*
******** gageVec* enum
**
** all the "items" that gage knows how to measure in a 3-vector volume
**
** The strings gives one of the gageVec airEnum identifiers, and [x]
** says how many scalars are associated with this value.
*/
enum {
  gageVecUnknown,         /*  0: nobody knows */
  gageVecVector,          /*  1: "v", component-wise-intrpolated
                                 (CWI) vec: [3] */
  gageVecVector0,         /*  2: "v0", vector[0]: [1] */
  gageVecVector1,         /*  3: "v1", vector[0]: [1] */
  gageVecVector2,         /*  4: "v2", vector[0]: [1] */
  gageVecLength,          /*  5: "l", length of CWI vector: [1] */
  gageVecNormalized,      /*  6: "n", normalized CWI vector: [3] */
  gageVecJacobian,        /*  7: "j", component-wise Jacobian: [9]
                                0:dv_x/dx  1:dv_x/dy  2:dv_x/dz
                                3:dv_y/dx  4:dv_y/dy  5:dv_y/dz
                                6:dv_z/dx  7:dv_z/dy  8:dv_z/dz */
  gageVecDivergence,      /*  8: "d", divergence (based on Jacobian): [1] */
  gageVecCurl,            /*  9: "c", curl (based on Jacobian): [3] */
  gageVecCurlNorm,        /* 10: "cm", curl magnitude: [1] */
  gageVecHelicity,        /* 11: "h", helicity: vec . curl: [1] */
  gageVecNormHelicity,    /* 12: "nh", normalized helicity: [1] */
  gageVecLambda2,         /* 13: "lambda2", lambda2 criterion: [1] */
  gageVecImaginaryPart,   /* 14: "imag", imag. part of jacobian's
                                eigenv: [1] */
  gageVecHessian,         /* 15: "vh", second-order derivative: [27] 
                                 HEY: indices here need to be double checked
                                 0:d2v_x/dxdx   1:d2v_x/dxdy   2:d2v_x/dxdz
                                 3:d2v_x/dydx   4:d2v_x/dydy   5:d2v_x/dydz
                                 6:d2v_x/dzdx   7:d2v_x/dzdy   8:d2v_x/dzdz
                                 9:d2v_y/dxdx       [..]
                                    [..]
                                24:dv2_z/dzdx  25:d2v_z/dzdy  26:d2v_z/dzdz */
  gageVecDivGradient,     /* 16: "dg", divergence gradient: [3] */
  gageVecCurlGradient,    /* 17: "cg", curl gradient: [9] */
  gageVecCurlNormGrad,    /* 18: "cng", curl norm gradient: [3] */
  gageVecNCurlNormGrad,   /* 19: "ncng", normalized curl norm gradient: [3] */
  gageVecHelGradient,     /* 20: "hg", helicity gradient: [3] */
  gageVecDirHelDeriv,     /* 21: "dhd", directional derivative
                                 of helicity: [1] */ 
  gageVecProjHelGradient, /* 22: "phg", projected helicity gradient: [3] */
  gageVecGradient0,       /* 23: "g0", gradient of 1st coeff of vector: [3] */
  gageVecGradient1,       /* 24: "g1", gradient of 2nd coeff of vector: [3] */
  gageVecGradient2,       /* 25: "g2", gradient of 3rd coeff of vector: [3] */
  gageVecMultiGrad,       /* 26: "mg", sum of outer products of grads: [9] */
  gageVecMGFrob,          /* 27: "mgfrob", frob norm of multi-gradient: [1] */
  gageVecMGEval,          /* 28: "mgeval", evals of multi-gradient: [3] */
  gageVecMGEvec,          /* 29: "mgevec", evecs of multi-gradient: [9] */
  gageVecLast
};
#define GAGE_VEC_ITEM_MAX     29

struct gageKind_t;       /* dumb forward declaraction, ignore */
struct gagePerVolume_t;  /* dumb forward declaraction, ignore */

/*
******** gageShape struct
**
** just a container for all the information related to the "shape"
** of all the volumes associated with a context
**
** Note that the utility of gageShape has extended well beyond doing
** convolution-based measurements in volumes- it has become the one-stop
** place for figuring out a reasonable way of locating a logically
** a volume in 3-D space, including using a nrrd's full orientation 
** information if it is known.
*/
typedef struct gageShape_t {
  int defaultCenter,          /* default centering to use when given volume
                                 has no centering set. *NOTE* this is the 
                                 only "input" field in the gageShape, all the
                                 rest are set by _gageShapeSet */
    center,                   /* the sample centering of the volume(s)- this
                                 determines the extent of the locations
                                 that may be probed */
    fromOrientation;          /* non-zero iff the spaceDirections and
                                 spaceOrigin information was used */
  unsigned int size[3];       /* raster dimensions of volume */
  double spacing[3];          /* spacings for each axis */
  /* fwScale[GAGE_KERNEL_MAX+1][3] has been superceded by the cleaner and
     more general ItoWSubInvTransp and ItoWSubInv matrices below */
  double volHalfLen[3],       /* half the lengths along each axis in order
                                 to bound the volume in a bi-unit cube */
    voxLen[3],                /* when bound in bi-unit cube, the dimensions
                                 of a single voxel */
    ItoW[16],                 /* homogeneous coord transform from index
                                 to world space (defined either by bi-unit
                                 cube or from full orientation info ) */
    WtoI[16],                 /* inverse of above */
    ItoWSubInvTransp[9],      /* inverse transpose of 3x3 sub-matrix of ItoW,
                                 to transform (covariant) gradients */
    ItoWSubInv[9];            /* tranpose of above, to transform hessians */

} gageShape;

/*
******** gageParm struct
**
** a container for the various switches and knobs which control
** gage, aside from the obvious inputs (kernels, queries, volumes)
*/
typedef struct gageParm_t {
  int renormalize;            /* hack to make sure that sum of
                                 discrete value reconstruction weights
                                 is same as kernel's continuous
                                 integral, and that the 1nd and 2nd
                                 deriv weights really sum to 0.0 */
  int checkIntegrals;         /* call the "integral" method of the
                                 kernel to verify that it is
                                 appropriate for the task for which
                                 the kernel is being set:
                                 reconstruction: 1.0, derivatives: 0.0 */
  int k3pack;                 /* non-zero (true) iff we do not use
                                 kernels for gageKernelIJ with I != J.
                                 So, we use the value reconstruction
                                 kernel (gageKernel00) for 1st and 2nd
                                 derivative reconstruction, and so on.
                                 This is faster because we can re-use
                                 results from low-order convolutions. */
  double gradMagCurvMin,      /* punt on computing curvature information if
                                 gradient magnitude is less than this. Yes,
                                 this is scalar-kind-specific, but there's
                                 no other good place for it */
    kernelIntegralNearZero,   /* tolerance with checkIntegrals on derivative
                                 kernels */
    defaultSpacing;           /* when requireAllSpacings is zero, what spacing
                                 to use when we have to invent one */
  int curvNormalSide,         /* determines direction of gradient that is used
                                 as normal in curvature calculations, exactly
                                 the same as miteUser's normalSide: 1 for
                                 normal pointing to lower values (higher
                                 values are more "inside"); -1 for normal
                                 pointing to higher values (low values more
                                 "inside") */
    requireAllSpacings,       /* if non-zero, require that spacings on all 3
                                 spatial axes are set, and are equal; this is
                                 the traditional way of gage.  If zero, then 
                                 one, two, or all three axes' spacing can be
                                 unset, and we'll use defaultSpacing instead */
    requireEqualCenters,      /* if non-zero, all centerings on spatial axes 
                                 must be the same (including the possibility 
                                 of all being nrrdCenterUnknown). If zero, its
                                 okay for axes' centers to be unset, but two
                                 that are set cannot be unequal */
    defaultCenter,            /* only meaningful when requireAllSpacings is
                                 zero- what centering to use when you have to
                                 invent one, because its not set */
    stackUse,                 /* if non-zero: treat the pvl's (all same kind)
                                 as multiple values of a single logical volume
                                 (e.g. for approximating scale space).
                                 The first pvl is effectively just a buffer;
                                 the N-1 pvls used are at index 1 through N-2.
                                 The query in pvl[0] will determine the
                                 computations done, and answers for the whole
                                 stack will be stored in pvl[0]. */
    stackRenormalize;         /* if non-zero (and if stackUse is non-zero):
                                 derivatives of filter stage are renormalized
                                 based on the stack parameter */
} gageParm;

/*
******** gagePoint struct
**
** stores location of last query location
**
** GK has gone back and forth over whether stack location should be part
** of this- logically it makes sense, but with the current organization of
** the stack implementation, it was just never used.
*/
typedef struct gagePoint_t {
  double xf, yf, zf;       /* fractional voxel location, used to
                              short-circuit calculation of filter sample
                              locations and weights */
  unsigned int xi, yi, zi; /* integral voxel location */
} gagePoint;

/*
******** gageQuery typedef
** 
** this short, fixed-length array is used as a bit-vector to store
** all the items that comprise a query.  Its length sets an upper
** bound on the maximum item value that a gageKind may use.
**
** The important thing to keep in mind is that a variable of type
** gageKind ends up being passed by reference, even though the
** syntax of its usage doesn't immediately announce that.
**
** gageKindCheck currently has the role of verifying that a gageKind's
** maximum item value is within the bounds set here. Using
** GAGE_QUERY_BYTES_NUM == 8 gives a max item value of 63, which is 
** far above anything being used now.
** 
** Sat Jan 21 18:12:01 EST 2006: ha! second derivatives of tensors blew
** past old GAGE_QUERY_BYTES_NUM, now GAGE_QUERY_BYTES_NUM == 16
**
** Tue Nov  7 19:51:05 EST 2006; tenGage items now pushing 127,
** guardedly changing GAGE_QUERY_BYTES_NUM to 24 --> max item 191
*/
#define GAGE_QUERY_BYTES_NUM 24
#define GAGE_ITEM_MAX ((8*GAGE_QUERY_BYTES_NUM)-1)
typedef unsigned char gageQuery[GAGE_QUERY_BYTES_NUM];

/*
******** GAGE_QUERY_RESET, GAGE_QUERY_TEST
******** GAGE_QUERY_ON, GAGE_QUERY_OFF
**
** Macros for manipulating a gageQuery.
**
** airSanity ensures that an unsigned char is in fact 8 bits
*/
#define GAGE_QUERY_RESET(q) \
  q[ 0] = q[ 1] = q[ 2] = q[ 3] = \
  q[ 4] = q[ 5] = q[ 6] = q[ 7] = \
  q[ 8] = q[ 9] = q[10] = q[11] = \
  q[12] = q[13] = q[14] = q[15] = \
  q[16] = q[17] = q[18] = q[19] = \
  q[20] = q[21] = q[22] = q[23] = 0

#define GAGE_QUERY_COPY(p, q) \
  p[ 0] = q[ 0]; p[ 1] = q[ 1]; p[ 2] = q[ 2]; p[ 3] = q[ 3]; \
  p[ 4] = q[ 4]; p[ 5] = q[ 5]; p[ 6] = q[ 6]; p[ 7] = q[ 7]; \
  p[ 8] = q[ 8]; p[ 9] = q[ 9]; p[10] = q[10]; p[11] = q[11]; \
  p[12] = q[12]; p[13] = q[13]; p[14] = q[14]; p[15] = q[15]; \
  p[16] = q[16]; p[17] = q[17]; p[18] = q[18]; p[19] = q[19]; \
  p[20] = q[20]; p[21] = q[21]; p[22] = q[22]; p[23] = q[23]

#define GAGE_QUERY_ADD(p, q) \
  p[ 0] |= q[ 0]; p[ 1] |= q[ 1]; p[ 2] |= q[ 2]; p[ 3] |= q[ 3]; \
  p[ 4] |= q[ 4]; p[ 5] |= q[ 5]; p[ 6] |= q[ 6]; p[ 7] |= q[ 7]; \
  p[ 8] |= q[ 8]; p[ 9] |= q[ 9]; p[10] |= q[10]; p[11] |= q[11]; \
  p[12] |= q[12]; p[13] |= q[13]; p[14] |= q[14]; p[15] |= q[15]; \
  p[16] |= q[16]; p[17] |= q[17]; p[18] |= q[18]; p[19] |= q[19]; \
  p[20] |= q[20]; p[21] |= q[21]; p[22] |= q[22]; p[23] |= q[23]

#define GAGE_QUERY_EQUAL(p, q) ( \
  p[ 0] == q[ 0] && p[ 1] == q[ 1] && p[ 2] == q[ 2] && p[ 3] == q[ 3] && \
  p[ 4] == q[ 4] && p[ 5] == q[ 5] && p[ 6] == q[ 6] && p[ 7] == q[ 7] && \
  p[ 8] == q[ 8] && p[ 9] == q[ 9] && p[10] == q[10] && p[11] == q[11] && \
  p[12] == q[12] && p[13] == q[13] && p[14] == q[14] && p[15] == q[15] && \
  p[16] == q[16] && p[17] == q[17] && p[18] == q[18] && p[19] == q[19] && \
  p[20] == q[20] && p[21] == q[21] && p[22] == q[22] && p[23] == q[23])

#define GAGE_QUERY_NONZERO(q) ( \
  q[ 0] | q[ 1] | q[ 2] | q[ 3] | \
  q[ 4] | q[ 5] | q[ 6] | q[ 7] | \
  q[ 8] | q[ 9] | q[10] | q[11] | \
  q[12] | q[13] | q[14] | q[15] | \
  q[16] | q[17] | q[18] | q[19] | \
  q[20] | q[21] | q[22] | q[23] )

#define GAGE_QUERY_ITEM_TEST(q, i) (q[i/8] & (1 << (i % 8)))
#define GAGE_QUERY_ITEM_ON(q, i) (q[i/8] |= (1 << (i % 8)))
#define GAGE_QUERY_ITEM_OFF(q, i) (q[i/8] &= ~(1 << (i % 8)))

  /* increment for ctx->pvlArr airArray */
#define GAGE_PERVOLUME_ARR_INCR 32

/*
******** gageContext struct
**
** The information here is specific to the dimensions, scalings, and
** radius of kernel support, but not to kind of volume (all kind-specific
** information is in the gagePerVolume).  One context can be used in
** conjuction with probing multiple volumes.
*/
typedef struct gageContext_t {
  /* INPUT ------------------------- */
  int verbose;                /* verbosity */
  gageParm parm;              /* all parameters */

  /* all the kernels we'll ever need, including the stack kernel */
  NrrdKernelSpec *ksp[GAGE_KERNEL_MAX+1];

  /* all the pervolumes attached to this context.  If using stack,
     the base pvl is the LAST, pvl[pvlNum-1], and the stack samples
     are pvl[0] through pvl[pvlNum-2] */
  struct gagePerVolume_t **pvl;

  /* number of pervolumes currently attached. If using stack,
     this is one more than number of stack samples (because of the
     base volume at the end) */
  unsigned int pvlNum;

  /* airArray for managing pvl and pvlNum */
  airArray *pvlArr;

  /* sizes, spacings, centering, and other geometric aspects of the
     volume */
  gageShape *shape;

  /* if stack is being used, allocated for length pvlNum-1, and
     stackPos[0] through stackPos[pvlNum-2] MUST exist and be
     monotonically increasing stack positions for each volume.
     Otherwise NULL */
  double *stackPos;

  /* INTERNAL ------------------------- */
  /* if using stack: allocated for length pvlNum-1, and filter sample
     locations and weights for reconstruction along the stack.
     Otherwise NULL. */
  double *stackFslw;

  /* all the flags used by gageUpdate() used to describe what changed
     in this context */
  int flag[GAGE_CTX_FLAG_MAX+1]; 

  /* which value/derivatives need to be calculated for all pervolumes
     (doV, doD1, doD2) */
  int needD[3];

  /* which kernels are needed for all pvls.  needK[gageKernelStack]
     is currently not set by the update function that sets needK[] */
  int needK[GAGE_KERNEL_MAX+1];

  /* radius of support of samples needed to satisfy query, given the
     set of kernels.  The "filter diameter" fd == 2*radius.  This is
     incremented by one if filtering across the stack with
     nrrdKernelHermiteFlag. */
  unsigned int radius;

  /* filter sample locations (all axes): logically a fd x 3 array */
  double *fsl;

  /* filter weights (all axes, all kernels): logically a
     fdx3xGAGE_KERNEL_MAX+1 array */
  double *fw;

  /* offsets to other fd^3 samples needed to fill 3D intermediate
     value cache. Allocated size is dependent on kernels, values
     inside are dependent on the dimensions of the volume. It may be
     more correct to be using size_t instead of uint, but the X and Y
     dimensions of the volume would have to be super-outrageous for
     that to be a problem */
  unsigned int *off;

  /* last probe location */
  gagePoint point;

  /* errStr and errNum are for describing errors that happen in gageProbe():
     using biff is too heavy-weight for this, and the idea is that no ill
     should occur if the error is repeatedly ignored.
     NOTE: these variables used to be globals "gageErrStr" and "gageErrNum" */
  char errStr[AIR_STRLEN_LARGE];
  int errNum;                 /* takes values from the gageErr enum */
} gageContext;

/*
******** gagePerVolume
**
** information that is specific to one volume, and to one kind of
** volume.
*/
typedef struct gagePerVolume_t {
  int verbose;                /* verbosity */
  const struct gageKind_t *kind;  /* what kind of volume is this for */
  gageQuery query;            /* the query, recursively expanded */
  int needD[3];               /* which derivatives need to be calculated for
                                 the query (above) in this volume */
  const Nrrd *nin;            /* the volume to sample within */
  int flag[GAGE_PVL_FLAG_MAX+1];/* for the kind-specific flags .. */
  double *iv3, *iv2, *iv1;    /* 3D, 2D, 1D, value caches.  These are cubical,
                                 square, and linear arrays, all length fd on
                                 each edge.  Currently gageIv3Fill() fills
                                 the iv3 (at a latter point this will be 
                                 delegated back to the gageKind to facilitate
                                 bricking), and currently the tuple axis (with
                                 length valLen) always slowest.  However, use
                                 of iv2 and iv1 is entirely up the kind's
                                 filter method. */
  double (*lup)(const void *ptr, size_t I); 
                              /* nrrd{F,D}Lookup[] element, according to
                                 nin->type and double */
  double *answer;             /* main buffer to hold all the answers */
  double **directAnswer;      /* array of pointers into answer */
  void *data;                 /* extra data, parameters, buffers, etc.
                                 required for answering some items
                                 (as per the gageItemEntry->needData)
                                 managed with kind->pvlDataNew,
                                 kind->pvlDataCopy, kind->pvlDataUpdate,
                                 and kind->pvlDataNix, so there is no channel
                                 for extra info to be passed into the pvl->data,
                                 other that what was put into kind->data */
} gagePerVolume;

/*
******** gageKind struct
**
** all the information and functions that are needed to handle one
** kind of volume (such as scalar, vector, etc.)
**
** these are either statically allocated (e.g. gageKindScl, gageKindVec,
** tenGageKind), or dynamically allocated, which case the kind itself
** needs a constructor (e.g. tenDwiGageKindNew()).  The "dynamicAlloc"
** variable indicates this distinction.
**
** Having dynamically allocated kinds raises the possibility of having
** to set and update (or modify and update) their internal state,
** which is currently completely outside the update framework of gage.
** So as far as the core gage functions are concerned, all kinds are
** static, because there is nothing to modify.  It also means that
** those who dynamically create kinds should try to minimize the
** state info that can/must be dynamically modified (i.e. maybe
** the kind constructor should take all the various parameters,
** and set everything in a single shot).
*/
typedef struct gageKind_t {
  int dynamicAlloc;                 /* non-zero if this kind struct was
                                       dynamically allocated */
  char name[AIR_STRLEN_SMALL];      /* short identifying string for kind */
  airEnum *enm;                     /* such as gageScl.  NB: the "unknown"
                                       value in the enum should be 0. */
  unsigned int baseDim,             /* dimension that x,y,z axes start on
                                       (e.g. 0 for scalars, 1 for vectors) */
    valLen;                         /* number of scalars per data point,
                                       -or- 0 to represent "this value will
                                       be learned later at runtime" */
  int itemMax;                      /* such as GAGE_SCL_ITEM_MAX */
  gageItemEntry *table;             /* array of gageItemEntry's, indexed
                                       by the item value,
                                       -or- NULL if the table cannot be
                                       statically allocated (not because it
                                       can come in different sizes, but
                                       because it needs to be a modified
                                       version of the compile-time table */
  void (*iv3Print)(FILE *,          /* such as _gageSclIv3Print() */
                   gageContext *,
                   gagePerVolume *),
    (*filter)(gageContext *,        /* such as _gageSclFilter() */
              gagePerVolume *),
    (*answer)(gageContext *,        /* such as _gageSclAnswer() */
              gagePerVolume *),
    /* for allocating, copying, updating, and nixing the pervolume->data */
    /* pvlDataNew and pvlDataCopy can use biff, but:
       --> they must use GAGE key (and not callback's library's key), and
       --> pvlDataNix can not use biff */
    *(*pvlDataNew)(const struct gageKind_t *kind),
    *(*pvlDataCopy)(const struct gageKind_t *kind, const void *data),
    *(*pvlDataNix)(const struct gageKind_t *kind, void *data);
  int (*pvlDataUpdate)(const struct gageKind_t *kind,
                       const gageContext *ctx,
                       const gagePerVolume *pvl,
                       const void *data);
  void *data;                       /* extra information about the kind of 
                                       volume that's being probed.  This
                                       is passed as "data" to pvlDataNew,
                                       pvlDataCopy, and pvlDataNix */
} gageKind;

/*
******** gageItemSpec struct
**
** dumb little way to store a kind/item pair.  Formerly known
** as a gageQuerySpec.  
*/
typedef struct {
  const gageKind *kind;       /* the kind of the volume to measure */
  int item;                   /* the quantity to measure */
} gageItemSpec;

/* defaultsGage.c */
GAGE_EXPORT const char *gageBiffKey;
GAGE_EXPORT int gageDefVerbose;
GAGE_EXPORT double gageDefGradMagCurvMin;
GAGE_EXPORT int gageDefRenormalize;
GAGE_EXPORT int gageDefCheckIntegrals;
GAGE_EXPORT int gageDefK3Pack;
GAGE_EXPORT double gageDefDefaultSpacing;
GAGE_EXPORT int gageDefCurvNormalSide;
GAGE_EXPORT double gageDefKernelIntegralNearZero;
GAGE_EXPORT int gageDefRequireAllSpacings;
GAGE_EXPORT int gageDefRequireEqualCenters;
GAGE_EXPORT int gageDefDefaultCenter;
GAGE_EXPORT int gageDefStackUse;
GAGE_EXPORT int gageDefStackRenormalize;

/* miscGage.c */
GAGE_EXPORT double gageZeroNormal[3];
GAGE_EXPORT airEnum *gageErr;
GAGE_EXPORT airEnum *gageKernel;
GAGE_EXPORT void gageParmReset(gageParm *parm);
GAGE_EXPORT void gagePointReset(gagePoint *point);
GAGE_EXPORT gageItemSpec *gageItemSpecNew(void);
GAGE_EXPORT void gageItemSpecInit(gageItemSpec *isp);
GAGE_EXPORT gageItemSpec *gageItemSpecNix(gageItemSpec *isp);

/* kind.c */
GAGE_EXPORT int gageKindCheck(const gageKind *kind);
GAGE_EXPORT int gageKindTotalAnswerLength(const gageKind *kind);
GAGE_EXPORT unsigned int gageKindAnswerLength(const gageKind *kind, int item);
GAGE_EXPORT int gageKindAnswerOffset(const gageKind *kind, int item);
GAGE_EXPORT int gageKindVolumeCheck(const gageKind *kind, const Nrrd *nrrd);

/* print.c */
GAGE_EXPORT void gageQueryPrint(FILE *file, const gageKind *kind,
                                gageQuery query);

/* sclfilter.c */
typedef void (gageScl3PFilter_t)(gageShape *shape,
                                 double *iv3, double *iv2, double *iv1,
                                 double *fw00, double *fw11, double *fw22,
                                 double *val, double *gvec, double *hess,
                                 int doV, int doD1, int doD2);
GAGE_EXPORT gageScl3PFilter_t gageScl3PFilter2;
GAGE_EXPORT gageScl3PFilter_t gageScl3PFilter4;
GAGE_EXPORT gageScl3PFilter_t gageScl3PFilter6;
GAGE_EXPORT gageScl3PFilter_t gageScl3PFilter8;
GAGE_EXPORT void gageScl3PFilterN(gageShape *shape, int fd,
                                  double *iv3, double *iv2, double *iv1,
                                  double *fw00, double *fw11, double *fw22,
                                  double *val, double *gvec, double *hess,
                                  int doV, int doD1, int doD2);

/* scl.c */
GAGE_EXPORT airEnum *const gageScl;
GAGE_EXPORT gageKind *const gageKindScl;

/* vecGage.c (together with vecprint.c, these contain everything to
   implement the "vec" kind, and could be used as examples of what it
   takes to create a new gageKind) */
GAGE_EXPORT airEnum *const gageVec;
GAGE_EXPORT gageKind *const gageKindVec;

/* shape.c */
GAGE_EXPORT void gageShapeReset(gageShape *shp);
GAGE_EXPORT gageShape *gageShapeNew();
GAGE_EXPORT gageShape *gageShapeCopy(gageShape *shp);
GAGE_EXPORT gageShape *gageShapeNix(gageShape *shape);
GAGE_EXPORT int gageShapeSet(gageShape *shp, const Nrrd *nin, int baseDim);
GAGE_EXPORT void gageShapeWtoI(gageShape *shape,
                               double index[3], double world[3]);
GAGE_EXPORT void gageShapeItoW(gageShape *shape,
                               double world[3], double index[3]);
GAGE_EXPORT int gageShapeEqual(gageShape *shp1, char *name1,
                               gageShape *shp2, char *name2);
GAGE_EXPORT void gageShapeBoundingBox(double min[3], double max[3],
                                      gageShape *shape);

/* the organization of the next two files used to be according to
   what the first argument is, not what appears in the function name,
   but that's just a complete mess now */
/* pvl.c */
GAGE_EXPORT int gageVolumeCheck(const gageContext *ctx, const Nrrd *nin,
                                const gageKind *kind);
GAGE_EXPORT gagePerVolume *gagePerVolumeNew(gageContext *ctx,
                                            const Nrrd *nin,
                                            const gageKind *kind);
GAGE_EXPORT gagePerVolume *gagePerVolumeNix(gagePerVolume *pvl);
GAGE_EXPORT const double *gageAnswerPointer(const gageContext *ctx, 
                                            const gagePerVolume *pvl,
                                            int item);
GAGE_EXPORT unsigned int gageAnswerLength(const gageContext *ctx,
                                          const gagePerVolume *pvl,
                                          int item);
GAGE_EXPORT int gageQueryReset(gageContext *ctx, gagePerVolume *pvl);
GAGE_EXPORT int gageQuerySet(gageContext *ctx, gagePerVolume *pvl,
                             gageQuery query);
GAGE_EXPORT int gageQueryAdd(gageContext *ctx, gagePerVolume *pvl,
                             gageQuery query);
GAGE_EXPORT int gageQueryItemOn(gageContext *ctx, gagePerVolume *pvl,
                                int item);

/* stack.c */
GAGE_EXPORT double gageTauOfTee(double tee);
GAGE_EXPORT double gageTeeOfTau(double tau);
GAGE_EXPORT double gageSigOfTau(double tau);
GAGE_EXPORT double gageTauOfSig(double sig);
GAGE_EXPORT int gageStackBlur(Nrrd *const nblur[], const double *scale,
                              unsigned int num,
                              const Nrrd *nin, unsigned int baseDim,
                              const NrrdKernelSpec *kspec,
                              int boundary, int renormalize, int verbose);
GAGE_EXPORT int gageStackPerVolumeNew(gageContext *ctx,
                                      gagePerVolume ***pvlP,
                                      const Nrrd *const *nblur,
                                      unsigned int blnum,
                                      const gageKind *kind);
GAGE_EXPORT int gageStackPerVolumeAttach(gageContext *ctx,
                                         gagePerVolume *pvlBase,
                                         gagePerVolume **pvlStack,
                                         const double *stackPos,
                                         unsigned int blnum);

/* ctx.c */
GAGE_EXPORT gageContext *gageContextNew();
GAGE_EXPORT gageContext *gageContextCopy(gageContext *ctx);
GAGE_EXPORT gageContext *gageContextNix(gageContext *ctx);
GAGE_EXPORT void gageParmSet(gageContext *ctx, int which, double val);
GAGE_EXPORT int gagePerVolumeIsAttached(const gageContext *ctx,
                                        const gagePerVolume *pvl);
GAGE_EXPORT int gagePerVolumeAttach(gageContext *ctx, gagePerVolume *pvl);
GAGE_EXPORT int gagePerVolumeDetach(gageContext *ctx, gagePerVolume *pvl);
GAGE_EXPORT int gageKernelSet(gageContext *ctx, int which,
                              const NrrdKernel *k, const double *kparm);
GAGE_EXPORT void gageKernelReset(gageContext *ctx);
GAGE_EXPORT int gageProbe(gageContext *ctx, double xi, double yi, double zi);
GAGE_EXPORT int gageProbeSpace(gageContext *ctx, double x, double y, double z,
                               int indexSpace, int clamp);
GAGE_EXPORT int gageStackProbe(gageContext *ctx,
                               double xi, double yi, double zi, double si);
GAGE_EXPORT int gageStackProbeSpace(gageContext *ctx,
                                    double x, double y, double z, double s,
                                    int indexSpace, int clamp);

/* update.c */
GAGE_EXPORT int gageUpdate(gageContext *ctx);

/* st.c */
GAGE_EXPORT int gageStructureTensor(Nrrd *nout, const Nrrd *nin,
                                    int dScale, int iScale, int dsmp);

/* deconvolve.c */
GAGE_EXPORT int gageDeconvolve(Nrrd *nout, double *lastDiffP,
                               const Nrrd *nin, const gageKind *kind,
                               const NrrdKernelSpec *ksp, int typeOut,
                               unsigned int maxIter, int saveAnyway,
                               double step, double epsilon, int verbose);

#ifdef __cplusplus
}
#endif

#endif /* GAGE_HAS_BEEN_INCLUDED */
