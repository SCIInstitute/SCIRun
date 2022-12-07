#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2020 Scientific Computing and Imaging Institute,
#  University of Utah.
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#


# Sources of Core/Basis classes

SET(Core_Basis_HEADERS
  Bases.h
  Constant.h
  CrvElementWeights.h
  CrvLinearLgn.h
  CrvQuadraticLgn.h
  HexElementsWeights.h
  HexSamplingSchemes.h
  HexTricubicHmtScaleFactors.h
  Locate.h
  NoData.h
  PntSamplingSchemes.h
  PrismElementWeights.h
  PrismLinearLgn.h
  PrismQuadraticLgn.h
  QuadBicubicHmt.h
  QuadBilinearLgn.h
  QuadBiquadraticLgn.h
  QuadSamplingSchemes.h
  TetCubicHmt.h
  TetSamplingSchemes.h
  TriCubicHmt.h
  TriElementWeights.h
  TriLinearLgn.h
  TriQuadraticLgn.h
  Basis.h
  CrvCubicHmt.h
  CrvSamplingSchemes.h
  HexElementWeights.h
  HexTricubicHmt.h
  HexTricubicHmtScaleFactorsEdges.h
  HexTrilinearLgn.h
  HexTriquadraticLgn.h
  NoElementWeights.h
  PrismCubicHmt.h
  PrismSamplingSchemes.h
  QuadElementWeights.h
  share.h
  TetElementWeights.h
  TetLinearLgn.h
  TetQuadraticLgn.h
  TriCubicHmtScaleFactors.h
  TriSamplingSchemes.h
)

SET(Core_Basis_SRCS
  Constant.cc
  CrvLinearLgn.cc
  CrvQuadraticLgn.cc
  HexTrilinearLgn.cc
  HexTriquadraticLgn.cc
  NoData.cc
  PrismLinearLgn.cc
  PrismQuadraticLgn.cc
  QuadBilinearLgn.cc
  QuadBiquadraticLgn.cc
  TetLinearLgn.cc
  TetQuadraticLgn.cc
  TriLinearLgn.cc
  TriQuadraticLgn.cc
)

SCIRUN_ADD_LIBRARY(Core_Basis
  ${Core_Basis_HEADERS}
  ${Core_Basis_SRCS}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Basis)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)
