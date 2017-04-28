#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

SET(data_SVN_URL "https://gforge.sci.utah.edu/svn/CIBCData")
SET(sci_data_DIR "${CMAKE_BINARY_DIR}/SCIRunData")

ExternalProject_Add(SCI_data_external
  SVN_REPOSITORY "${data_SVN_URL}"
  SVN_TRUST_CERT 1
  SVN_USERNAME "anonymous"
  SVN_PASSWORD ""
  DOWNLOAD_DIR "${sci_data_DIR}"
  PATCH_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  CONFIGURE_COMMAND ""
  CMAKE_COMMAND ""
  BUILD_COMMAND ""
)

ExternalProject_Get_Property(SCI_data_external BINARY_DIR)
SET(SCI_DATA_DIR ${sci_data_DIR} CACHE PATH "")
