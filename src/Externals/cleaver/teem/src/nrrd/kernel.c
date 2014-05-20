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

#include "nrrd.h"

/*   
** summary of information about how the kernel parameter vector is set:

                             numParm  parm[0]   parm[1]   parm[2]
             nrrdKernelHann     2      scale    cut-off
         nrrdKernelBlackman     2      scale    cut-off
             nrrdKernelZero     1      scale
              nrrdKernelBox     1      scale
            nrrdKernelCheap     1      scale
      nrrdKernelHermiteFlag     0
             nrrdKernelTent     1      scale
         nrrdKernelForwDiff     1      scale
         nrrdKernelCentDiff     1      scale
          nrrdKernelBCCubic     3      scale       B        C
         nrrdKernelAQuartic     2      scale       A
        nrrdKernelC3Quintic     1      scale
          nrrdKernelC4Hexic     1      scale
         nrrdKernelGaussian     2      sigma    cut-off
 nrrdKernelDiscreteGaussian     2      sigma    cut-off
        nrrdKernelTMF[][][]     1       a

*/

/* ------------------------------------------------------------ */

/* the "zero" kernel is here more as a template than for anything else */

#define _ZERO(x) 0

double
_nrrdZeroInt(const double *parm) {

  AIR_UNUSED(parm);
  return 0.0;
}

double
_nrrdZeroSup(const double *parm) {
  double S;
  
  S = parm[0];
  return S;
}

double
_nrrdZero1_d(double x, const double *parm) {
  double S;

  S = parm[0];
  x = AIR_ABS(x)/S;
  return _ZERO(x)/S;
}

float
_nrrdZero1_f(float x, const double *parm) {
  float S;

  S = AIR_CAST(float, parm[0]);
  x = AIR_ABS(x)/S;
  return _ZERO(x)/S;
}

void
_nrrdZeroN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t;
  size_t i;
  
  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t)/S;
    f[i] = _ZERO(t)/S;
  }
}

void
_nrrdZeroN_f(float *f, const float *x, size_t len, const double *parm) {
  float t, S;
  size_t i;
  
  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t)/S;
    f[i] = _ZERO(t)/S;
  }
}

NrrdKernel
_nrrdKernelZero = {
  "zero",
  1, _nrrdZeroSup, _nrrdZeroInt,
  _nrrdZero1_f, _nrrdZeroN_f, _nrrdZero1_d, _nrrdZeroN_d
};
NrrdKernel *const 
nrrdKernelZero = &_nrrdKernelZero;

/* ------------------------------------------------------------ */

#define _BOX(x) (x > 0.5 ? 0 : (x < 0.5 ? 1 : 0.5))

double
_nrrdBoxInt(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double
_nrrdBoxSup(const double *parm) {
  double S;
  
  S = parm[0];
  /* adding the 0.5 is to insure that weights computed within the
     support really do catch all the non-zero values */
  return S/2 + 0.5;
}

double
_nrrdBox1_d(double x, const double *parm) {
  double S;

  S = parm[0];
  x = AIR_ABS(x)/S;
  return _BOX(x)/S;
}

float
_nrrdBox1_f(float x, const double *parm) {
  float S;

  S = AIR_CAST(float, parm[0]);
  x = AIR_ABS(x)/S;
  return AIR_CAST(float, _BOX(x)/S);
}

void
_nrrdBoxN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t;
  size_t i;
  
  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t)/S;
    f[i] = _BOX(t)/S;
  }
}

void
_nrrdBoxN_f(float *f, const float *x, size_t len, const double *parm) {
  float t, S;
  size_t i;
  
  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t)/S;
    f[i] = AIR_CAST(float, _BOX(t)/S);
  }
}

NrrdKernel
_nrrdKernelBox = {
  "box",
  1, _nrrdBoxSup, _nrrdBoxInt,  
  _nrrdBox1_f,  _nrrdBoxN_f,  _nrrdBox1_d,  _nrrdBoxN_d
};
NrrdKernel *const
nrrdKernelBox = &_nrrdKernelBox;

/* ------------------------------------------------------------ */

/* The point here is that post-kernel-evaluation, we need to see
   which sample is closest to the origin, and this is one way of
   enabling that */
#define _CHEAP(x) AIR_ABS(x)

double
_nrrdCheapInt(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double
_nrrdCheapSup(const double *parm) {
  double S;
  
  S = parm[0];
  /* adding the 0.5 is to insure that weights computed within the
     support really do catch all the non-zero values */
  return S/2 + 0.5;
}

double
_nrrdCheap1_d(double x, const double *parm) {

  return _CHEAP(x)/parm[0];
}

float
_nrrdCheap1_f(float x, const double *parm) {

  return AIR_CAST(float, _CHEAP(x)/parm[0]);
}

void
_nrrdCheapN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;
  
  for (i=0; i<len; i++) {
    t = x[i];
    f[i] = _CHEAP(t)/parm[0];
  }
}

void
_nrrdCheapN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;
  
  for (i=0; i<len; i++) {
    t = x[i];
    f[i] = AIR_CAST(float, _CHEAP(t)/parm[0]);
  }
}

NrrdKernel
_nrrdKernelCheap = {
  "cheap",
  1, _nrrdCheapSup, _nrrdCheapInt,  
  _nrrdCheap1_f,  _nrrdCheapN_f,  _nrrdCheap1_d,  _nrrdCheapN_d
};
NrrdKernel *const
nrrdKernelCheap = &_nrrdKernelCheap;

/* ------------------------------------------------------------ */

#define _TENT(x) (x >= 1 ? 0 : 1 - x)

double
_nrrdTentInt(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double
_nrrdTentSup(const double *parm) {
  double S;
  
  S = parm[0];
  return S;
}

double
_nrrdTent1_d(double x, const double *parm) {
  double S;
  
  S = parm[0];
  x = AIR_ABS(x)/S;
  return S ? _TENT(x)/S : x == 0;
}

float
_nrrdTent1_f(float x, const double *parm) {
  float S;
  
  S = AIR_CAST(float, parm[0]);
  x = AIR_ABS(x)/S;
  return S ? _TENT(x)/S : x == 0;
}

void
_nrrdTentN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t;
  size_t i;
  
  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t)/S;
    f[i] = S ? _TENT(t)/S : t == 0;
  }
}

void
_nrrdTentN_f(float *f, const float *x, size_t len, const double *parm) {
  float t, S;
  size_t i;
  
  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t)/S;
    f[i] = S ? _TENT(t)/S : t == 0;
  }
}

NrrdKernel
_nrrdKernelTent = {
  "tent",
  1, _nrrdTentSup,_nrrdTentInt, 
  _nrrdTent1_f, _nrrdTentN_f, _nrrdTent1_d, _nrrdTentN_d
};
NrrdKernel *const
nrrdKernelTent = &_nrrdKernelTent;

/* ------------------------------------------------------------ */

/* 
** NOTE: THERE IS NOT REALLY A HERMITE KERNEL (at least not yet,
** because it takes both values and derivatives as arguments, which
** the NrrdKernel currently can't handle).  This isn't really a
** kernel, its mostly a flag (hence the name), but it also has the
** role of generating weights according to linear interpolation, which
** is useful for the eventual spline evaluation.
** 
** This hack is in sinister collusion with gage, to enable Hermite
** interpolation for its stack reconstruction.
*/

