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

#ifndef LIMN_PRIVATE_HAS_BEEN_INCLUDED
#define LIMN_PRIVATE_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* USAGE, PARSE: both copied verbatim from unrrdu/privateUnrrdu.h */

#define USAGE(info) \
  if (!argc) { \
    hestInfo(stderr, me, (info), hparm); \
    hestUsage(stderr, hopt, me, hparm); \
    hestGlossary(stderr, hopt, hparm); \
    airMopError(mop); \
    return 2; \
  }

#define PARSE() \
  if ((pret=hestParse(hopt, argc, argv, &perr, hparm))) { \
    if (1 == pret) { \
      fprintf(stderr, "%s: %s\n", me, perr); free(perr); \
      hestUsage(stderr, hopt, me, hparm); \
      airMopError(mop); \
      return 2; \
    } else { \
      /* ... like tears ... in rain. Time ... to die. */ \
      exit(1); \
    } \
  }


#ifdef __cplusplus
}
#endif

#endif /* LIMN_PRIVATE_HAS_BEEN_INCLUDED */
