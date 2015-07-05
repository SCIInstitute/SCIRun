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


void
tenFiberSingleInit(tenFiberSingle *tfbs) {
  /* char me[]="tenFiberSingleInit"; */
  unsigned idx;

  ELL_3V_SET(tfbs->seedPos, AIR_NAN, AIR_NAN, AIR_NAN);
  tfbs->dirIdx = tfbs->dirNum = 0;
  tfbs->nvert = nrrdNew();
  tfbs->halfLen[0] = tfbs->halfLen[1] = AIR_NAN;
  tfbs->seedIdx = tfbs->stepNum[0] = tfbs->stepNum[1] = 0;
  tfbs->whyStop[0] = tfbs->whyStop[1] = tenFiberStopUnknown;
  tfbs->whyNowhere = tenFiberStopUnknown; /* actually, the semantics of this
                                             field is reversed, so this is not
                                             really the way it should be set */
  tfbs->nval = nrrdNew();
  for (idx=0; idx<=NRRD_MEASURE_MAX; idx++) {
    tfbs->measr[idx] = AIR_NAN;
  }
  return;
}

void
tenFiberSingleDone(tenFiberSingle *tfbs) {

  tfbs->nvert = nrrdNuke(tfbs->nvert);
  tfbs->nval = nrrdNuke(tfbs->nval);
}

tenFiberSingle *
tenFiberSingleNew() {
  tenFiberSingle *ret;

  ret = AIR_CAST(tenFiberSingle *, calloc(1, sizeof(tenFiberSingle)));
  if (ret) {
    tenFiberSingleInit(ret);
  }
  return ret;
}

tenFiberSingle *
tenFiberSingleNix(tenFiberSingle *tfbs) {

  if (tfbs) {
    tenFiberSingleDone(tfbs);
    airFree(tfbs);
  }
  return NULL;
}

