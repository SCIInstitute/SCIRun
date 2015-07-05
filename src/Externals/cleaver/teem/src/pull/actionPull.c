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


#include "pull.h"
#include "privatePull.h"

/*
** issues:
** does everything work on the first iteration
** how to handle the needed extra probe for d strength / d scale
** how are force/energy along scale handled differently than in space?
*/

#ifdef PRAY
int _pullPraying = 0;
double _pullPrayCorner[2][2][3];
size_t _pullPrayRes[2] = {60,20};
#endif

unsigned int
_neighBinPoints(pullTask *task, pullBin *bin, pullPoint *point) {
  char me[]="_neighBinPoints";
  unsigned int nn, herPointIdx, herBinIdx;
  pullBin *herBin;
  pullPoint *herPoint;

  nn = 0;
  herBinIdx = 0;
  while ((herBin = bin->neighBin[herBinIdx])) {
    for (herPointIdx=0; herPointIdx<herBin->pointNum; herPointIdx++) {
      herPoint = herBin->point[herPointIdx];
      /*
      fprintf(stderr, "!%s(%u): neighbin %u has point %u\n", me,
              point->idtag, herBinIdx, herPoint->idtag);
      */
      /* can't interact with myself */
      if (point != herPoint) {
        if (nn < _PULL_NEIGH_MAXNUM) {
          task->neighPoint[nn++] = herPoint;
          /*
          fprintf(stderr, "%s(%u): neighPoint[%u] = %u\n", 
                  me, point->idtag, nn-1, herPoint->idtag);
          */
        } else {
          fprintf(stderr, "%s: maxed out at %u points!\n", me, 
                  _PULL_NEIGH_MAXNUM);
        }
      }
    }
    herBinIdx++;
  }
  return nn;
}


double
_energyInterParticle(pullTask *task, pullPoint *me, pullPoint *she, 
                     /* output */
                     double egrad[4]) {
  char meme[]="_energyInterParticle";
  double spadist, sparad, diff[4], rr, enr, frc, *parm;

  ELL_4V_SUB(diff, she->pos, me->pos);
  spadist = ELL_3V_LEN(diff);
  sparad = task->pctx->radiusSpace;
  
  rr = spadist/(2*sparad);
  /*
  fprintf(stderr, "!%s: rr(%u,%u) = %g\n", meme, me->idtag, she->idtag, rr);
  */
  if (rr > 1) {
    ELL_4V_SET(egrad, 0, 0, 0, 0);
    return 0;
  }
  if (rr == 0) {
    fprintf(stderr, "%s: pos of pts %u, %u equal: (%g,%g,%g,%g)\n",
            meme, me->idtag, she->idtag, 
            me->pos[0], me->pos[1], me->pos[2], me->pos[3]);
    ELL_4V_SET(egrad, 0, 0, 0, 0);
    return 0;
  }

  parm = task->pctx->energySpec->parm;
  enr = task->pctx->energySpec->energy->eval(&frc, rr, parm);
  frc *= -1.0/(2*sparad*spadist);
  ELL_3V_SCALE(egrad, frc, diff);
  egrad[3] = 0;
  /*
  fprintf(stderr, "%s: %u <-- %u = %g,%g,%g -> egrad = %g,%g,%g, enr = %g\n",
          meme, me->idtag, she->idtag, 
          diff[0], diff[1], diff[2],
          egrad[0], egrad[1], egrad[2], enr);
  */
  return enr;
}

