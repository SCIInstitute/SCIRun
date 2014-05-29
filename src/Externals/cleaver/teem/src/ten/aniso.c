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

/*
** learned: don't take sqrt(FLT_EPSILON) and expect it to still be
** negligible
*/

/*
******** !!!! NOTE NOTE NOTE NOTE NOTE !!!!
********
******** THIS CODE IS NOT REALLY MEANT TO BE EDITED BY HUMANS
******** (only GLK :)
********
******** It is the worst possible example of the dangers of cut-and-paste 
********
******** !!!! NOTE NOTE NOTE NOTE NOTE !!!!
*/
float  _tenAnisoEval_Conf_f(const float  eval[3]) {
  AIR_UNUSED(eval);
  return 1.0;
}
double _tenAnisoEval_Conf_d(const double eval[3]) {
  AIR_UNUSED(eval);
  return 1.0;  return 1.0;
}
float  _tenAnisoTen_Conf_f(const float  ten[7]) {
  return ten[0];
}
double _tenAnisoTen_Conf_d(const double ten[7]) {
  return ten[0];
}


float  _tenAnisoEval_Cl1_f(const float  eval[3]) {
  float sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return sum ? (eval[0] - eval[1])/sum : 0.0f;
}
double _tenAnisoEval_Cl1_d(const double eval[3]) {
  double sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return sum ? (eval[0] - eval[1])/sum : 0.0;
}
float  _tenAnisoTen_Cl1_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return _tenAnisoEval_Cl1_f(eval);
}
double _tenAnisoTen_Cl1_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return _tenAnisoEval_Cl1_d(eval);
}


float  _tenAnisoEval_Cp1_f(const float  eval[3]) {
  float sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return sum ? 2*(eval[1] - eval[2])/sum : 0.0f;
}
double _tenAnisoEval_Cp1_d(const double eval[3]) {
  double sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return sum ? 2*(eval[1] - eval[2])/sum : 0.0;
}
float  _tenAnisoTen_Cp1_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return _tenAnisoEval_Cp1_f(eval);
}
double _tenAnisoTen_Cp1_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return _tenAnisoEval_Cp1_d(eval);
}


float  _tenAnisoEval_Ca1_f(const float  eval[3]) {
  float sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return sum ? (eval[0] + eval[1] - 2*eval[2])/sum : 0.0f;
}
double _tenAnisoEval_Ca1_d(const double eval[3]) {
  double sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return sum ? (eval[0] + eval[1] - 2*eval[2])/sum : 0.0;
}
float  _tenAnisoTen_Ca1_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return _tenAnisoEval_Ca1_f(eval);
}
double _tenAnisoTen_Ca1_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return _tenAnisoEval_Ca1_d(eval);
}


float  _tenAnisoEval_Clpmin1_f(const float  eval[3]) {
  float cl, cp, sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  cl = sum ? (eval[0] - eval[1])/sum : 0.0f;
  cp = sum ? 2*(eval[1] - eval[2])/sum : 0.0f;
  return AIR_MIN(cl, cp);
}
double _tenAnisoEval_Clpmin1_d(const double eval[3]) {
  double cl, cp, sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  cl = sum ? (eval[0] - eval[1])/sum : 0.0;
  cp = sum ? 2*(eval[1] - eval[2])/sum : 0.0;
  return AIR_MIN(cl, cp);
}
float  _tenAnisoTen_Clpmin1_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return _tenAnisoEval_Clpmin1_f(eval);
}
double _tenAnisoTen_Clpmin1_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return _tenAnisoEval_Clpmin1_d(eval);
}


float  _tenAnisoEval_Cs1_f(const float  eval[3]) {
  float sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return sum ? 3*eval[2]/sum : 0.0f;
}
double _tenAnisoEval_Cs1_d(const double eval[3]) {
  double sum = eval[0] + eval[1] + eval[2];
  sum = AIR_MAX(0, sum);
  return sum ? 3*eval[2]/sum : 0.0;
}
float  _tenAnisoTen_Cs1_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return _tenAnisoEval_Cs1_f(eval);
}
double _tenAnisoTen_Cs1_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return _tenAnisoEval_Cs1_d(eval);
}


float  _tenAnisoEval_Ct1_f(const float  _eval[3]) {
  float dem, mn, eval[3];
  mn = (_eval[0] + _eval[1] + _eval[2])/3;
  ELL_3V_SET(eval, _eval[0] - mn, _eval[1] - mn, _eval[2] - mn);
  dem = eval[0] + eval[1] - 2*eval[2];
  return dem ? 2*(eval[1] - eval[2])/dem : 0.0f;
}
double _tenAnisoEval_Ct1_d(const double _eval[3]) {
  double dem, mn, eval[3];
  mn = (_eval[0] + _eval[1] + _eval[2])/3;
  ELL_3V_SET(eval, _eval[0] - mn, _eval[1] - mn, _eval[2] - mn);
  dem = eval[0] + eval[1] - 2*eval[2];
  return dem ? 2*(eval[1] - eval[2])/dem : 0.0;
}
float  _tenAnisoTen_Ct1_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return _tenAnisoEval_Ct1_f(eval);
}
double _tenAnisoTen_Ct1_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return _tenAnisoEval_Ct1_d(eval);
}


float  _tenAnisoEval_Cl2_f(const float  eval[3]) {
  float eval0 = AIR_MAX(0, eval[0]);
  return eval0 ? (eval[0] - eval[1])/eval0 : 0.0f;
}
double _tenAnisoEval_Cl2_d(const double eval[3]) {
  double eval0 = AIR_MAX(0, eval[0]);
  return eval0 ? (eval[0] - eval[1])/eval0 : 0.0;
}
float  _tenAnisoTen_Cl2_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return _tenAnisoEval_Cl2_f(eval);
}
double _tenAnisoTen_Cl2_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return _tenAnisoEval_Cl2_d(eval);
}


float  _tenAnisoEval_Cp2_f(const float  eval[3]) {
  float eval0 = AIR_MAX(0, eval[0]);
  return eval0 ? (eval[1] - eval[2])/eval0 : 0.0f;
}
double _tenAnisoEval_Cp2_d(const double eval[3]) {
  double eval0 = AIR_MAX(0, eval[0]);
  return eval0 ? (eval[1] - eval[2])/eval0 : 0.0;
}
float  _tenAnisoTen_Cp2_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return _tenAnisoEval_Cp2_f(eval);
}
double _tenAnisoTen_Cp2_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return _tenAnisoEval_Cp2_d(eval);
}


