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

#ifndef ELL_HAS_BEEN_INCLUDED
#define ELL_HAS_BEEN_INCLUDED

#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/nrrd.h>

#include "ellMacros.h"

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(ell_EXPORTS) || defined(teem_EXPORTS)
#    define ELL_EXPORT extern __declspec(dllexport)
#  else
#    define ELL_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define ELL_EXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ELL ell_biff_key
#define ELL_EPS 1.0e-10

/*
******** ell_cubic_root* enum
**
** return values for ell_cubic
*/
enum {
  ell_cubic_root_unknown,         /* 0 */
  ell_cubic_root_single,          /* 1 */
  ell_cubic_root_triple,          /* 2 */
  ell_cubic_root_single_double,   /* 3 */
  ell_cubic_root_three,           /* 4 */
  ell_cubic_root_last
};

#define ELL_CUBIC_ROOT_MAX           4

/* 
** Note: *** PRE-TEEM 1.7 *** matrix element ordering was:
**
**     0   3   6
**     1   4   7      for 3x3
**     2   5   8
**
**   0   4   8  12
**   1   5   9  13    for 4x4
**   2   6  10  14
**   3   7  11  15
**
** as of TEEM 1.7, matrix element ordering is:
**
**     0   1   2
**     3   4   5      for 3x3
**     6   7   8
**
**   0   1   2   3
**   4   5   6   7    for 4x4
**   8   9  10  11
**  12  13  14  15
**
** all vectors are still, logically, COLUMN vectors
*/

/* miscEll.c */
ELL_EXPORT const char *ell_biff_key;
ELL_EXPORT airEnum *ell_cubic_root;
ELL_EXPORT int ell_debug;
ELL_EXPORT void ell_3m_print_f(FILE *f, const float s[9]);
ELL_EXPORT void ell_3v_print_f(FILE *f, const float s[3]);
ELL_EXPORT void ell_3m_print_d(FILE *f, const double s[9]);
ELL_EXPORT void ell_3v_print_d(FILE *f, const double s[3]);
ELL_EXPORT void ell_4m_print_f(FILE *f, const float s[16]);
ELL_EXPORT void ell_4v_print_f(FILE *f, const float s[4]);
ELL_EXPORT void ell_4m_print_d(FILE *f, const double s[16]);
ELL_EXPORT void ell_4v_print_d(FILE *f, const double s[4]);

/* vecEll.c */
ELL_EXPORT void ell_3v_perp_f(float p[3], const float v[3]);
ELL_EXPORT void ell_3v_perp_d(double p[3], const double v[3]);
ELL_EXPORT void ell_3mv_mul_f(float v2[3],
                              const float m[9], const float v1[3]);
ELL_EXPORT void ell_3mv_mul_d(double v2[3],
                              const double m[9], const double v1[3]);
ELL_EXPORT void ell_4mv_mul_f(float v2[4],
                              const float m[16], const float v1[4]);
ELL_EXPORT void ell_4mv_mul_d(double v2[4],
                              const double m[16], const double v1[4]);
ELL_EXPORT float ell_3v_angle_f(float u[3], float v[3]);
ELL_EXPORT double ell_3v_angle_d(double u[3], double v[3]);

/* mat.c */
ELL_EXPORT void ell_3m_mul_f(float m3[9],
                             const float m1[9], const float m2[9]);
ELL_EXPORT void ell_3m_mul_d(double m3[9],
                             const double m1[9], const double m2[9]);
ELL_EXPORT void ell_3m_pre_mul_f(float m[9], const float x[9]);
ELL_EXPORT void ell_3m_pre_mul_d(double m[9], const double x[9]);
ELL_EXPORT void ell_3m_post_mul_f(float m[9], const float x[9]);
ELL_EXPORT void ell_3m_post_mul_d(double m[9], const double x[9]);
ELL_EXPORT float ell_3m_det_f(float m[9]);
ELL_EXPORT double ell_3m_det_d(double m[9]);
ELL_EXPORT void ell_3m_inv_f(float i[9], const float m[9]);
ELL_EXPORT void ell_3m_inv_d(double i[9], const double m[9]);
ELL_EXPORT void ell_4m_mul_f(float m3[16],
                             const float m1[16], const float m2[16]);
ELL_EXPORT void ell_4m_mul_d(double m3[16],
                             const double m1[16], const double m2[16]);
ELL_EXPORT void ell_4m_pre_mul_f(float m[16], const float x[16]);
ELL_EXPORT void ell_4m_pre_mul_d(double m[16], const double x[16]);
ELL_EXPORT void ell_4m_post_mul_f(float m[16], const float x[16]);
ELL_EXPORT void ell_4m_post_mul_d(double m[16], const double x[16]);
ELL_EXPORT float ell_4m_det_f(float m[16]);
ELL_EXPORT double ell_4m_det_d(double m[16]);
ELL_EXPORT void ell_4m_inv_f(float i[16], const float m[16]);
ELL_EXPORT void ell_4m_inv_d(double i[16], const double m[16]);
ELL_EXPORT void ell_6m_mul_d(double AB[36],
                             const double A[36], const double B[36]);
ELL_EXPORT void ell_3m_rotate_between_d(double rot[9], double from[3],
                                        double to[3]);

/* 
** Note: quaternion element ordering is:
**
**   w  x  y  z
**   0  1  2  3
**
**  where w is the real component and (x,y,z) is the imaginary component
**
** Nowhere in ell is there the assumption that any given quaternion is
** a unit-length quaternion.  Sometimes returned quaternions will be 
** unit-length, and sometimes given quaternions must be normalized internally
** prior to doing some operation, but its not something that ell users
** need worry about.
*/

