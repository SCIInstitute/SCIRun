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

#ifndef PUSH_HAS_BEEN_INCLUDED
#define PUSH_HAS_BEEN_INCLUDED

#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/gage.h>
#include <teem/ten.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(push_EXPORTS) || defined(teem_EXPORTS)
#    define PUSH_EXPORT extern __declspec(dllexport)
#  else
#    define PUSH_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define PUSH_EXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PUSH pushBiffKey
#define PUSH_THREAD_MAXNUM 512

/*
******** pushPoint
**
** information about a point in the simulation.  There are really two
** kinds of information here: "pos", "enr", "frc" pertain to the
** simulation of point dynamics, while "ten", "inv", "cnt", "grav",
** "gravGrad", "seedThresh" are properties of the field sampled at the
** point.
*/
typedef struct pushPoint_t {
  unsigned int ttaagg;
  double pos[3],               /* position in world space */
    enr,                       /* energy accumulator (current iteration) */
    frc[3],                    /* force accumulator (current iteration) */
    ten[7],                    /* tensor here */
    inv[7],                    /* inverse of tensor */
    cnt[3],                    /* mask's containment gradient */
    grav, gravGrad[3],         /* gravity stuff */
    seedThresh;                /* seed thresh */
  /* per-point list of active neighbors- which is updated only periodically.
     In addition to spatial binning, this greatly reduces the number of
     pair-wise interactions computed (based on idea from Meyer et al.) */
  struct pushPoint_t **neigh;
  unsigned int neighNum;
  airArray *neighArr;
} pushPoint;

/*
******** pushBin
**
** the data structure for doing spatial binning.
**
** in tractlet-less push, bins do own the points they contain
*/
typedef struct pushBin_t {
  unsigned int pointNum;       /* # of points in this bin */
  pushPoint **point;           /* dyn. alloc. array of point pointers */
  airArray *pointArr;          /* airArray around point and pointNum */
  struct pushBin_t **neighbor; /* pre-computed NULL-terminated list of all
                                  neighboring bins, including myself */
} pushBin;

/*
******** pushTask
**
** The information specific for a thread.  
*/
typedef struct pushTask_t {
  struct pushContext_t *pctx;  /* parent's context */
  gageContext *gctx;           /* result of gageContextCopy(pctx->gctx) */
  const double *tenAns,        /* results of gage probing */
    *invAns, *cntAns,
    *gravAns, *gravGradAns,
    *seedThreshAns;
  airThread *thread;           /* my thread */
  unsigned int threadIdx,      /* which thread am I */
    pointNum;                  /* # points I let live this iteration */
  double energySum,            /* sum of energies of points I processed */
    deltaFracSum;              /* contribution to pctx->deltaFrac */
  airRandMTState *rng;         /* state for my RNG */
  void *returnPtr;             /* for airThreadJoin */
} pushTask;

/*
******** pushEnergyType* enum
**
** the different shapes of potential energy profiles that can be used
*/
enum {
  pushEnergyTypeUnknown,       /* 0 */
  pushEnergyTypeSpring,        /* 1 */
  pushEnergyTypeGauss,         /* 2 */
  pushEnergyTypeCoulomb,       /* 3 */
  pushEnergyTypeCotan,         /* 4 */
  pushEnergyTypeZero,          /* 5 */
  pushEnergyTypeLast
};
#define PUSH_ENERGY_TYPE_MAX      5
#define PUSH_ENERGY_PARM_NUM 3

/*
******** pushEnergy
**
** the functions which determine inter-point forces
**
** NOTE: the eval() function probably does NOT check to see it was passed
** non-NULL pointers into which to store energy and force
*/
typedef struct {
  char name[AIR_STRLEN_SMALL];
  unsigned int parmNum;
  void (*eval)(double *energy, double *force,
               double dist, const double parm[PUSH_ENERGY_PARM_NUM]);
  double (*support)(const double parm[PUSH_ENERGY_PARM_NUM]);
} pushEnergy;

