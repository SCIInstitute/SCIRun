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

#ifndef MITE_PRIVATE_HAS_BEEN_INCLUDED
#define MITE_PRIVATE_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*
 # if GAGE_TYPE_FLOAT
 # define limnVtoQN_GT limnVtoQN_f
 # else
 # define limnVtoQN_GT limnVtoQN_d
 # endif
*/

/* txf.c */
extern double *_miteAnswerPointer(miteThread *mtt, gageItemSpec *isp);
extern int _miteNtxfAlphaAdjust(miteRender *mrr, miteUser *muu);
extern int _miteStageSet(miteThread *mtt, miteRender *mrr);
extern void _miteStageRun(miteThread *mtt, miteUser *muu);

/* user.c */
extern int _miteUserCheck(miteUser *muu);

#ifdef __cplusplus
}
#endif

#endif /* MITE_PRIVATE_HAS_BEEN_INCLUDED */
