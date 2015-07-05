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


#include "../ten.h"

char *info = ("Compute the makings of a new tensor.dat file.");

int
main(int argc, char *argv[]) {
  char *me, *err;
  hestOpt *hopt=NULL;
  airArray *mop;

  int E, optimizeEdge, workToDo;
  unsigned int ii, numRange[2], seedRange[2], seed, seedDone;
  double *log, minAngle, minEdge, pot, potNorm, time0, time1;
  char *outStr, logFilename[AIR_STRLEN_MED], gradFilename[AIR_STRLEN_MED],
    keyStr[AIR_STRLEN_MED], valStr[AIR_STRLEN_MED];
  tenGradientParm *tgparm;
  Nrrd *nlog, *ngrad;
  size_t size[2];
  
  mop = airMopNew();
  tgparm = tenGradientParmNew();
  airMopAdd(mop, tgparm, (airMopper)tenGradientParmNix, airMopAlways);
  tgparm->single = AIR_FALSE;
  tgparm->snap = 0;
  tgparm->minMeanImprovement = 0.0;

  nlog = nrrdNew();
  airMopAdd(mop, nlog, (airMopper)nrrdNuke, airMopAlways);
  ngrad = nrrdNew();
  airMopAdd(mop, ngrad, (airMopper)nrrdNuke, airMopAlways);
  
  me = argv[0];
  hestOptAdd(&hopt, "num", "min max", airTypeUInt, 2, 2, numRange, "6 129",
             "range of number of gradients to be computed");
  hestOptAdd(&hopt, "seed", "min max", airTypeUInt, 2, 2, seedRange, "1 0",
             "range of seed values to use with the RNG. Using max lower "
             "than min means that the seed values should be increased "
             "(and used for computation) without bound");
  hestOptAdd(&hopt, "p", "exponent", airTypeUInt, 1, 1, &(tgparm->expo), "1",
             "the exponent p that defines the 1/r^p potential energy "
             "(Coulomb is 1)");
  hestOptAdd(&hopt, "step", "step", airTypeDouble, 1, 1, &(tgparm->initStep),
             "1", "time increment in solver");
  hestOptAdd(&hopt, "miniter", "# iters", airTypeInt, 1, 1,
             &(tgparm->minIteration), "0",
             "required minimum number of simulation iterations");
  hestOptAdd(&hopt, "maxiter", "# iters", airTypeInt, 1, 1,
             &(tgparm->maxIteration), "1000000",
             "max number of simulations iterations");
  hestOptAdd(&hopt, "minvelo", "vel", airTypeDouble, 1, 1, 
             &(tgparm->minVelocity), "0.00000000001",
             "low threshold on mean velocity of repelling points, "
             "at which point repulsion phase of algorithm terminates.");
  hestOptAdd(&hopt, "dp", "potential change", airTypeDouble, 1, 1, 
             &(tgparm->minPotentialChange), "0.00000000001",
             "low threshold on fractional change of potential at "
             "which point repulsion phase of algorithm terminates.");
  hestOptAdd(&hopt, "minimprov", "delta", airTypeDouble, 1, 1, 
             &(tgparm->minMeanImprovement), "0.00005",
             "in the second phase of the algorithm, "
             "when stochastically balancing the sign of the gradients, "
             "the (small) improvement in length of mean gradient "
             "which triggers termination (as further improvements "
             "are unlikely. ");
  hestOptAdd(&hopt, "minmean", "len", airTypeDouble, 1, 1,
             &(tgparm->minMean), "0.0005",
             "if length of mean gradient falls below this, finish "
             "the balancing phase");
  hestOptAdd(&hopt, "oe", NULL, airTypeInt, 0, 0, &optimizeEdge, NULL,
             "optimize for the maximal minimal edge length, "
             "instead of potential.");
  hestOptAdd(&hopt, "odir", "out", airTypeString, 1, 1, &outStr, ".",
             "output directory for all grad files and logs, you should "
             "leave off the trailing /");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  if (0 == seedRange[0]) {
    fprintf(stderr, "%s: sorry, initial seed must be non-zero\n", me);
    airMopError(mop); return 1;
  }
  if (!( numRange[0] <= numRange[1]
         && numRange[0] >= 6 )) {
    fprintf(stderr, "%s: number range [%u,%u] invalid\n", me,
            numRange[0], numRange[1]);
    airMopError(mop); return 1;
  }

  /* in master log (per gradient set):
     0: # grads
     1: last seed tried
     2: seed of best so far
     3: best phi, not normalized
     4: best phi, normalized
     5: best edge min
     6: ideal edge
     7: iters used
     8: time used (in seconds)
  */

  /* see if we can open the log */
  sprintf(logFilename, "%s/000-%04u-log.nrrd", outStr, tgparm->expo);
  if (nrrdLoad(nlog, logFilename, NULL)) {
    /* no, we couldn't load it, and we don't care why */
    free(biffGetDone(NRRD));
    /* create a log nrrd of the correct size */
    size[0] = 9;
    size[1] = numRange[1]+1;
    if (nrrdMaybeAlloc_nva(nlog, nrrdTypeDouble, 2, size)) {
      airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble making log:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  } else {
    /* we could open the log, see if its the right size */
    if (!( nrrdTypeDouble == nlog->type
           && 2 == nlog->dim
           && 9 == nlog->axis[0].size
           && numRange[1]+1 == nlog->axis[1].size )) {
      fprintf(stderr, "%s: given log (%s %u-D %ux%ux?) doesn't match "
              "desired (%s 2-D 9x%u)\n", me,
              airEnumStr(nrrdType, nlog->type),
              nlog->dim, 
              AIR_CAST(unsigned int, nlog->axis[0].size),
              AIR_CAST(unsigned int, nlog->axis[1].size),
              airEnumStr(nrrdType, nrrdTypeDouble),
              numRange[1]+1);
      airMopError(mop); return 1;
    }
  }
  /* nlog is the right size */
  /* initialize log's first column and key/value pairs, and (re)save */
  log = AIR_CAST(double *, nlog->data);
  for (ii=numRange[0]; ii<=numRange[1]; ii++) {
    log[0 + 9*ii] = ii;
  }
  E = 0;
  if (!E) strcpy(keyStr, "maxiter");
  if (!E) sprintf(valStr, "%d", tgparm->maxIteration);
  if (!E) E |= nrrdKeyValueAdd(nlog, keyStr, valStr);
  if (!E) strcpy(keyStr, "step");
  if (!E) sprintf(valStr, "%g", tgparm->initStep);
  if (!E) E |= nrrdKeyValueAdd(nlog, keyStr, valStr);
  if (!E) strcpy(keyStr, "dp");
  if (!E) sprintf(valStr, "%g", tgparm->minPotentialChange);
  if (!E) E |= nrrdKeyValueAdd(nlog, keyStr, valStr);
  if (!E) strcpy(keyStr, "minvelo");
  if (!E) sprintf(valStr, "%g", tgparm->minVelocity);
  if (!E) E |= nrrdKeyValueAdd(nlog, keyStr, valStr);
  if (!E) strcpy(keyStr, "minmean");
  if (!E) sprintf(valStr, "%g", tgparm->minMean);
  if (!E) E |= nrrdKeyValueAdd(nlog, keyStr, valStr);
  if (!E) E |= nrrdSave(logFilename, nlog, NULL);
  if (E) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing log:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  /* in master log (per gradient set):
     0: # grads
     1: last seed for which solution was computed
     2: seed of best solution so far
     3: best phi, not edge normalized
     4: best phi, edge normalized
     5: best minimum edge length
     6: ideal edge length
     7: iters used
     8: time used (in seconds)
  */

  workToDo = AIR_FALSE;
  for (seed=seedRange[0];
       seedRange[1] < seedRange[0] || seed <= seedRange[1];
       seed++) {
    for (ii=numRange[0]; ii<=numRange[1]; ii++) {
      seedDone = AIR_CAST(unsigned int, log[1 + 9*ii]);
      /* if no seeds have been tried, seedDone will be zero */
      if (seedDone >= seed) {
        /* have already tried this seed, move on */
        continue;
      }
      workToDo = AIR_TRUE;
      tgparm->seed = seed;
      fprintf(stderr, "%s ================ %u %u\n", me, ii, tgparm->seed);
      time0 = airTime();
      if (tenGradientGenerate(ngrad, ii, tgparm)) {
        airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble making distribution:\n%s\n", me, err);
        airMopError(mop); return 1;
      }
      time1 = airTime();
      tenGradientMeasure(&potNorm, &minAngle, &minEdge,
                         ngrad, tgparm, AIR_TRUE);
      if (!seedDone
          || ((optimizeEdge && minEdge > log[5 + 9*ii])
              || potNorm < log[4 + 9*ii])) {
        /* this gradient set is best so far */
        tenGradientMeasure(&pot, NULL, NULL, ngrad, tgparm, AIR_FALSE);
        log[2 + 9*ii] = tgparm->seed;
        log[3 + 9*ii] = pot;
        log[4 + 9*ii] = potNorm;
        log[5 + 9*ii] = minEdge;
        log[6 + 9*ii] = tenGradientIdealEdge(ii, AIR_FALSE);
        log[7 + 9*ii] = tgparm->itersUsed;
        log[8 + 9*ii] = time1 - time0;
        sprintf(gradFilename, "%s/%03u-%04u.nrrd", outStr, ii, tgparm->expo);
        if (nrrdSave(gradFilename, ngrad, NULL)) {
          airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
          fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
          airMopError(mop); return 1;
        }
      }
      log[1 + 9*ii] = tgparm->seed;
      if (nrrdSave(logFilename, nlog, NULL)) {
        airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble writing log:\n%s\n", me, err);
        airMopError(mop); return 1;
      }
    }
  }
  if (!workToDo) {
    fprintf(stderr, "%s: apparently finished requested computations.\n", me);
  }

  airMopOkay(mop);
  return 0;
}
