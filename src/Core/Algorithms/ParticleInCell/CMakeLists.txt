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

find_package(openPMD REQUIRED)

SET(Algorithms_ParticleInCell_SRCS
  GravitySimulationAlgo.cc
  ElectroStaticAlgo.cc
  Output.cpp
  PotentialSolver.cpp
  Species.cpp
  World.cpp
  PIConGPUAlgo.cc
  ScalarMeshReaderAlgo.cc
  VectorMeshReaderAlgo.cc
  ParticleReaderAlgo.cc
  PIConGPUReaderAlgo.cc
)

SET(Algorithms_ParticleInCell_HEADERS
  GravitySimulationAlgo.h
  ElectroStaticAlgo.h
  Output.h
  PotentialSolver.h
  Species.h
  World.h
  PIConGPUAlgo.h
  ScalarMeshReaderAlgo.h
  VectorMeshReaderAlgo.h
  ParticleReaderAlgo.h
  PIConGPUReaderAlgo.h
)

SCIRUN_ADD_LIBRARY(Algorithms_ParticleInCell
  ${Algorithms_ParticleInCell_HEADERS}
  ${Algorithms_ParticleInCell_SRCS}
)

TARGET_LINK_LIBRARIES(Algorithms_ParticleInCell
  Core_Datatypes
  Core_Thread
  Algorithms_Base
  Core_Parser
  openPMD::openPMD
  ${SCI_BOOST_LIBRARY}

)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Algorithms_ParticleInCell)
ENDIF(BUILD_SHARED_LIBS)

#SCIRUN_ADD_TEST_DIR(Tests)
