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


#include "../air.h"

typedef union {
  unsigned char **c;
  void **v;
} ptrHack;

int
main(int argc, char *argv[]) {
  char *me, *fname, *incrS;
  airArray *mop, *dataArr;
  FILE *file;
  unsigned int incr, numRed;
  unsigned char *data;
  int datum; /* must be int, so it can store EOF */
  ptrHack hack;

  me = argv[0];
  if (3 != argc) {
    /*                      0      1         2    (3) */
    fprintf(stderr, "usage: %s <filename> <incr>\n", me);
    return 1;
  }
  fname = argv[1];
  incrS = argv[2];

  /* the "mop" is for management of dynamically allocated resources
     cleanly in combination with error handling, its not important
     for understanding how airArrays work (although as you can tell
     from the declaration, the "mop" is built on an airArray) */

  mop = airMopNew();
  if (!(file = fopen(fname, "rb"))) {
    fprintf(stderr, "%s: couldn't open %s for reading\n", me, fname);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);
  if (1 != sscanf(incrS, "%ud", &incr)) {
    fprintf(stderr, "%s: couln't parse incr \"%s\" as unsigned int\n",
            me, incrS);
    airMopError(mop); return 1;
  }

  /* now "file" is the open file that we read from, and "incr is the
     size increment (the granularity) for re-allocating the data */

  /* the arguments here are as follows:
     1) &data: the address of the array (itself a pointer) into which
     we'll copy the data.  Whenever the airArray re-allocates the array,
     it will update the value of the array variable to the new location.
     So, while it seems a little weird at first, the value of the "data"
     variable can change as a side-effect of calling the airArray functions.
     2) NULL: we could pass the address of a variable to record the current
     allocated length of the array, and this might be useful, but isn't
     necessary
     3) sizeof(unsigned char): this is the size of the individual elements
     that we are saving in the array.  Because memory is allocated and 
     addressed at the level of individual bytes (and files can be read 
     one byte at-a-time), we manage the buffer as an array of unsigned chars.
     4) incr: when the array length is a multiple of incr, the memory
     segment is re-allocated, so this determines how often the re-allocation
     happens (we want it to happen fairly infrequently) */
  /* dataArr = airArrayNew(&data, NULL, sizeof(unsigned char), incr); */
  /* but wait: to play well with type checking, we have to use a stupid
     union to pass in the address of the array.  So, hack.v == &data, 
     but the types are right.  We don't do a cast because recent versions
     of gcc will complain about breaking "strict-aliasing rules" ... */
  hack.c = &data;
  dataArr = airArrayNew(hack.v, NULL, sizeof(unsigned char), incr);
  if (!dataArr) {
    fprintf(stderr, "%s: couldn't allocate airArray\n", me);
    airMopError(mop); return 1;
  }

  /* numRed will keep track of the number of bytes that have been
     successfully read from the file AND stored in data[] */
  numRed = 0;
  /* try to get the first byte of data */
  datum = fgetc(file);
  if (EOF == datum) {
    fprintf(stderr, "%s: hit EOF trying to get first byte\n", me);
    airMopError(mop); return 1;
  }
  while (EOF != datum) {
    airArrayLenSet(dataArr, numRed+1);
    if (!data) {
      fprintf(stderr, "%s: couldn't re-allocated data buffer\n", me);
      airMopError(mop); return 1;
    }
    /* now "data" is the address of a sufficiently large array */
    data[numRed++] = datum;
    datum = fgetc(file);
  }
  /* loop finishes when there's nothing more to read from file */

  printf("%s: read %u bytes into memory\n", me, numRed);

  /* destroy the airArray, but keep the data allocated */
  airArrayNix(dataArr);

  printf("%s: first value was %u\n", me, data[0]);

  /* free up the data array itself */
  free(data);

  airMopOkay(mop); 
  return 0;
}
