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

#include <stdio.h>
#include <float.h>

#if defined(__BORLANDC__)
# include <math.h>
# include <float.h>
#endif


int
main(int argc, char *argv[])
{
   const char * const me=argv[0];
   const float zero=0.0F;
   union {
     float flt32bit;
     int   int32bit;
   } qnan;

#if defined(__BORLANDC__)
   // Disable floating point exceptions in Borland
   _control87(MCW_EM, MCW_EM);
#endif // defined(__BORLANDC__)

   if (sizeof(float) != sizeof(int))
     {
     fprintf(stderr, "%s: MADNESS:  sizeof(float)=%d != sizeof(int)=%d\n",
           me, (int)sizeof(float), (int)sizeof(int));
     return -1;
     }
   qnan.flt32bit=zero/zero;
   printf("-DTEEM_QNANHIBIT=%d\n", (qnan.int32bit >> 22) & 1);
   return (int)((qnan.int32bit >> 22) & 1);
}
