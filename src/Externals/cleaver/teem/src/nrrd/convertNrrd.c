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
#include "privateNrrd.h"

/* 
** making these typedefs here allows us to use one token for both
** constructing function names, and for specifying argument types
*/
typedef signed char CH;
typedef unsigned char UC;
typedef signed short SH;
typedef unsigned short US;
/* Microsoft apparently uses 'IN' as a keyword, so we changed 'IN' to 'JN'. */
typedef signed int JN;
typedef unsigned int UI;
typedef airLLong LL;
/* ui64 to double conversion is not implemented, sorry */
#if _MSC_VER < 1300
typedef airLLong UL;
#else
typedef airULLong UL;
#endif
typedef float FL;
typedef double DB;
typedef size_t IT;
/* typedef long double LD; */

/*
** I don't think that I can get out of defining this macro twice,
** because of the rules of C preprocessor macro expansion.  If
** you can figure out a way to not use two identical macros, then
** email me (gk@bwh.harvard.edu) and I'll send you money for dinner.
**
** >>> MAP1 and MAP2 need to be identical <<<
*/

#define MAP1(F, A) \
F(A, CH) \
F(A, UC) \
F(A, SH) \
F(A, US) \
F(A, JN) \
F(A, UI) \
F(A, LL) \
F(A, UL) \
F(A, FL) \
F(A, DB)
/* F(A, LD) */

#define MAP2(F, A) \
F(A, CH) \
F(A, UC) \
F(A, SH) \
F(A, US) \
F(A, JN) \
F(A, UI) \
F(A, LL) \
F(A, UL) \
F(A, FL) \
F(A, DB)
/* F(A, LD) */

/* 
** _nrrdConv<Ta><Tb>()
** 
** given two arrays, a and b, of different types (Ta and Tb) but equal
** size N, _nrrdConvTaTb(a, b, N) will copy all the values from b into
** a, thereby effecting the same type-conversion as one gets with a
** cast.  See K+R Appendix A6 (pg. 197) for the details of what that
** entails.  There are plenty of situations where the results are
** "undefined" (assigning -23 to an unsigned char); the point here is
** simply to make available on arrays all the same behavior you can
** get from scalars.
*/
#define CONV_DEF(TA, TB) \
static void \
_nrrdConv##TA##TB(TA *a, const TB *b, IT N) { \
  size_t ii; \
  for (ii=0; ii<N; ii++) { \
    a[ii] = AIR_CAST(TA, b[ii]); \
  } \
}

/*
** _nrrdClCv<Ta><Tb>()
**
** same as _nrrdConv<Ta><Tb>(), but with clamping to the representable
** range of values of the output type, using a double as intermediate
** storage type HEY WHICH MEANS THAT LONG LONG (BOTH SIGNED AND UNSIGNED)
** may suffer loss of data!!!
*/
#define CLCV_DEF(TA, TB) \
static void \
_nrrdClCv##TA##TB(TA *a, const TB *b, IT N) { \
  size_t ii; \
  for (ii=0; ii<N; ii++) { \
    a[ii] = AIR_CAST(TA, _nrrdDClamp##TA(AIR_CAST(double, b[ii]))); \
  } \
}

/*
** This typedef makes the definition of _nrrdConv[][] shorter
*/
typedef void (*CF)(void *, const void *, IT);

/* 
** the individual converter's appearance in the array initialization,
** using the cast to the "CF" typedef
*/
#define CONV_LIST(TA, TB) (CF)_nrrdConv##TA##TB,
#define CLCV_LIST(TA, TB) (CF)_nrrdClCv##TA##TB,

/* 
** the brace-delimited list of all converters _to_ type TA
*/
#define CONVTO_LIST(_dummy_, TA) {NULL, MAP2(CONV_LIST, TA) NULL},
#define CLCVTO_LIST(_dummy_, TA) {NULL, MAP2(CLCV_LIST, TA) NULL},



/*
** This is where the actual emitted code starts ...
*/


/*
** the clamping functions where moved here from accessors.c in order
** to create the combined clamp-and-convert functions
*/