float  _tenAnisoEval_Ca2_f(const float  eval[3]) {
  float eval0 = AIR_MAX(0, eval[0]);
  return eval0 ? (eval[0] - eval[2])/eval0 : 0.0f;
}
double _tenAnisoEval_Ca2_d(const double eval[3]) {
  double eval0 = AIR_MAX(0, eval[0]);
  return eval0 ? (eval[0] - eval[2])/eval0 : 0.0;
}
float  _tenAnisoTen_Ca2_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return _tenAnisoEval_Ca2_f(eval);
}
double _tenAnisoTen_Ca2_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return _tenAnisoEval_Ca2_d(eval);
}


float  _tenAnisoEval_Clpmin2_f(const float  eval[3]) {
  float cl, cp, eval0 = AIR_MAX(0, eval[0]);
  cl = eval0 ? (eval[0] - eval[1])/eval0 : 0.0f;
  cp = eval0 ? (eval[1] - eval[2])/eval0 : 0.0f;
  return AIR_MIN(cl, cp);
}
double _tenAnisoEval_Clpmin2_d(const double eval[3]) {
  double cl, cp, eval0 = AIR_MAX(0, eval[0]);
  cl = eval0 ? (eval[0] - eval[1])/eval0 : 0.0;
  cp = eval0 ? (eval[1] - eval[2])/eval0 : 0.0;
  return AIR_MIN(cl, cp);
}
float  _tenAnisoTen_Clpmin2_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return _tenAnisoEval_Clpmin2_f(eval);
}
double _tenAnisoTen_Clpmin2_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return _tenAnisoEval_Clpmin2_d(eval);
}


float  _tenAnisoEval_Cs2_f(const float  eval[3]) {
  float eval0 = AIR_MAX(0, eval[0]);
  return eval0 ? eval[2]/eval0 : 0.0f;
}
double _tenAnisoEval_Cs2_d(const double eval[3]) {
  double eval0 = AIR_MAX(0, eval[0]);
  return eval0 ? eval[2]/eval0 : 0.0;
}
float  _tenAnisoTen_Cs2_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return _tenAnisoEval_Cs2_f(eval);
}
double _tenAnisoTen_Cs2_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return _tenAnisoEval_Cs2_d(eval);
}


float  _tenAnisoEval_Ct2_f(const float  eval[3]) {
  float denom;
  denom = eval[0] - eval[2];
  return denom ? (eval[1] - eval[2])/denom : 0.0f;
}
double _tenAnisoEval_Ct2_d(const double eval[3]) {
  double denom;
  denom = eval[0] - eval[2];
  return denom ? (eval[1] - eval[2])/denom : 0.0;
}
float  _tenAnisoTen_Ct2_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return _tenAnisoEval_Ct2_f(eval);
}
double _tenAnisoTen_Ct2_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return _tenAnisoEval_Ct2_d(eval);
}


#define SQRT6 2.44948974278317809819
float  _tenAnisoEval_RA_f(const float  eval[3]) {
  float mean, stdv;
  mean = (eval[0] + eval[1] + eval[2])/3;
  stdv = AIR_CAST(float,
                  sqrt((mean-eval[0])*(mean-eval[0])   /* not exactly stdv */
                       + (mean-eval[1])*(mean-eval[1]) 
                       + (mean-eval[2])*(mean-eval[2])));
  return mean ? AIR_CAST(float, stdv/(mean*SQRT6)) : 0.0f;
}
double _tenAnisoEval_RA_d(const double eval[3]) {
  double mean, stdv;
  mean = (eval[0] + eval[1] + eval[2])/3;
  stdv = sqrt((mean-eval[0])*(mean-eval[0])   /* not exactly standard dev */
              + (mean-eval[1])*(mean-eval[1]) 
              + (mean-eval[2])*(mean-eval[2]));
  return mean ? stdv/(mean*SQRT6) : 0.0;
}
float  _tenAnisoTen_RA_f(const float  tt[7]) {
  float mn, stdv, dev[7];
  mn = TEN_T_TRACE(tt)/3;
  TEN_T_SET(dev, tt[0], tt[1]-mn, tt[2], tt[3], tt[4]-mn, tt[5], tt[6]-mn);
  stdv = AIR_CAST(float, sqrt(TEN_T_DOT(dev, dev)));
  return mn ? AIR_CAST(float, stdv/(mn*SQRT6)) : 0.0f;
}
double _tenAnisoTen_RA_d(const double tt[7]) {
  double mn, stdv, dev[7];
  mn = TEN_T_TRACE(tt)/3;
  TEN_T_SET(dev, tt[0], tt[1]-mn, tt[2], tt[3], tt[4]-mn, tt[5], tt[6]-mn);
  stdv = sqrt(TEN_T_DOT(dev, dev));
  return mn ? stdv/(mn*SQRT6) : 0.0;
}


float  _tenAnisoEval_FA_f(const float  eval[3]) {
  float denom, mean, stdv;
  denom = 2.0f*(eval[0]*eval[0] + eval[1]*eval[1] + eval[2]*eval[2]);
  mean = (eval[0] + eval[1] + eval[2])/3;
  stdv = AIR_CAST(float,
                  (mean-eval[0])*(mean-eval[0]) /* not exactly stdv */
                  + (mean-eval[1])*(mean-eval[1]) 
                  + (mean-eval[2])*(mean-eval[2]));
  return denom ? AIR_CAST(float, sqrt(3.0*stdv/denom)) : 0.0f;
}
double _tenAnisoEval_FA_d(const double eval[3]) {
  double denom, mean, stdv;
  denom = 2.0*(eval[0]*eval[0] + eval[1]*eval[1] + eval[2]*eval[2]);
  mean = (eval[0] + eval[1] + eval[2])/3;
  stdv = ((mean-eval[0])*(mean-eval[0])   /* not exactly standard dev */
          + (mean-eval[1])*(mean-eval[1]) 
          + (mean-eval[2])*(mean-eval[2]));
  return denom ? sqrt(3.0*stdv/denom) : 0.0;
}
float  _tenAnisoTen_FA_f(const float  tt[7]) {
  float denom, mn, stdv, dev[7];
  denom = AIR_CAST(float, 2.0*TEN_T_DOT(tt, tt));
  mn = TEN_T_TRACE(tt)/3;
  TEN_T_SET(dev, tt[0], tt[1]-mn, tt[2], tt[3], tt[4]-mn, tt[5], tt[6]-mn);
  stdv = TEN_T_DOT(dev, dev);
  return denom ? AIR_CAST(float, sqrt(3.0*stdv/denom)) : 0.0f;
}
double _tenAnisoTen_FA_d(const double tt[7]) {
  double denom, mn, stdv, dev[7];
  denom = 2.0*TEN_T_DOT(tt, tt);
  mn = TEN_T_TRACE(tt)/3;
  TEN_T_SET(dev, tt[0], tt[1]-mn, tt[2], tt[3], tt[4]-mn, tt[5], tt[6]-mn);
  stdv = TEN_T_DOT(dev, dev);
  return denom ? AIR_CAST(float, sqrt(3.0*stdv/denom)) : 0.0;
}


