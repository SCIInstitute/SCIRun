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

#ifndef GAGE_PRIVATE_HAS_BEEN_INCLUDED
#define GAGE_PRIVATE_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*
 define ell_3v_PRINT ell_3v_print_d
 define ell_3m_PRINT ell_3m_print_d
 define ell_3v_PERP ell_3v_perp_d
 define nrrdLOOKUP nrrdDLookup
 define EVALN evalN_d       / * NrrdKernel method * /
*/

/* shape.c */
extern int _gageShapeSet(const gageContext *ctx, gageShape *shape,
                         const Nrrd *nin, unsigned int baseDim);

/* ctx.c */
extern int _gageProbe(gageContext *ctx, double xi, double yi, double zi,
                      double stackIdx);
extern int _gageProbeSpace(gageContext *ctx, double xx, double yy, double zz,
                           double ss, int indexSpace, int clamp);

/* pvl.c */
extern gagePerVolume *_gagePerVolumeCopy(gagePerVolume *pvl, unsigned int fd);
extern double *_gageAnswerPointer(const gageContext *ctx, 
                                  gagePerVolume *pvl, int item);

/* print.c */
extern void _gagePrint_off(FILE *, gageContext *ctx);
extern void _gagePrint_fslw(FILE *, gageContext *ctx);

/* filter.c */
extern int _gageLocationSet(gageContext *ctx,
                            double x, double y, double z, double s);

/* stack.c */
extern int _gageStackIv3Fill(gageContext *ctx);

/* sclprint.c */
extern void _gageSclIv3Print(FILE *, gageContext *ctx, gagePerVolume *pvl);

/* sclfilter.c */
extern void _gageSclFilter(gageContext *ctx, gagePerVolume *pvl);

/* sclanswer.c */
extern void _gageSclAnswer(gageContext *ctx, gagePerVolume *pvl);

/* vecprint.c */
extern void _gageVecIv3Print(FILE *, gageContext *ctx, gagePerVolume *pvl);

#ifdef __cplusplus
}
#endif

#endif /* GAGE_PRIVATE_HAS_BEEN_INCLUDED */