typedef struct {
  const pushEnergy *energy;
  double parm[PUSH_ENERGY_PARM_NUM];
} pushEnergySpec;

/*
******** pushContext
**
** everything for doing one simulation computation
**
*/
typedef struct pushContext_t {
  /* INPUT ----------------------------- */
  unsigned int pointNum;           /* number points to start simulation w/ */
  Nrrd *nin,                       /* 3D image of 3D masked tensors, though
                                      it may only be a single slice */
    *npos;                         /* positions to start with
                                      (overrides pointNum) */
  double stepInitial,              /* initial time step in integration 
                                      (which will be reduced as the system
                                      converges) */
    scale,                         /* scaling from tensor to glyph size */
    wall,                          /* spring constant of walls */
    cntScl,                        /* magnitude of containment gradient */
    deltaLimit,                    /* speed limit on particles' motion, as a
                                      fraction of glyph radius along
                                      direction of motion */
    deltaFracMin,                  /* lowest value of deltaFrac (see below)
                                      that is allowed without decreasing 
                                      step size */
    energyStepFrac,                /* when energy goes up instead of down, the
                                      fraction by which to scale step size */
    deltaFracStepFrac,             /* when deltaFrac goes below deltaFracMin,
                                      fraction by which to scale step size */
    neighborTrueProb,              /* probability that we find the true
                                      neighbors of the particle, as opposed to
                                      using a cached list */
    probeProb,                     /* probability that we gageProbe() to find
                                      the local tensor value, instead of
                                      re-using last value */
    energyImprovMin;               /* convergence threshold: stop when
                                      fracional improvement (decrease) in
                                      energy dips below this */
  int detReject,                   /* determinant-based rejection at init */
    midPntSmp,                     /* sample midpoint btw part.s for physics */
    verbose;                       /* blah blah blah */
  unsigned int seedRNG,            /* seed value for random number generator */
    threadNum,                     /* number of threads to use */
    maxIter,                       /* if non-zero, max number of iterations */
    snap;                          /* if non-zero, interval between iterations
                                      at which output snapshots are saved */
  int gravItem,                    /* tenGage item (scalar) for "height"
                                      potential energy associated w/ gravity */
    gravGradItem;                  /* tenGage item (vector) for gravity */
  double gravScl,                  /* sign and magnitude of gravity's effect:
                                      when this is positive, higher values of
                                      gravItem have higher potential energy */
    gravZero;                      /* the height that corresponds to zero 
                                      potential energy from gravity */
  
  int seedThreshItem,              /* item for constraining random seeding */
    seedThreshSign;                /* +1: need val > thresh; -1: opposite */
  double seedThresh;               /* threshold for seed constraint */

  pushEnergySpec *ensp;            /* potential energy function to use */

  int binSingle;                   /* disable binning (for debugging) */
  unsigned int binIncr;            /* increment for per-bin airArray */

  NrrdKernelSpec *ksp00,           /* for sampling tensor field */
    *ksp11,                        /* for gradient of mask, other 1st derivs */
    *ksp22;                        /* for 2nd derivatives */

  /* INTERNAL -------------------------- */

  unsigned int ttaagg;             /* next value for per-point ID */
  Nrrd *nten,                      /* 3D image of 3D masked tensors */
    *ninv,                         /* pre-computed inverse of nten */
    *nmask;                        /* mask image from nten */
  gageContext *gctx;               /* gage context around nten, ninv, nmask */
  gagePerVolume *tpvl, *ipvl;      /* gage pervolumes around nten and ninv */
  int finished;                    /* used to signal all threads to return */
  unsigned int dimIn,              /* dim (2 or 3) of input, meaning whether
                                      it was a single slice or a full volume */
    sliceAxis;                     /* got a single 3-D slice, which axis had
                                      only a single sample */

  pushBin *bin;                    /* volume of bins (see binsEdge, binNum) */
  unsigned int binsEdge[3],        /* # bins along each volume edge,
                                      determined by maxEval and scale */
    binNum,                        /* total # bins in grid */
    binIdx;                        /* *next* bin of points needing to be
                                      processed.  Stage is done when
                                      binIdx == binNum */
  airThreadMutex *binMutex;        /* mutex around bin */

  double step,                     /* current working step size */
    maxDist,                       /* max distance btween interacting points */
    maxEval, meanEval,             /* max and mean principal eval in field */
    maxDet,
    energySum;                     /* potential energy of entire particles */
  pushTask **task;                 /* dynamically allocated array of tasks */
  airThreadBarrier *iterBarrierA;  /* barriers between iterations */
  airThreadBarrier *iterBarrierB;  /* barriers between iterations */
  double deltaFrac;                /* mean (over all particles in last 
                                      iteration) of fraction of distance 
                                      actually travelled to distance that it
                                      wanted to travel (due to speed limit) */

  /* OUTPUT ---------------------------- */

  double timeIteration,            /* time needed for last (single) iter */
    timeRun;                       /* total time spent in computation */
  unsigned int iter;               /* how many iterations were needed */
  Nrrd *noutPos,                   /* list of 2D or 3D positions */
    *noutTen;                      /* list of 2D or 3D masked tensors */
} pushContext;

