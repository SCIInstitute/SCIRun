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


#include "../ten.h"

char *info = ("tests conversions between info triples");

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt=NULL;
  airArray *mop;

  int *itype, itypeNum, ii;
  double src[3], last[3], dst[3];
  char space[] = "             ";

  me = argv[0];
  hestOptAdd(&hopt, NULL, "v1 v2 v3", airTypeDouble, 3, 3, src, NULL,
             "source triple");
  hestOptAdd(&hopt, "t", "type", airTypeEnum, 2, -1, &itype, NULL,
             "sequence of triple types to convert through",
             &itypeNum, tenTripleType);
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  printf("%s", space + strlen(airEnumStr(tenTripleType, itype[0])));
  printf("%s", airEnumStr(tenTripleType, itype[0]));
  ell_3v_print_d(stdout, src);
  ELL_3V_COPY(last, src);
  for (ii=1; ii<itypeNum; ii++) {
    tenTripleConvertSingle_d(dst, itype[ii], src, itype[ii-1]);
    printf("%s", space + strlen(airEnumStr(tenTripleType, itype[ii])));
    printf("%s", airEnumStr(tenTripleType, itype[ii]));
    ell_3v_print_d(stdout, dst);
    ELL_3V_COPY(src, dst);
  }
  /*
  tenTripleConvert_d(dst, dstType, src, srcType);
  tenTripleConvert_d(tst, srcType, dst, dstType);
  */

  /*
  printf("%s: %s %s --> %s --> %s\n", me, 
         tenTriple->name,
         airEnumStr(tenTriple, srcType),
         airEnumStr(tenTriple, dstType),
         airEnumStr(tenTriple, srcType));
  ell_3v_print_d(stdout, src);
  ell_3v_print_d(stdout, dst);
  ell_3v_print_d(stdout, tst);
  */

  airMopOkay(mop);
  return 0;
}
