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

#include "push.h"
#include "privatePush.h"

#define SPRING  "spring"
#define GAUSS   "gauss"
#define COULOMB "coulomb"
#define COTAN   "cotan"
#define ZERO    "zero"

char
_pushEnergyTypeStr[PUSH_ENERGY_TYPE_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_energy)",
  SPRING,
  GAUSS,
  COULOMB,
  COTAN,
  ZERO
};

char
_pushEnergyTypeDesc[PUSH_ENERGY_TYPE_MAX+1][AIR_STRLEN_MED] = {
  "unknown_energy",
  "Hooke's law-based potential, with a tunable region of attraction",
  "Gaussian potential",
  "Coulomb electrostatic potential, with tunable cut-off",
  "Cotangent-based potential (from Meyer et al. SMI '05)",
  "no energy"
};

airEnum
_pushEnergyType = {
  "energy",
  PUSH_ENERGY_TYPE_MAX,
  _pushEnergyTypeStr,  NULL,
  _pushEnergyTypeDesc,
  NULL, NULL,
  AIR_FALSE
};
airEnum *
pushEnergyType = &_pushEnergyType;

/* ----------------------------------------------------------------
** ------------------------------ UNKNOWN -------------------------
** ----------------------------------------------------------------
*/
void
_pushEnergyUnknownEval(double *enr, double *frc,
                       double dist, const double *parm) {
  char me[]="_pushEnergyUnknownEval";

  AIR_UNUSED(dist);
  AIR_UNUSED(parm);
  *enr = AIR_NAN;
  *frc = AIR_NAN;
  fprintf(stderr, "%s: ERROR- using unknown energy.\n", me);
  return;
}

double
_pushEnergyUnknownSupport(const double *parm) {
  char me[]="_pushEnergyUnknownSupport";

  AIR_UNUSED(parm);
  fprintf(stderr, "%s: ERROR- using unknown energy.\n", me);
  return AIR_NAN;
}

pushEnergy
_pushEnergyUnknown = {
  "unknown",
  0,
  _pushEnergyUnknownEval,
  _pushEnergyUnknownSupport
};
const pushEnergy *const
pushEnergyUnknown = &_pushEnergyUnknown;

/* ----------------------------------------------------------------
** ------------------------------ SPRING --------------------------
** ----------------------------------------------------------------
** 1 parms:
** parm[0]: width of pull region (beyond 1.0)
**
** learned: "1/2" is not 0.5 !!!!!
*/
void
_pushEnergySpringEval(double *enr, double *frc,
                      double dist, const double *parm) {
  /* char me[]="_pushEnergySpringEval"; */
  double xx, pull;

  pull = parm[0];
  xx = dist - 1.0;
  if (xx > pull) {
    *enr = 0;
    *frc = 0;
  } else if (xx > 0) {
    *enr = xx*xx*(xx*xx/(4*pull*pull) - 2*xx/(3*pull) + 1.0/2.0);
    *frc = xx*(xx*xx/(pull*pull) - 2*xx/pull + 1);
  } else {
    *enr = xx*xx/2;
    *frc = xx;
  }
  /*
  if (!AIR_EXISTS(ret)) {
    fprintf(stderr, "!%s: dist=%g, pull=%g, blah=%d --> ret=%g\n",
            me, dist, pull, blah, ret);
  }
  */
  return;
}

double
_pushEnergySpringSupport(const double *parm) {

  return 1.0 + parm[0];
}

const pushEnergy
_pushEnergySpring = {
  SPRING,
  1,
  _pushEnergySpringEval,
  _pushEnergySpringSupport
};
const pushEnergy *const
pushEnergySpring = &_pushEnergySpring;

/* ----------------------------------------------------------------
** ------------------------------ GAUSS --------------------------
** ----------------------------------------------------------------
** 1 parms:
** (distance to inflection point of force function is always 1.0)
** parm[0]: cut-off (as a multiple of standard dev (which is 1.0))
*/
/* HEY: copied from teem/src/nrrd/kernel.c */
#define _GAUSS(x, sig, cut) ( \
   x >= sig*cut ? 0           \
   : exp(-x*x/(2.0*sig*sig))/(sig*2.50662827463100050241))

#define _DGAUSS(x, sig, cut) ( \
   x >= sig*cut ? 0            \
   : -exp(-x*x/(2.0*sig*sig))*x/(sig*sig*sig*2.50662827463100050241))

void
_pushEnergyGaussEval(double *enr, double *frc,
                     double dist, const double *parm) {
  double cut;

  cut = parm[0];
  *enr = _GAUSS(dist, 1.0, cut);
  *frc = _DGAUSS(dist, 1.0, cut);
  return;
}

