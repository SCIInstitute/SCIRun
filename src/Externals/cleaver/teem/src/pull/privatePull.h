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

#ifdef __cplusplus
extern "C" {
#endif

/* this has to be big enough to do experiments where binning is turned off */
#define _PULL_NEIGH_MAXNUM 4096

#define _PULL_IMPROV(ell, enn) (2*((ell) - (enn))                   \
                                 / (AIR_ABS(ell) + AIR_ABS(enn)))
/* volumePull.c */
extern pullVolume *_pullVolumeCopy(pullVolume *pvol);
extern int _pullVolumeSetup(pullContext *pctx);

/* infoPull.c */
extern unsigned int _pullInfoAnswerLen[PULL_INFO_MAX+1];
extern void (*_pullInfoAnswerCopy[10])(double *, const double *);
extern int _pullInfoSetup(pullContext *pctx);

/* contextPull.c */
extern int _pullContextCheck(pullContext *pctx);

/* taskPull.c */
extern pullTask *_pullTaskNew(pullContext *pctx, int threadIdx);
extern pullTask *_pullTaskNix(pullTask *task);
extern int _pullTaskSetup(pullContext *pctx);
extern void _pullTaskFinish(pullContext *pctx);

/* actionPull.c */
extern int _pullPraying;
extern double _pullPrayCorner[2][2][3];
extern size_t _pullPrayRes[2];
extern double _pullDistLimit(pullTask *task, pullPoint *point);

/* constraints.c */
extern int _pullConstraintSatisfy(pullTask *task, pullPoint *point,
                                  int *constrFailP);
extern void _pullConstraintTangent(pullTask *task, pullPoint *point, 
                                   double proj[9]);

/* pointPull.c */
extern double _pullPointScalar(const pullContext *pctx,
                               const pullPoint *point, int sclInfo,
                               double grad[4], double hess[9]);
extern void _pullPointCopy(pullPoint *dst, const pullPoint *src,
                           unsigned int ilen);
extern void _pullPointHistInit(pullPoint *point);
extern void _pullPointHistAdd(pullPoint *point, int cond);
extern void _pullPointNixMeRemove(pullContext *pctx);
extern unsigned int _pullPointNumber(const pullContext *pctx);
extern double _pullStepInterAverage(const pullContext *pctx);
extern double _pullStepConstrAverage(const pullContext *pctx);
extern double _pullEnergyTotal(const pullContext *pctx);
extern double _pullPointEnergyTotal(pullTask *task, pullBin *bin,
                                    pullPoint *point,
                                    double force[4]);
extern int _pullProbe(pullTask *task, pullPoint *point);
extern void _pullPointStepEnergyScale(pullContext *pctx, double scale);
extern int _pullPointSetup(pullContext *pctx);
extern void _pullPointFinish(pullContext *pctx);

/* binningPull.c */
extern void _pullBinInit(pullBin *bin, unsigned int incr);
extern void _pullBinDone(pullBin *bin);
extern pullBin *_pullBinLocate(pullContext *pctx, double *pos);
extern void _pullBinPointAdd(pullContext *pctx,
                             pullBin *bin, pullPoint *point);
extern void _pullBinPointRemove(pullContext *pctx, pullBin *bin, int loseIdx);
extern void _pullBinNeighborSet(pullBin *bin, pullBin **nei, unsigned int num);
extern int _pullBinSetup(pullContext *pctx);
extern void _pullBinFinish(pullContext *pctx);

/* corePull.c */
extern int _pullProcess(pullTask *task);
extern void *_pullWorker(void *_task);

#ifdef __cplusplus
}
#endif