static double
_nrrdHermiteInt(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

static double
_nrrdHermiteSup(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

static double
_nrrdHermite1_d(double x, const double *parm) {
  AIR_UNUSED(parm);
  
  x = AIR_ABS(x);
  return _TENT(x);
}

static float
_nrrdHermite1_f(float x, const double *parm) {
  AIR_UNUSED(parm);
  
  x = AIR_ABS(x);
  return _TENT(x);
}

static void
_nrrdHermiteN_d(double *f, const double *x, size_t len, const double *parm) {
  double t;
  size_t i;
  AIR_UNUSED(parm);
  
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t);
    f[i] = _TENT(t);
  }
}

static void
_nrrdHermiteN_f(float *f, const float *x, size_t len, const double *parm) {
  float t;
  size_t i;
  AIR_UNUSED(parm);
  
  for (i=0; i<len; i++) {
    t = x[i]; t = AIR_ABS(t);
    f[i] = _TENT(t);
  }
}

NrrdKernel
_nrrdKernelHermiteFlag = {
  "hermiteFlag",
  0, _nrrdHermiteSup,_nrrdHermiteInt, 
  _nrrdHermite1_f, _nrrdHermiteN_f, _nrrdHermite1_d, _nrrdHermiteN_d
};
NrrdKernel *const
nrrdKernelHermiteFlag = &_nrrdKernelHermiteFlag;

/* ------------------------------------------------------------ */

#define _FORDIF(x) (x < -1 ?  0 :        \
                   (x <  0 ?  1 :        \
                   (x <  1 ? -1 : 0 )))

double
_nrrdFDInt(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double
_nrrdFDSup(const double *parm) {
  double S;
  
  S = parm[0];
  return S+0.0001;  /* sigh */
}

double
_nrrdFD1_d(double x, const double *parm) {
  double S;
  
  S = parm[0];
  x /= S;
  return _FORDIF(x)/(S*S);
}

float
_nrrdFD1_f(float x, const double *parm) {
  float S;
  
  S = AIR_CAST(float, parm[0]);
  x /= S;
  return _FORDIF(x)/(S*S);
}

void
_nrrdFDN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t;
  size_t i;
  
  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i]/S;
    f[i] = _FORDIF(t)/(S*S);
  }
}

void
_nrrdFDN_f(float *f, const float *x, size_t len, const double *parm) {
  float t, S;
  size_t i;
  
  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i]/S;
    f[i] = _FORDIF(t)/(S*S);
  }
}

NrrdKernel
_nrrdKernelFD = {
  "fordif",
  1, _nrrdFDSup,  _nrrdFDInt,   
  _nrrdFD1_f,   _nrrdFDN_f,   _nrrdFD1_d,   _nrrdFDN_d
};
NrrdKernel *const
nrrdKernelForwDiff = &_nrrdKernelFD;

/* ------------------------------------------------------------ */

#define _CENDIF(x) (x <= -2 ?  0         :        \
                   (x <= -1 ?  0.5*x + 1 :        \
                   (x <=  1 ? -0.5*x     :        \
                   (x <=  2 ?  0.5*x - 1 : 0 ))))

double
_nrrdCDInt(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double
_nrrdCDSup(const double *parm) {
  double S;
  
  S = parm[0];
  return 2*S;
}

double
_nrrdCD1_d(double x, const double *parm) {
  double S;
  
  S = parm[0];
  x /= S;
  return _CENDIF(x)/(S*S);
}

float
_nrrdCD1_f(float x, const double *parm) {
  float S;
  
  S = AIR_CAST(float, parm[0]);
  x /= S;
  return AIR_CAST(float, _CENDIF(x)/(S*S));
}

void
_nrrdCDN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t;
  size_t i;
  
  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i]/S;
    f[i] = _CENDIF(t)/(S*S);
  }
}

void
_nrrdCDN_f(float *f, const float *x, size_t len, const double *parm) {
  float t, S;
  size_t i;
  
  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i]/S;
    f[i] = AIR_CAST(float, _CENDIF(t)/(S*S));
  }
}

NrrdKernel
_nrrdKernelCD = {
  "cendif",
  1, _nrrdCDSup,  _nrrdCDInt,   
  _nrrdCD1_f,   _nrrdCDN_f,   _nrrdCD1_d,   _nrrdCDN_d
};
NrrdKernel *const
nrrdKernelCentDiff = &_nrrdKernelCD;

/* ------------------------------------------------------------ */

#define _BCCUBIC(x, B, C)                                     \
  (x >= 2.0 ? 0 :                                             \
  (x >= 1.0                                                   \
   ? (((-B/6 - C)*x + B + 5*C)*x -2*B - 8*C)*x + 4*B/3 + 4*C  \
   : ((2 - 3*B/2 - C)*x - 3 + 2*B + C)*x*x + 1 - B/3))

double
_nrrdBCInt(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double
_nrrdBCSup(const double *parm) {
  double S;

  S = parm[0];
  return 2*S;
}

double
_nrrdBC1_d(double x, const double *parm) {
  double S;
  double B, C;
  
  S = parm[0]; B = parm[1]; C = parm[2]; 
  x = AIR_ABS(x)/S;
  return _BCCUBIC(x, B, C)/S;
}

float
_nrrdBC1_f(float x, const double *parm) {
  float B, C, S;
  
  S = AIR_CAST(float, parm[0]);
  B = AIR_CAST(float, parm[1]);
  C = AIR_CAST(float, parm[2]); 
  x = AIR_ABS(x)/S;
  return _BCCUBIC(x, B, C)/S;
}

void
_nrrdBCN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t, B, C;
  size_t i;
  
  S = parm[0]; B = parm[1]; C = parm[2]; 
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _BCCUBIC(t, B, C)/S;
  }
}

void
_nrrdBCN_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t, B, C;
  size_t i;
  
  S = AIR_CAST(float, parm[0]);
  B = AIR_CAST(float, parm[1]);
  C = AIR_CAST(float, parm[2]); 
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _BCCUBIC(t, B, C)/S;
  }
}

NrrdKernel
_nrrdKernelBC = {
  "BCcubic",
  3, _nrrdBCSup,  _nrrdBCInt,   
  _nrrdBC1_f,   _nrrdBCN_f,   _nrrdBC1_d,   _nrrdBCN_d
};
NrrdKernel *const
nrrdKernelBCCubic = &_nrrdKernelBC;

/* ------------------------------------------------------------ */

#define _DBCCUBIC(x, B, C)                        \
   (x >= 2.0 ? 0 :                                \
   (x >= 1.0                                      \
    ? ((-B/2 - 3*C)*x + 2*B + 10*C)*x -2*B - 8*C  \
    : ((6 - 9*B/2 - 3*C)*x - 6 + 4*B + 2*C)*x))

double
_nrrdDBCInt(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double
_nrrdDBCSup(const double *parm) {
  double S;

  S = parm[0];
  return 2*S;
}

double
_nrrdDBC1_d(double x, const double *parm) {
  double S;
  double B, C;
  int sgn = 1;
  
  S = parm[0]; B = parm[1]; C = parm[2]; 
  if (x < 0) { x = -x; sgn = -1; }
  x /= S;
  return sgn*_DBCCUBIC(x, B, C)/(S*S);
}

float
_nrrdDBC1_f(float x, const double *parm) {
  float B, C, S;
  int sgn = 1;
  
  S = AIR_CAST(float, parm[0]);
  B = AIR_CAST(float, parm[1]);
  C = AIR_CAST(float, parm[2]); 
  if (x < 0) { x = -x; sgn = -1; }
  x /= S;
  return sgn*_DBCCUBIC(x, B, C)/(S*S);
}

void
_nrrdDBCN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t, B, C;
  size_t i;
  int sgn;
  
  S = parm[0]; B = parm[1]; C = parm[2]; 
  for (i=0; i<len; i++) {
    t = x[i]/S;
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DBCCUBIC(t, B, C)/(S*S);
  }
}