double
_pushEnergyGaussSupport(const double *parm) {

  return parm[0];
}

const pushEnergy
_pushEnergyGauss = {
  GAUSS,
  1,
  _pushEnergyGaussEval,
  _pushEnergyGaussSupport
};
const pushEnergy *const
pushEnergyGauss = &_pushEnergyGauss;

/* ----------------------------------------------------------------
** ------------------------------ CHARGE --------------------------
** ----------------------------------------------------------------
** 1 parms:
** (scale: distance to "1.0" in graph of x^(-2))
** parm[0]: cut-off (as multiple of "1.0")
*/
void
_pushEnergyCoulombEval(double *enr, double *frc,
                       double dist, const double *parm) {

  *enr = (dist > parm[0] ? 0 : 1.0/dist);
  *frc = (dist > parm[0] ? 0 : -1.0/(dist*dist));
  return;
}

double
_pushEnergyCoulombSupport(const double *parm) {

  return parm[0];
}

const pushEnergy
_pushEnergyCoulomb = {
  COULOMB,
  1,
  _pushEnergyCoulombEval,
  _pushEnergyCoulombSupport
};
const pushEnergy *const
pushEnergyCoulomb = &_pushEnergyCoulomb;

/* ----------------------------------------------------------------
** ------------------------------ COTAN ---------------------------
** ----------------------------------------------------------------
** 0 parms!
*/
void
_pushEnergyCotanEval(double *enr, double *frc,
                     double dist, const double *parm) {
  double pot, cc;

  AIR_UNUSED(parm);
  pot = AIR_PI/2.0;
  cc = 1.0/(FLT_MIN + tan(dist*pot));
  *enr = dist > 1 ? 0 : cc + dist*pot - pot;
  *frc = dist > 1 ? 0 : -cc*cc*pot;
  return;
}

double
_pushEnergyCotanSupport(const double *parm) {

  AIR_UNUSED(parm);
  return 1;
}

const pushEnergy
_pushEnergyCotan = {
  COTAN,
  0,
  _pushEnergyCotanEval,
  _pushEnergyCotanSupport
};
const pushEnergy *const
pushEnergyCotan = &_pushEnergyCotan;

/* ----------------------------------------------------------------
** ------------------------------- ZERO ---------------------------
** ----------------------------------------------------------------
** 0 parms:
*/
void
_pushEnergyZeroEval(double *enr, double *frc,
                    double dist, const double *parm) {

  AIR_UNUSED(dist);
  AIR_UNUSED(parm);
  *enr = 0;
  *frc = 0;
  return;
}

double
_pushEnergyZeroSupport(const double *parm) {

  AIR_UNUSED(parm);
  return 1.0;
}

const pushEnergy
_pushEnergyZero = {
  ZERO,
  0,
  _pushEnergyZeroEval,
  _pushEnergyZeroSupport
};
const pushEnergy *const
pushEnergyZero = &_pushEnergyZero;

/* ----------------------------------------------------------------
** ----------------------------------------------------------------
** ----------------------------------------------------------------
*/

const pushEnergy *const pushEnergyAll[PUSH_ENERGY_TYPE_MAX+1] = {
  &_pushEnergyUnknown,  /* 0 */
  &_pushEnergySpring,   /* 1 */
  &_pushEnergyGauss,    /* 2 */
  &_pushEnergyCoulomb,  /* 3 */
  &_pushEnergyCotan,    /* 4 */
  &_pushEnergyZero      /* 5 */
};

pushEnergySpec *
pushEnergySpecNew() {
  pushEnergySpec *ensp;
  int pi;

  ensp = (pushEnergySpec *)calloc(1, sizeof(pushEnergySpec));
  if (ensp) {
    ensp->energy = pushEnergyUnknown;
    for (pi=0; pi<PUSH_ENERGY_PARM_NUM; pi++) {
      ensp->parm[pi] = AIR_NAN;
    }
  }
  return ensp;
}

void
pushEnergySpecSet(pushEnergySpec *ensp, const pushEnergy *energy,
                  const double parm[PUSH_ENERGY_PARM_NUM]) {
  unsigned int pi;

  if (ensp && energy && parm) {
    ensp->energy = energy;
    for (pi=0; pi<PUSH_ENERGY_PARM_NUM; pi++) {
      ensp->parm[pi] = parm[pi];
    }
  }
  return;
}

