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


#include "ell.h"

void
ell_3m_mul_f(float m3[9], const float _m1[9], const float _m2[9]) {
  float m1[9], m2[9];

  ELL_3M_COPY(m1, _m1);
  ELL_3M_COPY(m2, _m2);
  ELL_3M_MUL(m3, m1, m2);
}

void
ell_3m_mul_d(double m3[9], const double _m1[9], const double _m2[9]) {
  double m1[9], m2[9];

  ELL_3M_COPY(m1, _m1);
  ELL_3M_COPY(m2, _m2);
  ELL_3M_MUL(m3, m1, m2);
}

void
ell_3m_pre_mul_f(float _m[9], const float x[9]) {
  float m[9];
  ELL_3M_MUL(m, _m, x);
  ELL_3M_COPY(_m, m);
}

void
ell_3m_pre_mul_d(double _m[9], const double x[9]) {
  double m[9];
  ELL_3M_MUL(m, _m, x);
  ELL_3M_COPY(_m, m);
}

void
ell_3m_post_mul_f(float _m[9], const float x[9]) {
  float m[9];
  ELL_3M_MUL(m, x, _m);
  ELL_3M_COPY(_m, m);
}

void
ell_3m_post_mul_d(double _m[9], const double x[9]) {
  double m[9];
  ELL_3M_MUL(m, x, _m);
  ELL_3M_COPY(_m, m);
}

float
ell_3m_det_f(float m[9]) {
  return ELL_3M_DET(m);
}

double
ell_3m_det_d(double m[9]) {
  return ELL_3M_DET(m);
}

void
ell_3m_inv_f(float i[9], const float m[9]) {
  float det;

  ELL_3M_INV(i, m, det);
}