/* quat.c */
ELL_EXPORT void ell_3m_to_q_f( float q[4], const  float m[9]);
ELL_EXPORT void ell_3m_to_q_d(double q[4], const double m[9]);
ELL_EXPORT void ell_4m_to_q_f( float q[4], const  float m[16]);
ELL_EXPORT void ell_4m_to_q_d(double q[4], const double m[16]);
ELL_EXPORT void ell_q_to_3m_f( float m[9], const  float q[4]);
ELL_EXPORT void ell_q_to_3m_d(double m[9], const double q[4]);
ELL_EXPORT void ell_q_to_4m_f( float m[16], const  float q[4]);
ELL_EXPORT void ell_q_to_4m_d(double m[16], const double q[4]);
ELL_EXPORT  float ell_q_to_aa_f( float axis[3], const  float q[4]);
ELL_EXPORT double ell_q_to_aa_d(double axis[3], const double q[4]);
ELL_EXPORT void ell_aa_to_q_f(float q[4],
                              const  float angle, const  float axis[3]);
ELL_EXPORT void ell_aa_to_q_d(double q[4],
                              const double angle, const double axis[3]);
ELL_EXPORT void ell_aa_to_3m_f(float m[9],
                               const  float angle, const  float axis[3]);
ELL_EXPORT void ell_aa_to_3m_d(double m[9],
                               const double angle, const double axis[3]);
ELL_EXPORT void ell_aa_to_4m_f(float m[16],
                               const  float angle, const  float axis[3]);
ELL_EXPORT void ell_aa_to_4m_d(double m[16],
                               const double angle, const double axis[3]);
ELL_EXPORT  float ell_3m_to_aa_f( float axis[3], const  float m[9]);
ELL_EXPORT double ell_3m_to_aa_d(double axis[3], const double m[9]);
ELL_EXPORT  float ell_4m_to_aa_f( float axis[3], const  float m[16]);
ELL_EXPORT double ell_4m_to_aa_d(double axis[3], const double m[16]);
ELL_EXPORT void ell_q_mul_f(float q3[4],
                            const  float q1[4], const  float q2[4]);
ELL_EXPORT void ell_q_mul_d(double q3[4],
                            const double q1[4], const double q2[4]);
ELL_EXPORT void ell_q_inv_f( float qi[4], const  float q[4]);
ELL_EXPORT void ell_q_inv_d(double qi[4], const double q[4]);
ELL_EXPORT void ell_q_pow_f( float q2[4], const  float q1[4], const  float p);
ELL_EXPORT void ell_q_pow_d(double q2[4], const double q1[4], const double p);
ELL_EXPORT void ell_q_div_f(float q3[4],
                            const  float q1[4], const  float q2[4]);
ELL_EXPORT void ell_q_div_d(double q3[4],
                            const double q1[4], const double q2[4]);
ELL_EXPORT void ell_q_exp_f( float q2[4], const  float q1[4]);
ELL_EXPORT void ell_q_exp_d(double q2[4], const double q1[4]);
ELL_EXPORT void ell_q_log_f( float q2[4], const  float q1[4]);
ELL_EXPORT void ell_q_log_d(double q2[4], const double q1[4]);
ELL_EXPORT void ell_q_3v_rotate_f(float v2[3],
                                  const  float q[4], const  float v1[3]);
ELL_EXPORT void ell_q_3v_rotate_d(double v2[3],
                                  const double q[4], const double v1[3]);
ELL_EXPORT void ell_q_4v_rotate_f(float v2[4],
                                  const  float q[4], const  float v1[4]);
ELL_EXPORT void ell_q_4v_rotate_d(double v2[4],
                                  const double q[4], const double v1[4]);
ELL_EXPORT int ell_q_avg4_d(double m[4], unsigned int *iterP,
                            const double _q1[4], const double _q2[4],
                            const double _q3[4], const double _q4[4],
                            const double _wght[4], 
                            const double eps, const unsigned int maxIter);
ELL_EXPORT int ell_q_avgN_d(double mm[4], unsigned int *iterP,
                            const double *qq, double *qbuff,
                            const double *wght,
                            const unsigned int NN,
                            const double eps, const unsigned int maxIter);

/* genmat.c */
ELL_EXPORT int ell_Nm_check(Nrrd *mat, int doNrrdCheck);
ELL_EXPORT int ell_Nm_tran(Nrrd *dest, Nrrd *src);
ELL_EXPORT int ell_Nm_mul(Nrrd *dest, Nrrd *A, Nrrd *B);
ELL_EXPORT int ell_Nm_inv(Nrrd *dest, Nrrd *src);
ELL_EXPORT int ell_Nm_pseudo_inv(Nrrd *dest, Nrrd *src);
ELL_EXPORT int ell_Nm_wght_pseudo_inv(Nrrd *ninv, Nrrd *nA, Nrrd *nW);

/* cubic.c */
ELL_EXPORT int ell_cubic(double root[3],
                         double A, double B, double C, int newton);

/* eigen.c */
ELL_EXPORT void ell_3m_1d_nullspace_d(double ans[3], const double n[9]);
ELL_EXPORT void ell_3m_2d_nullspace_d(double ans0[3],
                                      double ans1[3], const double n[9]);
ELL_EXPORT int ell_3m_eigenvalues_d(double eval[3], 
                                    const double m[9], const int newton);
ELL_EXPORT int ell_3m_eigensolve_d(double eval[3], double evec[9],
                                   const double m[9], const int newton);
ELL_EXPORT int ell_3m_svd_d(double uu[9], double sval[3], double vv[9], 
                            const double mat[9], const int newton);
ELL_EXPORT int ell_6ms_eigensolve_d(double eval[6], double evec[36],
                                    const double mat[21], const double eps);

/* bessy.c */


#ifdef __cplusplus
}
#endif

#endif /* ELL_HAS_BEEN_INCLUDED */
