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

#ifndef PULL_HAS_BEEN_INCLUDED
#define PULL_HAS_BEEN_INCLUDED

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/gage.h>
#include <teem/limn.h>
#include <teem/ten.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(pull_EXPORTS) || defined(teem_EXPORTS)
#    define PULL_EXPORT extern __declspec(dllexport)
#  else
#    define PULL_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define PULL_EXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PULL pullBiffKey
#define PULL_THREAD_MAXNUM 512
#define PULL_VOLUME_MAXNUM 4
#define PULL_POINT_NEIGH_INCR 16

#define PULL_PHIST 0

/*
******** pullInfo enum
**
** all the things that might be learned via gage from some kind, that
** can be used to control particle dynamics.
**
** There are multiple scalars (and associated) derivatives that can
** be used for dynamics:
** - Inside: just for nudging things to stay inside a mask
** - Height: value for computer-vision-y features of ridges, valleys,
**   and edges.  Setting pullInfoHeight as a constraint does valley
**   sampling (flip the sign to get ridges), based on Tangent1, Tangent2,
**   and TangentMode.  Setting pullInfoHeightLaplacian as a constraint
*    does zero-crossing edge detection.
** - Isovalue: just for implicit surfaces f=0
** - Strength: some measure of feature strength, with the assumption
**   that it can't be analytically differentiated in space or scale.
*/
enum {
  pullInfoUnknown,            /*  0 */
  pullInfoTensor,             /*  1: [7] tensor here */
  pullInfoTensorInverse,      /*  2: [7] inverse of tensor here */
  pullInfoHessian,            /*  3: [9] hessian used for force distortion */
  pullInfoInside,             /*  4: [1] containment scalar */
  pullInfoInsideGradient,     /*  5: [3] containment vector */
  pullInfoHeight,             /*  6: [1] for gravity, and edge and crease 
                                         feature detection */
  pullInfoHeightGradient,     /*  7: [3] */
  pullInfoHeightHessian,      /*  8: [9] */
  pullInfoHeightLaplacian,    /*  9: [1] for zero-crossing edge detection */
  pullInfoSeedThresh,         /* 10: [1] scalar for thresholding seeding */
  pullInfoTangent1,           /* 11: [3] first tangent to constraint surf */
  pullInfoTangent2,           /* 12: [3] second tangent to constraint surf */
  pullInfoTangentMode,        /* 13: [1] for morphing between co-dim 1 and 2;
                                 User must set scale so mode from -1 to 1
                                 means co-dim 1 (surface) to 2 (line) */
  pullInfoIsovalue,           /* 14: [1] for isosurface extraction */
  pullInfoIsovalueGradient,   /* 15: [3] */
  pullInfoIsovalueHessian,    /* 16: [9] */
  pullInfoStrength,           /* 17: [1] */
  pullInfoLast
};
#define PULL_INFO_MAX            17

/*
** the various properties of particles in the system 
**
** consider adding: dot between normalized directions of force and movmt 
*/
enum {
  pullPropUnknown,            /*  0: nobody knows */
  pullPropIdtag,              /*  1: [1] idtag (unsigned int) */
  pullPropEnergy,             /*  2: [1] energy from last iteration */
  pullPropStepEnergy,         /*  3: [1] step size for minimizing energy */
  pullPropStepConstr,         /*  4: [1] step size for constraint satis. */
  pullPropStuck,              /*  5: [1] (0 or 1) got stuck last iter */
  pullPropPosition,           /*  6: [4] position */
  pullPropForce,              /*  7: [4] force accumulation */
  pullPropNeighDist,          /*  8: [1] "mean distance" to neighbors */
  pullPropLast
};

/*
** the components of a point's status that are set as a bitflag 
** in point->status
*/
enum {
  pullStatusUnknown,             /* 0: nobody knows */
  pullStatusStuck,               /* 1: couldn't move to decrease energy */
#define PULL_STATUS_STUCK_BIT  (1<< 1)
  pullStatusNew,                 /* 2: just added to system, bin me */
#define PULL_STATUS_NEW_BIT    (1<< 2)
  pullStatusNixMe,               /* 3: nix me at the end of this iter */
#define PULL_STATUS_NIXME_BIT  (1<< 3)
  pullStatusLast
};