void
ell_3m_inv_d(double i[9], const double m[9]) {
  double det;

  ELL_3M_INV(i, m, det);
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */

void
ell_4m_mul_f(float m3[16], const float _m1[16], const float _m2[16]) {
  float m1[16], m2[16];

  ELL_4M_COPY(m1, _m1);
  ELL_4M_COPY(m2, _m2);
  ELL_4M_MUL(m3, m1, m2);
}

void
ell_4m_mul_d(double m3[16], const double _m1[16], const double _m2[16]) {
  double m1[16], m2[16];

  ELL_4M_COPY(m1, _m1);
  ELL_4M_COPY(m2, _m2);
  ELL_4M_MUL(m3, m1, m2);
}

void
ell_4m_pre_mul_f(float _m[16], const float x[16]) {
  float m[16];
  ELL_4M_MUL(m, _m, x);
  ELL_4M_COPY(_m, m);
}

void
ell_4m_pre_mMul_d(double _m[16], const double x[16]) {
  double m[16];
  ELL_4M_MUL(m, _m, x);
  ELL_4M_COPY(_m, m);
}

void
ell_4m_post_mul_f(float _m[16], const float x[16]) {
  float m[16];
  ELL_4M_MUL(m, x, _m);
  ELL_4M_COPY(_m, m);
}

void
ell_4m_post_mul_d(double _m[16], const double x[16]) {
  double m[16];
  ELL_4M_MUL(m, x, _m);
  ELL_4M_COPY(_m, m);
}

float
ell_4m_det_f(float m[16]) {
  return ELL_4M_DET(m);
}

double
ell_4m_det_d(double m[16]) {
  return ELL_4M_DET(m);
}

#define _4INV \
  det = ELL_4M_DET(m); \
  i[ 0] =  _ELL_3M_DET((m)[ 5],(m)[ 6],(m)[ 7], \
                       (m)[ 9],(m)[10],(m)[11], \
                       (m)[13],(m)[14],(m)[15])/det; \
  i[ 1] = -_ELL_3M_DET((m)[ 1],(m)[ 2],(m)[ 3], \
                       (m)[ 9],(m)[10],(m)[11], \
                       (m)[13],(m)[14],(m)[15])/det; \
  i[ 2] =  _ELL_3M_DET((m)[ 1],(m)[ 2],(m)[ 3], \
                       (m)[ 5],(m)[ 6],(m)[ 7], \
                       (m)[13],(m)[14],(m)[15])/det; \
  i[ 3] = -_ELL_3M_DET((m)[ 1],(m)[ 2],(m)[ 3], \
                       (m)[ 5],(m)[ 6],(m)[ 7], \
                       (m)[ 9],(m)[10],(m)[11])/det; \
  i[ 4] = -_ELL_3M_DET((m)[ 4],(m)[ 6],(m)[ 7], \
                       (m)[ 8],(m)[10],(m)[11], \
                       (m)[12],(m)[14],(m)[15])/det; \
  i[ 5] =  _ELL_3M_DET((m)[ 0],(m)[ 2],(m)[ 3], \
                       (m)[ 8],(m)[10],(m)[11], \
                       (m)[12],(m)[14],(m)[15])/det; \
  i[ 6] = -_ELL_3M_DET((m)[ 0],(m)[ 2],(m)[ 3], \
                       (m)[ 4],(m)[ 6],(m)[ 7], \
                       (m)[12],(m)[14],(m)[15])/det; \
  i[ 7] =  _ELL_3M_DET((m)[ 0],(m)[ 2],(m)[ 3], \
                       (m)[ 4],(m)[ 6],(m)[ 7], \
                       (m)[ 8],(m)[10],(m)[11])/det; \
  i[ 8] =  _ELL_3M_DET((m)[ 4],(m)[ 5],(m)[ 7], \
                       (m)[ 8],(m)[ 9],(m)[11], \
                       (m)[12],(m)[13],(m)[15])/det; \
  i[ 9] = -_ELL_3M_DET((m)[ 0],(m)[ 1],(m)[ 3], \
                       (m)[ 8],(m)[ 9],(m)[11], \
                       (m)[12],(m)[13],(m)[15])/det; \
  i[10] =  _ELL_3M_DET((m)[ 0],(m)[ 1],(m)[ 3], \
                       (m)[ 4],(m)[ 5],(m)[ 7], \
                       (m)[12],(m)[13],(m)[15])/det; \
  i[11] = -_ELL_3M_DET((m)[ 0],(m)[ 1],(m)[ 3], \
                       (m)[ 4],(m)[ 5],(m)[ 7], \
                       (m)[ 8],(m)[ 9],(m)[11])/det; \
  i[12] = -_ELL_3M_DET((m)[ 4],(m)[ 5],(m)[ 6], \
                       (m)[ 8],(m)[ 9],(m)[10], \
                       (m)[12],(m)[13],(m)[14])/det; \
  i[13] =  _ELL_3M_DET((m)[ 0],(m)[ 1],(m)[ 2], \
                       (m)[ 8],(m)[ 9],(m)[10], \
                       (m)[12],(m)[13],(m)[14])/det; \
  i[14] = -_ELL_3M_DET((m)[ 0],(m)[ 1],(m)[ 2], \
                       (m)[ 4],(m)[ 5],(m)[ 6], \
                       (m)[12],(m)[13],(m)[14])/det; \
  i[15] =  _ELL_3M_DET((m)[ 0],(m)[ 1],(m)[ 2], \
                       (m)[ 4],(m)[ 5],(m)[ 6], \
                       (m)[ 8],(m)[ 9],(m)[10])/det

void
ell_4m_inv_f(float i[16], const float m[16]) {
  float det;

  _4INV;
}

void
ell_4m_inv_d(double i[16], const double m[16]) {
  double det;

  _4INV;
}

void
ell_6m_mul_d(double AB[36], const double A[36], const double B[36]) {
  unsigned int ll, mm, nn;
  double tmp;

  if (!( AB && A && B )) {
    return;
  }
  for (ll=0; ll<6; ll++) {
    for (nn=0; nn<6; nn++) {
      tmp = 0;
      for (mm=0; mm<6; mm++) {
        tmp += A[mm + 6*ll]*B[nn + 6*mm];
      }
      AB[nn + 6*ll] = tmp;
    }
  }
  return;
}

/*
** Thanks to:
** http://jgt.akpeters.com/papers/MollerHughes99/code.html
*/
void
ell_3m_rotate_between_d(double rot[9], double from[3], double to[3]) {
  double v[3];
  double e, h, f;

  if (!( rot && from && to)) {
    return;
  }
  ELL_3V_CROSS(v, from, to);
  e = ELL_3V_DOT(from, to);
  f = AIR_ABS(e);
  if (f > 0.9999999) {   /* "from" and "to"-vector almost parallel */
    double u[3], v[3];   /* temporary storage vectors */
    double x[3];         /* vector most nearly orthogonal to "from" */
    double c1, c2, c3;   /* coefficients for later use */
    int i, j;
    
    x[0] = AIR_ABS(from[0]);
    x[1] = AIR_ABS(from[1]);
    x[2] = AIR_ABS(from[2]);

    if (x[0] < x[1]) {
      if (x[0] < x[2]) {
        x[0] = 1.0; x[1] = x[2] = 0.0;
      } else {
        x[2] = 1.0; x[0] = x[1] = 0.0;
      }
    } else {
      if (x[1] < x[2]) {
        x[1] = 1.0; x[0] = x[2] = 0.0;
      } else {
        x[2] = 1.0; x[0] = x[1] = 0.0;
      }
    }

    u[0] = x[0] - from[0]; u[1] = x[1] - from[1]; u[2] = x[2] - from[2];
    v[0] = x[0] - to[0];   v[1] = x[1] - to[1];   v[2] = x[2] - to[2];

    c1 = 2.0 / ELL_3V_DOT(u, u);
    c2 = 2.0 / ELL_3V_DOT(v, v);
    c3 = c1 * c2  * ELL_3V_DOT(u, v);

    for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
        rot[3*i + j] =  - c1 * u[i] * u[j]
                     - c2 * v[i] * v[j]
                     + c3 * v[i] * u[j];
      }
      rot[3*i + i] += 1.0;
    }
  } else { /* the most common case, unless "from"="to", or "from"=-"to" */
    double hvx, hvz, hvxy, hvxz, hvyz;
    h = 1.0/(1.0 + e);      /* optimization by Gottfried Chen */
    hvx = h * v[0];
    hvz = h * v[2];
    hvxy = hvx * v[1];
    hvxz = hvx * v[2];
    hvyz = hvz * v[1];
    rot[3*0 + 0] = e + hvx * v[0];
    rot[3*0 + 1] = hvxy - v[2];
    rot[3*0 + 2] = hvxz + v[1];

    rot[3*1 + 0] = hvxy + v[2];
    rot[3*1 + 1] = e + h * v[1] * v[1];
    rot[3*1 + 2] = hvyz - v[0];

    rot[3*2 + 0] = hvxz - v[1];
    rot[3*2 + 1] = hvyz + v[0];
    rot[3*2 + 2] = e + hvz * v[2];
  }
  return;
}