float  _tenAnisoEval_VF_f(const float  eval[3]) {
  float mean;
  mean = (eval[0] + eval[1] + eval[2])/3.0f;
  mean = mean*mean*mean;
  return 1.0f - (mean ? eval[0]*eval[1]*eval[2]/mean : 0.0f);
}
double _tenAnisoEval_VF_d(const double eval[3]) {
  double mean;
  mean = (eval[0] + eval[1] + eval[2])/3;
  mean = mean*mean*mean;
  return 1.0 - (mean ? eval[0]*eval[1]*eval[2]/mean : 0.0);
}
float  _tenAnisoTen_VF_f(const float  ten[7]) {
  float mean;
  mean = TEN_T_TRACE(ten)/3.0f;
  mean = mean*mean*mean;
  return 1.0f - (mean ? TEN_T_DET(ten)/mean : 0.0f);
}
double _tenAnisoTen_VF_d(const double ten[7]) {
  double mean;
  mean = TEN_T_TRACE(ten)/3.0;
  mean = mean*mean*mean;
  return 1.0 - (mean ? TEN_T_DET(ten)/mean : 0.0);
}


float  _tenAnisoEval_B_f(const float  eval[3]) {
  return eval[0]*eval[1] + eval[0]*eval[2] + eval[1]*eval[2];
}
double _tenAnisoEval_B_d(const double eval[3]) {
  return eval[0]*eval[1] + eval[0]*eval[2] + eval[1]*eval[2];
}
float  _tenAnisoTen_B_f(const float  ten[7]) {
  return (ten[1]*ten[4] + ten[1]*ten[6] + ten[4]*ten[6] 
          - ten[2]*ten[2] - ten[3]*ten[3] - ten[5]*ten[5]);
}
double _tenAnisoTen_B_d(const double ten[7]) {
  return (ten[1]*ten[4] + ten[1]*ten[6] + ten[4]*ten[6] 
          - ten[2]*ten[2] - ten[3]*ten[3] - ten[5]*ten[5]);
}


float  _tenAnisoEval_Q_f(const float  eval[3]) {
  float A, B;
  A = -(eval[0] + eval[1] + eval[2]);
  B = _tenAnisoEval_B_f(eval);
  A = (A*A - 3.0f*B)/9.0f;
  return AIR_MAX(0, A);
}
double _tenAnisoEval_Q_d(const double eval[3]) {
  double A, B;
  A = -(eval[0] + eval[1] + eval[2]);
  B = _tenAnisoEval_B_d(eval);
  A = (A*A - 3.0*B)/9.0;
  return AIR_MAX(0, A);
}
float  _tenAnisoTen_Q_f(const float  ten[7]) {
  float A, B;
  A = -TEN_T_TRACE(ten);
  B = _tenAnisoTen_B_f(ten);
  A = (A*A - 3.0f*B)/9.0f;
  return AIR_MAX(0, A);
}
double _tenAnisoTen_Q_d(const double ten[7]) {
  double A, B;
  A = -TEN_T_TRACE(ten);
  B = _tenAnisoTen_B_d(ten);
  A = (A*A - 3.0*B)/9.0;
  return AIR_MAX(0, A);
}


float  _tenAnisoEval_R_f(const float  eval[3]) {
  float A, B, C;
  A = -(eval[0] + eval[1] + eval[2]);
  B = _tenAnisoEval_B_f(eval);
  C = -eval[0]*eval[1]*eval[2];
  return (-2*A*A*A + 9*A*B - 27*C)/54;
}
double _tenAnisoEval_R_d(const double eval[3]) {
  double A, B, C;
  A = -(eval[0] + eval[1] + eval[2]);
  B = _tenAnisoEval_B_d(eval);
  C = -eval[0]*eval[1]*eval[2];
  return (-2*A*A*A + 9*A*B - 27*C)/54;
}
float  _tenAnisoTen_R_f(const float  ten[7]) {
  float A, B, C;
  A = -TEN_T_TRACE(ten);
  B = _tenAnisoTen_B_f(ten);
  C = -TEN_T_DET(ten);
  return (-2*A*A*A + 9*A*B - 27*C)/54;
}
double _tenAnisoTen_R_d(const double ten[7]) {
  double A, B, C;
  A = -TEN_T_TRACE(ten);
  B = _tenAnisoTen_B_d(ten);
  C = -TEN_T_DET(ten);
  return (-2*A*A*A + 9*A*B - 27*C)/54;
}


float  _tenAnisoEval_S_f(const float  eval[3]) {
  return eval[0]*eval[0] + eval[1]*eval[1] + eval[2]*eval[2];
}
double _tenAnisoEval_S_d(const double eval[3]) {
  return eval[0]*eval[0] + eval[1]*eval[1] + eval[2]*eval[2];
}
float  _tenAnisoTen_S_f(const float  ten[7]) {
  return TEN_T_DOT(ten, ten);
}
double _tenAnisoTen_S_d(const double ten[7]) {
  return TEN_T_DOT(ten, ten);
}

#define OOSQRT2 0.70710678118654752440
float  _tenAnisoEval_Skew_f(const float  _eval[3]) {
  float Q, num, dnm, ret, mn, eval[3];
  mn = (_eval[0] + _eval[1] + _eval[2])/3;
  ELL_3V_SET(eval, _eval[0] - mn, _eval[1] - mn, _eval[2] - mn);
  Q = _tenAnisoEval_Q_f(eval);
  num = _tenAnisoEval_R_f(eval);
  dnm = AIR_CAST(float, Q*sqrt(2*Q));
  ret = dnm ? AIR_CAST(float, num/dnm) : 0.0f;
  return AIR_CAST(float, AIR_CLAMP(-OOSQRT2, ret, OOSQRT2));
}
double _tenAnisoEval_Skew_d(const double _eval[3]) {
  double Q, num, dnm, ret, mn, eval[3];
  mn = (_eval[0] + _eval[1] + _eval[2])/3;
  ELL_3V_SET(eval, _eval[0] - mn, _eval[1] - mn, _eval[2] - mn);
  Q = _tenAnisoEval_Q_d(eval);
  num = _tenAnisoEval_R_d(eval);
  dnm = Q*sqrt(2*Q);
  ret = dnm ? num/dnm : 0.0;
  return AIR_CLAMP(-OOSQRT2, ret, OOSQRT2);
}
float  _tenAnisoTen_Skew_f(const float  _t[7]) {
  float Q, num, dnm, ret, mn, ten[7];
  mn = TEN_T_TRACE(_t)/3;
  TEN_T_SET(ten, _t[0], _t[1]-mn, _t[2], _t[3], _t[4]-mn, _t[5], _t[6]-mn);
  Q = _tenAnisoTen_Q_f(ten);
  num = _tenAnisoTen_R_f(ten);
  dnm = AIR_CAST(float, Q*sqrt(2*Q));
  ret = dnm ? AIR_CAST(float, num/dnm) : 0.0f;
  return AIR_CAST(float, AIR_CLAMP(-OOSQRT2, ret, OOSQRT2));
}
double _tenAnisoTen_Skew_d(const double _t[7]) {
  double Q, num, dnm, ret, mn, ten[7];
  mn = TEN_T_TRACE(_t)/3;
  TEN_T_SET(ten, _t[0], _t[1]-mn, _t[2], _t[3], _t[4]-mn, _t[5], _t[6]-mn);
  Q = _tenAnisoTen_Q_d(ten);
  num = _tenAnisoTen_R_d(ten);
  dnm = Q*sqrt(2*Q);
  ret = dnm ? num/dnm : 0.0;
  return AIR_CLAMP(-OOSQRT2, ret, OOSQRT2);
}


