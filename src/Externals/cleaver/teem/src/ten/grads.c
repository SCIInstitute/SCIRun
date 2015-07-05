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

#include "ten.h"
#include "privateTen.h"

tenGradientParm *
tenGradientParmNew(void) {
  tenGradientParm *ret;

  ret = (tenGradientParm *)calloc(1, sizeof(tenGradientParm));
  if (ret) {
    ret->step = 1;
    ret->jitter = 0.2;
    ret->minVelocity = 0.000000001;
    ret->minPotentialChange = 0.000000001;
    ret->minMean = 0.0001;
    ret->minMeanImprovement = 0.00001;
    ret->single = AIR_FALSE;
    ret->snap = 0;
    ret->report = 400;
    ret->expo = 1;
    ret->expo_d = 0;
    ret->seed = 42;
    ret->maxEdgeShrink = 20;
    ret->minIteration = 0;
    ret->maxIteration = 1000000;
    ret->step = 0;
    ret->nudge = 0;
    ret->itersUsed = 0;
    ret->potential = 0;
    ret->potentialNorm = 0;
    ret->angle = 0;
    ret->edge = 0;
  }
  return ret;
}

tenGradientParm *
tenGradientParmNix(tenGradientParm *tgparm) {
  
  airFree(tgparm);
  return NULL;
}

