#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2009 Scientific Computing and Imaging Institute,
#  University of Utah.
# 
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

# CMakeLists.txt for Dataflow/Modules/Bundle

SET(Dataflow_Modules_Bundle_SRCS
  CreateParameterBundle.cc
  InsertEnvironmentIntoBundle.cc
  InsertBundlesIntoBundle.cc
  InsertFieldsIntoBundle.cc
  InsertMatricesIntoBundle.cc
  InsertNrrdsIntoBundle.cc
  InsertColorMapsIntoBundle.cc
  InsertStringsIntoBundle.cc
  ReportBundleInfo.cc
  GetBundlesFromBundle.cc
  GetFieldsFromBundle.cc
  GetMatricesFromBundle.cc
  GetNrrdsFromBundle.cc
  GetColorMapsFromBundle.cc
  GetStringsFromBundle.cc
  JoinBundles.cc
)

SCIRUN_ADD_LIBRARY(Dataflow_Modules_Bundle ${Dataflow_Modules_Bundle_SRCS})

TARGET_LINK_LIBRARIES(Dataflow_Modules_Bundle
  Dataflow_Network
  Dataflow_GuiInterface
  Dataflow_TkExtensions
  Core_Algorithms_Util
  Core_Algorithms_Regression
  Core_Basis
  Core_Datatypes
  Core_Exceptions
  Core_Persistent
  Core_Util
  Core_Exceptions
  Core_Thread
  Core_Geom
  Core_Datatypes
  Core_Geometry
  Core_GeomInterface
  ${TK_LIBRARY}
  ${GL_LIBRARY}
  ${M_LIBRARY}
  ${SCI_TEEM_LIBRARY}
  ${SLIVR_LIBRARY}
)
# Removed - Core_Volume

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Dataflow_Modules_Bundle)
ENDIF(BUILD_SHARED_LIBS)