float  _tenAnisoEval_Mode_f(const float  _eval[3]) {
  float n, d, mn, e[3], ret;
  mn = (_eval[0] + _eval[1] + _eval[2])/3;
  ELL_3V_SET(e, _eval[0] - mn, _eval[1] - mn, _eval[2] - mn);
  n = (e[0] + e[1] - 2*e[2])*(2*e[0] - e[1] - e[2])*(e[0] - 2*e[1] + e[2]);
  d = (e[0]*e[0] + e[1]*e[1] + e[2]*e[2] 
       - e[0]*e[1] - e[1]*e[2] - e[0]*e[2]);
  d = AIR_CAST(float, sqrt(AIR_MAX(0, d)));
  d = 2*d*d*d;
  ret = d ? AIR_CAST(float, n/d) : 0.0f;
  return AIR_CLAMP(-1, ret, 1);
}
double _tenAnisoEval_Mode_d(const double _eval[3]) {
  double n, d, mn, e[3], ret;
  mn = (_eval[0] + _eval[1] + _eval[2])/3;
  ELL_3V_SET(e, _eval[0] - mn, _eval[1] - mn, _eval[2] - mn);
  n = (e[0] + e[1] - 2*e[2])*(2*e[0] - e[1] - e[2])*(e[0] - 2*e[1] + e[2]);
  d = (e[0]*e[0] + e[1]*e[1] + e[2]*e[2] 
       - e[0]*e[1] - e[1]*e[2] - e[0]*e[2]);
  d = sqrt(AIR_MAX(0, d));
  d = 2*d*d*d;
  ret = d ? n/d : 0.0;
  return AIR_CLAMP(-1, ret, 1);
}
float  _tenAnisoTen_Mode_f(const float  tt[7]) {
  float mn, dev[7], tmp, ret;
  mn = TEN_T_TRACE(tt)/3.0f;
  TEN_T_SET(dev, tt[0], tt[1]-mn, tt[2], tt[3], tt[4]-mn, tt[5], tt[6]-mn);
  tmp = AIR_CAST(float, TEN_T_NORM(dev));
  tmp = tmp ? 1.0f/tmp : 0.0f;
  TEN_T_SCALE(dev, tmp, dev);
  ret = AIR_CAST(float, 3*SQRT6*TEN_T_DET(dev));
  return AIR_CLAMP(-1, ret, 1);
}
double _tenAnisoTen_Mode_d(const double tt[7]) {
  double mn, dev[7], tmp, ret;
  mn = TEN_T_TRACE(tt)/3.0;
  TEN_T_SET(dev, tt[0], tt[1]-mn, tt[2], tt[3], tt[4]-mn, tt[5], tt[6]-mn);
  tmp = TEN_T_NORM(dev);
  tmp = tmp ? 1.0/tmp : 0.0;
  TEN_T_SCALE(dev, tmp, dev);
  ret = 3*SQRT6*TEN_T_DET(dev);
  return AIR_CLAMP(-1, ret, 1);
}

/* NOTE: yes, the AIR_CLAMPs here are needed even though
** the _Skew_ functions clamp their output 
*/
#define SQRT2 1.41421356237309504880
float  _tenAnisoEval_Th_f(const float  eval[3]) {
  float tmp;
  tmp = AIR_CAST(float, SQRT2*_tenAnisoEval_Skew_f(eval));
  return AIR_CAST(float, acos(AIR_CLAMP(-1, tmp, 1))/3);
}
double _tenAnisoEval_Th_d(const double eval[3]) {
  double tmp;
  tmp = SQRT2*_tenAnisoEval_Skew_d(eval);
  return acos(AIR_CLAMP(-1, tmp, 1))/3;
}
float  _tenAnisoTen_Th_f(const float  ten[7]) {
  float tmp;
  tmp = AIR_CAST(float, SQRT2*_tenAnisoTen_Skew_f(ten));
  return AIR_CAST(float, acos(AIR_CLAMP(-1, tmp, 1))/3);
}
double _tenAnisoTen_Th_d(const double ten[7]) {
  double tmp;
  tmp = SQRT2*_tenAnisoTen_Skew_d(ten);
  return acos(AIR_CLAMP(-1, tmp, 1))/3;
}


float  _tenAnisoEval_Omega_f(const float  eval[3]) {
  return _tenAnisoEval_FA_f(eval)*(1.0f+_tenAnisoEval_Mode_f(eval))/2.0f;
}
double _tenAnisoEval_Omega_d(const double eval[3]) {
  return _tenAnisoEval_FA_d(eval)*(1.0f+_tenAnisoEval_Mode_d(eval))/2.0f;
}
float  _tenAnisoTen_Omega_f(const float  ten[7]) {
  return _tenAnisoTen_FA_f(ten)*(1.0f+_tenAnisoTen_Mode_f(ten))/2.0f;
}
double _tenAnisoTen_Omega_d(const double ten[7]) {
  return _tenAnisoTen_FA_d(ten)*(1.0f+_tenAnisoTen_Mode_d(ten))/2.0f;
}


float  _tenAnisoEval_Det_f(const float  eval[3]) {
  return eval[0]*eval[1]*eval[2];
}
double _tenAnisoEval_Det_d(const double eval[3]) {
  return eval[0]*eval[1]*eval[2];
}
float  _tenAnisoTen_Det_f(const float  ten[7]) {
  return TEN_T_DET(ten);
}
double _tenAnisoTen_Det_d(const double ten[7]) {
  return TEN_T_DET(ten);
}


