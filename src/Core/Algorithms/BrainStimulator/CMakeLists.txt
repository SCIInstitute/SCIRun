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


SET(Algorithms_BrainStimulator_SRCS
  ElectrodeCoilSetupAlgorithm.cc
  SetConductivitiesToTetMeshAlgorithm.cc
  GenerateROIStatisticsAlgorithm.cc
  SetupRHSforTDCSandTMSAlgorithm.cc
  SimulateForwardMagneticFieldAlgorithm.cc
  BiotSavartSolverAlgorithm.cc
  ModelGenericCoilAlgorithm.cc
)

SET(Algorithms_BrainStimulator_HEADERS
  ElectrodeCoilSetupAlgorithm.h
  SetConductivitiesToTetMeshAlgorithm.h
  GenerateROIStatisticsAlgorithm.h
  SetupRHSforTDCSandTMSAlgorithm.h
  SimulateForwardMagneticFieldAlgorithm.h
  BiotSavartSolverAlgorithm.h
  ModelGenericCoilAlgorithm.h
  share.h
)

SCIRUN_ADD_LIBRARY(Algorithms_BrainStimulator
  ${Algorithms_BrainStimulator_HEADERS}
  ${Algorithms_BrainStimulator_SRCS}
)

TARGET_LINK_LIBRARIES(Algorithms_BrainStimulator
  Core_Datatypes #matrices
  Core_Datatypes_Legacy_Field
  Core_Geometry_Primitives  #vectors
  Core_Basis #field basis
  Core_Algorithms_Legacy_Fields
#  Core_Datatypes_Legacy_BrainStimulator
  Algorithms_Base
  ${SCI_BOOST_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Algorithms_BrainStimulator)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)