static
tenFiberContext *
_tenFiberContextCommonNew(const Nrrd *vol, int useDwi,
                          double thresh, double soft, double valueMin,
                          int ten1method, int ten2method) {
  char me[]="_tenFiberContextCommonNew", err[BIFF_STRLEN];
  tenFiberContext *tfx;
  gageKind *kind;

  if (!( tfx = (tenFiberContext *)calloc(1, sizeof(tenFiberContext)) )) {
    sprintf(err, "%s: couldn't allocate new context", me);
    biffAdd(TEN, err); return NULL;
  }

  if (useDwi) {
    Nrrd *ngrad=NULL, *nbmat=NULL;
    double bval=0;
    unsigned int *skip, skipNum;

    tfx->useDwi = AIR_TRUE;
    /* default fiber type */
    tfx->fiberType = tenDwiFiberTypeUnknown;
    
    if (tenDWMRIKeyValueParse(&ngrad, &nbmat, &bval, &skip, &skipNum, vol)) {
      sprintf(err, "%s: trouble parsing DWI info", me );
      biffAdd(TEN, err); return NULL;
    }
    if (skipNum) {
      sprintf(err, "%s: sorry, can't do DWI skipping here", me);
      biffAdd(TEN, err); return NULL;
    }
    kind = tenDwiGageKindNew();
    if (tenDwiGageKindSet(kind,
                          thresh, soft, bval, valueMin,
                          ngrad, NULL,
                          ten1method, ten2method, 42)) {
      sprintf(err, "%s: trouble setting DWI kind", me);
      biffAdd(TEN, err); return NULL;
    }
  } else {
    /* it should be a tensor volume */
    tfx->useDwi = AIR_FALSE;
    /* default fiber type */
    tfx->fiberType = tenFiberTypeUnknown;
    if (tenTensorCheck(vol, nrrdTypeUnknown, AIR_TRUE, AIR_TRUE)) {
      sprintf(err, "%s: didn't get a tensor volume", me);
      biffAdd(TEN, err); return NULL;
    }
    kind = tenGageKind;
  }

  if ( !(tfx->gtx = gageContextNew())
       || !(tfx->pvl = gagePerVolumeNew(tfx->gtx, vol, kind))
       || (gagePerVolumeAttach(tfx->gtx, tfx->pvl)) ) {
    sprintf(err, "%s: gage trouble", me);
    biffMove(TEN, err, GAGE); free(tfx); return NULL;
  }

  tfx->nin = vol;
  tfx->ksp = nrrdKernelSpecNew();
  if (nrrdKernelSpecParse(tfx->ksp, tenDefFiberKernel)) {
    sprintf(err, "%s: couldn't parse tenDefFiberKernel \"%s\"",
            me,  tenDefFiberKernel);
    biffMove(TEN, err, NRRD); return NULL;
  }
  if (tenFiberKernelSet(tfx, tfx->ksp->kernel, tfx->ksp->parm)) {
    sprintf(err, "%s: couldn't set default kernel", me);
    biffAdd(TEN, err); return NULL;
  }
  /* looks to GK like GK says that we must set some stop criterion */
  tfx->intg = tenDefFiberIntg;
  tfx->anisoStopType = tenDefFiberAnisoStopType;
  tfx->anisoSpeedType = tenAnisoUnknown;
  tfx->stop = 0;
  tfx->anisoThresh = tenDefFiberAnisoThresh;
  /* so I'm not using the normal default mechanism, shoot me */
  tfx->anisoSpeedFunc[0] = 0;
  tfx->anisoSpeedFunc[1] = 0;
  tfx->anisoSpeedFunc[2] = 0;
  tfx->maxNumSteps = tenDefFiberMaxNumSteps;
  tfx->minNumSteps = 0;
  tfx->useIndexSpace = tenDefFiberUseIndexSpace;
  tfx->verbose = 0;
  tfx->stepSize = tenDefFiberStepSize;
  tfx->maxHalfLen = tenDefFiberMaxHalfLen;
  tfx->minWholeLen = 0.0;
  tfx->confThresh = 0.5; /* why do I even bother setting these- they'll
                            only get read if the right tenFiberStopSet has
                            been called, in which case they'll be set... */
  tfx->minRadius = 1;    /* above lament applies here as well */
  tfx->minFraction = 0.5; /* and here */
  tfx->wPunct = tenDefFiberWPunct;

  GAGE_QUERY_RESET(tfx->query);
  tfx->mframe[0] = vol->measurementFrame[0][0];
  tfx->mframe[1] = vol->measurementFrame[1][0];
  tfx->mframe[2] = vol->measurementFrame[2][0];
  tfx->mframe[3] = vol->measurementFrame[0][1];
  tfx->mframe[4] = vol->measurementFrame[1][1];
  tfx->mframe[5] = vol->measurementFrame[2][1];
  tfx->mframe[6] = vol->measurementFrame[0][2];
  tfx->mframe[7] = vol->measurementFrame[1][2];
  tfx->mframe[8] = vol->measurementFrame[2][2];
  if (ELL_3M_EXISTS(tfx->mframe)) {
    tfx->mframeUse = AIR_TRUE;
    ELL_3M_TRANSPOSE(tfx->mframeT, tfx->mframe);
  } else {
    tfx->mframeUse = AIR_FALSE;
  }

  tfx->gageAnisoStop = NULL;
  tfx->gageAnisoSpeed = NULL;
  tfx->ten2AnisoStop = AIR_NAN;
  /* ... don't really see the point of initializing the ten2 stuff here;
     its properly done in tenFiberTraceSet() ... */
  tfx->radius = AIR_NAN;

  return tfx;
}

tenFiberContext *
tenFiberContextDwiNew(const Nrrd *dwivol,
                      double thresh, double soft, double valueMin,
                      int ten1method, int ten2method) {
  char me[]="tenFiberContextDwiNew", err[BIFF_STRLEN];
  tenFiberContext *tfx;

  if (!( tfx = _tenFiberContextCommonNew(dwivol, AIR_TRUE,
                                         thresh, soft, valueMin,
                                         ten1method, ten2method) )) {
    sprintf(err, "%s: couldn't create new context", me);
    biffAdd(TEN, err); return NULL;
  }
  return tfx;
}