float  _tenAnisoEval_Tr_f(const float  eval[3]) {
  return eval[0] + eval[1] + eval[2];
}
double _tenAnisoEval_Tr_d(const double eval[3]) {
  return eval[0] + eval[1] + eval[2];
}
float  _tenAnisoTen_Tr_f(const float  ten[7]) {
  return TEN_T_TRACE(ten);
}
double _tenAnisoTen_Tr_d(const double ten[7]) {
  return TEN_T_TRACE(ten);
}


float  _tenAnisoEval_eval0_f(const float  eval[3]) { return eval[0]; }
double _tenAnisoEval_eval0_d(const double eval[3]) { return eval[0]; }
float  _tenAnisoTen_eval0_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return eval[0];
}
double _tenAnisoTen_eval0_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return eval[0];
}

float  _tenAnisoEval_eval1_f(const float  eval[3]) { return eval[1]; }
double _tenAnisoEval_eval1_d(const double eval[3]) { return eval[1]; }
float  _tenAnisoTen_eval1_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return eval[1];
}
double _tenAnisoTen_eval1_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return eval[1];
}


float  _tenAnisoEval_eval2_f(const float  eval[3]) { return eval[2]; }
double _tenAnisoEval_eval2_d(const double eval[3]) { return eval[2]; }
float  _tenAnisoTen_eval2_f(const float  ten[7]) {
  float eval[3];
  tenEigensolve_f(eval, NULL, ten);
  return eval[2];
}
double _tenAnisoTen_eval2_d(const double ten[7]) {
  double eval[3];
  tenEigensolve_d(eval, NULL, ten);
  return eval[2];
}


float  (*_tenAnisoEval_f[TEN_ANISO_MAX+1])(const float  eval[3]) = {
  NULL,
  _tenAnisoEval_Conf_f,
  _tenAnisoEval_Cl1_f,
  _tenAnisoEval_Cp1_f,
  _tenAnisoEval_Ca1_f,
  _tenAnisoEval_Clpmin1_f,
  _tenAnisoEval_Cs1_f,
  _tenAnisoEval_Ct1_f,
  _tenAnisoEval_Cl2_f,
  _tenAnisoEval_Cp2_f,
  _tenAnisoEval_Ca2_f,
  _tenAnisoEval_Clpmin2_f,
  _tenAnisoEval_Cs2_f,
  _tenAnisoEval_Ct2_f,
  _tenAnisoEval_RA_f,
  _tenAnisoEval_FA_f,
  _tenAnisoEval_VF_f,
  _tenAnisoEval_B_f,
  _tenAnisoEval_Q_f,
  _tenAnisoEval_R_f,
  _tenAnisoEval_S_f,
  _tenAnisoEval_Skew_f,
  _tenAnisoEval_Mode_f,
  _tenAnisoEval_Th_f,
  _tenAnisoEval_Omega_f,
  _tenAnisoEval_Det_f,
  _tenAnisoEval_Tr_f,
  _tenAnisoEval_eval0_f,
  _tenAnisoEval_eval1_f,
  _tenAnisoEval_eval2_f
};
   
double (*_tenAnisoEval_d[TEN_ANISO_MAX+1])(const double eval[3]) = {
  NULL,
  _tenAnisoEval_Conf_d,
  _tenAnisoEval_Cl1_d,
  _tenAnisoEval_Cp1_d,
  _tenAnisoEval_Ca1_d,
  _tenAnisoEval_Clpmin1_d,
  _tenAnisoEval_Cs1_d,
  _tenAnisoEval_Ct1_d,
  _tenAnisoEval_Cl2_d,
  _tenAnisoEval_Cp2_d,
  _tenAnisoEval_Ca2_d,
  _tenAnisoEval_Clpmin2_d,
  _tenAnisoEval_Cs2_d,
  _tenAnisoEval_Ct2_d,
  _tenAnisoEval_RA_d,
  _tenAnisoEval_FA_d,
  _tenAnisoEval_VF_d,
  _tenAnisoEval_B_d,
  _tenAnisoEval_Q_d,
  _tenAnisoEval_R_d,
  _tenAnisoEval_S_d,
  _tenAnisoEval_Skew_d,
  _tenAnisoEval_Mode_d,
  _tenAnisoEval_Th_d,
  _tenAnisoEval_Omega_d,
  _tenAnisoEval_Det_d,
  _tenAnisoEval_Tr_d,
  _tenAnisoEval_eval0_d,
  _tenAnisoEval_eval1_d,
  _tenAnisoEval_eval2_d
};
   
float  (*_tenAnisoTen_f[TEN_ANISO_MAX+1])(const float  ten[7]) = {
  NULL,
  _tenAnisoTen_Conf_f,
  _tenAnisoTen_Cl1_f,
  _tenAnisoTen_Cp1_f,
  _tenAnisoTen_Ca1_f,
  _tenAnisoTen_Clpmin1_f,
  _tenAnisoTen_Cs1_f,
  _tenAnisoTen_Ct1_f,
  _tenAnisoTen_Cl2_f,
  _tenAnisoTen_Cp2_f,
  _tenAnisoTen_Ca2_f,
  _tenAnisoTen_Clpmin2_f,
  _tenAnisoTen_Cs2_f,
  _tenAnisoTen_Ct2_f,
  _tenAnisoTen_RA_f,
  _tenAnisoTen_FA_f,
  _tenAnisoTen_VF_f,
  _tenAnisoTen_B_f,
  _tenAnisoTen_Q_f,
  _tenAnisoTen_R_f,
  _tenAnisoTen_S_f,
  _tenAnisoTen_Skew_f,
  _tenAnisoTen_Mode_f,
  _tenAnisoTen_Th_f,
  _tenAnisoTen_Omega_f,
  _tenAnisoTen_Det_f,
  _tenAnisoTen_Tr_f,
  _tenAnisoTen_eval0_f,
  _tenAnisoTen_eval1_f,
  _tenAnisoTen_eval2_f
};
   
double (*_tenAnisoTen_d[TEN_ANISO_MAX+1])(const double ten[7]) = {
  NULL,
  _tenAnisoTen_Conf_d,
  _tenAnisoTen_Cl1_d,
  _tenAnisoTen_Cp1_d,
  _tenAnisoTen_Ca1_d,
  _tenAnisoTen_Clpmin1_d,
  _tenAnisoTen_Cs1_d,
  _tenAnisoTen_Ct1_d,
  _tenAnisoTen_Cl2_d,
  _tenAnisoTen_Cp2_d,
  _tenAnisoTen_Ca2_d,
  _tenAnisoTen_Clpmin2_d,
  _tenAnisoTen_Cs2_d,
  _tenAnisoTen_Ct2_d,
  _tenAnisoTen_RA_d,
  _tenAnisoTen_FA_d,
  _tenAnisoTen_VF_d,
  _tenAnisoTen_B_d,
  _tenAnisoTen_Q_d,
  _tenAnisoTen_R_d,
  _tenAnisoTen_S_d,
  _tenAnisoTen_Skew_d,
  _tenAnisoTen_Mode_d,
  _tenAnisoTen_Th_d,
  _tenAnisoTen_Omega_d,
  _tenAnisoTen_Det_d,
  _tenAnisoTen_Tr_d,
  _tenAnisoTen_eval0_d,
  _tenAnisoTen_eval1_d,
  _tenAnisoTen_eval2_d
};
   
