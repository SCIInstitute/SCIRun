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

/*
** dehex: simple stand-alone hex decoder
**
** Compile with:
**    cc -o dehex dehex.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void
dehexUsage(char *me) {
  /*                       0   1     2   (2/3) */
  fprintf(stderr, "usage: %s <in> [<out>]\n", me);
  fprintf(stderr, " <in>: file to read hex data from\n");
  fprintf(stderr, "<out>: file to write raw data to; "
          "uses stdout by default\n");
  fprintf(stderr, " \"-\" can be used to refer to stdin/stdout\n");
  exit(1);
}

void
dehexFclose(FILE *file) {
  
  if (!( stdin == file || stdout == file )) {
    fclose(file);
  }
}

int
dehexTable[128] = {
/* 0   1   2   3   4   5   6   7   8   9 */
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -1,  /*   0 */
  -1, -1, -1, -1, -2, -2, -2, -2, -2, -2,  /*  10 */
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  /*  20 */
  -2, -2, -1, -2, -2, -2, -2, -2, -2, -2,  /*  30 */
  -2, -2, -2, -2, -2, -2, -2, -2,  0,  1,  /*  40 */
   2,  3,  4,  5,  6,  7,  8,  9, -2, -2,  /*  50 */
  -2, -2, -2, -2, -2, 10, 11, 12, 13, 14,  /*  60 */
  15, -2, -2, -2, -2, -2, -2, -2, -2, -2,  /*  70 */
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  /*  80 */
  -2, -2, -2, -2, -2, -2, -2, 10, 11, 12,  /*  90 */
  13, 14, 15, -2, -2, -2, -2, -2, -2, -2,  /* 100 */
  -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  /* 110 */
  -2, -2, -2, -2, -2, -2, -2, -2           /* 120 */
};

int
main(int argc, char *argv[]) {
  char *me, *inS, *outS;
  FILE *fin, *fout;
  int car=0, byte, nibble, even;

  me = argv[0];
  if (!( 2 == argc || 3 == argc ))
    dehexUsage(me);

  inS = argv[1];
  if (!strcmp("-", inS)) {
    fin = stdin;
  } else {
    fin = fopen(inS, "r");
    if (!fin) {
      fprintf(stderr, "\n%s: couldn't fopen(\"%s\",\"rb\"): %s\n\n",
              me, inS, strerror(errno));
      dehexUsage(me);
    }
  }
  if (2 == argc) {
    fout = stdout;
  } else {
    outS = argv[2];
    if (!strcmp("-", outS)) {
      fout = stdout;
#ifdef _WIN32
      _setmode(_fileno(fout), _O_BINARY);
#endif
    } else {
      fout = fopen(outS, "w");
      if (!fout) {
        fprintf(stderr, "\n%s: couldn't fopen(\"%s\",\"w\"): %s\n\n",
                me, outS, strerror(errno));
        dehexUsage(me);
      }
    }
  }

  byte = 0;
  even = 1;
  for (car=fgetc(fin); EOF != car; car=fgetc(fin)) {
    nibble = dehexTable[car & 127];
    if (-2 == nibble) {
      /* its an invalid character */
      break;
    }
    if (-1 == nibble) {
      /* its white space */
      continue;
    }
    if (even) {
      byte = nibble << 4;
    } else {
      byte += nibble;
      if (EOF == fputc(byte, fout)) {
        fprintf(stderr, "%s: error writing!!!\n", me);
        exit(1);
      }
    }
    even = 1 - even;
  }
  if (EOF != car) {
    fprintf(stderr, "\n%s: got invalid character '%c'\n\n", me, car);
    dehexUsage(me);
  }

  dehexFclose(fin);
  dehexFclose(fout);
  exit(0);
}
