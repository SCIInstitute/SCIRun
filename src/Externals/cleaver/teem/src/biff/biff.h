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

#ifndef BIFF_HAS_BEEN_INCLUDED
#define BIFF_HAS_BEEN_INCLUDED

/* ---- BEGIN non-NrrdIO */

#include <stdio.h>
#include <string.h>

#include <teem/air.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(biff_EXPORTS) || defined(teem_EXPORTS)
#    define BIFF_EXPORT extern __declspec(dllexport)
#  else
#    define BIFF_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define BIFF_EXPORT extern
#endif
/* ---- END non-NrrdIO */


#ifdef __cplusplus
extern "C" {
#endif

#define BIFF_STRLEN (256+1) /* convenience for strlen of error messages */

#define BIFF_MAXKEYLEN 128  /* maximum allowed key length (not counting 
                               the null termination) */

BIFF_EXPORT void biffAdd(const char *key, const char *err);
BIFF_EXPORT void biffMaybeAdd(const char *key, const char *err, int useBiff);
BIFF_EXPORT int biffCheck(const char *key);
BIFF_EXPORT void biffDone(const char *key);
BIFF_EXPORT void biffMove(const char *destKey, const char *err,
                          const char *srcKey);
BIFF_EXPORT char *biffGet(const char *key);
BIFF_EXPORT int biffGetStrlen(const char *key);
BIFF_EXPORT void biffSetStr(char *str, const char *key);
BIFF_EXPORT char *biffGetDone(const char *key);
BIFF_EXPORT void biffSetStrDone(char *str, const char *key);

#ifdef __cplusplus
}
#endif

#endif /* BIFF_HAS_BEEN_INCLUDED */