void
_nrrdDBCN_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t, B, C;
  int sgn;
  size_t i;
  
  S = AIR_CAST(float, parm[0]);
  B = AIR_CAST(float, parm[1]);
  C = AIR_CAST(float, parm[2]); 
  for (i=0; i<len; i++) {
    t = x[i]/S;
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DBCCUBIC(t, B, C)/(S*S);
  }
}

NrrdKernel
_nrrdKernelDBC = {
  "BCcubicD",
  3, _nrrdDBCSup, _nrrdDBCInt,  
  _nrrdDBC1_f,  _nrrdDBCN_f,  _nrrdDBC1_d,  _nrrdDBCN_d
};
NrrdKernel *const
nrrdKernelBCCubicD = &_nrrdKernelDBC;

/* ------------------------------------------------------------ */

#define _DDBCCUBIC(x, B, C)                    \
   (x >= 2.0 ? 0 :                             \
   (x >= 1.0                                   \
    ? (-B - 6*C)*x + 2*B + 10*C                \
    : (12 - 9*B - 6*C)*x - 6 + 4*B + 2*C  ))

double
_nrrdDDBCInt(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double
_nrrdDDBCSup(const double *parm) {
  double S;

  S = parm[0];
  return 2*S;
}

double
_nrrdDDBC1_d(double x, const double *parm) {
  double S;
  double B, C;
  
  S = parm[0]; B = parm[1]; C = parm[2]; 
  x = AIR_ABS(x)/S;
  return _DDBCCUBIC(x, B, C)/(S*S*S);
}

float
_nrrdDDBC1_f(float x, const double *parm) {
  float B, C, S;
  
  S = AIR_CAST(float, parm[0]);
  B = AIR_CAST(float, parm[1]);
  C = AIR_CAST(float, parm[2]); 
  x = AIR_ABS(x)/S;
  return _DDBCCUBIC(x, B, C)/(S*S*S);
}

void
_nrrdDDBCN_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t, B, C;
  size_t i;
  
  S = parm[0]; B = parm[1]; C = parm[2]; 
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _DDBCCUBIC(t, B, C)/(S*S*S);
  }
}

void
_nrrdDDBCN_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t, B, C;
  size_t i;
  
  S = AIR_CAST(float, parm[0]);
  B = AIR_CAST(float, parm[1]);
  C = AIR_CAST(float, parm[2]); 
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _DDBCCUBIC(t, B, C)/(S*S*S);
  }
}

NrrdKernel
_nrrdKernelDDBC = {
  "BCcubicDD",
  3, _nrrdDDBCSup,_nrrdDDBCInt, 
  _nrrdDDBC1_f, _nrrdDDBCN_f, _nrrdDDBC1_d, _nrrdDDBCN_d
};
NrrdKernel *const
nrrdKernelBCCubicDD = &_nrrdKernelDDBC;

/* ------------------------------------------------------------ */

#define _AQUARTIC(x, A) \
   (x >= 3.0 ? 0 :      \
   (x >= 2.0            \
    ? A*(-54 + x*(81 + x*(-45 + x*(11 - x)))) \
    : (x >= 1.0                               \
       ? 4 - 6*A + x*(-10 + 25*A + x*(9 - 33*A                         \
                                 + x*(-3.5 + 17*A + x*(0.5 - 3*A))))   \
       : 1 + x*x*(-3 + 6*A + x*((2.5 - 10*A) + x*(-0.5 + 4*A))))))

double
_nrrdA4Int(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

double
_nrrdA4Sup(const double *parm) {
  double S;

  S = parm[0];
  return 3*S;
}

double
_nrrdA41_d(double x, const double *parm) {
  double S;
  double A;
  
  S = parm[0]; A = parm[1];
  x = AIR_ABS(x)/S;
  return _AQUARTIC(x, A)/S;
}

float
_nrrdA41_f(float x, const double *parm) {
  float A, S;
  
  S = AIR_CAST(float, parm[0]); A = AIR_CAST(float, parm[1]);
  x = AIR_ABS(x)/S;
  return AIR_CAST(float, _AQUARTIC(x, A)/S);
}

void
_nrrdA4N_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t, A;
  size_t i;
  
  S = parm[0]; A = parm[1];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _AQUARTIC(t, A)/S;
  }
}

void
_nrrdA4N_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t, A;
  size_t i;
  
  S = AIR_CAST(float, parm[0]); A = AIR_CAST(float, parm[1]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = AIR_CAST(float, _AQUARTIC(t, A)/S);
  }
}

NrrdKernel
_nrrdKernelA4 = {
  "Aquartic",
  2, _nrrdA4Sup,  _nrrdA4Int,   
  _nrrdA41_f,   _nrrdA4N_f,   _nrrdA41_d,   _nrrdA4N_d
};
NrrdKernel *const
nrrdKernelAQuartic = &_nrrdKernelA4;

/* ------------------------------------------------------------ */

#define _DAQUARTIC(x, A) \
   (x >= 3.0 ? 0 :       \
   (x >= 2.0             \
    ? A*(81 + x*(-90 + x*(33 - 4*x))) \
    : (x >= 1.0                       \
       ? -10 + 25*A + x*(18 - 66*A + x*(-10.5 + 51*A + x*(2 - 12*A))) \
       : x*(-6 + 12*A + x*(7.5 - 30*A + x*(-2 + 16*A))))))

double
_nrrdDA4Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double
_nrrdDA4Sup(const double *parm) {
  double S;

  S = parm[0];
  return 3*S;
}

double
_nrrdDA41_d(double x, const double *parm) {
  double S;
  double A;
  int sgn = 1;
  
  S = parm[0]; A = parm[1];
  if (x < 0) { x = -x; sgn = -1; }
  x /= S;
  return sgn*_DAQUARTIC(x, A)/(S*S);
}

float
_nrrdDA41_f(float x, const double *parm) {
  float A, S;
  int sgn = 1;
  
  S = AIR_CAST(float, parm[0]); A = AIR_CAST(float, parm[1]);
  if (x < 0) { x = -x; sgn = -1; }
  x /= S;
  return AIR_CAST(float, sgn*_DAQUARTIC(x, A)/(S*S));
}

void
_nrrdDA4N_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t, A;
  size_t i;
  int sgn;
  
  S = parm[0]; A = parm[1];
  for (i=0; i<len; i++) {
    t = x[i]/S;
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DAQUARTIC(t, A)/(S*S);
  }
}

void
_nrrdDA4N_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t, A;
  size_t i;
  int sgn;
  
  S = AIR_CAST(float, parm[0]); A = AIR_CAST(float, parm[1]);
  for (i=0; i<len; i++) {
    t = x[i]/S;
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*_DAQUARTIC(t, A)/(S*S));
  }
}

NrrdKernel
_nrrdKernelDA4 = {
  "AquarticD",
  2, _nrrdDA4Sup, _nrrdDA4Int,  
  _nrrdDA41_f,  _nrrdDA4N_f,  _nrrdDA41_d,  _nrrdDA4N_d
};
NrrdKernel *const
nrrdKernelAQuarticD = &_nrrdKernelDA4;

/* ------------------------------------------------------------ */

#define _DDAQUARTIC(x, A) \
   (x >= 3.0 ? 0 :        \
   (x >= 2.0              \
    ? A*(-90 + x*(66 - x*12)) \
    : (x >= 1.0               \
       ? 18 - 66*A + x*(-21 + 102*A + x*(6 - 36*A))   \
       : -6 + 12*A + x*(15 - 60*A + x*(-6 + 48*A)))))