int
tenGradientCheck(const Nrrd *ngrad, int type, unsigned int minnum) {
  char me[]="tenGradientCheck", err[BIFF_STRLEN];
  
  if (nrrdCheck(ngrad)) {
    sprintf(err, "%s: basic validity check failed", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (!( 3 == ngrad->axis[0].size && 2 == ngrad->dim )) {
    sprintf(err, "%s: need a 3xN 2-D array (not a " _AIR_SIZE_T_CNV 
            "x? %u-D array)",
            me, ngrad->axis[0].size, ngrad->dim);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdTypeDefault != type && type != ngrad->type) {
    sprintf(err, "%s: requested type %s but got type %s", me,
            airEnumStr(nrrdType, type), airEnumStr(nrrdType, ngrad->type));
    biffAdd(TEN, err); return 1;
  }
  if (nrrdTypeBlock == ngrad->type) {
    sprintf(err, "%s: sorry, can't use %s type", me, 
            airEnumStr(nrrdType, nrrdTypeBlock));
    biffAdd(TEN, err); return 1;
  }
  if (!( minnum <= ngrad->axis[1].size )) {
    sprintf(err, "%s: have only " _AIR_SIZE_T_CNV " gradients, "
            "need at least %d",
            me, ngrad->axis[1].size, minnum);
    biffAdd(TEN, err); return 1;
  }

  return 0;
}

/*
******** tenGradientRandom
**
** generates num random unit vectors of type double
*/
int
tenGradientRandom(Nrrd *ngrad, unsigned int num, unsigned int seed) {
  char me[]="tenGradientRandom", err[BIFF_STRLEN];
  double *grad, len;
  unsigned int gi;
  
  if (nrrdMaybeAlloc_va(ngrad, nrrdTypeDouble, 2,
                        AIR_CAST(size_t, 3), AIR_CAST(size_t, num))) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  airSrandMT(seed);
  grad = AIR_CAST(double*, ngrad->data);
  for (gi=0; gi<num; gi++) {
    do {
      grad[0] = AIR_AFFINE(0, airDrandMT(), 1, -1, 1);
      grad[1] = AIR_AFFINE(0, airDrandMT(), 1, -1, 1);
      grad[2] = AIR_AFFINE(0, airDrandMT(), 1, -1, 1);
      len = ELL_3V_LEN(grad);
    } while (len > 1 || !len);
    ELL_3V_SCALE(grad, 1.0/len, grad);
    grad += 3;
  }
  return 0;
}

/*
******** tenGradientIdealEdge
**
** edge length of delauney triangulation of idealized distribution of
** N gradients (2*N points), but also allowing a boolean "single" flag
** saying that we actually care about N points
*/
double
tenGradientIdealEdge(unsigned int N, int single) {

  return sqrt((!single ? 4 : 8)*AIR_PI/(sqrt(3)*N));
}

/*
******** tenGradientJitter
**
** moves all gradients by amount dist on tangent plane, in a random
** direction, and then renormalizes. The distance is a fraction
** of the ideal edge length (via tenGradientIdealEdge)
*/
int
tenGradientJitter(Nrrd *nout, const Nrrd *nin, double dist) {
  char me[]="tenGradientJitter", err[BIFF_STRLEN];
  double *grad, perp0[3], perp1[3], len, theta, cc, ss, edge;
  unsigned int gi, num;

  if (nrrdConvert(nout, nin, nrrdTypeDouble)) {
    sprintf(err, "%s: trouble converting input to double", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (tenGradientCheck(nout, nrrdTypeDouble, 3)) {
    sprintf(err, "%s: didn't get valid gradients", me);
    biffAdd(TEN, err); return 1;
  }
  grad = AIR_CAST(double*, nout->data);
  num = nout->axis[1].size;
  /* HEY: possible confusion between single and not */
  edge = tenGradientIdealEdge(num, AIR_FALSE);
  for (gi=0; gi<num; gi++) {
    ELL_3V_NORM(grad, grad, len);
    ell_3v_perp_d(perp0, grad);
    ELL_3V_CROSS(perp1, perp0, grad);
    theta = AIR_AFFINE(0, airDrandMT(), 1, 0, 2*AIR_PI);
    cc = dist*edge*cos(theta);
    ss = dist*edge*sin(theta);
    ELL_3V_SCALE_ADD3(grad, 1.0, grad, cc, perp0, ss, perp1);
    ELL_3V_NORM(grad, grad, len);
    grad += 3;
  }
  
  return 0;
}

void
tenGradientMeasure(double *pot, double *minAngle, double *minEdge,
                    const Nrrd *npos, tenGradientParm *tgparm,
                    int edgeNormalize) {
  /* char me[]="tenGradientMeasure"; */
  double diff[3], *pos, atmp=0, ptmp, edge, len;
  int ii, jj, num;

  /* allow minAngle NULL */
  if (!(pot && npos && tgparm )) {
    return;
  }

  num = npos->axis[1].size;
  pos = AIR_CAST(double *, npos->data);
  edge = (edgeNormalize
          ? tenGradientIdealEdge(num, tgparm->single)
          : 1.0);
  *pot = 0;
  if (minAngle) {
    *minAngle = AIR_PI;
  }
  if (minEdge) {
    *minEdge = 2;
  }
  for (ii=0; ii<num; ii++) {
    for (jj=0; jj<ii; jj++) {
      ELL_3V_SUB(diff, pos + 3*ii, pos + 3*jj);
      len = ELL_3V_LEN(diff);
      if (minEdge) {
        *minEdge = AIR_MIN(*minEdge, len);
      }
      if (tgparm->expo) {
        ptmp = airIntPow(edge/len, tgparm->expo);
      } else {
        ptmp = pow(edge/len, tgparm->expo_d);
      }
      *pot += ptmp;
      if (minAngle) {
        atmp = ell_3v_angle_d(pos + 3*ii, pos + 3*jj);
        *minAngle = AIR_MIN(atmp, *minAngle);
      }
      if (!tgparm->single) {
        *pot += ptmp;
        ELL_3V_ADD2(diff, pos + 3*ii, pos + 3*jj);
        len = ELL_3V_LEN(diff);
        if (minEdge) {
          *minEdge = AIR_MIN(*minEdge, len);
        }
        if (tgparm->expo) {
          *pot += 2*airIntPow(edge/len, tgparm->expo);
        } else {
          *pot += 2*pow(edge/len, tgparm->expo_d);
        }
        if (minAngle) {
          *minAngle = AIR_MIN(AIR_PI-atmp, *minAngle);
        }
      }
    }
  }
  return;
}

/*
** Do asynchronous update of positions in "npos', based on force
** calculations wherein the distances are normalized "edge".  Using a
** small "edge" allows forces to either underflow to zero, or be
** finite, instead of exploding to infinity, for high exponents.
**
** The smallest seen edge length is recorded in "*edgeMin", which is
** initialized to the given "edge".  This allows, for example, the
** caller to try again with a smaller edge normalization.
**
** The mean velocity of the points through the update is recorded in
** "*meanVel".
**
** Based on the observation that when using large exponents, numerical
** difficulties arise from the (force-based) update of the positions
** of the two (or few) closest particles, this function puts a speed
** limit (variable "limit") on the distance a particle may move during
** update, expressed as a fraction of the normalizing edge length.
** "limit" has been set heuristically, according to the exponent (we
** have to clamp speeds more aggresively with higher exponents), as
** well as (even more heuristically) according to the number of times
** the step size has been decreased.  This latter factor has to be
** bounded, so that the update is not unnecessarily bounded when the
** step size gets very small at the last stages of computation.
** Without the step-size-based speed limit, the step size would
** sometimes (e.g. num=200, expo=300) have to reduced to a miniscule
** value, which slows subsequent convergence terribly.
** 
** this function is not static, though it could be, so that mac's
** "Sampler" app can profile this
*/
int
_tenGradientUpdate(double *meanVel, double *edgeMin,
                   Nrrd *npos, double edge, tenGradientParm *tgparm) {
  /* char me[]="_tenGradientUpdate"; */
  double *pos, newpos[3], grad[3], ngrad[3],
    dir[3], len, rep, step, diff[3], limit, expo;
  int num, ii, jj, E;

  E = 0;
  pos = AIR_CAST(double *, npos->data);
  num = npos->axis[1].size;
  *meanVel = 0;
  *edgeMin = edge;
  expo = tgparm->expo ? tgparm->expo : tgparm->expo_d;
  limit = expo*AIR_MIN(sqrt(expo),
                       log(1 + tgparm->initStep/tgparm->step));
  for (ii=0; ii<num; ii++) {
    ELL_3V_SET(grad, 0, 0, 0);
    for (jj=0; jj<num; jj++) {
      if (ii == jj) {
        continue;
      }
      ELL_3V_SUB(dir, pos + 3*ii, pos + 3*jj);
      ELL_3V_NORM(dir, dir, len);
      *edgeMin = AIR_MIN(*edgeMin, len);
      if (tgparm->expo) {
        rep = airIntPow(edge/len, tgparm->expo+1);
      } else {
        rep = pow(edge/len, tgparm->expo_d+1);
      }
      ELL_3V_SCALE_INCR(grad, rep/num, dir);
      if (!tgparm->single) {
        ELL_3V_ADD2(dir, pos + 3*ii, pos + 3*jj);
        ELL_3V_NORM(dir, dir, len);
        *edgeMin = AIR_MIN(*edgeMin, len);
        if (tgparm->expo) {
          rep = airIntPow(edge/len, tgparm->expo+1);
        } else {
          rep = pow(edge/len, tgparm->expo_d+1);
        }
        ELL_3V_SCALE_INCR(grad, rep/num, dir);
      }
    }
    ELL_3V_NORM(ngrad, grad, len);
    if (!( AIR_EXISTS(len) )) {
      /* things blew up, either in incremental force
         additions, or in the attempt at normalization */
      E = 1;
      *meanVel = AIR_NAN;
      break;
    }
    if (0 == len) {
      /* if the length of grad[] underflowed to zero, we can
         legitimately zero out ngrad[] */
      ELL_3V_SET(ngrad, 0, 0, 0);
    }
    step = AIR_MIN(len*tgparm->step, edge/limit);
    ELL_3V_SCALE_ADD2(newpos,
                      1.0, pos + 3*ii,
                      step, ngrad);
    ELL_3V_NORM(newpos, newpos, len);
    ELL_3V_SUB(diff, pos + 3*ii, newpos);
    *meanVel += ELL_3V_LEN(diff);
    ELL_3V_COPY(pos + 3*ii, newpos);
  }
  *meanVel /= num;

  return E;
}

/*
** assign random signs to the vectors and measures the length of their
** mean, as quickly as possible
*/
static double
party(Nrrd *npos, airRandMTState *rstate) {
  double *pos, mean[3];
  unsigned int ii, num, rnd, rndBit;

  pos = (double *)(npos->data);
  num = npos->axis[1].size;
  rnd = airUIrandMT_r(rstate);
  rndBit = 0;
  ELL_3V_SET(mean, 0, 0, 0);
  for (ii=0; ii<num; ii++) {
    if (32 == rndBit) {
      rnd = airUIrandMT_r(rstate);
      rndBit = 0;
    }
    if (rnd & (1 << rndBit++)) {
      ELL_3V_SCALE(pos + 3*ii, -1, pos + 3*ii);
    }
    ELL_3V_INCR(mean, pos + 3*ii);
  }
  ELL_3V_SCALE(mean, 1.0/num, mean);
  return ELL_3V_LEN(mean);
}

/*
** parties until the gradients settle down
*/
int
tenGradientBalance(Nrrd *nout, const Nrrd *nin,
                   tenGradientParm *tgparm) {
  char me[]="tenGradientBalance", err[BIFF_STRLEN];
  double len, lastLen, improv;
  airRandMTState *rstate;

  if (!nout || tenGradientCheck(nin, nrrdTypeUnknown, 2)) {
    sprintf(err, "%s: got NULL pointer or invalid input", me);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdConvert(nout, nin, nrrdTypeDouble)) {
    sprintf(err, "%s: can't initialize output with input", me);
    biffMove(TEN, err, NRRD); return 1;
  }

  rstate = airRandMTStateNew(tgparm->seed);
  lastLen = 1.0;
  do {
    do {
      len = party(nout, rstate);
    } while (len > lastLen);
    improv = lastLen - len;
    lastLen = len;
    fprintf(stderr, "%s: improvement: %g  (mean length = %g)\n",
            me, improv, len);
  } while (improv > tgparm->minMeanImprovement
           && len > tgparm->minMean);
  airRandMTStateNix(rstate);
  
  return 0;
}

/*
******** tenGradientDistribute
**
** Takes the given list of gradients, normalizes their lengths,
** optionally jitters their positions, does point repulsion, and then
** (optionally) selects a combination of directions with minimum vector sum.
**
** The complicated part of this is the point repulsion, which uses a
** gradient descent with variable set size. The progress of the system
** is measured by decrease in potential (when its measurement doesn't
** overflow to infinity) or an increase in the minimum angle.  When a
** step results in negative progress, the step size is halved, and the
** iteration is attempted again.  Based on the observation that at
** some points the step size must be made very small to get progress,
** the step size is cautiously increased ("nudged") at every
** iteration, to try to avoid using an overly small step.  The amount
** by which the step is nudged is halved everytime the step is halved,
** to avoid endless cycling through step sizes.
*/
int
tenGradientDistribute(Nrrd *nout, const Nrrd *nin,
                      tenGradientParm *tgparm) {
  char me[]="tenGradientDistribute", err[BIFF_STRLEN],
    filename[AIR_STRLEN_SMALL];
  unsigned int ii, num, iter, oldIdx, newIdx, edgeShrink;
  airArray *mop;
  Nrrd *npos[2];
  double *pos, len, meanVelocity, pot, potNew, potD,
    edge, edgeMin, angle, angleNew;
  int E;

  if (!nout || tenGradientCheck(nin, nrrdTypeUnknown, 2) || !tgparm) {
    sprintf(err, "%s: got NULL pointer or invalid input", me);
    biffAdd(TEN, err); return 1;
  }

  num = nin->axis[1].size;
  mop = airMopNew();
  npos[0] = nrrdNew();
  npos[1] = nrrdNew();
  airMopAdd(mop, npos[0], (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, npos[1], (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(npos[0], nin, nrrdTypeDouble)
      || nrrdConvert(npos[1], nin, nrrdTypeDouble)) {
    sprintf(err, "%s: trouble allocating temp buffers", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }

  pos = (double*)(npos[0]->data);
  for (ii=0; ii<num; ii++) {
    ELL_3V_NORM(pos, pos, len);
    pos += 3;
  }
  if (tgparm->jitter) {
    if (tenGradientJitter(npos[0], npos[0], tgparm->jitter)) {
      sprintf(err, "%s: problem jittering input", me);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
  }

  /* initialize things prior to first iteration; have to
     make sure that loop body tests pass 1st time around */
  meanVelocity = 2*tgparm->minVelocity;
  potD = -2*tgparm->minPotentialChange;
  oldIdx = 0;
  newIdx = 1;
  tgparm->step = tgparm->initStep;
  tgparm->nudge = 0.1;
  tenGradientMeasure(&pot, &angle, NULL,
                     npos[oldIdx], tgparm, AIR_TRUE);
  for (iter = 0;
       ((!!tgparm->minIteration && iter < tgparm->minIteration)
        ||
        (iter < tgparm->maxIteration
         && (!tgparm->minPotentialChange
             || !AIR_EXISTS(potD) 
             || -potD > tgparm->minPotentialChange)
         && (!tgparm->minVelocity
             || meanVelocity > tgparm->minVelocity)
         && tgparm->step > FLT_MIN));
       iter++) {
    /* copy positions from old to new */
    memcpy(npos[newIdx]->data, npos[oldIdx]->data, 3*num*sizeof(double));
    edge = tenGradientIdealEdge(num, tgparm->single);
    edgeShrink = 0;
    /* try to do a position update, which will fail if repulsion values
       explode, from having an insufficiently small edge normalization,
       so retry with smaller edge next time */
    do {
      E = _tenGradientUpdate(&meanVelocity, &edgeMin,
                             npos[newIdx], edge, tgparm);
      if (E) {
        if (edgeShrink > tgparm->maxEdgeShrink) {
          sprintf(err, "%s: %u > %u edge shrinks (%g), update still failed",
                  me, edgeShrink, tgparm->maxEdgeShrink, edge);
          biffAdd(TEN, err); airMopError(mop); return 1;
        }
        edgeShrink++;
        /* re-initialize positions (HEY ugly code logic) */
        memcpy(npos[newIdx]->data, npos[oldIdx]->data, 3*num*sizeof(double));
        edge = edgeMin;
      }
    } while (E);
    tenGradientMeasure(&potNew, &angleNew, NULL,
                       npos[newIdx], tgparm, AIR_TRUE);
    if ((AIR_EXISTS(pot) && AIR_EXISTS(potNew) && potNew <= pot)
        || angleNew >= angle) {
      /* there was progress of some kind, either through potential
         decrease, or angle increase */
      potD = 2*(potNew - pot)/(potNew + pot);
      if (!(iter % tgparm->report)) {
        fprintf(stderr, "%s(%d): . . . . . . step = %g, edgeShrink = %u\n"
                "   velo = %g<>%g, phi = %g ~ %g<>%g, angle = %g ~ %g\n",
                me, iter, tgparm->step, edgeShrink,
                meanVelocity, tgparm->minVelocity,
                pot, potD, tgparm->minPotentialChange,
                angle, angleNew - angle);
      }
      if (tgparm->snap && !(iter % tgparm->snap)) {
        sprintf(filename, "%05d.nrrd", iter/tgparm->snap);
        fprintf(stderr, "%s(%d): . . . . . . saving %s\n",
                me, iter, filename);
        if (nrrdSave(filename, npos[newIdx], NULL)) {
          char *serr;
          serr = biffGetDone(NRRD);
          fprintf(stderr, "%s: iter=%d, couldn't save snapshot:\n%s"
                  "continuing ...\n", me, iter, serr);
          free(serr);
        }
      }
      tgparm->step *= 1 + tgparm->nudge;
      tgparm->step = AIR_MIN(tgparm->initStep, tgparm->step);
      pot = potNew;
      angle = angleNew;
      /* swap buffers */
      newIdx = 1 - newIdx;
      oldIdx = 1 - oldIdx;
    } else {    
      /* oops, did not make progress; back off and try again */
      fprintf(stderr, "%s(%d): ######## step %g --> %g\n"
              " phi = %g --> %g ~ %g, angle = %g --> %g\n",
              me, iter, tgparm->step, tgparm->step/2,
              pot, potNew, potD, angle, angleNew);
      tgparm->step /= 2;
      tgparm->nudge /= 2;
    }
  }

  /* when the for-loop test fails, we stop before computing the next
     iteration (which starts with copying from npos[oldIdx] to
     npos[newIdx]) ==> the final results are in npos[oldIdx] */

  fprintf(stderr, "%s: .......................... done distribution:\n"
          "  (%d && %d) || (%d \n"
          "               && (%d || %d || %d) \n"
          "               && (%d || %d) \n"
          "               && %d) is false\n", me,
          !!tgparm->minIteration, iter < tgparm->minIteration, 
          iter < tgparm->maxIteration,
          !tgparm->minPotentialChange,
          !AIR_EXISTS(potD), AIR_ABS(potD) > tgparm->minPotentialChange,
          !tgparm->minVelocity, meanVelocity > tgparm->minVelocity,
          tgparm->step > FLT_MIN);
  fprintf(stderr, "  iter=%d, velo = %g<>%g, phi = %g ~ %g<>%g;\n",
          iter, meanVelocity, tgparm->minVelocity, pot,
          potD, tgparm->minPotentialChange);
  fprintf(stderr, "  minEdge = %g; idealEdge = %g\n", 
          2*sin(angle/2), tenGradientIdealEdge(num, tgparm->single));
  
  tenGradientMeasure(&pot, NULL, NULL, npos[oldIdx], tgparm, AIR_FALSE);
  tgparm->potential = pot;
  tenGradientMeasure(&pot, &angle, &edge, npos[oldIdx], tgparm, AIR_TRUE);
  tgparm->potentialNorm = pot;
  tgparm->angle = angle;
  tgparm->edge = edge;
  tgparm->itersUsed = iter;

  if ((tgparm->minMeanImprovement || tgparm->minMean)
      && !tgparm->single) {
    fprintf(stderr, "%s: optimizing balance:\n", me);
    if (tenGradientBalance(nout, npos[oldIdx], tgparm)) {
      sprintf(err, "%s: failed to minimize vector sum of gradients", me);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
    fprintf(stderr, "%s: .......................... done balancing.\n", me);
  } else {
    fprintf(stderr, "%s: .......................... (no balancing)\n", me);
    if (nrrdConvert(nout, npos[oldIdx], nrrdTypeDouble)) {
      sprintf(err, "%s: couldn't set output", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
  }

  airMopOkay(mop); 
  return 0;
}

int
tenGradientGenerate(Nrrd *nout, unsigned int num, tenGradientParm *tgparm) {
  char me[]="tenGradientGenerate", err[BIFF_STRLEN];
  Nrrd *nin;
  airArray *mop;

  if (!(nout && tgparm)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( num >= 3 )) {
    sprintf(err, "%s: can generate minimum of 3 gradient directions "
            "(not %d)", me, num);
    biffAdd(TEN, err); return 1;
  }
  mop = airMopNew();
  nin = nrrdNew();
  airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);

  if (tenGradientRandom(nin, num, tgparm->seed)
      || tenGradientDistribute(nout, nin, tgparm)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