/*
** the conditions under which a point may find itself at some position 
*/
enum {
  pullCondUnknown,            /* 0 */
  pullCondOld,                /* 1 */
  pullCondConstraintSatA,     /* 2 */
  pullCondConstraintSatB,     /* 3 */
  pullCondEnergyTry,          /* 4 */
  pullCondConstraintFail,     /* 5 */
  pullCondEnergyBad,          /* 6 */
  pullCondNew,                /* 7 */
  pullCondLast
};

/* 
** how the gageItem for a pullInfo is specified
*/
typedef struct pullInfoSpec_t {
  /* ------ INPUT ------ */
  int info;                     /* from the pullInfo* enum */
  char *volName;                /* volume name */
  int item;                     /* which item */
  double scale,                 /* scaling factor (including sign) */
    zero;                       /* for height and inside: where is zero,
                                   for seedThresh, threshold value */
  int constraint;               /* (for scalar items) minimizing this
                                   is a constraint to enforce per-point
                                   per-iteration, not merely a contribution 
                                   to the point's energy */
  /* ------ INTERNAL ------ */
  unsigned int volIdx;          /* which volume */
} pullInfoSpec;

/*
******** pullPoint
**
*/
typedef struct pullPoint_t {
  unsigned int idtag;         /* unique point ID */
  struct pullPoint_t **neighPoint; /* list of neighboring points */
  unsigned int neighNum;
  airArray *neighArr;         /* airArray around neigh and neigNum
                                 (no callbacks used here) */
  double neighDist, neighMode;
  unsigned int neighInterNum;
#if PULL_PHIST
  double *phist;              /* history of positions tried in the last iter,
                                 in sets of 5 doubles: (x,y,z,t,info) */
  unsigned int phistNum;      /* number of positions stored */
  airArray *phistArr;         /* airArray around phist */
#endif
  unsigned int status;        /* bit-flag of status info, though right now
                                 its just a boolean for having gotten stuck */
  double pos[4],              /* position in space and scale */
    energy,                   /* energy accumulator for this iteration */
    force[4],                 /* force accumulator for this iteration */
    stepEnergy,               /* step size for energy minimization */
    stepConstr,               /* step size for constraint satisfaction */
    info[1];                  /* all information learned from gage that matters
                                 for particle dynamics.  This is sneakily
                                 allocated for *more*, depending on needs,
                                 so this MUST be last field */
} pullPoint;

/*
******** pullBin
**
** the data structure for doing spatial binning.
*/
typedef struct pullBin_t {
  pullPoint **point;         /* dyn. alloc. array of point pointers */
  unsigned int pointNum;     /* # of points in this bin */
  airArray *pointArr;        /* airArray around point and pointNum 
                                (no callbacks used here) */
  struct pullBin_t **neighBin;  /* pre-computed NULL-terminated list of all
                                neighboring bins, including myself */
} pullBin;

/*
******** pullEnergyType* enum
**
** the different shapes of potential energy profiles that can be used
*/
enum {
  pullEnergyTypeUnknown,       /* 0 */
  pullEnergyTypeSpring,        /* 1 */
  pullEnergyTypeGauss,         /* 2 */
  pullEnergyTypeCotan,         /* 3 */
  pullEnergyTypeQuartic,       /* 4 */
  pullEnergyTypeZero,          /* 5 */
  pullEnergyTypeLast
};
#define PULL_ENERGY_TYPE_MAX      5
#define PULL_ENERGY_PARM_NUM 3

/*
******** pullEnergy
**
** the functions which determine inter-point forces
**
** NOTE: the eval() function probably does NOT check to see it was passed
** non-NULL pointers into which to store energy and force
**
** Thu Apr 10 12:40:08 EDT 2008: nixed the "support" function, since it
** was annoying to deal with variable support potentials.  Now everything
** cuts off at dist=1.  You can still use the parm vector to change the
** shape inside the support.
*/
typedef struct {
  char name[AIR_STRLEN_SMALL];
  unsigned int parmNum;
  double (*eval)(double *force, double dist,
                 const double parm[PULL_ENERGY_PARM_NUM]);
} pullEnergy;

typedef struct {
  const pullEnergy *energy;
  double parm[PULL_ENERGY_PARM_NUM];
} pullEnergySpec;