double
_nrrdDDA4Int(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

double
_nrrdDDA4Sup(const double *parm) {
  double S;

  S = parm[0];
  return 3*S;
}

double
_nrrdDDA41_d(double x, const double *parm) {
  double S;
  double A;
  
  S = parm[0]; A = parm[1];
  x = AIR_ABS(x)/S;
  return _DDAQUARTIC(x, A)/(S*S*S);
}

float
_nrrdDDA41_f(float x, const double *parm) {
  float S, A;
  
  S = AIR_CAST(float, parm[0]); A = AIR_CAST(float, parm[1]);
  x = AIR_ABS(x)/S;
  return _DDAQUARTIC(x, A)/(S*S*S);
}

void
_nrrdDDA4N_d(double *f, const double *x, size_t len, const double *parm) {
  double S;
  double t, A;
  size_t i;
  
  S = parm[0]; A = parm[1];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _DDAQUARTIC(t, A)/(S*S*S);
  }
}

void
_nrrdDDA4N_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t, A;
  size_t i;
  
  S = AIR_CAST(float, parm[0]); A = AIR_CAST(float, parm[1]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _DDAQUARTIC(t, A)/(S*S*S);
  }
}

NrrdKernel
_nrrdKernelDDA4 = {
  "AquarticDD",
  2, _nrrdDDA4Sup,_nrrdDDA4Int, 
  _nrrdDDA41_f, _nrrdDDA4N_f, _nrrdDDA41_d, _nrrdDDA4N_d
};
NrrdKernel *const
nrrdKernelAQuarticDD = &_nrrdKernelDDA4;

/* ------------------------------------------------------------ */

/* 
** This is the unique, four-sample support, quintic, C^3 kernel,
** with 1st and 3rd derivatives zero at origin, which integrates
** to unity on [-2,2], with 0th and 1st accuracy.
** Unlike previously believed, this does NOT have 2nd order accuracy!
** It doesn't interpolate.
**
** The same kernel is also available as 
** nrrdKernelTMF w/ D,C,A = -1,3,2 => nrrdKernelTMF[0][4][2] => "tmf:-1,3,2"
** the advantage here being that you have access to the first
** and second derivatives of this quintic kernel as
** nrrdKernelC3QuinticD and nrrdKernelC3QuinticDD
*/

#define _C3QUINTIC(x) \
  (x >= 2.0 ? 0 :                                             \
  (x >= 1.0                                                   \
   ? 0.8 + x*x*(-2 + x*(2 + x*(-0.75 + x*0.1)))               \
   : 0.7 + x*x*(-1 + x*x*(0.75 - x*0.3))))

static double
_c3quintInt(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

static double
_c3quintSup(const double *parm) {
  double S;

  S = parm[0];
  return 2*S;
}

static double
_c3quint1_d(double x, const double *parm) {
  double S;
  
  S = parm[0];
  x = AIR_ABS(x)/S;
  return _C3QUINTIC(x)/S;
}

static float
_c3quint1_f(float x, const double *parm) {
  float S;
  
  S = AIR_CAST(float, parm[0]);
  x = AIR_ABS(x)/S;
  return AIR_CAST(float, _C3QUINTIC(x)/S);
}

static void
_c3quintN_d(double *f, const double *x, size_t len, const double *parm) {
  double S, t;
  size_t i;
  
  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _C3QUINTIC(t)/S;
  }
}

static void
_c3quintN_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t;
  size_t i;
  
  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = AIR_CAST(float, _C3QUINTIC(t)/S);
  }
}

static NrrdKernel
_c3quint = {
  "C3Quintic",
  1, _c3quintSup,  _c3quintInt,   
  _c3quint1_f,   _c3quintN_f,   _c3quint1_d,   _c3quintN_d
};
NrrdKernel *const
nrrdKernelC3Quintic = &_c3quint;

/* ------------------------------------------------------------ */

#define _DC3QUINTIC(x) \
  (x >= 2.0 ? 0 :                                             \
  (x >= 1.0                                                   \
   ? x*(-4 + x*(6 + x*(-3 + x*0.5)))                          \
   : x*(-2 + x*x*(3 - x*1.5))))

static double
_Dc3quintInt(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

static double
_Dc3quintSup(const double *parm) {
  double S;

  S = parm[0];
  return 2*S;
}

static double
_Dc3quint1_d(double x, const double *parm) {
  double S;
  int sgn = 1;
  
  S = parm[0];
  if (x < 0) { x = -x; sgn = -1; }
  x /= S;
  return sgn*_DC3QUINTIC(x)/(S*S);
}

static float
_Dc3quint1_f(float x, const double *parm) {
  float S;
  int sgn = 1;
  
  S = AIR_CAST(float, parm[0]);
  if (x < 0) { x = -x; sgn = -1; }
  x /= S;
  return AIR_CAST(float, sgn*_DC3QUINTIC(x)/(S*S));
}

static void
_Dc3quintN_d(double *f, const double *x, size_t len, const double *parm) {
  double S, t;
  size_t i;
  int sgn;
  
  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i]/S;
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DC3QUINTIC(t)/(S*S);
  }
}

static void
_Dc3quintN_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t;
  size_t i;
  int sgn;
  
  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i]/S;
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*_DC3QUINTIC(t)/(S*S));
  }
}

static NrrdKernel
_nrrdKernelDC3Quintic = {
  "C3QuinticD",
  1, _Dc3quintSup, _Dc3quintInt,  
  _Dc3quint1_f,  _Dc3quintN_f,  _Dc3quint1_d,  _Dc3quintN_d
};
NrrdKernel *const
nrrdKernelC3QuinticD = &_nrrdKernelDC3Quintic;

/* ------------------------------------------------------------ */

#define _DDC3QUINTIC(x) \
  (x >= 2.0 ? 0 :                                             \
  (x >= 1.0                                                   \
   ? -4 + x*(12 + x*(-9 + x*2))                               \
   : -2 + x*x*(9 - x*6)))

static double
_DDc3quintInt(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

static double
_DDc3quintSup(const double *parm) {
  double S;

  S = parm[0];
  return 2*S;
}

static double
_DDc3quint1_d(double x, const double *parm) {
  double S;
  
  S = parm[0];
  x = AIR_ABS(x)/S;
  return _DDC3QUINTIC(x)/S;
}

static float
_DDc3quint1_f(float x, const double *parm) {
  float S;
  
  S = AIR_CAST(float, parm[0]);
  x = AIR_ABS(x)/S;
  return AIR_CAST(float, _DDC3QUINTIC(x)/S);
}

static void
_DDc3quintN_d(double *f, const double *x, size_t len, const double *parm) {
  double S, t;
  size_t i;
  
  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _DDC3QUINTIC(t)/S;
  }
}

static void
_DDc3quintN_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t;
  size_t i;
  
  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = AIR_CAST(float, _DDC3QUINTIC(t)/S);
  }
}

static NrrdKernel
_DDc3quint = {
  "C3QuinticDD",
  1, _DDc3quintSup,  _DDc3quintInt,   
  _DDc3quint1_f,   _DDc3quintN_f,   _DDc3quint1_d,   _DDc3quintN_d
};
NrrdKernel *const
nrrdKernelC3QuinticDD = &_DDc3quint;

/* ------------------------------------------------------------ */

/* 
** This is the unique, 6-sample support, hexic, C^4 kernel,
** with 1st and 3rd derivatives zero at origin, which integrates
** to unity on [-2,2], with 0th, 1st, 2nd, and 3rd order accuracy.
** It doesn't interpolate, but it actually rings once.
**
** The same kernel is also available as 
** nrrdKernelTMF with D,C,A = -1,3,2 ---> nrrdKernelTMF[0][4][2],
** the advantage here being that you have access to the first
** and second derivatives of tthis quintic kernel as
** nrrdKernelC3QuinticD and nrrdKernelC3QuinticDD
*/