float
tenAnisoEval_f(const float  eval[3], int aniso) {

  return (AIR_IN_OP(tenAnisoUnknown, aniso, tenAnisoLast)
          ? _tenAnisoEval_f[aniso](eval)
          : 0);
}

double
tenAnisoEval_d(const double eval[3], int aniso) {

  return (AIR_IN_OP(tenAnisoUnknown, aniso, tenAnisoLast)
          ? _tenAnisoEval_d[aniso](eval)
          : 0);
}

float
tenAnisoTen_f(const float  ten[7], int aniso) {

  return (AIR_IN_OP(tenAnisoUnknown, aniso, tenAnisoLast)
          ? _tenAnisoTen_f[aniso](ten)
          : 0);
}

double
tenAnisoTen_d(const double ten[7], int aniso) {

  return (AIR_IN_OP(tenAnisoUnknown, aniso, tenAnisoLast)
          ? _tenAnisoTen_d[aniso](ten)
          : 0);
}

#if 0
/*
******** tenAnisoCalc_f
**
** !!! THIS FUNCTION HAS BEEN MADE OBSOLETE BY THE NEW 
** !!! tenAnisoEval_{f,d} AND tenAnisoTen_{f,d} FUNCTIONS
** !!! THIS WILL LIKELY BE REMOVED FROM FUTURE RELEASES
**
** Because this function does not subtract out the eigenvalue mean
** when computing quantities like Skew and Mode, it has really lousy
** accuracy on those measures compared to tenAnisoEval_{f,d}.
** 
** given an array of three SORTED (descending) eigenvalues "e",
** calculates the anisotropy coefficients of Westin et al.,
** as well as various others.
**
** NOTE: with time, so many metrics have ended up here that under
** no cases should this be used in any kind of time-critical operations
**
** This does NOT use biff.  
*/
void
tenAnisoCalc_f(float c[TEN_ANISO_MAX+1], const float e[3]) {
  float e0, e1, e2, stdv, mean, sum, cl, cp, ca, ra, fa, vf, denom;
  float A, B, C, R, Q, N, D;

  if (!( e[0] >= e[1] && e[1] >= e[2] )) {
    fprintf(stderr, "tenAnisoCalc_f: eigen values not sorted: "
            "%g %g %g (%d %d)\n",
            e[0], e[1], e[2], e[0] >= e[1], e[1] >= e[2]);
  }
  if ((tenVerbose > 1) && !( e[0] >= 0 && e[1] >= 0 && e[2] >= 0 )) {
    fprintf(stderr, "tenAnisoCalc_f: eigen values not all >= 0: %g %g %g\n",
            e[0], e[1], e[2]);
  }
  e0 = AIR_MAX(e[0], 0);
  e1 = AIR_MAX(e[1], 0);
  e2 = AIR_MAX(e[2], 0);
  sum = e0 + e1 + e2;
  
  /* first version of cl, cp, cs */
  cl = sum ? (e0 - e1)/sum : 0.0f;
  c[tenAniso_Cl1] = cl;
  cp = sum ? 2*(e1 - e2)/sum : 0.0f;
  c[tenAniso_Cp1] = cp;
  ca = cl + cp;
  c[tenAniso_Ca1] = ca;
  c[tenAniso_Clpmin1] = AIR_MIN(cl, cp);
  /* extra logic here for equality with expressions above */
  c[tenAniso_Cs1] = sum ? 1 - ca : 0.0f; 
  c[tenAniso_Ct1] = ca ? cp/ca : 0;
  /* second version of cl, cp, cs */
  cl = e0 ? (e0 - e1)/e0 : 0.0f;
  c[tenAniso_Cl2] = cl;
  cp = e0 ? (e1 - e2)/e0 : 0.0f;
  c[tenAniso_Cp2] = cp;
  ca = cl + cp;
  c[tenAniso_Ca2] = ca;
  c[tenAniso_Clpmin2] = AIR_MIN(cl, cp);
  /* extra logic here for equality with expressions above */
  c[tenAniso_Cs2] = e0 ? 1 - ca : 0.0f;
  c[tenAniso_Ct2] = ca ? cp/ca : 0.0f;
  /* non-westin anisos */
  mean = sum/3.0f;
  stdv = AIR_CAST(float,
                  sqrt((mean-e0)*(mean-e0) /* okay, not exactly standard dev */
                       + (mean-e1)*(mean-e1) 
                       + (mean-e2)*(mean-e2)));
  ra = mean ? AIR_CAST(float, stdv/(mean*SQRT6)) : 0.0f;
  ra = AIR_CLAMP(0.0f, ra, 1.0f);
  c[tenAniso_RA] = ra;
  denom = 2.0f*(e0*e0 + e1*e1 + e2*e2);
  if (denom) {
    fa = AIR_CAST(float, stdv*sqrt(3.0/denom));
    fa = AIR_CLAMP(0.0f, fa, 1.0f);
  } else {
    fa = 0.0f;
  }
  c[tenAniso_FA] = fa;
  vf = 1 - (mean ? e0*e1*e2/(mean*mean*mean) : 0.0f);
  vf = AIR_CLAMP(0.0f, vf, 1.0f);
  c[tenAniso_VF] = vf;

  A = (-e0 - e1 - e2);
  B = c[tenAniso_B] = e0*e1 + e0*e2 + e1*e2;
  C = -e0*e1*e2;
  Q = c[tenAniso_Q] = (A*A - 3*B)/9;
  R = c[tenAniso_R] = (-2*A*A*A + 9*A*B - 27*C)/54;
  c[tenAniso_S] = e0*e0 + e1*e1 + e2*e2;
  c[tenAniso_Skew] = Q ? AIR_CAST(float, R/(Q*sqrt(2*Q))) : 0.0f;
  c[tenAniso_Skew] = AIR_CLAMP(-OOSQRT2, c[tenAniso_Skew], OOSQRT2);
  N = (e0 + e1 - 2*e2)*(2*e0 - e1 - e2)*(e0 - 2*e1 + e2);
  D = AIR_CAST(float, sqrt(e0*e0+e1*e1+e2*e2 - e0*e1-e1*e2-e0*e2));
  c[tenAniso_Mode] = D ? N/(2*D*D*D) : 0.0f;
  c[tenAniso_Mode] = AIR_CLAMP(-1, c[tenAniso_Mode], 1);
  c[tenAniso_Th] =
    AIR_CAST(float, acos(AIR_CLAMP(-1, sqrt(2)*c[tenAniso_Skew], 1))/3);
  c[tenAniso_Omega] = c[tenAniso_FA]*(1+c[tenAniso_Mode])/2;
  c[tenAniso_Det] = e0*e1*e2;
  c[tenAniso_Tr] = e0 + e1 + e2;
  c[tenAniso_eval0] = e0;
  c[tenAniso_eval1] = e1;
  c[tenAniso_eval2] = e2;
  return;
}
#endif