/* defaultsPush.c */
PUSH_EXPORT const char *pushBiffKey;

/* methodsPush.c */
PUSH_EXPORT pushPoint *pushPointNew(pushContext *pctx);
PUSH_EXPORT pushPoint *pushPointNix(pushPoint *pnt);
PUSH_EXPORT pushContext *pushContextNew(void);
PUSH_EXPORT pushContext *pushContextNix(pushContext *pctx);

/* forces.c (legacy name for info about (derivatives of) energy functions) */
PUSH_EXPORT airEnum *pushEnergyType;
PUSH_EXPORT const pushEnergy *const pushEnergyUnknown;
PUSH_EXPORT const pushEnergy *const pushEnergySpring;
PUSH_EXPORT const pushEnergy *const pushEnergyGauss;
PUSH_EXPORT const pushEnergy *const pushEnergyCoulomb;
PUSH_EXPORT const pushEnergy *const pushEnergyCotan;
PUSH_EXPORT const pushEnergy *const pushEnergyZero;
PUSH_EXPORT const pushEnergy *const pushEnergyAll[PUSH_ENERGY_TYPE_MAX+1];
PUSH_EXPORT pushEnergySpec *pushEnergySpecNew();
PUSH_EXPORT void pushEnergySpecSet(pushEnergySpec *ensp,
                                   const pushEnergy *energy,
                                   const double parm[PUSH_ENERGY_PARM_NUM]);
PUSH_EXPORT pushEnergySpec *pushEnergySpecNix(pushEnergySpec *ensp);
PUSH_EXPORT int pushEnergySpecParse(pushEnergySpec *ensp, const char *str);
PUSH_EXPORT hestCB *pushHestEnergySpec;

/* corePush.c */
PUSH_EXPORT int pushStart(pushContext *pctx);
PUSH_EXPORT int pushIterate(pushContext *pctx);
PUSH_EXPORT int pushRun(pushContext *pctx);
PUSH_EXPORT int pushFinish(pushContext *pctx);

/* binning.c */
PUSH_EXPORT void pushBinInit(pushBin *bin, unsigned int incr);
PUSH_EXPORT void pushBinDone(pushBin *bin);
PUSH_EXPORT int pushBinPointAdd(pushContext *pctx, pushPoint *point);
PUSH_EXPORT void pushBinAllNeighborSet(pushContext *pctx);
PUSH_EXPORT int pushRebin(pushContext *pctx);

/* action.c */
PUSH_EXPORT int pushBinProcess(pushTask *task, unsigned int myBinIdx);
PUSH_EXPORT int pushOutputGet(Nrrd *nPos, Nrrd *nTen, Nrrd *nEnr,
                              pushContext *pctx);

#ifdef __cplusplus
}
#endif

#endif /* PUSH_HAS_BEEN_INCLUDED */