#define _C4HEXIC(x) \
  (x >= 3.0 \
   ? 0 \
   : (x >= 2.0 \
      ? 1539.0/160.0 + x*(-189.0/8.0 + x*(747.0/32.0 + x*(-12.0 + x*(109.0/32.0 + x*(-61.0/120.0 + x/32.0))))) \
      : (x >= 1.0 \
         ? 3.0/160.0 + x*(35.0/8.0 + x*(-341.0/32.0 + x*(10.0 + x*(-147.0/32.0 + x*(25.0/24.0 - x*3.0/32.0))))) \
         : 69.0/80.0 + x*x*(-23.0/16.0 + x*x*(19.0/16.0 + x*(-7.0/12.0 + x/16.0)))  )))

static double
_c4hexInt(const double *parm) {
  AIR_UNUSED(parm);
  return 1.0;
}

static double
_c4hexSup(const double *parm) {
  double S;

  S = parm[0];
  return 3*S;
}

static double
_c4hex1_d(double x, const double *parm) {
  double S;
  
  S = parm[0];
  x = AIR_ABS(x)/S;
  return _C4HEXIC(x)/S;
}

static float
_c4hex1_f(float x, const double *parm) {
  float S;
  
  S = AIR_CAST(float, parm[0]);
  x = AIR_ABS(x)/S;
  return AIR_CAST(float, _C4HEXIC(x)/S);
}

static void
_c4hexN_d(double *f, const double *x, size_t len, const double *parm) {
  double S, t;
  size_t i;
  
  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _C4HEXIC(t)/S;
  }
}

static void
_c4hexN_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t;
  size_t i;
  
  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = AIR_CAST(float, _C4HEXIC(t)/S);
  }
}

static NrrdKernel
_c4hex = {
  "C4hexic",
  1, _c4hexSup,  _c4hexInt,   
  _c4hex1_f,   _c4hexN_f,   _c4hex1_d,   _c4hexN_d
};
NrrdKernel *const
nrrdKernelC4Hexic = &_c4hex;

/* ------------------------------------------------------------ */

#define _DC4HEXIC(x) \
  (x >= 3.0 \
   ? 0 \
   : (x >= 2.0 \
      ? -189.0/8.0 + x*(747.0/16.0 + x*(-36.0 + x*(109.0/8.0 + x*(-61.0/24.0 + x*(3.0/16.0))))) \
      : (x >= 1.0 \
         ? 35.0/8.0 + x*(-341.0/16.0 + x*(30 + x*(-147.0/8.0 + x*(125.0/24.0 + x*(-9.0/16.0))))) \
         : x*(-23.0/8.0 + x*x*(19.0/4.0 + x*(-35.0/12.0 + x*(3.0/8.0))))  )))

static double
_Dc4hexInt(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

static double
_Dc4hexSup(const double *parm) {
  double S;

  S = parm[0];
  return 3*S;
}

static double
_Dc4hex1_d(double x, const double *parm) {
  double S;
  int sgn = 1;
  
  S = parm[0];
  if (x < 0) { x = -x; sgn = -1; }
  x /= S;
  return sgn*_DC4HEXIC(x)/(S*S);
}

static float
_Dc4hex1_f(float x, const double *parm) {
  float S;
  int sgn = 1;
  
  S = AIR_CAST(float, parm[0]);
  if (x < 0) { x = -x; sgn = -1; }
  x /= S;
  return AIR_CAST(float, sgn*_DC4HEXIC(x)/(S*S));
}

static void
_Dc4hexN_d(double *f, const double *x, size_t len, const double *parm) {
  double S, t;
  size_t i;
  int sgn;
  
  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i]/S;
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DC4HEXIC(t)/(S*S);
  }
}

static void
_Dc4hexN_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t;
  size_t i;
  int sgn;
  
  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i]/S;
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*_DC4HEXIC(t)/(S*S));
  }
}

static NrrdKernel
_nrrdKernelDC4hexic = {
  "C4hexicD",
  1, _Dc4hexSup, _Dc4hexInt,  
  _Dc4hex1_f,  _Dc4hexN_f,  _Dc4hex1_d,  _Dc4hexN_d
};
NrrdKernel *const
nrrdKernelC4HexicD = &_nrrdKernelDC4hexic;

/* ------------------------------------------------------------ */

#define _DDC4HEXIC(x) \
  (x >= 3.0 \
   ? 0 \
   : (x >= 2.0 \
      ? 747.0/16.0 + x*(-72.0 + x*(327.0/8.0 + x*(-61.0/6.0 + x*15.0/16.0))) \
      : (x >= 1.0 \
         ? -341.0/16.0 + x*(60 + x*(-441.0/8.0 + x*(125.0/6.0 - x*45.0/16.0))) \
         : -23.0/8.0 + x*x*(57.0/4.0 + x*(-35.0/3.0 + x*(15.0/8.0)))  )))

static double
_DDc4hexInt(const double *parm) {
  AIR_UNUSED(parm);
  return 0.0;
}

static double
_DDc4hexSup(const double *parm) {
  double S;

  S = parm[0];
  return 3*S;
}

static double
_DDc4hex1_d(double x, const double *parm) {
  double S;
  
  S = parm[0];
  x = AIR_ABS(x)/S;
  return _DDC4HEXIC(x)/S;
}

static float
_DDc4hex1_f(float x, const double *parm) {
  float S;
  
  S = AIR_CAST(float, parm[0]);
  x = AIR_ABS(x)/S;
  return AIR_CAST(float, _DDC4HEXIC(x)/S);
}

static void
_DDc4hexN_d(double *f, const double *x, size_t len, const double *parm) {
  double S, t;
  size_t i;
  
  S = parm[0];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = _DDC4HEXIC(t)/S;
  }
}

static void
_DDc4hexN_f(float *f, const float *x, size_t len, const double *parm) {
  float S, t;
  size_t i;
  
  S = AIR_CAST(float, parm[0]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t)/S;
    f[i] = AIR_CAST(float, _DDC4HEXIC(t)/S);
  }
}

static NrrdKernel
_DDc4hex = {
  "C4hexicDD",
  1, _DDc4hexSup,  _DDc4hexInt,   
  _DDc4hex1_f,   _DDc4hexN_f,   _DDc4hex1_d,   _DDc4hexN_d
};
NrrdKernel *const
nrrdKernelC4HexicDD = &_DDc4hex;

/* ------------------------------------------------------------ */

#define _GAUSS(x, sig, cut) ( \
   x >= sig*cut ? 0           \
   : exp(-x*x/(2.0*sig*sig))/(sig*2.50662827463100050241))

double
_nrrdGInt(const double *parm) {
  double cut;
  
  cut = parm[1];
  return airErf(cut/sqrt(2.0));
}

double
_nrrdGSup(const double *parm) {
  double sig, cut;

  sig = parm[0];
  cut = parm[1];
  return sig*cut;
}

double
_nrrdG1_d(double x, const double *parm) {
  double sig, cut;
  
  sig = parm[0];
  cut = parm[1];
  x = AIR_ABS(x);
  return _GAUSS(x, sig, cut);
}

float
_nrrdG1_f(float x, const double *parm) {
  float sig, cut;
  
  sig = AIR_CAST(float, parm[0]);
  cut = AIR_CAST(float, parm[1]);
  x = AIR_ABS(x);
  return AIR_CAST(float, _GAUSS(x, sig, cut));
}

