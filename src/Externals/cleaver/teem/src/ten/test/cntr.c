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

char *info = ("does contraction between 2 2nd-order "
              "and 1 4rth-order tensor.");

int
main(int argc, char *argv[]) {
  char *me, *err;
  hestOpt *hopt=NULL;
  airArray *mop;

  char *outS;
  Nrrd *_ncov, *ncov, *_nten[2], *nten[2], *nout;
  double *cc, *t0, *t1, *out, ww[21];
  size_t nn, ii;

  mop = airMopNew();
  me = argv[0];
  hestOptAdd(&hopt, "i4", "volume", airTypeOther, 1, 1, &_ncov, NULL,
             "4th-order tensor volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "i2", "v0 v1", airTypeOther, 2, 2, _nten, NULL,
             "two 2nd-order tensor volumes", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS, "-",
             "file to write output nrrd to");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (tenTensorCheck(_nten[0], nrrdTypeDefault, AIR_TRUE, AIR_TRUE)
      || tenTensorCheck(_nten[1], nrrdTypeDefault, AIR_TRUE, AIR_TRUE)) {
    airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: didn't like input:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  if (!(4 == _ncov->dim && 21 == _ncov->axis[0].size)) {
    fprintf(stderr, "%s: didn't get a 4-D 21-by-X volume (got %u-D %u-by-X)\n",
            me, _ncov->dim, AIR_CAST(unsigned int, _ncov->axis[0].size));
    airMopError(mop); 
    return 1;
  }
  if (!(nrrdElementNumber(_ncov)/21 == nrrdElementNumber(_nten[0])/7
        && nrrdElementNumber(_nten[0])/7 == nrrdElementNumber(_nten[1])/7)) {
    fprintf(stderr, "%s: number voxels %u %u %u don't all match\n", me,
            AIR_CAST(unsigned int, nrrdElementNumber(_ncov)/21),
            AIR_CAST(unsigned int, nrrdElementNumber(_nten[0])/7),
            AIR_CAST(unsigned int, nrrdElementNumber(_nten[1])/7));
    airMopError(mop); 
    return 1;
  }

  ncov = nrrdNew();
  nten[0] = nrrdNew();
  nten[1] = nrrdNew();
  nout = nrrdNew();
  airMopAdd(mop, ncov, (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nten[0], (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nten[1], (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(ncov, _ncov, nrrdTypeDouble)
      || nrrdConvert(nten[0], _nten[0], nrrdTypeDouble)
      || nrrdConvert(nten[1], _nten[1], nrrdTypeDouble)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble converting to %s:\n%s\n",
            me, airEnumStr(nrrdType, nrrdTypeDouble), err);
    airMopError(mop); 
    return 1;
  }
  if (nrrdSlice(nout, nten[0], 0, 0)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble prepping output:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }

  cc = AIR_CAST(double *, ncov->data);
  t0 = AIR_CAST(double *, nten[0]->data);
  t1 = AIR_CAST(double *, nten[1]->data);
  out = AIR_CAST(double *, nout->data);
  nn = nrrdElementNumber(nout);

  ww[ 0] = 1*1; ww[ 1] = 2*1; ww[ 2] = 2*1; ww[ 3] = 1*1; ww[ 4] = 2*1; ww[ 5] = 1*1; 
  /* */         ww[ 6] = 2*2; ww[ 7] = 2*2; ww[ 8] = 1*2; ww[ 9] = 2*2; ww[10] = 1*2; 
  /* */                       ww[11] = 2*2; ww[12] = 1*2; ww[13] = 2*2; ww[14] = 1*2;
  /* */                                     ww[15] = 1*1; ww[16] = 2*1; ww[17] = 1*1;
  /* */                                                   ww[18] = 2*2; ww[19] = 1*2;
  /* */                                                                 ww[20] = 1*1;
  
  /*
  for (ii=0; ii<21; ii++) {
    ww[ii] = sqrt(ww[ii]);
  }
  */

  for (ii=0; ii<nn; ii++) {
      
    out[ii] = (+ cc[ 0]*ww[ 0]*t0[1]*t1[1] + cc[ 1]*ww[ 1]*t0[2]*t1[1] + cc[ 2]*ww[ 2]*t0[3]*t1[1] + cc[ 3]*ww[ 3]*t0[4]*t1[1] + cc[ 4]*ww[ 4]*t0[5]*t1[1] + cc[ 5]*ww[ 5]*t0[6]*t1[1] + 
               + cc[ 1]*ww[ 1]*t0[1]*t1[2] + cc[ 6]*ww[ 6]*t0[2]*t1[2] + cc[ 7]*ww[ 7]*t0[3]*t1[2] + cc[ 8]*ww[ 8]*t0[4]*t1[2] + cc[ 9]*ww[ 9]*t0[5]*t1[2] + cc[10]*ww[10]*t0[6]*t1[2] + 
               + cc[ 2]*ww[ 2]*t0[1]*t1[3] + cc[ 7]*ww[ 7]*t0[2]*t1[3] + cc[11]*ww[11]*t0[3]*t1[3] + cc[12]*ww[12]*t0[4]*t1[3] + cc[13]*ww[13]*t0[5]*t1[3] + cc[14]*ww[14]*t0[6]*t1[3] + 
               + cc[ 3]*ww[ 3]*t0[1]*t1[4] + cc[ 8]*ww[ 8]*t0[2]*t1[4] + cc[12]*ww[12]*t0[3]*t1[4] + cc[15]*ww[15]*t0[4]*t1[4] + cc[16]*ww[16]*t0[5]*t1[4] + cc[17]*ww[17]*t0[6]*t1[4] + 
               + cc[ 4]*ww[ 4]*t0[1]*t1[5] + cc[ 9]*ww[ 9]*t0[2]*t1[5] + cc[13]*ww[13]*t0[3]*t1[5] + cc[16]*ww[16]*t0[4]*t1[5] + cc[18]*ww[18]*t0[5]*t1[5] + cc[19]*ww[19]*t0[6]*t1[5] + 
               + cc[ 5]*ww[ 5]*t0[1]*t1[6] + cc[10]*ww[10]*t0[2]*t1[6] + cc[14]*ww[14]*t0[3]*t1[6] + cc[17]*ww[17]*t0[4]*t1[6] + cc[19]*ww[19]*t0[5]*t1[6] + cc[20]*ww[20]*t0[6]*t1[6]);
    
    /*         0:xxxx  1:xxxy  2:xxxz  3:xxyy  4:xxyz  5:xxzz
     *                 6:xyxy  7:xyxz  8:xyyy  9:xyyz 10:xyzz
     *                        11:xzxz 12:xzyy 13:xzyz 14:xzzz
     *                                15:yyyy 16:yyyz 17:yyzz
     *                                        18:yzyz 19:yzzz
     *                                                20:zzzz */
    cc += 21;
    t0 += 7;
    t1 += 7;
  }
  
  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving output:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }

  airMopOkay(mop);
  return 0;
}