int
tenAnisoPlot(Nrrd *nout, int aniso, unsigned int res,
             int hflip, int whole, int nanout) {
  char me[]="tenAnisoMap", err[BIFF_STRLEN];
  float *out, tmp;
  unsigned int x, y;
  float m0[3], m1[3], m2[3], c0, c1, c2, e[3];
  float S = 1/3.0f, L = 1.0f, P = 1/2.0f;  /* these make Westin's original
                                              (cl,cp,cs) align with the 
                                              barycentric coordinates */

  if (airEnumValCheck(tenAniso, aniso)) {
    sprintf(err, "%s: invalid aniso (%d)", me, aniso);
    biffAdd(TEN, err); return 1;
  }
  if (!(res > 2)) {
    sprintf(err, "%s: resolution (%d) invalid", me, res);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 2,
                        AIR_CAST(size_t, res), AIR_CAST(size_t, res))) {
    sprintf(err, "%s: ", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  out = (float *)nout->data;
  if (whole) {
    ELL_3V_SET(m0, 1, 0, 0);
    ELL_3V_SET(m1, 0, 1, 0);
    ELL_3V_SET(m2, 0, 0, 1);
  } else {
    ELL_3V_SET(m0, S, S, S);
    if (hflip) {
      ELL_3V_SET(m1, P, P, 0);
      ELL_3V_SET(m2, L, 0, 0);
    } else {
      ELL_3V_SET(m1, L, 0, 0);
      ELL_3V_SET(m2, P, P, 0);
    }
  }
  for (y=0; y<res; y++) {
    for (x=0; x<=y; x++) {
      /* (c0,c1,c2) are the barycentric coordinates */
      c0 = AIR_CAST(float, 1.0 - AIR_AFFINE(-0.5, y, res-0.5, 0.0, 1.0));
      c2 = AIR_CAST(float, AIR_AFFINE(-0.5, x, res-0.5, 0.0, 1.0));
      c1 = 1 - c0 - c2;
      e[0] = c0*m0[0] + c1*m1[0] + c2*m2[0];
      e[1] = c0*m0[1] + c1*m1[1] + c2*m2[1];
      e[2] = c0*m0[2] + c1*m1[2] + c2*m2[2];
      ELL_SORT3(e[0], e[1], e[2], tmp); /* got some warnings w/out this */
      out[x + res*y] = tenAnisoEval_f(e, aniso);
    }
    if (nanout) {
      for (x=y+1; x<res; x++) {
        out[x + res*y] = AIR_NAN;
      }
    }
  }

  return 0;
}

int
tenAnisoVolume(Nrrd *nout, const Nrrd *nin, int aniso, double confThresh) {
  char me[]="tenAnisoVolume", err[BIFF_STRLEN];
  size_t N, I;
  float *out, *in, *tensor;
  int map[NRRD_DIM_MAX];
  size_t sx, sy, sz, size[3];

  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenAniso, aniso)) {
    sprintf(err, "%s: invalid aniso (%d)", me, aniso);
    biffAdd(TEN, err); return 1;
  }
  confThresh = AIR_CLAMP(0.0, confThresh, 1.0);

  size[0] = sx = nin->axis[1].size;
  size[1] = sy = nin->axis[2].size;
  size[2] = sz = nin->axis[3].size;
  N = sx*sy*sz;
  if (nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 3, sx, sy, sz)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  out = (float *)nout->data;
  in = (float *)nin->data;
  for (I=0; I<=N-1; I++) {
    /* tenVerbose = (I == 911327); */
    tensor = in + I*7;
    if (tenAniso_Conf != aniso && tensor[0] < confThresh) {
      out[I] = 0.0;
      continue;
    }
    /* no longer used 
    tenEigensolve_f(eval, NULL, tensor);
    if (!(AIR_EXISTS(eval[0]) && AIR_EXISTS(eval[1]) && AIR_EXISTS(eval[2]))) {
      copyI = I;
      NRRD_COORD_GEN(coord, size, 3, copyI);
      sprintf(err, "%s: not all eigenvalues exist (%g,%g,%g) at sample "
              "%d = (%d,%d,%d)",
              me, eval[0], eval[1], eval[2], (int)I,
              (int)coord[0], (int)coord[1], (int)coord[2]);
      biffAdd(TEN, err); return 1;
    }
    */
    out[I] = tenAnisoTen_f(tensor, aniso);
    if (!AIR_EXISTS(out[I])) {
      size_t copyI, coord[3];
      copyI = I;
      NRRD_COORD_GEN(coord, size, 3, copyI);
      sprintf(err, "%s: generated non-existent aniso %g from tensor "
              "(%g) %g %g %g   %g %g   %g at sample %u = (%u,%u,%u)", me,
              out[I],
              tensor[0], tensor[1], tensor[2], tensor[3],
              tensor[4], tensor[5], tensor[6],
              AIR_CAST(unsigned int, I),
              AIR_CAST(unsigned int, coord[0]),
              AIR_CAST(unsigned int, coord[1]),
              AIR_CAST(unsigned int, coord[2]));
      biffAdd(TEN, err); return 1;
    }
  }
  ELL_3V_SET(map, 1, 2, 3);
  if (nrrdAxisInfoCopy(nout, nin, map, NRRD_AXIS_INFO_SIZE_BIT)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (nrrdBasicInfoCopy(nout, nin,
                        NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }

  return 0;
}

int
tenAnisoHistogram(Nrrd *nout, const Nrrd *nin, const Nrrd *nwght,
                  int right, int version, unsigned int res) {
  char me[]="tenAnisoHistogram", err[BIFF_STRLEN];
  size_t N, I;
  int csIdx, clIdx, cpIdx;
  float *tdata, *out, eval[3],
    cs, cl, cp, (*wlup)(const void *data, size_t idx), weight;
  unsigned int yres, xi, yi;

  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nwght) {
    if (nrrdCheck(nwght)) {
      sprintf(err, "%s: trouble with weighting nrrd", me);
      biffMove(TEN, err, NRRD); return 1;
    }
    if (nrrdElementNumber(nwght)
        != nrrdElementNumber(nin)/nrrdKindSize(nrrdKind3DMaskedSymMatrix) ) {
      sprintf(err, "%s: # elements in weight nrrd (" _AIR_SIZE_T_CNV 
              ") != # tensors (" _AIR_SIZE_T_CNV ")", me,
              nrrdElementNumber(nwght),
              nrrdElementNumber(nin)/nrrdKindSize(nrrdKind3DMaskedSymMatrix));
      biffAdd(TEN, err); return 1;
    }
  }
  if (!( 1 == version || 2 == version )) {
    sprintf(err, "%s: version (%d) wasn't 1 or 2", me, version);
    biffAdd(TEN, err); return 1;
  }
  if (!(res > 10)) {
    sprintf(err, "%s: resolution (%d) invalid", me, res);
    biffAdd(TEN, err); return 1;
  }
  if (right) {
    yres = AIR_CAST(unsigned int, AIR_CAST(double, res)/sqrt(3));
  } else {
    yres = res;
  }
  if (nwght) {
    wlup = nrrdFLookup[nwght->type];
  } else {
    wlup = NULL;
  }
  if (nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 2,
                        AIR_CAST(size_t, res), AIR_CAST(size_t, yres))) {
    sprintf(err, "%s: ", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  out = (float *)nout->data;
  tdata = (float *)nin->data;
  if (right || 1 == version) {
    clIdx = tenAniso_Cl1;
    cpIdx = tenAniso_Cp1;
    csIdx = tenAniso_Cs1;
  } else {
    clIdx = tenAniso_Cl2;
    cpIdx = tenAniso_Cp2;
    csIdx = tenAniso_Cs2;
  }
  N = nrrdElementNumber(nin)/nrrdKindSize(nrrdKind3DMaskedSymMatrix);
  for (I=0; I<N; I++) {
    tenEigensolve_f(eval, NULL, tdata);
    cl = tenAnisoEval_f(eval, clIdx);
    cp = tenAnisoEval_f(eval, cpIdx);
    cs = tenAnisoEval_f(eval, csIdx);
    if (right) {
      xi = AIR_CAST(unsigned int, cs*0 + cl*(res-1) + cp*0);
      yi = AIR_CAST(unsigned int, cs*0 + cl*(yres-1) + cp*(yres-1));
    } else {
      xi = AIR_CAST(unsigned int, cs*0 + cl*0 + cp*(res-1));
      yi = AIR_CAST(unsigned int, cs*0 + cl*(res-1) + cp*(res-1));
    }
    weight = wlup ? wlup(nwght->data, I) : 1.0f;
    if (xi < res && yi < yres-1) {
      out[xi + res*yi] += tdata[0]*weight;
    }
    tdata += nrrdKindSize(nrrdKind3DMaskedSymMatrix);
  }
  
  return 0;
}

tenEvecRGBParm *
tenEvecRGBParmNew() {
  tenEvecRGBParm *rgbp;
  
  rgbp = AIR_CAST(tenEvecRGBParm *, calloc(1, sizeof(tenEvecRGBParm)));
  if (rgbp) {
    rgbp->which = 0;
    rgbp->aniso = tenAniso_Cl2;
    rgbp->confThresh = 0.5;
    rgbp->anisoGamma = 1.0;
    rgbp->gamma = 1.0;
    rgbp->bgGray = 0.0;
    rgbp->isoGray = 0.0;
    rgbp->maxSat = 1.0;
    rgbp->typeOut = nrrdTypeFloat;
    rgbp->genAlpha = AIR_FALSE;
  }
  return rgbp;
}

tenEvecRGBParm *
tenEvecRGBParmNix(tenEvecRGBParm *rgbp) {
  
  if (rgbp) {
    airFree(rgbp);
  }
  return NULL;
}

int
tenEvecRGBParmCheck(const tenEvecRGBParm *rgbp) {
  char me[]="tenEvecRGBParmCheck", err[BIFF_STRLEN];

  if (!rgbp) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( rgbp->which <= 2 )) {
    sprintf(err, "%s: which must be 0, 1, or 2 (not %u)", me, rgbp->which);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenAniso, rgbp->aniso)) {
    sprintf(err, "%s: anisotropy metric %d not valid", me, rgbp->aniso);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdTypeDefault != rgbp->typeOut
      && airEnumValCheck(nrrdType, rgbp->typeOut)) {
    sprintf(err, "%s: output type (%d) not valid", me, rgbp->typeOut);
    biffAdd(TEN, err); return 1;
  }
  return 0;
}