void
_nrrdGN_d(double *f, const double *x, size_t len, const double *parm) {
  double sig, cut, t;
  size_t i;
  
  sig = parm[0];
  cut = parm[1];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = _GAUSS(t, sig, cut);
  }
}

void
_nrrdGN_f(float *f, const float *x, size_t len, const double *parm) {
  float sig, cut, t;
  size_t i;
  
  sig = AIR_CAST(float, parm[0]);
  cut = AIR_CAST(float, parm[1]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = AIR_CAST(float, _GAUSS(t, sig, cut));
  }
}

NrrdKernel
_nrrdKernelG = {
  "gauss",
  2, _nrrdGSup,  _nrrdGInt,   
  _nrrdG1_f,   _nrrdGN_f,   _nrrdG1_d,   _nrrdGN_d
};
NrrdKernel *const
nrrdKernelGaussian = &_nrrdKernelG;

/* ------------------------------------------------------------ */

#define _DISCRETEGAUSS(xx, sig, abscut)                         \
  (sig > 0                                                      \
   ? (xx > abscut                                               \
      ? 0                                                       \
      : airBesselInExpScaled(AIR_CAST(int, xx + 0.5), sig*sig)) \
   : xx <= 0.5)

#define _DGABSCUT(ret, sig, cut) \
  (ret) = 0.5 + ceil((sig)*(cut));  \
  (ret) = AIR_MAX(0.5, (ret))

double
_nrrdDiscGaussianSup(const double *parm) {
  double ret;

  _DGABSCUT(ret, parm[0], parm[1]);
  return ret;
}

/* the functions are out of their usual definition order because we
   use the function evaluation to determine the integral, rather than
   trying to do it analytically */
  
double
_nrrdDiscGaussian1_d(double xx, const double *parm) {
  double sig, cut;
  
  sig = parm[0];
  _DGABSCUT(cut, sig, parm[1]);
  xx = AIR_ABS(xx);
  return _DISCRETEGAUSS(xx, sig, cut);
}

double
_nrrdDiscGaussianInt(const double *parm) {
  double sum, cut;
  int ii, supp;

  _DGABSCUT(cut, parm[0], parm[1]);
  supp = AIR_CAST(int, cut);
  sum = 0.0;
  for (ii=-supp; ii<=supp; ii++) {
    sum += _nrrdDiscGaussian1_d(ii, parm);
  }
  return sum;
}

float
_nrrdDiscGaussian1_f(float xx, const double *parm) {
  double sig, cut;
  
  sig = parm[0];
  _DGABSCUT(cut, sig, parm[1]);
  xx = AIR_ABS(xx);
  return AIR_CAST(float, _DISCRETEGAUSS(xx, sig, cut));
}

void
_nrrdDiscGaussianN_d(double *f, const double *x,
                  size_t len, const double *parm) {
  double sig, cut, tt;
  size_t ii;
  
  sig = parm[0];
  _DGABSCUT(cut, sig, parm[1]);
  for (ii=0; ii<len; ii++) {
    tt = AIR_ABS(x[ii]);
    f[ii] = _DISCRETEGAUSS(tt, sig, cut);
  }
}

void
_nrrdDiscGaussianN_f(float *f, const float *x, size_t len, const double *parm) {
  double sig, cut, tt;
  size_t ii;
  
  sig = parm[0];
  _DGABSCUT(cut, sig, parm[1]);
  for (ii=0; ii<len; ii++) {
    tt = AIR_ABS(x[ii]);
    f[ii] = AIR_CAST(float, _DISCRETEGAUSS(tt, sig, cut));
  }
}

NrrdKernel
_nrrdKernelDiscreteGaussian = {
  "discretegauss", 2,
  _nrrdDiscGaussianSup,  _nrrdDiscGaussianInt,   
  _nrrdDiscGaussian1_f, _nrrdDiscGaussianN_f,
  _nrrdDiscGaussian1_d, _nrrdDiscGaussianN_d
};
NrrdKernel *const
nrrdKernelDiscreteGaussian = &_nrrdKernelDiscreteGaussian;

/* ------------------------------------------------------------ */

#define _DGAUSS(x, sig, cut) (                                               \
   x >= sig*cut ? 0                                                          \
   : -exp(-x*x/(2.0*sig*sig))*x/(sig*sig*sig*2.50662827463100050241))

double
_nrrdDGInt(const double *parm) {
  AIR_UNUSED(parm);
  return 0;
}

double
_nrrdDGSup(const double *parm) {
  double sig, cut;

  sig = parm[0];
  cut = parm[1];
  return sig*cut;
}

double
_nrrdDG1_d(double x, const double *parm) {
  double sig, cut;
  int sgn = 1;
  
  sig = parm[0];
  cut = parm[1];
  if (x < 0) { x = -x; sgn = -1; }
  return sgn*_DGAUSS(x, sig, cut);
}

float
_nrrdDG1_f(float x, const double *parm) {
  float sig, cut;
  int sgn = 1;
  
  sig = AIR_CAST(float, parm[0]);
  cut = AIR_CAST(float, parm[1]);
  if (x < 0) { x = -x; sgn = -1; }
  return AIR_CAST(float, sgn*_DGAUSS(x, sig, cut));
}

void
_nrrdDGN_d(double *f, const double *x, size_t len, const double *parm) {
  double sig, cut, t;
  size_t i;
  int sgn;
  
  sig = parm[0];
  cut = parm[1];
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = sgn*_DGAUSS(t, sig, cut);
  }
}

void
_nrrdDGN_f(float *f, const float *x, size_t len, const double *parm) {
  float sig, cut, t;
  size_t i;
  int sgn;
  
  sig = AIR_CAST(float, parm[0]);
  cut = AIR_CAST(float, parm[1]);
  for (i=0; i<len; i++) {
    t = x[i];
    if (t < 0) { t = -t; sgn = -1; } else { sgn = 1; }
    f[i] = AIR_CAST(float, sgn*_DGAUSS(t, sig, cut));
  }
}

NrrdKernel
_nrrdKernelDG = {
  "gaussD",
  2, _nrrdDGSup,  _nrrdDGInt,   
  _nrrdDG1_f,   _nrrdDGN_f,   _nrrdDG1_d,   _nrrdDGN_d
};
NrrdKernel *const
nrrdKernelGaussianD = &_nrrdKernelDG;

/* ------------------------------------------------------------ */

#define _DDGAUSS(x, sig, cut) ( \
   x >= sig*cut ? 0             \
   : exp(-x*x/(2.0*sig*sig))*(x*x-sig*sig) /       \
     (sig*sig*sig*sig*sig*2.50662827463100050241))

double
_nrrdDDGInt(const double *parm) {
  double sig, cut;
  
  sig = parm[0];
  cut = parm[1];
  return -0.79788456080286535587*cut*exp(-cut*cut/2)/(sig*sig);
}

double
_nrrdDDGSup(const double *parm) {
  double sig, cut;

  sig = parm[0];
  cut = parm[1];
  return sig*cut;
}

double
_nrrdDDG1_d(double x, const double *parm) {
  double sig, cut;
  
  sig = parm[0];
  cut = parm[1];
  x = AIR_ABS(x);
  return _DDGAUSS(x, sig, cut);
}

float
_nrrdDDG1_f(float x, const double *parm) {
  float sig, cut;
  
  sig = AIR_CAST(float, parm[0]);
  cut = AIR_CAST(float, parm[1]);
  x = AIR_ABS(x);
  return AIR_CAST(float, _DDGAUSS(x, sig, cut));
}

void
_nrrdDDGN_d(double *f, const double *x, size_t len, const double *parm) {
  double sig, cut, t;
  size_t i;
  
  sig = parm[0];
  cut = parm[1];
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = _DDGAUSS(t, sig, cut);
  }
}