/*
** In the interests of simplicity (rather than avoiding redundancy), 
** this is going to copied per-task, which is why it contains the gageContext
** The idea is that the first of these is somehow set up by the user
** or something, and the rest of them are created within pull per-task.
*/
typedef struct {
  char *name;                  /* how the volume will be identified
                                  (like its a variable name) */
  const gageKind *kind;
  const Nrrd *ninSingle;       /* don't own */
  const Nrrd *const *ninScale; /* don't own;
                                  NOTE: only one of ninSingle and ninScale
                                  can be non-NULL */
  unsigned int scaleNum;       /* number of scale-space samples (volumes) */
  double *scalePos;            /* location of all samples in scale */
  NrrdKernelSpec *ksp00,       /* for sampling tensor field */
    *ksp11,                    /* for gradient of mask, other 1st derivs */
    *ksp22,                    /* for 2nd derivatives */
    *kspSS;                    /* for reconstructing from scale-space
                                  samples */
  gageContext *gctx;           /* do own, and set based on info here */
  gagePerVolume *gpvl;         /* stupid gage API ... */
  int seedOnly;                /* volume only required for seeding */
} pullVolume;

/*
******** pullTask
**
** The information specific for a thread.  
*/
typedef struct pullTask_t {
  struct pullContext_t
    *pctx;                      /* parent's context; not const because the
                                   tasks assign themselves bins to do work */
  pullVolume
    *vol[PULL_VOLUME_MAXNUM];   /* volumes copied from parent */
  const double
    *ans[PULL_INFO_MAX+1];      /* answer *pointers* for all possible infos,
                                   pointing into per-task per-volume gctxs,
                                   or: NULL if that info is not being used */
  airThread *thread;            /* my thread */
  unsigned int threadIdx;       /* which thread am I */
  airRandMTState *rng;          /* state for my RNG */
  pullPoint *pointBuffer,       /* place for copying point into during 
                                   strength ascent computation; can't be
                                   statically allocated because pullPoint
                                   size is known only at run-time */
    **neighPoint;               /* array of point pointers, either all
                                   possible points from neighbor bins, or
                                   last learned interacting neighbors */
  pullPoint *addPoint;          /* points to add before next iter */
  airArray *addPointArr;        /* airArray around addPoint */
  void *returnPtr;              /* for airThreadJoin */
  unsigned int stuckNum;        /* # stuck particles seen by this task */
} pullTask;