/*
******** nrrdFClamp
**
** for integral types, clamps a given float to the range representable
** by that type; for floating point types we just return
** the given number, since every float must fit in a double.
*/
static float _nrrdFClampCH(FL v) { return AIR_CLAMP(SCHAR_MIN, v, SCHAR_MAX);}
static float _nrrdFClampUC(FL v) { return AIR_CLAMP(0, v, UCHAR_MAX);}
static float _nrrdFClampSH(FL v) { return AIR_CLAMP(SHRT_MIN, v, SHRT_MAX);}
static float _nrrdFClampUS(FL v) { return AIR_CLAMP(0, v, USHRT_MAX);}
static float _nrrdFClampJN(FL v) { return AIR_CLAMP(INT_MIN, v, INT_MAX);}
static float _nrrdFClampUI(FL v) { return AIR_CLAMP(0, v, UINT_MAX);}
static float _nrrdFClampLL(FL v) { return AIR_CLAMP(NRRD_LLONG_MIN, v, 
                                                    NRRD_LLONG_MAX);}
static float _nrrdFClampUL(FL v) { return AIR_CLAMP(0, v, NRRD_ULLONG_MAX);}
static float _nrrdFClampFL(FL v) { return v; }
static float _nrrdFClampDB(FL v) { return v; }
float (*
nrrdFClamp[NRRD_TYPE_MAX+1])(FL) = {
  NULL,
  _nrrdFClampCH,
  _nrrdFClampUC,
  _nrrdFClampSH,
  _nrrdFClampUS,
  _nrrdFClampJN,
  _nrrdFClampUI,
  _nrrdFClampLL,
  _nrrdFClampUL,
  _nrrdFClampFL,
  _nrrdFClampDB,
  NULL};

/*
******** nrrdDClamp
**
** same as nrrdDClamp, but for doubles.  One change: in the case of
** floats, doubles are clamped to the range -FLT_MAX to FLT_MAX.
*/
static double _nrrdDClampCH(DB v) { return AIR_CLAMP(SCHAR_MIN, v, SCHAR_MAX);}
static double _nrrdDClampUC(DB v) { return AIR_CLAMP(0, v, UCHAR_MAX);}
static double _nrrdDClampSH(DB v) { return AIR_CLAMP(SHRT_MIN, v, SHRT_MAX);}
static double _nrrdDClampUS(DB v) { return AIR_CLAMP(0, v, USHRT_MAX);}
static double _nrrdDClampJN(DB v) { return AIR_CLAMP(INT_MIN, v, INT_MAX);}
static double _nrrdDClampUI(DB v) { return AIR_CLAMP(0, v, UINT_MAX);}
static double _nrrdDClampLL(DB v) { return AIR_CLAMP(NRRD_LLONG_MIN, v, 
                                                     NRRD_LLONG_MAX);}
static double _nrrdDClampUL(DB v) { return AIR_CLAMP(0, v, NRRD_ULLONG_MAX);}
static double _nrrdDClampFL(DB v) { return AIR_CLAMP(-FLT_MAX, v, FLT_MAX); }
static double _nrrdDClampDB(DB v) { return v; }
double (*
nrrdDClamp[NRRD_TYPE_MAX+1])(DB) = {
  NULL,
  _nrrdDClampCH,
  _nrrdDClampUC,
  _nrrdDClampSH,
  _nrrdDClampUS,
  _nrrdDClampJN,
  _nrrdDClampUI,
  _nrrdDClampLL,
  _nrrdDClampUL,
  _nrrdDClampFL,
  _nrrdDClampDB,
  NULL};


/* 
** Define all 100 of both converters.
*/
MAP1(MAP2, CONV_DEF)
MAP1(MAP2, CLCV_DEF)


/* 
** Initialize the whole converter array.
** 
** This generates one incredibly long line of text, which hopefully will not
** break a poor compiler with limitations on line-length...
*/
CF
_nrrdConv[NRRD_TYPE_MAX+1][NRRD_TYPE_MAX+1] = {
{NULL}, 
MAP1(CONVTO_LIST, _dummy_)
{NULL}
};

CF
_nrrdClampConv[NRRD_TYPE_MAX+1][NRRD_TYPE_MAX+1] = {
{NULL}, 
MAP1(CLCVTO_LIST, _dummy_)
{NULL}
};