double
_energyFromPoints(pullTask *task, pullBin *bin, pullPoint *point, 
                  /* output */
                  double egradSum[4]) {
  /* char me[]="_energyFromPoints"; */
  double energySum, distSqSum, spaDistSqMax, wghtSum;
  int nopt,     /* optimiziation: we sometimes re-use neighbor lists */
    ntrue;      /* we search all possible neighbors, stored in the bins
                   (either because !nopt, or, this iter we learn true
                   subset of interacting neighbors).  This could also
                   be called "dontreuse" or something like that */
  unsigned int nidx,
    nnum;       /* how much of task->neigh[] we use */

  /* set nopt and ntrue */
  if (task->pctx->neighborTrueProb < 1) {
    nopt = AIR_TRUE;
    if (egradSum) {
      /* We allow the neighbor list optimization only when we're also asked
         to compute the energy gradient.  When we're not getting the energy
         gradient, we're being called to test the waters at possible new
         locations, in which case we can't be changing the effective particle 
         neighborhood */
      ntrue = (0 == task->pctx->iter
               || airDrandMT_r(task->rng) < task->pctx->neighborTrueProb);
    } else {
      ntrue = AIR_FALSE;
    }
  } else {
    nopt = AIR_FALSE;
    ntrue = AIR_TRUE;
  }
  /*
  fprintf(stderr, "!%s(%u), nopt = %d, ntrue = %d\n", me, point->idtag,
          nopt, ntrue);
  */
  /* set nnum and task->neigh[] */
  if (ntrue) {
    nnum = _neighBinPoints(task, bin, point);
    if (nopt) {
      airArrayLenSet(point->neighArr, 0);
    }
  } else {
    /* (nopt true) this iter we re-use existing neighbor list */
    nnum = point->neighNum;
    for (nidx=0; nidx<nnum; nidx++) {
      task->neighPoint[nidx] = point->neighPoint[nidx];
    }
  }

  /* loop through neighbor points */
  spaDistSqMax = 4*task->pctx->radiusSpace*task->pctx->radiusSpace;
  /*
  fprintf(stderr, "%s: radiusSpace = %g -> spaDistSqMax = %g\n", me,
          task->pctx->radiusSpace, spaDistSqMax);
  */
  wghtSum = 0;
  energySum = 0;
  distSqSum = 0;
  point->neighInterNum = 0;
  point->neighDist = 0.0;
  point->neighMode = 0.0;
  if (egradSum) {
    ELL_4V_SET(egradSum, 0, 0, 0, 0);
  }
  for (nidx=0; nidx<nnum; nidx++) {
    double diff[4], spaDistSq, enr, egrad[4];
    pullPoint *herPoint;
    herPoint = task->neighPoint[nidx];
    ELL_4V_SUB(diff, herPoint->pos, point->pos);
    spaDistSq = ELL_3V_DOT(diff, diff);
    /*
    fprintf(stderr, "!%s: %u:%g,%g,%g <-- %u:%g,%g,%g = sqd %g %s %g\n", me,
            point->idtag, point->pos[0], point->pos[1], point->pos[2], 
            herPoint->idtag,
            herPoint->pos[0], herPoint->pos[1], herPoint->pos[2],
            spaDistSq, spaDistSq > spaDistSqMax ? ">" : "<=", spaDistSqMax);
    */
    if (spaDistSq > spaDistSqMax) {
      continue;
    }
    if (AIR_ABS(diff[3] > task->pctx->radiusScale)) {
      continue;
    }
    enr = _energyInterParticle(task, point, herPoint, egrad);
    /*
    fprintf(stderr, "!%s: energySum = %g + %g = %g\n", me, 
            energySum, enr, energySum + enr);
    */
    energySum += enr;
    if (egradSum) {
      ELL_4V_INCR(egradSum, egrad);
      if (ELL_4V_DOT(egrad, egrad)) {
        point->neighInterNum++;
        point->neighDist = spaDistSq;
        if (task->pctx->ispec[pullInfoTangentMode]) {
          double w, m;
          m = _pullPointScalar(task->pctx, herPoint, pullInfoTangentMode,
                               NULL, NULL);
          w = 1.0/spaDistSq;
          point->neighMode += w*m;
          wghtSum += w;
        }
        if (nopt && ntrue) {
          unsigned int ii;
          ii = airArrayLenIncr(point->neighArr, 1);
          point->neighPoint[ii] = herPoint;
        }
      }
    }
  }
  
  /* finish computing things averaged over neighbors */
  if (point->neighInterNum) {
    point->neighDist = sqrt(point->neighDist/point->neighInterNum);
    point->neighMode /= wghtSum;
  } else {
    point->neighDist = -1;
    point->neighMode = AIR_NAN;
  }

  return energySum;
}