void
_nrrdDDGN_f(float *f, const float *x, size_t len, const double *parm) {
  float sig, cut, t;
  size_t i;
  
  sig = AIR_CAST(float, parm[0]);
  cut = AIR_CAST(float, parm[1]);
  for (i=0; i<len; i++) {
    t = x[i];
    t = AIR_ABS(t);
    f[i] = AIR_CAST(float, _DDGAUSS(t, sig, cut));
  }
}

NrrdKernel
_nrrdKernelDDG = {
  "gaussDD",
  2, _nrrdDDGSup,  _nrrdDDGInt,   
  _nrrdDDG1_f,   _nrrdDDGN_f,   _nrrdDDG1_d,   _nrrdDDGN_d
};
NrrdKernel *const
nrrdKernelGaussianDD = &_nrrdKernelDDG;


/* ------------------------------------------------------------ */

NrrdKernel *
_nrrdKernelStrToKern(char *str) {
  
  if (!strcmp("zero", str))       return nrrdKernelZero;
  if (!strcmp("box", str))        return nrrdKernelBox;
  if (!strcmp("cheap", str))      return nrrdKernelCheap;
  if (!strcmp("hermiteFlag", str))    return nrrdKernelHermiteFlag;
  if (!strcmp("hermite", str))    return nrrdKernelHermiteFlag;
  if (!strcmp("herm", str))       return nrrdKernelHermiteFlag;
  if (!strcmp("tent", str))       return nrrdKernelTent;
  if (!strcmp("forwdiff", str))   return nrrdKernelForwDiff;
  if (!strcmp("fordif", str))     return nrrdKernelForwDiff;
  if (!strcmp("centdiff", str))   return nrrdKernelCentDiff;
  if (!strcmp("cendif", str))     return nrrdKernelCentDiff;
  if (!strcmp("bccubic", str))    return nrrdKernelBCCubic;
  if (!strcmp("cubic", str))      return nrrdKernelBCCubic;
  if (!strcmp("bccubicd", str))   return nrrdKernelBCCubicD;
  if (!strcmp("cubicd", str))     return nrrdKernelBCCubicD;
  if (!strcmp("bccubicdd", str))  return nrrdKernelBCCubicDD;
  if (!strcmp("cubicdd", str))    return nrrdKernelBCCubicDD;
  if (!strcmp("aquartic", str))   return nrrdKernelAQuartic;
  if (!strcmp("quartic", str))    return nrrdKernelAQuartic;
  if (!strcmp("aquarticd", str))  return nrrdKernelAQuarticD;
  if (!strcmp("quarticd", str))   return nrrdKernelAQuarticD;
  if (!strcmp("aquarticdd", str)) return nrrdKernelAQuarticDD;
  if (!strcmp("quarticdd", str))  return nrrdKernelAQuarticDD;
  if (!strcmp("c3quintic", str))  return nrrdKernelC3Quintic;
  if (!strcmp("c3q", str))        return nrrdKernelC3Quintic;
  if (!strcmp("c3quinticd", str)) return nrrdKernelC3QuinticD;
  if (!strcmp("c3qd", str))       return nrrdKernelC3QuinticD;
  if (!strcmp("c3quinticdd", str)) return nrrdKernelC3QuinticDD;
  if (!strcmp("c3qdd", str))      return nrrdKernelC3QuinticDD;
  if (!strcmp("c4hexic", str))    return nrrdKernelC4Hexic;
  if (!strcmp("c4h", str))        return nrrdKernelC4Hexic;
  if (!strcmp("c4hexicd", str))   return nrrdKernelC4HexicD;
  if (!strcmp("c4hd", str))       return nrrdKernelC4HexicD;
  if (!strcmp("c4hexicdd", str))  return nrrdKernelC4HexicDD;
  if (!strcmp("c4hdd", str))      return nrrdKernelC4HexicDD;
  if (!strcmp("gaussian", str))   return nrrdKernelGaussian;
  if (!strcmp("gauss", str))      return nrrdKernelGaussian;
  if (!strcmp("gaussiand", str))  return nrrdKernelGaussianD;
  if (!strcmp("gaussd", str))     return nrrdKernelGaussianD;
  if (!strcmp("gd", str))         return nrrdKernelGaussianD;
  if (!strcmp("gaussiandd", str)) return nrrdKernelGaussianDD;
  if (!strcmp("gaussdd", str))    return nrrdKernelGaussianDD;
  if (!strcmp("gdd", str))        return nrrdKernelGaussianDD;
  if (!strcmp("ds", str))         return nrrdKernelDiscreteGaussian;
  if (!strcmp("dscale", str))     return nrrdKernelDiscreteGaussian;
  if (!strcmp("dg", str))         return nrrdKernelDiscreteGaussian;
  if (!strcmp("dgauss", str))     return nrrdKernelDiscreteGaussian;
  if (!strcmp("dgaussian", str))  return nrrdKernelDiscreteGaussian;
  if (!strcmp("discretegauss", str))  return nrrdKernelDiscreteGaussian;
  if (!strcmp("hann", str))       return nrrdKernelHann;
  if (!strcmp("hannd", str))      return nrrdKernelHannD;
  if (!strcmp("hanndd", str))     return nrrdKernelHannDD;
  if (!strcmp("bkmn", str))       return nrrdKernelBlackman;
  if (!strcmp("black", str))      return nrrdKernelBlackman;
  if (!strcmp("blackman", str))   return nrrdKernelBlackman;
  if (!strcmp("bkmnd", str))      return nrrdKernelBlackmanD;
  if (!strcmp("blackd", str))     return nrrdKernelBlackmanD;
  if (!strcmp("blackmand", str))  return nrrdKernelBlackmanD;
  if (!strcmp("bkmndd", str))     return nrrdKernelBlackmanDD;
  if (!strcmp("blackdd", str))    return nrrdKernelBlackmanDD;
  if (!strcmp("blackmandd", str)) return nrrdKernelBlackmanDD;
  return NULL;
}

/* this returns a number between -1 and max;
   it does NOT do the increment-by-one;
   it does NOT do range checking */
int
_nrrdKernelParseTMFInt(int *val, char *str) {
  char me[]="nrrdKernelParseTMFInt", err[128];

  if (!strcmp("n", str)) {
    *val = -1;
  } else {
    if (1 != sscanf(str, "%d", val)) {
      sprintf(err, "%s: couldn't parse \"%s\" as int", me, str);
      biffAdd(NRRD, err); return 1;
    }
  }
  return 0;
}

