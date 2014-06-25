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

#ifndef TEN_PRIVATE_HAS_BEEN_INCLUDED
#define TEN_PRIVATE_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define TEND_CMD(name, info) \
unrrduCmd tend_##name##Cmd = { #name, info, tend_##name##Main }

/* USAGE, PARSE: both copied verbatim from unrrdu/privateUnrrdu.h, but
** then some hacking was added ... 
*/
#define USAGE(info) \
  if (!argc) { \
    hestInfo(stdout, me, (info), hparm); \
    hestUsage(stdout, hopt, me, hparm); \
    hestGlossary(stdout, hopt, hparm); \
    airMopError(mop); \
    return 2; \
  }

/* JUSTPARSE is called by the tend functions that do *not* take an
** input 7-component tensor volume
*/
#define JUSTPARSE()                                             \
  if ((pret=hestParse(hopt, argc, argv, &perr, hparm))) {       \
    if (1 == pret) {                                            \
      fprintf(stderr, "%s: %s\n", me, perr); free(perr);        \
      hestUsage(stderr, hopt, me, hparm);                       \
      airMopError(mop);                                         \
      return 2;                                                 \
    } else {                                                    \
      /* ... like tears ... in rain. Time ... to die. */        \
      exit(1);                                                  \
    }                                                           \
  }
  
/* 
** PARSE is called by tend functions that do take a 7-component tensor 
** volume, so that as a hack, we can process 6-component volumes as well,
** by padding on the confidence channel (fixed at 1.0)
*/
#define PARSE()                                                         \
  JUSTPARSE();                                                          \
  if (4 == nin->dim                                                     \
      && 6 == nin->axis[0].size                                         \
      && nrrdTypeBlock != nin->type) {                                  \
    ptrdiff_t min[4], max[4];                                           \
    Nrrd *ntmp;                                                         \
    /* create a confidence channel by padding on 1s */                  \
    min[0] = -1; min[1] = min[2] = min[3] = 0;                          \
    max[0] = nin->axis[0].size-1;                                       \
    max[1] = nin->axis[1].size-1;                                       \
    max[2] = nin->axis[2].size-1;                                       \
    max[3] = nin->axis[3].size-1;                                       \
    ntmp = nrrdNew();                                                   \
    if (nrrdPad_nva(ntmp, nin, min, max, nrrdBoundaryPad, 1.0)          \
        || nrrdCopy(nin, ntmp)) {                                       \
      char *err;                                                        \
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);   \
      fprintf(stderr, "%s: can't pad 6-comp tensor:\n%s", me, err);     \
      airMopError(mop);                                                 \
      nrrdNuke(ntmp);                                                   \
      return 2;                                                         \
    }                                                                   \
    nrrdNuke(ntmp);                                                     \
  }
  
/* qseg.c: 2-tensor estimation */
extern void _tenQball(const double b, const int gradcount,
                      const double svals[], const double grads[],
                      double qvals[] );
extern void _tenSegsamp2(const int gradcount, const double qvals[],
                         const double grads[], const double qpoints[],
                         unsigned int seg[], double dists[] );
extern void _tenCalcdists(const int centcount, const double centroid[6],
                          const int gradcount, const double qpoints[],
                          double dists[] );
extern void _tenInitcent2(const int gradcount, const double qvals[],
                          const double grads[], double centroids[6] );
extern int _tenCalccent2(const int gradcount, const double qpoints[],
                         const double dists[], double centroid[6],
                         unsigned int seg[] );
extern void _tenSeg2weights(const int gradcount, const int seg[],
                            const int segcount, double weights[] );
extern void _tenQvals2points(const int gradcount, const double qvals[],
                             const double grads[], double qpoints[] );
extern double _tenPldist(const double point[], const double line[] );
  
/* arishFuncs.c: Arish's implementation of Peled's 2-tensor fit */
#define VEC_SIZE 3

extern void twoTensFunc(double *p, double *x, int m, int n, void *data);
extern void formTensor2D(double ten[7], double lam1, double lam3, double phi);

/* qglox.c: stuff for quaternion geodesic-loxodromes that has dubious
   utility for the general public */
extern void tenQGLInterpTwoEvalK(double oeval[3],
                                 const double evalA[3],
                                 const double evalB[3],
                                 const double tt);
extern void tenQGLInterpTwoEvalR(double oeval[3],
                                 const double evalA[3],
                                 const double evalB[3],
                                 const double tt);
extern void tenQGLInterpTwoEvec(double oevec[9],
                                const double evecA[9], const double evecB[9],
                                double tt);
extern void tenQGLInterpTwo(double oten[7],
                            const double tenA[7], const double tenB[7],
                            int ptype, double aa, tenInterpParm *tip);
extern int _tenQGLInterpNEval(double evalOut[3],
                              const double *evalIn, /* size 3 -by- NN */
                              const double *wght,   /* size NN */
                              unsigned int NN,
                              int ptype, tenInterpParm *tip);
extern int _tenQGLInterpNEvec(double evecOut[9],
                              const double *evecIn, /* size 9 -by- NN */
                              const double *wght,   /* size NN */
                              unsigned int NN,
                              tenInterpParm *tip);
extern int tenQGLInterpN(double tenOut[7],
                         const double *tenIn,
                         const double *wght, 
                         unsigned int NN, int ptype, tenInterpParm *tip);

#ifdef __cplusplus
}
#endif

#endif /* TEN_PRIVATE_HAS_BEEN_INCLUDED */