double
_energyFromImage(pullTask *task, pullPoint *point,
                 /* output */
                 double egradSum[4]) {
  char me[]="_energyFromImage";
  double energy, grad3[3];
  int probed;

  /* not sure I have the logic for this right 
  int ptrue;
  if (task->pctx->probeProb < 1 && allowProbeProb) {
    if (egrad) {
      ptrue = (0 == task->pctx->iter
               || airDrandMT_r(task->rng) < task->pctx->probeProb);
    } else {
      ptrue = AIR_FALSE;
    }
  } else {
    ptrue = AIR_TRUE;
  }
  */
  probed = AIR_FALSE;
#define MAYBEPROBE \
  if (!probed) { \
    if (_pullProbe(task, point)) { \
      fprintf(stderr, "%s: problem probing!!!\n%s\n", me, biffGetDone(PULL)); \
    } \
    probed = AIR_TRUE; \
  }
  energy = 0;
  if (egradSum) {
    ELL_4V_SET(egradSum, 0, 0, 0, 0);
  }
  if (task->pctx->ispec[pullInfoHeight]
      && !task->pctx->ispec[pullInfoHeight]->constraint
      && (!task->pctx->ispec[pullInfoHeightLaplacian]
          || !task->pctx->ispec[pullInfoHeightLaplacian]->constraint)) {
    MAYBEPROBE;
    energy += _pullPointScalar(task->pctx, point, pullInfoHeight,
                               grad3, NULL);
    if (egradSum) {
      ELL_3V_INCR(egradSum, grad3);
    }
  }
  if (task->pctx->ispec[pullInfoIsovalue]
      && !task->pctx->ispec[pullInfoIsovalue]->constraint) {
    /* we're only going towards an isosurface, not constrained to it
       ==> set up a parabolic potential well around the isosurface */
    double val;
    MAYBEPROBE;
    val = _pullPointScalar(task->pctx, point, pullInfoIsovalue,
                           grad3, NULL);
    energy += val*val;
    if (egradSum) {
      ELL_3V_SCALE_INCR(egradSum, 2*val, grad3);
    }
  }
  return energy;
}
#undef MAYBEPROBE

/*
** its in here that we scale from "energy gradient" to "force"
*/
double
_pullPointEnergyTotal(pullTask *task, pullBin *bin, pullPoint *point,
                      /* output */
                      double force[4]) {
  char me[]="_pullPointEnergyTotal";
  double enrIm, enrPt, egradIm[4], egradPt[4], energy;
    
  ELL_4V_SET(egradIm, 0, 0, 0, 0); /* sssh */
  ELL_4V_SET(egradPt, 0, 0, 0, 0); /* sssh */
  enrIm = _energyFromImage(task, point, 
                           force ? egradIm : NULL);
  enrPt = _energyFromPoints(task, bin, point,
                            force ? egradPt : NULL);
  energy = AIR_LERP(task->pctx->alpha, enrIm, enrPt);
  /*
  fprintf(stderr, "!%s(%u): energy = lerp(%g, im %g, pt %g) = %g\n", me,
          point->idtag, task->pctx->alpha, enrIm, enrPt, energy);
  */
  if (force) {
    ELL_4V_LERP(force, task->pctx->alpha, egradIm, egradPt);
    ELL_4V_SCALE(force, -1, force);
    /*
    fprintf(stderr, "!%s(%u): egradIm = %g %g %g %g\n", me, point->idtag,
            egradIm[0], egradIm[1], egradIm[2], egradIm[3]);
    fprintf(stderr, "!%s(%u): egradPt = %g %g %g %g\n", me, point->idtag,
            egradPt[0], egradPt[1], egradPt[2], egradPt[3]);
    fprintf(stderr, "!%s(%u): ---> force = %g %g %g %g\n", me,
            point->idtag, force[0], force[1], force[2], force[3]);
    */
  }
  if (task->pctx->wall) {
    unsigned int axi;
    double frc;
    for (axi=0; axi<4; axi++) {
      frc = task->pctx->bboxMin[axi] - point->pos[axi];
      if (frc < 0) {
        /* not below min */
        frc = task->pctx->bboxMax[axi] - point->pos[axi];
        if (frc > 0) {
          /* not above max */
          frc = 0;
        }
      } 
      energy += task->pctx->wall*frc*frc/2;
      if (force) {
        force[axi] += task->pctx->wall*frc;
      }
    }
  }
  if (!AIR_EXISTS(energy)) {
    fprintf(stderr, "!%s(%u): HEY! non-exist energy %g\n", me, point->idtag,
            energy);
  }
  return energy;
}