pushEnergySpec *
pushEnergySpecNix(pushEnergySpec *ensp) {

  airFree(ensp);
  return NULL;
}

int
pushEnergySpecParse(pushEnergySpec *ensp, const char *_str) {
  char me[]="pushEnergySpecParse", err[BIFF_STRLEN];
  char *str, *col, *_pstr, *pstr;
  int etype;
  unsigned int pi, haveParm;
  airArray *mop;
  double pval;

  if (!( ensp && _str )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(PUSH, err); return 1;
  }

  /* see if its the name of something that needs no parameters */
  etype = airEnumVal(pushEnergyType, _str);
  if (pushEnergyTypeUnknown != etype) {
    /* the string is the name of some energy */
    ensp->energy = pushEnergyAll[etype];
    if (0 != ensp->energy->parmNum) {
      sprintf(err, "%s: need %u parms for %s energy, but got none", me,
              ensp->energy->parmNum, ensp->energy->name);
      biffAdd(PUSH, err); return 1;
    }
    /* the energy needs 0 parameters */
    for (pi=0; pi<PUSH_ENERGY_PARM_NUM; pi++) {
      ensp->parm[pi] = AIR_NAN;
    }
    return 0;
  }

  /* start parsing parms after ':' */
  mop = airMopNew();
  str = airStrdup(_str);
  airMopAdd(mop, str, (airMopper)airFree, airMopAlways);
  col = strchr(str, ':');
  if (!col) {
    sprintf(err, "%s: \"%s\" isn't a parameter-free energy, but it has no "
            "\":\" separator to indicate parameters", me, str);
    biffAdd(PUSH, err); airMopError(mop); return 1;
  }
  *col = '\0';
  etype = airEnumVal(pushEnergyType, str);
  if (pushEnergyTypeUnknown == etype) {
    sprintf(err, "%s: didn't recognize \"%s\" as a %s", me,
            str, pushEnergyType->name);
    biffAdd(PUSH, err); airMopError(mop); return 1;
  }

  ensp->energy = pushEnergyAll[etype];
  if (0 == ensp->energy->parmNum) {
    sprintf(err, "%s: \"%s\" energy has no parms, but got something", me,
            ensp->energy->name);
    biffAdd(PUSH, err); return 1;
  }

  _pstr = pstr = col+1;
  /* code lifted from teem/src/nrrd/kernel.c, should probably refactor... */
  for (haveParm=0; haveParm<ensp->energy->parmNum; haveParm++) {
    if (!pstr) {
      break;
    }
    if (1 != sscanf(pstr, "%lg", &pval)) {
      sprintf(err, "%s: trouble parsing \"%s\" as double (in \"%s\")",
              me, _pstr, _str);
      biffAdd(PUSH, err); airMopError(mop); return 1;
    }
    ensp->parm[haveParm] = pval;
    if ((pstr = strchr(pstr, ','))) {
      pstr++;
      if (!*pstr) {
        sprintf(err, "%s: nothing after last comma in \"%s\" (in \"%s\")",
                me, _pstr, _str);
        biffAdd(PUSH, err); airMopError(mop); return 1;
      }
    }
  }
  /* haveParm is now the number of parameters that were parsed. */
  if (haveParm < ensp->energy->parmNum) {
    sprintf(err, "%s: parsed only %u of %u required parms (for %s energy)"
            "from \"%s\" (in \"%s\")",
            me, haveParm, ensp->energy->parmNum,
            ensp->energy->name, _pstr, _str);
    biffAdd(PUSH, err); airMopError(mop); return 1;
  } else {
    if (pstr) {
      sprintf(err, "%s: \"%s\" (in \"%s\") has more than %u doubles",
              me, _pstr, _str, ensp->energy->parmNum);
      biffAdd(PUSH, err); airMopError(mop); return 1;
    }
  }
  
  airMopOkay(mop);
  return 0;
}

int
_pushHestEnergyParse(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  pushEnergySpec **enspP;
  char me[]="_pushHestForceParse", *perr;

  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  enspP = (pushEnergySpec **)ptr;
  *enspP = pushEnergySpecNew();
  if (pushEnergySpecParse(*enspP, str)) {
    perr = biffGetDone(PUSH);
    strncpy(err, perr, AIR_STRLEN_HUGE-1);
    free(perr);
    return 1;
  }
  return 0;
}

hestCB
_pushHestEnergySpec = {
  sizeof(pushEnergySpec*),
  "energy specification",
  _pushHestEnergyParse,
  (airMopper)pushEnergySpecNix
};

hestCB *
pushHestEnergySpec = &_pushHestEnergySpec;