/*
******** pullContext
**
** everything for doing one computation
**
** NOTE: right now there is no API for setting the input fields (as there
** is in gage and tenFiber) eventually there will be...
*/
typedef struct pullContext_t {
  /* INPUT ----------------------------- */
  int verbose;                     /* blah blah blah */
  unsigned int pointNumInitial;    /* number points to start simulation w/ */
  Nrrd *npos;                      /* positions (4xN array) to start with
                                      (overrides pointNumInitial) */
  pullVolume 
    *vol[PULL_VOLUME_MAXNUM];      /* the volumes we analyze (we DO OWN) */
  unsigned int volNum;             /* actual length of vol[] used */

  pullInfoSpec
    *ispec[PULL_INFO_MAX+1];       /* info ii is in effect if ispec[ii] is
                                      non-NULL (and we DO OWN ispec[ii]) */
  
  double stepInitial,              /* initial (time) step for dynamics */
    radiusSpace,                   /* radius/scaling of inter-particle
                                      interactions in the spatial domain */
    radiusScale,                   /* radius/scaling of inter-particle
                                      interactions in the scale domain */

  /* concerning the probability-based optimizations */
    neighborTrueProb,              /* probability that we find the true
                                      neighbors of the particle, as opposed to
                                      using a cached list */
    probeProb,                     /* probability that we do image probing
                                      to find out what's really going on */

  /* how the (per-point) time-step is adaptively varied to reach convergence:
     moveFrac is computed for each particle as the fraction of distance
     actually travelled, to the distance that it wanted to travel, but 
     couldn't due to the moveLimit */
    opporStepScale,                /* (>= 1.0) how much to opportunistically
                                      scale  up step size (for energy
                                      minimization) with every iteration */
    stepScale,                     /* (< 1.0) when energy goes up instead of
                                      down, or when constraint satisfaction
                                      seems to be going the wrong way, how to
                                      scale (down) step size */
    energyImprovMin,               /* convergence threshold: stop when
                                      fractional improvement (decrease) in
                                      total system energy dips below this */
    constraintStepMin,             /* convergence threshold for constraint
                                      satisfaction: finished if stepsize goes
                                      below this times constraintVoxelSize */
    wall;                          /* spring constant on bbox wall */

  unsigned int 
    pointPerVoxel,                 /* number of initial points per voxel, in
                                      seed thresh volume. If 0, then use old
                                      behavior of just finding pointNumInitial
                                      (see above) seedpoint locations randomly.
                                      If non-0 (over-riding pointNumInitial),
                                      then jitter seedPerVox seed points in
                                      every sample of the seed threshold
                                      volume */
    rngSeed,                       /* seed value for random number generator,
                                      NOT directly related to seed point
                                      placement*/
    threadNum,                     /* number of threads to use */
    iterMax,                       /* if non-zero, max number of iterations
                                      for whole system */
    constraintIterMax,             /* if non-zero, max number of iterations
                                      for enforcing each constraint */
    snap;                          /* if non-zero, interval between iterations
                                      at which output snapshots are saved */
  
  pullEnergySpec *energySpec;      /* starting point for radial potential
                                      energy function, phi(r) */
  double alpha,                    /* alpha = 0: only particle-image, 
                                      alpha = 1: only inter-particle */
    beta,                          /* tuning parameter for amount of 
                                      scale-space attraction */
    jitter;                        /* when using pointPerVoxel, how much to
                                      jitter the samples within the voxel;
                                      0: no jitter, 1: full jitter */
  int radiusSingle;                /* if non-zero, combine scale-space 
                                      offset into a single radius, else
                                      use beta for Phi_{x-G}(r,s) */
  int binSingle;                   /* disable binning (for debugging) */
  unsigned int binIncr;            /* increment for per-bin airArray */

  /* INTERNAL -------------------------- */

  double bboxMin[4], bboxMax[4];   /* scale-space bounding box of all volumes:
                                      region over which binning is defined.
                                      In 3-D space, the bbox is axis aligned,
                                      even when the volume is not so aligned,
                                      which means that some bins might be
                                      under- or un- utilized, oh well. 
                                      bboxMin[3] and bboxMax[3] are the 
                                      bounds of the volume in *scale* (sigma),
                                      not t, or tau */
  unsigned int infoTotalLen,       /* total length of the info buffers needed,
                                      which determines size of allocated
                                      binPoint */
    infoIdx[PULL_INFO_MAX+1];      /* index of answer within pullPoint->info */
  unsigned int idtagNext;          /* next per-point igtag value */
  int haveScale,                   /* non-zero iff one of the volumes is in
                                      scale-space */
    constraint,                    /* if non-zero, we have a constraint to
                                      satisfy, and this is its info number  */
    finished;                      /* used to signal all threads to return */
  double maxDist,                  /* max dist of point-point interaction */
    constraintVoxelSize;           /* if there's a constraint, mean voxel edge
                                      length, to use for limiting distance 
                                      to travel for constraint satisfaction */
  pullBin *bin;                    /* volume of bins (see binsEdge, binNum) */
  unsigned int binsEdge[3],        /* # bins along each volume edge,
                                      determined by maxEval and scale */
    binNum,                        /* total # bins in grid */
    binNextIdx;                    /* next bin of points to be processed,
                                      we're done when binNextIdx == binNum */
  unsigned int *pointPerm;
  pullPoint **pointBuff;
  airThreadMutex *binMutex;        /* mutex around bin, needed because bins
                                      are the unit of work for the tasks */

  pullTask **task;                 /* dynamically allocated array of tasks */
  airThreadBarrier *iterBarrierA;  /* barriers between iterations */
  airThreadBarrier *iterBarrierB;  /* barriers between iterations */

  /* OUTPUT ---------------------------- */

  double timeIteration,            /* time needed for last (single) iter */
    timeRun,                       /* total time spent in pullRun() */
    energy;                        /* final energy of system */
  unsigned int stuckNum,           /* # stuck particles in last iter */
    iter;                          /* how many iterations were needed */
  Nrrd *noutPos;                   /* list of 4D positions */
} pullContext;