double
_pullDistLimit(pullTask *task, pullPoint *point) {
  double ret;

  if (point->neighDist < 0 /* no neighbors! */
      || pullEnergyZero == task->pctx->energySpec->energy) {
    ret = task->pctx->radiusSpace;
  } else {
    ret = point->neighDist;
  }
  /* task->pctx->constraintVoxelSize might be considered here */
  return ret;
}

int
_pullPointProcess(pullTask *task, pullBin *bin, pullPoint *point) {
  char me[]="pullPointProcess", err[BIFF_STRLEN];
  double energyOld, energyNew, force[4], distLimit, posOld[4],
    capvec[3], caplen, capscl;  /* related to capping distance traveled
                                   in a per-iteration way */
  int stepBad, giveUp;

  if (!point->stepEnergy) {
    sprintf(err, "%s: whoa, point %u step is zero!", me, point->idtag);
    biffAdd(PULL, err); return 1;
  }

#ifdef PRAYING
  if (0 && 162 == point->idtag) {
    fprintf(stderr, "!%s: !!!!!!!!!!!! praying ON!\n", me);
    _pullPraying = AIR_TRUE;
    ELL_3V_COPY(_pullPrayCorner[0][0], point->pos);
    _pullPrayCorner[0][0][2] -= 1;
    ELL_3V_COPY(_pullPrayCorner[0][1], point->pos);
    _pullPrayCorner[0][1][2] += 1;
    fprintf(stderr, "!%s: corner[0][0] = %g %g %g\n", me, 
            _pullPrayCorner[0][0][0],
            _pullPrayCorner[0][0][1],
            _pullPrayCorner[0][0][2]);
    fprintf(stderr, "!%s: corner[0][1] = %g %g %g\n", me, 
            _pullPrayCorner[0][1][0],
            _pullPrayCorner[0][1][1],
            _pullPrayCorner[0][1][2]);
  } else {
    _pullPraying = AIR_FALSE;
  }

  if (_pullPraying) {
    fprintf(stderr, "%s: =============================== (%u) hi @ %g %g %g\n",
            me, point->idtag, point->pos[0], point->pos[1], point->pos[2]);
  }
#endif
  energyOld = _pullPointEnergyTotal(task, bin, point, force);
#ifdef PRAYING
  if (_pullPraying) {
    fprintf(stderr, "!%s: =================== point %u has:\n "
            "     energy = %g ; ndist = %g, force %g %g %g %g\n", me,
            point->idtag, energyOld, point->neighDist,
            force[0], force[1], force[2], force[3]);
  }
#endif
  if (!( AIR_EXISTS(energyOld) && ELL_4V_EXISTS(force) )) {
    sprintf(err, "%s: point %u non-exist energy or force", me, point->idtag);
    biffAdd(PULL, err); return 1;
  }

  if (task->pctx->constraint) {
    /* we have a constraint, so do something to get the force more
       tangential to the constriant surface */
    double proj[9], pfrc[3];
    _pullConstraintTangent(task, point, proj);
    ELL_3MV_MUL(pfrc, proj, force);
    ELL_3V_COPY(force, pfrc);
  }

  point->status = 0; /* reset status bitflag */
  ELL_4V_COPY(posOld, point->pos);
  _pullPointHistInit(point);
  _pullPointHistAdd(point, pullCondOld);
  
  if (!ELL_4V_LEN(force)) {
    /* this particle has no reason to go anywhere; we're done with it */
    point->energy = energyOld;
    return 0;
  }
  distLimit = _pullDistLimit(task, point);

  /* find capscl */
  ELL_3V_SCALE(capvec, point->stepEnergy, force);
  caplen = ELL_3V_LEN(capvec);
  if (caplen > distLimit) {
    capscl = distLimit/caplen;
  } else {
    capscl = 1;
  }
#ifdef PRAYING
  if (_pullPraying) {
    fprintf(stderr, "%s: ======= (%u) capscl = %g\n", me,
            point->idtag, capscl);
  }

  if (_pullPraying) {
    double nfrc[3], len, phold[4], ee;
    int cfail;
    ELL_4V_COPY(phold, point->pos);
    
    fprintf(stderr, "!%s(%u,%u): energy(%g,%g,%g) = %f (original)\n",
            me, task->pctx->iter, point->idtag,
            point->pos[0], point->pos[1], point->pos[2], energyOld);
    
    ELL_4V_SCALE_ADD2(point->pos, 1.0, posOld,
                      capscl*point->stepEnergy, force);
    ELL_3V_COPY(_pullPrayCorner[1][0], point->pos);
    _pullPrayCorner[1][0][2] -= 1;
    ELL_3V_COPY(_pullPrayCorner[1][1], point->pos);
    _pullPrayCorner[1][1][2] += 1;
    fprintf(stderr, "!%s: corner[1][0] = %g %g %g\n", me, 
            _pullPrayCorner[1][0][0],
            _pullPrayCorner[1][0][1],
            _pullPrayCorner[1][0][2]);
    fprintf(stderr, "!%s: corner[1][1] = %g %g %g\n", me, 
            _pullPrayCorner[1][1][0],
            _pullPrayCorner[1][1][1],
            _pullPrayCorner[1][1][2]);

#define PROBE(l)  if (_pullProbe(task, point)) {                   \
      sprintf(err, "%s: while praying", me);                       \
      biffAdd(PULL, err); return 1;                                \
    }                                                              \
    (l) = _pullPointScalar(task->pctx, point,                      \
                           pullInfoHeight, NULL, NULL);
    if (1) {
      double *enr, *lpl, uu, vv, vpos[2][3], ll, mid[3];
      unsigned int ui, vi;
      Nrrd *nenr, *nlpl;
      nenr = nrrdNew();
      nlpl = nrrdNew();
      nrrdMaybeAlloc_nva(nenr, nrrdTypeDouble, 2, _pullPrayRes);
      enr = AIR_CAST(double *, nenr->data);
      nrrdMaybeAlloc_nva(nlpl, nrrdTypeDouble, 2, _pullPrayRes);
      lpl = AIR_CAST(double *, nlpl->data);
      for (vi=0; vi<_pullPrayRes[1]; vi++) {
        vv = AIR_AFFINE(0, vi, _pullPrayRes[1]-1, 0, 1);
        ELL_3V_LERP(vpos[0], vv, _pullPrayCorner[0][0], _pullPrayCorner[0][1]);
        ELL_3V_LERP(vpos[1], vv, _pullPrayCorner[1][0], _pullPrayCorner[1][1]);
        for (ui=0; ui<_pullPrayRes[0]; ui++) {
          uu = AIR_AFFINE(0, ui, _pullPrayRes[0]-1, 0, 1);
          ELL_3V_LERP(point->pos, uu, vpos[0], vpos[1]);
          PROBE(ll);
          lpl[ui + _pullPrayRes[0]*vi] = ll;
          enr[ui + _pullPrayRes[0]*vi] = _pullPointEnergyTotal(task, bin,
                                                               point, NULL);
        }
      }
      nrrdSave("nenr.nrrd", nenr, NULL);
      nrrdSave("nlpl.nrrd", nlpl, NULL);
      nenr = nrrdNuke(nenr);
      nlpl = nrrdNuke(nlpl);
    }
#undef PROBE

    ELL_4V_COPY(point->pos, phold);
    _pullPointEnergyTotal(task, bin, point, NULL);
  }