tenFiberContext *
tenFiberContextNew(const Nrrd *dtvol) {
  char me[]="tenFiberContextNew", err[BIFF_STRLEN];
  tenFiberContext *tfx;

  if (!( tfx = _tenFiberContextCommonNew(dtvol, AIR_FALSE,
                                         AIR_NAN, AIR_NAN, AIR_NAN,
                                         tenEstimate1MethodUnknown,
                                         tenEstimate2MethodUnknown) )) {
    sprintf(err, "%s: couldn't create new context", me);
    biffAdd(TEN, err); return NULL;
  }

  return tfx;
}

void
tenFiberVerboseSet(tenFiberContext *tfx, int verbose) {

  if (tfx) {
    tfx->verbose = verbose;
  }
  return;
}

int
tenFiberTypeSet(tenFiberContext *tfx, int ftype) {
  char me[]="tenFiberTypeSet", err[BIFF_STRLEN];

  if (!tfx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tfx->useDwi) {
    fprintf(stderr, "!%s(%d)--- hello\n", me, ftype);
    switch (ftype) {
    case tenDwiFiberType1Evec0:
      GAGE_QUERY_ITEM_ON(tfx->query, tenDwiGageTensorLLS);
      tfx->gageTen = gageAnswerPointer(tfx->gtx, tfx->pvl,
                                       tenDwiGageTensorLLS);
      tfx->gageTen2 = NULL;
      break;
    case tenDwiFiberType2Evec0:
      GAGE_QUERY_ITEM_ON(tfx->query, tenDwiGage2TensorPeled);
      tfx->gageTen = NULL;
      tfx->gageTen2 = gageAnswerPointer(tfx->gtx, tfx->pvl,
                                        tenDwiGage2TensorPeled);
      break;
    case tenDwiFiberType12BlendEvec0:
      GAGE_QUERY_ITEM_ON(tfx->query, tenDwiGageTensorLLS);
      tfx->gageTen = gageAnswerPointer(tfx->gtx, tfx->pvl,
                                       tenDwiGageTensorLLS);
      GAGE_QUERY_ITEM_ON(tfx->query, tenDwiGage2TensorPeled);
      tfx->gageTen2 = gageAnswerPointer(tfx->gtx, tfx->pvl,
                                        tenDwiGage2TensorPeled);
      break;
    default:
      sprintf(err, "%s: unimplemented %s %d", me,
              tenDwiFiberType->name, ftype); 
      biffAdd(TEN, err); return 1;
      break;
    }
    tfx->gageEval = NULL;
    tfx->gageEvec = NULL;
  } else {
    /* working with tensor volume */
    switch(ftype) {
    case tenFiberTypeEvec0:
      GAGE_QUERY_ITEM_ON(tfx->query, tenGageEvec0);
      /* HEY: COPY AND PASTE */
      tfx->gageEvec 
        = gageAnswerPointer(tfx->gtx, tfx->pvl,
                            (tenFiberTypeEvec0 == tfx->fiberType
                             ? tenGageEvec0
                             : (tenFiberTypeEvec1 == tfx->fiberType
                                ? tenGageEvec1
                                : tenGageEvec2)));
      break;
    case tenFiberTypeEvec1:
      GAGE_QUERY_ITEM_ON(tfx->query, tenGageEvec1);
      /* HEY: COPY AND PASTE */
      tfx->gageEvec 
        = gageAnswerPointer(tfx->gtx, tfx->pvl,
                            (tenFiberTypeEvec0 == tfx->fiberType
                             ? tenGageEvec0
                             : (tenFiberTypeEvec1 == tfx->fiberType
                                ? tenGageEvec1
                                : tenGageEvec2)));
      break;
    case tenFiberTypeEvec2:
      GAGE_QUERY_ITEM_ON(tfx->query, tenGageEvec2);
      /* HEY: COPY AND PASTE */
      tfx->gageEvec 
        = gageAnswerPointer(tfx->gtx, tfx->pvl,
                            (tenFiberTypeEvec0 == ftype
                             ? tenGageEvec0
                             : (tenFiberTypeEvec1 == ftype
                                ? tenGageEvec1
                                : tenGageEvec2)));
      break;
    case tenFiberTypeTensorLine:
      GAGE_QUERY_ITEM_ON(tfx->query, tenGageTensor);
      GAGE_QUERY_ITEM_ON(tfx->query, tenGageEval0);
      GAGE_QUERY_ITEM_ON(tfx->query, tenGageEval1);
      GAGE_QUERY_ITEM_ON(tfx->query, tenGageEval2);
      GAGE_QUERY_ITEM_ON(tfx->query, tenGageEvec0);
      GAGE_QUERY_ITEM_ON(tfx->query, tenGageEvec1);
      GAGE_QUERY_ITEM_ON(tfx->query, tenGageEvec2);
      tfx->gageEvec = gageAnswerPointer(tfx->gtx, tfx->pvl, tenGageEvec0);
      tfx->gageTen = gageAnswerPointer(tfx->gtx, tfx->pvl, tenGageTensor);
      tfx->gageEval = gageAnswerPointer(tfx->gtx, tfx->pvl, tenGageEval);
      break;
    case tenFiberTypePureLine:
      GAGE_QUERY_ITEM_ON(tfx->query, tenGageTensor);
      break;
    case tenFiberTypeZhukov:
      sprintf(err, "%s: sorry, Zhukov oriented tensors not implemented", me);
      biffAdd(TEN, err); return 1;
      break;
    default:
      sprintf(err, "%s: fiber type %d not recognized", me, ftype);
      biffAdd(TEN, err); return 1;
      break;
    }  /* switch */
    if (tenFiberTypeEvec0 == ftype
        || tenFiberTypeEvec1 == ftype
        || tenFiberTypeEvec2 == ftype
        || tenFiberTypeTensorLine == ftype) {
      tfx->gageTen = gageAnswerPointer(tfx->gtx, tfx->pvl, tenGageTensor);
      tfx->gageEval = gageAnswerPointer(tfx->gtx, tfx->pvl, tenGageEval0);
      tfx->gageEvec 
        = gageAnswerPointer(tfx->gtx, tfx->pvl,
                            (tenFiberTypeEvec0 == ftype
                             ? tenGageEvec0
                             : (tenFiberTypeEvec1 == ftype
                                ? tenGageEvec1
                                : (tenFiberTypeEvec2 == ftype
                                   ? tenGageEvec2
                                   : tenGageEvec))));
      tfx->gageTen2 = NULL;
    }
    tfx->ten2Which = 0;
  }
  tfx->fiberType = ftype;

  return 0;
}

