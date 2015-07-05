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

#include "limn.h"

#define INFO "Information about this program and its use"

int
limnpu_aboutMain(int argc, char **argv, char *me, hestParm *hparm) {
  char buff[AIR_STRLEN_MED], fmt[AIR_STRLEN_MED];
  char par1[] = "\t\t\t\t"
    "\"lpu\" is a complete hack.\n";
  char par2[] = "\t\t\t\t"
    "Users are encouraged to take cover.\n";

  AIR_UNUSED(argc);
  AIR_UNUSED(argv);
  AIR_UNUSED(me);

  fprintf(stderr, "\n");
  sprintf(buff, "--- lpu: LimnPolyData command-line interface ---");
  sprintf(fmt, "%%%ds\n",
          (int)((hparm->columns-strlen(buff))/2 + strlen(buff) - 1));
  fprintf(stderr, fmt, buff);
  sprintf(buff, "(Teem version %s, %s)",
          airTeemVersion, airTeemReleaseDate);
  sprintf(fmt, "%%%ds\n",
          (int)((hparm->columns-strlen(buff))/2 + strlen(buff) - 1));
  fprintf(stderr, fmt, buff);
  fprintf(stderr, "\n");

  _hestPrintStr(stderr, 1, 0, 78, par1, AIR_FALSE);
  _hestPrintStr(stderr, 1, 0, 78, par2, AIR_FALSE);

  return 0;
}

unrrduCmd limnpu_aboutCmd = { "about", INFO, limnpu_aboutMain };