#endif

  do {
    int constrFail;
    
    giveUp = AIR_FALSE;
    ELL_4V_SCALE_ADD2(point->pos, 1.0, posOld,
                      capscl*point->stepEnergy, force);

    _pullPointHistAdd(point, pullCondEnergyTry);
#ifdef PRAYING
    if (_pullPraying) {
      fprintf(stderr, "!%s: ======= (%u) try step %g to pos %g %g %g %g\n", me,
              point->idtag, capscl*point->stepEnergy, 
              point->pos[0], point->pos[1], point->pos[2], point->pos[3]);
    }
#endif
    if (task->pctx->constraint) {
      if (_pullConstraintSatisfy(task, point, &constrFail)) {
        sprintf(err, "%s: trouble", me);
        biffAdd(PULL, err); return 1;
      }
    } else {
      constrFail = AIR_FALSE;
    }
    if (constrFail) {
      energyNew = AIR_NAN;
#ifdef PRAYING
      if (_pullPraying) {
        fprintf(stderr, "!%s: ======= constraint fail\n", me);
      }
#endif
    } else {
      energyNew = _pullPointEnergyTotal(task, bin, point,  NULL);
#ifdef PRAYING
      if (_pullPraying) {
        fprintf(stderr, "!%s: ======= e new = %g %s old %g %s\n", me,
                energyNew, energyNew > energyOld ? ">" : "<=", energyOld,
                energyNew > energyOld ? "!! BADSTEP !!" : "ok");
      }
#endif
    }
    stepBad = constrFail || (energyNew > energyOld);
    if (stepBad) {
      point->stepEnergy *= task->pctx->stepScale;
      if (constrFail) {
        _pullPointHistAdd(point, pullCondConstraintFail);
      } else {
        _pullPointHistAdd(point, pullCondEnergyBad);
      }
      /* you have a problem if you had a non-trivial force, but you can't
         ever seem to take a small enough step to reduce energy */
      if (point->stepEnergy < 0.000000000000001) {
        if (task->pctx->verbose > 1) {
          fprintf(stderr, "\n%s: %u (%g,%g,%g,%g) stepEnr %g stuck; "
                  "capscl %g <<<<\n\n\n", me, point->idtag, 
                  point->pos[0], point->pos[1], point->pos[2], point->pos[3],
                  point->stepEnergy, capscl);
        }
        /* This point is fuct, may as well reset its step, maybe things
           will go better next time.  Without this resetting, it will stay
           effectively frozen */
        ELL_4V_COPY(point->pos, posOld);
        energyNew = energyOld; /* to be copied into point->energy below */
        point->stepEnergy = task->pctx->stepInitial/100;
        point->status |= PULL_STATUS_STUCK_BIT;
        giveUp = AIR_TRUE;
      }
    }  
  } while (stepBad && !giveUp);
  /* now: energy decreased, and, if we have one, constraint has been met */

  _pullPointHistAdd(point, pullCondNew);
  ELL_4V_COPY(point->force, force);

  /* not recorded for the sake of this function, but for system accounting */
  point->energy = energyNew;
  if (!AIR_EXISTS(energyNew)) {
    sprintf(err, "%s: point %u has non-exist final energy %g\n", 
            me, point->idtag, energyNew);
    biffAdd(PULL, err); return 1;
  }

  /* cheezy demo of how to remove points 
  if (1) {
    double dd[3], pp[3] = {0.80828, 0.191011, -0.0317192};
    ELL_3V_SUB(dd, point->pos, pp);
    if (ELL_3V_LEN(dd) < 0.04) {
      point->status |= PULL_STATUS_NIXME_BIT;
    }
  }
  */
  
  return 0;
}

int
pullBinProcess(pullTask *task, unsigned int myBinIdx) {
  char me[]="pullBinProcess", err[BIFF_STRLEN];
  pullBin *myBin;
  unsigned int myPointIdx;

  if (task->pctx->verbose > 2) {
    fprintf(stderr, "%s(%u): doing bin %u\n", me, task->threadIdx, myBinIdx);
  }
  myBin = task->pctx->bin + myBinIdx;
  for (myPointIdx=0; myPointIdx<myBin->pointNum; myPointIdx++) {
    if (_pullPointProcess(task, myBin, myBin->point[myPointIdx])) {
      sprintf(err, "%s: on point %u of bin %u\n", me, 
              myPointIdx, myBinIdx);
      biffAdd(PULL, err); return 1;
    }
    task->stuckNum += (myBin->point[myPointIdx]->status
                       & PULL_STATUS_STUCK_BIT);
  } /* for myPointIdx */

  return 0;
}
