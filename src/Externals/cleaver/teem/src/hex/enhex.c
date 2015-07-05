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
** enhex: simple stand-alone hex encoder
**
** Compile with:
**    cc -o enhex enhex.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int
enhexColumns = 70;  /* number of characters per line */

void
enhexUsage(char *me) {
  /*                       0   1     2   (2/3) */
  fprintf(stderr, "usage: %s <in> [<out>]\n", me);
  fprintf(stderr, " <in>: file to read raw data from\n");
  fprintf(stderr, "<out>: file to write hex data to; "
          "uses stdout by default\n");
  fprintf(stderr, " \"-\" can be used to refer to stdin/stdout\n");
  exit(1);
}

void
enhexFclose(FILE *file) {
  
  if (!( stdin == file || stdout == file )) {
    fclose(file);
  }
}

int
enhexTable[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};


int
main(int argc, char *argv[]) {
  char *me, *inS, *outS;
  FILE *fin, *fout;
  int car=0, col;

  me = argv[0];
  if (!( 2 == argc || 3 == argc ))
    enhexUsage(me);

  inS = argv[1];
  if (!strcmp("-", inS)) {
    fin = stdin;
#ifdef _WIN32
    _setmode(_fileno(fin), _O_BINARY);
#endif
  } else {
    fin = fopen(inS, "rb");
    if (!fin) {
      fprintf(stderr, "\n%s: couldn't fopen(\"%s\",\"rb\"): %s\n\n",
              me, inS, strerror(errno));
      enhexUsage(me);
    }
  }
  if (2 == argc) {
    fout = stdout;
  } else {
    outS = argv[2];
    if (!strcmp("-", outS)) {
      fout = stdout;
    } else {
      fout = fopen(outS, "w");
      if (!fout) {
        fprintf(stderr, "\n%s: couldn't fopen(\"%s\",\"w\"): %s\n\n",
                me, outS, strerror(errno));
        enhexUsage(me);
      }
    }
  }

  col = 0;
  car = fgetc(fin);
  while (EOF != car) {
    if (col > enhexColumns) {
      fprintf(fout, "\n");
      col = 0;
    }
    fprintf(fout, "%c%c", enhexTable[car>>4], enhexTable[car&15]);
    col += 2;
    car = fgetc(fin);
  }
  if (2 != col) {
    fprintf(fout, "\n");
  }

  enhexFclose(fin);
  enhexFclose(fout);
  exit(0);
}