float
_tenEvecRGBComp_f(float conf, float aniso, float comp,
                  const tenEvecRGBParm *rgbp) {
  double X;

  X = AIR_ABS(comp);
  X = pow(X, 1.0/rgbp->gamma);
  X = AIR_LERP(rgbp->maxSat*aniso, rgbp->isoGray, X);
  return AIR_CAST(float, conf > rgbp->confThresh ? X : rgbp->bgGray);
}

double
_tenEvecRGBComp_d(double conf, double aniso, double comp,
                  const tenEvecRGBParm *rgbp) {
  double X;

  X = AIR_ABS(comp);
  X = pow(X, 1.0/rgbp->gamma);
  X = AIR_LERP(rgbp->maxSat*aniso, rgbp->isoGray, X);
  return conf > rgbp->confThresh ? X : rgbp->bgGray;
}

void
tenEvecRGBSingle_f(float RGB[3], float conf, const float eval[3], 
                   const float evec[3], const tenEvecRGBParm *rgbp) {
  float aniso;

  if (RGB && eval && rgbp) {
    aniso = tenAnisoEval_f(eval, rgbp->aniso);
    aniso = AIR_CAST(float, pow(aniso, 1.0/rgbp->anisoGamma));
    ELL_3V_SET(RGB,
               _tenEvecRGBComp_f(conf, aniso, evec[0], rgbp),
               _tenEvecRGBComp_f(conf, aniso, evec[1], rgbp),
               _tenEvecRGBComp_f(conf, aniso, evec[2], rgbp));
  }
  return;
}

void
tenEvecRGBSingle_d(double RGB[3], double conf, const double eval[3], 
                   const double evec[3], const tenEvecRGBParm *rgbp) {
  double aniso;

  if (RGB && eval && rgbp) {
    aniso = tenAnisoEval_d(eval, rgbp->aniso);
    aniso = pow(aniso, 1.0/rgbp->anisoGamma);
    ELL_3V_SET(RGB,
               _tenEvecRGBComp_d(conf, aniso, evec[0], rgbp),
               _tenEvecRGBComp_d(conf, aniso, evec[1], rgbp),
               _tenEvecRGBComp_d(conf, aniso, evec[2], rgbp));
  }
  return;
}