/*
******** tenFiberStopSet
**
** how to set stop criteria and their parameters.  a little tricky because
** of the use of varargs
**
** valid calls:
** tenFiberStopSet(tfx, tenFiberStopLength, double max)
** tenFiberStopSet(tfx, tenFiberStopMinLength, double min)
** tenFiberStopSet(tfx, tenFiberStopAniso, int anisoType, double anisoThresh)
** tenFiberStopSet(tfx, tenFiberStopNumSteps, unsigned int num)
** tenFiberStopSet(tfx, tenFiberStopMinNumSteps, unsigned int num)
** tenFiberStopSet(tfx, tenFiberStopConfidence, double conf)
** tenFiberStopSet(tfx, tenFiberStopRadius, double radius)
** tenFiberStopSet(tfx, tenFiberStopBounds)
** tenFiberStopSet(tfx, tenFiberStopFraction, double fraction)
** tenFiberStopSet(tfx, tenFiberStopStub)
*/
int
tenFiberStopSet(tenFiberContext *tfx, int stop, ...) {
  char me[]="tenFiberStopSet", err[BIFF_STRLEN];
  va_list ap;
  int ret=0;
  int anisoGage;

  if (!tfx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  va_start(ap, stop);
  switch(stop) {
  case tenFiberStopAniso:
    tfx->anisoStopType = va_arg(ap, int);
    tfx->anisoThresh = va_arg(ap, double);
    if (!(AIR_IN_OP(tenAnisoUnknown, tfx->anisoStopType, tenAnisoLast))) {
      sprintf(err, "%s: given aniso stop type %d not valid", me,
              tfx->anisoStopType);
      biffAdd(TEN, err); ret = 1; goto end;
    }
    if (!(AIR_EXISTS(tfx->anisoThresh))) {
      sprintf(err, "%s: given aniso threshold doesn't exist", me);
      biffAdd(TEN, err); ret = 1; goto end;
    }
    if (tfx->useDwi) {
      /* the tensor of which we measure anisotropy can come from lots of
         places, not just a 1-tensor gage item, so there's no specific 
         item to turn on here... */
      tfx->gageAnisoStop = NULL;
    } else { /* using tensors */
      switch(tfx->anisoStopType) {
      case tenAniso_FA:
        anisoGage = tenGageFA;
        break;
      case tenAniso_Cl1:
        anisoGage = tenGageCl1;
        break;
      case tenAniso_Cp1:
        anisoGage = tenGageCp1;
        break;
      case tenAniso_Ca1:
        anisoGage = tenGageCa1;
        break;
      case tenAniso_Clpmin1:
        anisoGage = tenGageClpmin1;
        break;
      case tenAniso_Cl2:
        anisoGage = tenGageCl2;
        break;
      case tenAniso_Cp2:
        anisoGage = tenGageCp2;
        break;
      case tenAniso_Ca2:
        anisoGage = tenGageCa2;
        break;
      case tenAniso_Clpmin2:
        anisoGage = tenGageClpmin2;
        break;
      default:
        sprintf(err, "%s: sorry, currently don't have fast %s computation "
                "via gage", me, airEnumStr(tenAniso, tfx->anisoStopType));
        biffAdd(TEN, err); ret = 1; goto end;
        break;
      }
      /* NOTE: we are no longer computing ALL anisotropy measures ...
         GAGE_QUERY_ITEM_ON(tfx->query, tenGageAniso);
      */
      GAGE_QUERY_ITEM_ON(tfx->query, anisoGage);
      tfx->gageAnisoStop = gageAnswerPointer(tfx->gtx, tfx->pvl, anisoGage);
      /*
      fprintf(stderr, "!%s: stopping on aniso %s < %g\n", me,
              airEnumStr(tenAniso, tfx->anisoStopType), tfx->anisoThresh);
      */
    }
    break;
  case tenFiberStopLength:
    tfx->maxHalfLen = va_arg(ap, double);
    if (!( AIR_EXISTS(tfx->maxHalfLen) && tfx->maxHalfLen > 0.0 )) {
      sprintf(err, "%s: given maxHalfLen %g doesn't exist or isn't > 0.0",
              me, tfx->maxHalfLen);
      biffAdd(TEN, err); ret = 1; goto end;
    }
    /* no query modifications needed */
    break;
  case tenFiberStopMinLength:
    tfx->minWholeLen = va_arg(ap, double);
    if (!( AIR_EXISTS(tfx->minWholeLen) && tfx->minWholeLen >= 0.0 )) {
      sprintf(err, "%s: given minWholeLen %g doesn't exist or isn't >= 0.0",
              me, tfx->minWholeLen);
      biffAdd(TEN, err); ret = 1; goto end;
    }
    /* no query modifications needed */
    break;
  case tenFiberStopNumSteps:
    tfx->maxNumSteps = va_arg(ap, unsigned int);
    if (!( tfx->maxNumSteps > 0 )) {
      sprintf(err, "%s: given maxNumSteps isn't > 0.0", me);
      biffAdd(TEN, err); ret = 1; goto end;
    }
    /* no query modifications needed */
    break;
  case tenFiberStopMinNumSteps:
    tfx->minNumSteps = va_arg(ap, unsigned int);
    /* no query modifications needed */
    break;
  case tenFiberStopConfidence:
    tfx->confThresh = va_arg(ap, double);
    if (!( AIR_EXISTS(tfx->confThresh) )) {
      sprintf(err, "%s: given confThresh doesn't exist", me);
      biffAdd(TEN, err); ret = 1; goto end;
    }
    GAGE_QUERY_ITEM_ON(tfx->query, tenGageTensor);
    break;
  case tenFiberStopRadius:
    tfx->minRadius = va_arg(ap, double);
    if (!( AIR_EXISTS(tfx->minRadius) )) {
      sprintf(err, "%s: given minimum radius doesn't exist", me);
      biffAdd(TEN, err); ret = 1; goto end;
    }
    /* no query modifications needed */
    break;
  case tenFiberStopBounds:
    /* nothing to set; always used as a stop criterion */
    break;
  case tenFiberStopFraction:
    if (!tfx->useDwi) {
      sprintf(err, "%s: can only use %s-based termination in DWI tractography",
              me, airEnumStr(tenFiberStop, tenFiberStopFraction));
      biffAdd(TEN, err); ret = 1; goto end;
    }
    tfx->minFraction = va_arg(ap, double);
    if (!( AIR_EXISTS(tfx->minFraction) )) {
      sprintf(err, "%s: given minimum fraction doesn't exist", me);
      biffAdd(TEN, err); ret = 1; goto end;
    }
    /* no query modifications needed */
    break;
  case tenFiberStopStub:
    /* no var-args to grab */
    /* no query modifications needed */
    break;
  default:
    sprintf(err, "%s: stop criterion %d not recognized", me, stop);
    biffAdd(TEN, err); ret = 1; goto end;
  }
  tfx->stop = tfx->stop | (1 << stop);
 end:
  va_end(ap);
  return ret;
}

/* to avoid var-args */
int
tenFiberStopAnisoSet(tenFiberContext *tfx, int anisoType, double anisoThresh) {
  char me[]="tenFiberStopAnisoSet", err[BIFF_STRLEN];

  if (tenFiberStopSet(tfx, tenFiberStopAniso, anisoType, anisoThresh)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

/* to avoid var-args */
int
tenFiberStopDoubleSet(tenFiberContext *tfx, int stop, double val) {
  char me[]="tenFiberStopDoubleSet", err[BIFF_STRLEN];

  switch (stop) {
  case tenFiberStopLength:
  case tenFiberStopMinLength:
  case tenFiberStopConfidence:
  case tenFiberStopRadius:
  case tenFiberStopFraction:
    if (tenFiberStopSet(tfx, stop, val)) {
      sprintf(err, "%s: trouble", me);
      biffAdd(TEN, err); return 1;
    }
    break;
  default:
    sprintf(err, "%s: given stop criterion %d (%s) isn't a double", me,
            stop, airEnumStr(tenFiberStop, stop));
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

/* to avoid var-args */
int
tenFiberStopUIntSet(tenFiberContext *tfx, int stop, unsigned int val) {
  char me[]="tenFiberStopUIntSet", err[BIFF_STRLEN];

  switch (stop) {
  case tenFiberStopNumSteps:
  case tenFiberStopMinNumSteps:
    if (tenFiberStopSet(tfx, stop, val)) {
      sprintf(err, "%s: trouble", me);
      biffAdd(TEN, err); return 1;
    }
    break;
  default:
    sprintf(err, "%s: given stop criterion %d (%s) isn't an unsigned int", me,
            stop, airEnumStr(tenFiberStop, stop));
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

void
tenFiberStopOn(tenFiberContext *tfx, int stop) {

  if (tfx && !airEnumValCheck(tenFiberStop, stop)) {
    tfx->stop = tfx->stop | (1 << stop);
  }
  return;
}

void
tenFiberStopOff(tenFiberContext *tfx, int stop) {

  if (tfx && !airEnumValCheck(tenFiberStop, stop)) {
    tfx->stop = tfx->stop & ~(1 << stop);
  }
  return;
}

void
tenFiberStopReset(tenFiberContext *tfx) {

  if (tfx) {
    tfx->stop = 0;
  }
  return;
}

int
tenFiberAnisoSpeedSet(tenFiberContext *tfx, int aniso,
                      double lerp, double thresh, double soft) {
  char me[]="tenFiberAnisoSpeedSet", err[BIFF_STRLEN];
  int anisoGage;

  if (!tfx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  
  if (tfx->useDwi) {
    fprintf(stderr, "!%s: sorry, can't yet work on DWIs; bye.\n", me);
    exit(1);
  }
  
  if (airEnumValCheck(tenAniso, aniso)) {
    sprintf(err, "%s: aniso %d not valid", me, aniso);
    biffAdd(TEN, err); return 1;
  }
  switch(aniso) {
  case tenAniso_FA:
    anisoGage = tenGageFA;
    break;
  case tenAniso_Cl1:
    anisoGage = tenGageCl1;
    break;
  case tenAniso_Cp1:
    anisoGage = tenGageCp1;
    break;
  case tenAniso_Ca1:
    anisoGage = tenGageCa1;
    break;
  case tenAniso_Cl2:
    anisoGage = tenGageCl2;
    break;
  case tenAniso_Cp2:
    anisoGage = tenGageCp2;
    break;
  case tenAniso_Ca2:
    anisoGage = tenGageCa2;
    break;
  default:
    sprintf(err, "%s: sorry, currently don't have fast %s computation "
            "via gage", me, airEnumStr(tenAniso, tfx->anisoStopType));
    biffAdd(TEN, err); return 1;
    break;
  }
  tfx->anisoSpeedType = aniso;
  if (tfx->useDwi) {
    /* actually, finding anisotropy in the context of 2-tensor
       tracking is not currently done by gage */
  } else {
    GAGE_QUERY_ITEM_ON(tfx->query, anisoGage);
    tfx->gageAnisoSpeed = gageAnswerPointer(tfx->gtx, tfx->pvl, anisoGage);
  }
  tfx->anisoSpeedFunc[0] = lerp;
  tfx->anisoSpeedFunc[1] = thresh;
  tfx->anisoSpeedFunc[2] = soft;

  return 0;
}

int
tenFiberAnisoSpeedReset(tenFiberContext *tfx) {
  char me[]="tenFiberAnisoSpeedReset", err[BIFF_STRLEN];

  if (!tfx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  tfx->anisoSpeedType = tenAnisoUnknown;
  /* HEY: GAGE_QUERY_ITEM_OFF something? */
  /* HEY: for both tensor and DWI */
  tfx->gageAnisoSpeed = NULL;
  return 0;
}

int
tenFiberKernelSet(tenFiberContext *tfx,
                  const NrrdKernel *kern,
                  const double parm[NRRD_KERNEL_PARMS_NUM]) {
  char me[]="tenFiberKernelSet", err[BIFF_STRLEN];

  if (!(tfx && kern)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  nrrdKernelSpecSet(tfx->ksp, kern, parm);
  if (gageKernelSet(tfx->gtx, gageKernel00,
                    tfx->ksp->kernel, tfx->ksp->parm)) {
    sprintf(err, "%s: problem setting kernel", me);
    biffMove(TEN, err, GAGE); return 1;
  }

  return 0;
}

int
tenFiberIntgSet(tenFiberContext *tfx, int intg) {
  char me[]="tenFiberIntTypeSet", err[BIFF_STRLEN];

  if (!(tfx)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( AIR_IN_OP(tenFiberIntgUnknown, intg, tenFiberIntgLast) )) {
    sprintf(err, "%s: got invalid integration type %d", me, intg);
    biffAdd(TEN, err); return 1;
  }
  tfx->intg = intg;

  return 0;
}

int
tenFiberParmSet(tenFiberContext *tfx, int parm, double val) {
  char me[]="tenFiberParmSet";

  if (tfx) {
    switch(parm) {
    case tenFiberParmStepSize:
      tfx->stepSize = val;
      break;
    case tenFiberParmUseIndexSpace:
      tfx->useIndexSpace = !!val;
      break;
    case tenFiberParmWPunct:
      tfx->wPunct = val;
      break;
    case tenFiberParmVerbose:
      tfx->verbose = AIR_CAST(int, val);
      break;
    default:
      fprintf(stderr, "%s: WARNING!!! tenFiberParm %d not handled\n",
              me, parm);
      break;
    }
  }
  return 0;
}

int
tenFiberUpdate(tenFiberContext *tfx) {
  char me[]="tenFiberUpdate", err[BIFF_STRLEN];

  if (!tfx) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenFiberTypeUnknown == tfx->fiberType) {
    sprintf(err, "%s: fiber type not set", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( AIR_IN_OP(tenFiberTypeUnknown, tfx->fiberType, tenFiberTypeLast) )) {
    sprintf(err, "%s: tfx->fiberType set to bogus value (%d)", me,
            tfx->fiberType);
    biffAdd(TEN, err); return 1;
  }
  if (tenFiberIntgUnknown == tfx->intg) {
    sprintf(err, "%s: integration type not set", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( AIR_IN_OP(tenFiberIntgUnknown, tfx->intg, tenFiberIntgLast) )) {
    sprintf(err, "%s: tfx->intg set to bogus value (%d)", me, tfx->intg);
    biffAdd(TEN, err); return 1;
  }
  if (0 == tfx->stop) {
    sprintf(err, "%s: no fiber stopping criteria set", me);
    biffAdd(TEN, err); return 1;
  }
  if (gageQuerySet(tfx->gtx, tfx->pvl, tfx->query)
      || gageUpdate(tfx->gtx)) {
    sprintf(err, "%s: trouble with gage", me);
    biffMove(TEN, err, GAGE); return 1;
  }
  if (tfx->useDwi) {
    if (!(0 == tfx->ten2Which || 1 == tfx->ten2Which)) {
      sprintf(err, "%s: ten2Which must be 0 or 1 (not %u)",
              me, tfx->ten2Which);
      biffAdd(TEN, err); return 1;
    }
  }
  return 0;
}

/*
** exact same precautions about utility of this as with gageContextCopy!!!
** So: only after tenFiberUpdate, and don't touch anything, and don't
** call anything except tenFiberTrace and tenFiberContextNix
*/
tenFiberContext *
tenFiberContextCopy(tenFiberContext *oldTfx) {
  char me[]="tenFiberContextCopy";
  tenFiberContext *tfx;

  if (oldTfx->useDwi) {
    fprintf(stderr, "!%s: sorry, can't copy DWI contexts; bye.\n", me);
    exit(1);
  }
  tfx = (tenFiberContext *)calloc(1, sizeof(tenFiberContext));
  memcpy(tfx, oldTfx, sizeof(tenFiberContext));
  tfx->ksp = nrrdKernelSpecCopy(oldTfx->ksp);
  tfx->gtx = gageContextCopy(oldTfx->gtx);
  tfx->pvl = tfx->gtx->pvl[0];  /* HEY! gage API sucks */
  tfx->gageTen = gageAnswerPointer(tfx->gtx, tfx->pvl, tenGageTensor);
  tfx->gageEval = gageAnswerPointer(tfx->gtx, tfx->pvl, tenGageEval0);
  /* HEY: COPY AND PASTE */
  tfx->gageEvec 
    = gageAnswerPointer(tfx->gtx, tfx->pvl,
                        (tenFiberTypeEvec0 == tfx->fiberType
                         ? tenGageEvec0
                         : (tenFiberTypeEvec1 == tfx->fiberType
                            ? tenGageEvec1
                            : tenGageEvec2)));
  tfx->gageAnisoStop = gageAnswerPointer(tfx->gtx, tfx->pvl,
                                         tfx->anisoStopType);
  tfx->gageAnisoSpeed = (tfx->anisoSpeedType
                         ? gageAnswerPointer(tfx->gtx, tfx->pvl,
                                             tfx->anisoSpeedType)
                         : NULL);
  return tfx;
}

tenFiberContext *
tenFiberContextNix(tenFiberContext *tfx) {

  if (tfx) {
    tfx->ksp = nrrdKernelSpecNix(tfx->ksp);
    tfx->gtx = gageContextNix(tfx->gtx);
    free(tfx);
  }
  return NULL;
}
