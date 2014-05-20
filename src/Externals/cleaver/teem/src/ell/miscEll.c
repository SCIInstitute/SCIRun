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

const char *
ell_biff_key = "ell";

/*
******** ell_debug
**
** some functions may use this value to control printing of
** verbose debugging information
*/
int ell_debug = 0;


char
_ell_cubic_root_str[][AIR_STRLEN_SMALL] = {
  "(unknown ell_cubic_root)",
  "single",
  "triple",
  "single and double",
  "three"
};

char
_ell_cubic_root_desc[][AIR_STRLEN_MED] = {
  "(unknown ell_cubic_root)",
  "one single root",
  "one triple root",
  "a single and a double root",
  "three distinct roots"
};

airEnum
_ell_cubic_root = {
  "cubic root solutions",
  ELL_CUBIC_ROOT_MAX,
  _ell_cubic_root_str, NULL,
  _ell_cubic_root_desc,
  NULL, NULL,
  AIR_FALSE
};
airEnum *
ell_cubic_root = &_ell_cubic_root;


void
ell_3m_print_f(FILE *f, const float s[9]) {

  fprintf(f, "% 15.7f % 15.7f % 15.7f\n", 
          s[0], s[1], s[2]);
  fprintf(f, "% 15.7f % 15.7f % 15.7f\n", 
          s[3], s[4], s[5]);
  fprintf(f, "% 15.7f % 15.7f % 15.7f\n", 
          s[6], s[7], s[8]);
}

void
ell_3v_print_f(FILE *f, const float s[3]) {

  fprintf(f, "% 15.7f % 15.7f % 15.7f\n", 
          s[0], s[1], s[2]);
}

void
ell_3m_print_d(FILE *f, const double s[9]) {

  fprintf(f, "% 31.15f % 31.15f % 31.15f\n", 
          s[0], s[1], s[2]);
  fprintf(f, "% 31.15f % 31.15f % 31.15f\n", 
          s[3], s[4], s[5]);
  fprintf(f, "% 31.15f % 31.15f % 31.15f\n", 
          s[6], s[7], s[8]);
}

void
ell_3v_print_d(FILE *f, const double s[3]) {

  fprintf(f, "% 31.15f % 31.15f % 31.15f\n",
          s[0], s[1], s[2]);
}

void
ell_4m_print_f(FILE *f, const float s[16]) {

  fprintf(f, "% 15.7f % 15.7f % 15.7f % 15.7f\n", 
          s[ 0], s[ 1], s[ 2], s[ 3]);
  fprintf(f, "% 15.7f % 15.7f % 15.7f % 15.7f\n", 
          s[ 4], s[ 5], s[ 6], s[ 7]);
  fprintf(f, "% 15.7f % 15.7f % 15.7f % 15.7f\n", 
          s[ 8], s[ 9], s[10], s[11]);
  fprintf(f, "% 15.7f % 15.7f % 15.7f % 15.7f\n", 
          s[12], s[13], s[14], s[15]);
}

void
ell_4v_print_f(FILE *f, const float s[4]) {

  fprintf(f, "% 15.7f % 15.7f % 15.7f % 15.7f\n", 
          s[0], s[1], s[2], s[3]);
}

void
ell_4m_print_d(FILE *f, const double s[16]) {

  fprintf(f, "% 31.15f % 31.15f % 31.15f % 31.15f\n", 
          s[ 0], s[ 1], s[ 2], s[ 3]);
  fprintf(f, "% 31.15f % 31.15f % 31.15f % 31.15f\n", 
          s[ 4], s[ 5], s[ 6], s[ 7]);
  fprintf(f, "% 31.15f % 31.15f % 31.15f % 31.15f\n", 
          s[ 8], s[ 9], s[10], s[11]);
  fprintf(f, "% 31.15f % 31.15f % 31.15f % 31.15f\n", 
          s[12], s[13], s[14], s[15]);
}

void
ell_4v_print_d(FILE *f, const double s[4]) {

  fprintf(f, "% 31.15f % 31.15f % 31.15f % 31.15f\n", 
          s[0], s[1], s[2], s[3]);
}