int
nrrdKernelParse(const NrrdKernel **kernelP, 
                double *parm, const char *_str) {
  char me[]="nrrdKernelParse", err[128], str[AIR_STRLEN_HUGE],
    kstr[AIR_STRLEN_MED], *_pstr=NULL, *pstr, *tmfStr[4];
  int j, tmfD, tmfC, tmfA;
  unsigned int haveParm, needParm;
  
  if (!(kernelP && parm && _str)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }

  /* [jorik] (if i understood this correctly) parm is always of length
  ** NRRD_KERNEL_PARMS_NUM. We have to clear all parameters here, since
  ** nrrdKernelSet copies all arguments into its own array later, and
  ** copying uninitialised memory is bad (it traps my memory debugger).
  */
  for (j=0; j<NRRD_KERNEL_PARMS_NUM; j++) {
    parm[j] = 0;
  }

  strcpy(str, _str);
  strcpy(kstr, "");
  pstr = NULL;
  pstr = strchr(str, ':');
  if (pstr) {
    *pstr = '\0';
    _pstr = ++pstr;
  }
  strcpy(kstr, str);
  airToLower(kstr);
  /* first see if its a TMF, then try parsing it as the other stuff */
  if (kstr == strstr(kstr, "tmf")) {
    if (4 == airParseStrS(tmfStr, pstr, ",", 4)) {
      /* a TMF with a parameter: D,C,A,a */
      if (1 != sscanf(tmfStr[3], "%lg", parm)) {
        sprintf(err, "%s: couldn't parse TMF parameter \"%s\" as double",
                me, tmfStr[3]);
        biffAdd(NRRD, err); return 1;
      }
    } else if (3 == airParseStrS(tmfStr, pstr, ",", 3)) {
      /* a TMF without a parameter: D,C,A */
      parm[0] = 0.0;
    } else {
      sprintf(err, "%s: TMF kernels require 3 arguments D, C, A "
              "in the form tmf:D,C,A", me);
      biffAdd(NRRD, err); return 1;
    }
    if (_nrrdKernelParseTMFInt(&tmfD, tmfStr[0])
        || _nrrdKernelParseTMFInt(&tmfC, tmfStr[1])
        || _nrrdKernelParseTMFInt(&tmfA, tmfStr[2])) {
      sprintf(err, "%s: problem parsing \"%s,%s,%s\" as D,C,A "
              "for TMF kernel", me, tmfStr[0], tmfStr[1], tmfStr[2]);
      biffAdd(NRRD, err); return 1;
    }
    if (!AIR_IN_CL(-1, tmfD, (int)nrrdKernelTMF_maxD)) {
      sprintf(err, "%s: derivative value %d outside range [-1,%d]",
              me, tmfD, nrrdKernelTMF_maxD);
      biffAdd(NRRD, err); return 1;
    }
    if (!AIR_IN_CL(-1, tmfC, (int)nrrdKernelTMF_maxC)) {
      sprintf(err, "%s: continuity value %d outside range [-1,%d]",
              me, tmfC, nrrdKernelTMF_maxC);
      biffAdd(NRRD, err); return 1;
    }
    if (!AIR_IN_CL(1, tmfA, (int)nrrdKernelTMF_maxA)) {
      sprintf(err, "%s: accuracy value %d outside range [1,%d]",
              me, tmfA, nrrdKernelTMF_maxA);
      biffAdd(NRRD, err); return 1;
    }
    /*
    fprintf(stderr, "!%s: D,C,A = %d,%d,%d --> %d,%d,%d\n", me,
            tmfD, tmfC, tmfA, tmfD+1, tmfC+1, tmfA);
    */
    *kernelP = nrrdKernelTMF[tmfD+1][tmfC+1][tmfA];
  } else {
    /* its not a TMF */
    if (!(*kernelP = _nrrdKernelStrToKern(kstr))) {
      sprintf(err, "%s: kernel \"%s\" not recognized", me, kstr);
      biffAdd(NRRD, err); return 1;
    }
    if ((*kernelP)->numParm > NRRD_KERNEL_PARMS_NUM) {
      sprintf(err, "%s: kernel \"%s\" requests %d parameters > max %d",
              me, kstr, (*kernelP)->numParm, NRRD_KERNEL_PARMS_NUM);
      biffAdd(NRRD, err); return 1;
    }
    if (*kernelP == nrrdKernelGaussian ||
        *kernelP == nrrdKernelGaussianD ||
        *kernelP == nrrdKernelGaussianDD ||
        *kernelP == nrrdKernelDiscreteGaussian) {
      /* for Gaussians or DiscreteGaussian, we need all the parameters
         given explicitly */
      needParm = (*kernelP)->numParm;
    } else {
      /*  For everything else (note that TMF kernels are handled
          separately), we can make do with one less than the required,
          by using the default spacing  */
      needParm = ((*kernelP)->numParm > 0
                  ? (*kernelP)->numParm - 1
                  : 0);
    }
    if (needParm > 0 && !pstr) {
      sprintf(err, "%s: didn't get any of %d required doubles after "
              "colon in \"%s\"",
              me, needParm, kstr);
      biffAdd(NRRD, err); return 1;
    }
    for (haveParm=0; haveParm<(*kernelP)->numParm; haveParm++) {
      if (!pstr)
        break;
      if (1 != sscanf(pstr, "%lg", parm+haveParm)) {
        sprintf(err, "%s: trouble parsing \"%s\" as double (in \"%s\")",
                me, _pstr, _str);
        biffAdd(NRRD, err); return 1;
      }
      if ((pstr = strchr(pstr, ','))) {
        pstr++;
        if (!*pstr) {
          sprintf(err, "%s: nothing after last comma in \"%s\" (in \"%s\")",
                  me, _pstr, _str);
          biffAdd(NRRD, err); return 1;
        }
      }
    }
    /* haveParm is now the number of parameters that were parsed. */
    if (haveParm < needParm) {
      sprintf(err, "%s: parsed only %d of %d required doubles "
              "from \"%s\" (in \"%s\")",
              me, haveParm, needParm, _pstr, _str);
      biffAdd(NRRD, err); return 1;
    } else if (haveParm == needParm &&
               needParm == (*kernelP)->numParm-1) {
      /* shift up parsed values, and set parm[0] to default */
      for (j=haveParm; j>=1; j--) {
        parm[j] = parm[j-1];
      }
      parm[0] = nrrdDefaultKernelParm0;
    } else {
      if (pstr) {
        sprintf(err, "%s: \"%s\" (in \"%s\") has more than %d doubles",
                me, _pstr, _str, (*kernelP)->numParm);
        biffAdd(NRRD, err); return 1;
      }
    }
  }
  /*
  fprintf(stderr, "%s: %g %g %g %g %g\n", me,
          parm[0], parm[1], parm[2], parm[3], parm[4]);
  */
  return 0;
}

int
nrrdKernelSpecParse(NrrdKernelSpec *ksp, const char *str) {
  char me[]="nrrdKernelSpecParse", err[BIFF_STRLEN];
  const NrrdKernel *kern;
  double kparm[NRRD_KERNEL_PARMS_NUM];
  
  if (!( ksp && str )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdKernelParse(&kern, kparm, str)) {
    sprintf(err, "%s: ", me);
    biffAdd(NRRD, err); return 1;
  }
  nrrdKernelSpecSet(ksp, kern, kparm);
  return 0;
}

/*
** note that the given string has to be allocated for a certain size
** which is plenty big
*/
int
nrrdKernelSpecSprint(char str[AIR_STRLEN_LARGE], NrrdKernelSpec *ksp) {
  char me[]="nrrdKernelSpecSprint", err[BIFF_STRLEN];
  unsigned int warnLen = AIR_STRLEN_LARGE/2;

  if (!( str && ksp )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }

  if (strlen(ksp->kernel->name) > warnLen) {
    sprintf(err, "%s: kernel name (len " _AIR_SIZE_T_CNV 
            ") might lead to overflow", me,
            strlen(ksp->kernel->name));
    biffAdd(NRRD, err); return 1;
  }
  strcpy(str, ksp->kernel->name);
  if (ksp->kernel->numParm) {
    unsigned int ki;
    char sp[AIR_STRLEN_LARGE];
    for (ki=0; ki<ksp->kernel->numParm; ki++) {
      sprintf(sp, "%c%g", (!ki ? ':' : ','), ksp->parm[ki]);
      if (strlen(str) + strlen(sp) > warnLen) {
        sprintf(err, "%s: kernel parm %u might lead to overflow", me, ki);
        biffAdd(NRRD, err); return 1;
      }
      strcat(str, sp);
    }
  }
  return 0;
}