/* defaultsPull.c */
PULL_EXPORT int pullPhistEnabled;
PULL_EXPORT const char *pullBiffKey;

/* energy.c */
PULL_EXPORT airEnum *pullEnergyType;
PULL_EXPORT const pullEnergy *const pullEnergyUnknown;
PULL_EXPORT const pullEnergy *const pullEnergySpring;
PULL_EXPORT const pullEnergy *const pullEnergyGauss;
PULL_EXPORT const pullEnergy *const pullEnergyCoulomb;
PULL_EXPORT const pullEnergy *const pullEnergyCotan;
PULL_EXPORT const pullEnergy *const pullEnergyZero;
PULL_EXPORT const pullEnergy *const pullEnergyAll[PULL_ENERGY_TYPE_MAX+1];
PULL_EXPORT pullEnergySpec *pullEnergySpecNew();
PULL_EXPORT int pullEnergySpecSet(pullEnergySpec *ensp,
                                  const pullEnergy *energy,
                                  const double parm[PULL_ENERGY_PARM_NUM]);
PULL_EXPORT pullEnergySpec *pullEnergySpecNix(pullEnergySpec *ensp);
PULL_EXPORT int pullEnergySpecParse(pullEnergySpec *ensp, const char *str);
PULL_EXPORT hestCB *pullHestEnergySpec;

/* volumePull.c */
PULL_EXPORT gageKind *pullGageKindParse(const char *str);
PULL_EXPORT pullVolume *pullVolumeNew();
PULL_EXPORT pullVolume *pullVolumeNix(pullVolume *vol);
PULL_EXPORT int pullVolumeSingleAdd(pullContext *pctx, 
                                    char *name, const Nrrd *nin,
                                    const gageKind *kind, 
                                    const NrrdKernelSpec *ksp00,
                                    const NrrdKernelSpec *ksp11,
                                    const NrrdKernelSpec *ksp22);
PULL_EXPORT int pullVolumeStackAdd(pullContext *pctx,
                                   char *name, const Nrrd *const *nin,
                                   double *scale,
                                   unsigned int ninNum,
                                   const gageKind *kind, 
                                   const NrrdKernelSpec *ksp00,
                                   const NrrdKernelSpec *ksp11,
                                   const NrrdKernelSpec *ksp22,
                                   const NrrdKernelSpec *kspSS);

/* infoPull.c */
PULL_EXPORT airEnum *const pullInfo;
PULL_EXPORT unsigned int pullInfoAnswerLen(int info);
PULL_EXPORT pullInfoSpec *pullInfoSpecNew();
PULL_EXPORT pullInfoSpec *pullInfoSpecNix(pullInfoSpec *ispec);
PULL_EXPORT int pullInfoSpecAdd(pullContext *pctx, pullInfoSpec *ispec,
                                int info, const char *volName, int item);

/* contextPull.c */
PULL_EXPORT pullContext *pullContextNew(void);
PULL_EXPORT pullContext *pullContextNix(pullContext *pctx);
PULL_EXPORT int pullOutputGet(Nrrd *nPosOut, Nrrd *nTenOut,
                              pullContext *pctx);
PULL_EXPORT int pullPositionHistoryGet(limnPolyData *pld, pullContext *pctx);
PULL_EXPORT int pullPropGet(Nrrd *nprop, int prop, pullContext *pctx);

/* pointPull.c */
PULL_EXPORT pullPoint *pullPointNew(pullContext *pctx);
PULL_EXPORT pullPoint *pullPointNix(pullPoint *pnt);

/* binningPull.c */
PULL_EXPORT int pullBinsPointAdd(pullContext *pctx, pullPoint *point);
PULL_EXPORT void pullBinsNeighborSet(pullContext *pctx);
PULL_EXPORT int pullRebin(pullContext *pctx);

/* actionPull.c */
PULL_EXPORT int pullBinProcess(pullTask *task, unsigned int myBinIdx);

/* corePull.c */
PULL_EXPORT int pullStart(pullContext *pctx);
PULL_EXPORT int pullRun(pullContext *pctx);
PULL_EXPORT int pullFinish(pullContext *pctx);

#ifdef __cplusplus
}
#endif

#endif /* PULL_HAS_BEEN_INCLUDED */

