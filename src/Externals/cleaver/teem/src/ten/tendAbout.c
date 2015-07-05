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

#define INFO "Information about this program and its use"
char *_tend_aboutInfoL =
  (INFO
   ". ");

int
tend_aboutMain(int argc, char **argv, char *me, hestParm *hparm) {
  char buff[AIR_STRLEN_MED], fmt[AIR_STRLEN_MED];
  char par1[] = "\t\t\t\t"
    "\"tend\" is a command-line interface to much of the functionality "
    "in \"ten\", a C library for diffusion image processing. Ten is one "
    "library in the \"Teem\" collection of libraries.  More information "
    "about Teem is at <http://teem.sourceforge.net>.\n";
  char par2[] = "\t\t\t\t"
    "Users are strongly encouraged to join the teem-users mailing list: "
    "<http://lists.sourceforge.net/lists/listinfo/teem-users>. This is "
    "the primary forum for feedback, questions, and feature requests.\n";
  char par3[] = "\t\t\t\t"
    "Like \"unu\", another Teem command-line binary, it is often useful "
    "to chain together invocations of tend with pipes, as in the "
    "following, which estimates tensors from DWIs, takes a slice of the "
    "tensor volume, computes the standard RGB colormap of the principal "
    "eigenvector, and then quantizes it to an 8-bit PNG:\n";
  char par4[] = "\ttend estim -i dwi.nhdr -B kvp -knownB0 true \\\n "
    "  | tend slice -a 2 -p 30 \\\n "
    "  | tend evecrgb -c 0 -a cl2 -gam 1.2 \\\n "
    "  | unu quantize -b 8 -min 0 -max 1 -o z30-rgb.png\n";
  char par5[] = "\t\t\t\t"
    "If tend repeatedly proves itself useful for your research, an "
    "acknowledgment to that effect in your publication would be greatly "
    "appreciated, such as (for LaTeX): "
    "\"Dataset processing performed with the {\\tt tend} tool "
    "part of the {\\tt Teem} toolkit available at "
    "{\\tt\t$<$http://teem.sf.net$>$}\"\n ";

  AIR_UNUSED(argc);
  AIR_UNUSED(argv);
  AIR_UNUSED(me);

  fprintf(stdout, "\n");
  sprintf(buff, "--- tend: Diffusion Image Processing and Analysis ---");
  sprintf(fmt, "%%%ds\n",
          (int)((hparm->columns-strlen(buff))/2 + strlen(buff) - 1));
  fprintf(stdout, fmt, buff);
  sprintf(buff, "(Teem version %s, %s)",
          airTeemVersion, airTeemReleaseDate);
  sprintf(fmt, "%%%ds\n",
          (int)((hparm->columns-strlen(buff))/2 + strlen(buff) - 1));
  fprintf(stdout, fmt, buff);
  fprintf(stdout, "\n");

  _hestPrintStr(stdout, 1, 0, 78, par1, AIR_FALSE);
  _hestPrintStr(stdout, 1, 0, 78, par2, AIR_FALSE);
  _hestPrintStr(stdout, 1, 0, 78, par3, AIR_FALSE);
  _hestPrintStr(stdout, 2, 0, 78, par4, AIR_FALSE);
  _hestPrintStr(stdout, 1, 0, 78, par5, AIR_FALSE);

  return 0;
}

TEND_CMD(about, INFO);
