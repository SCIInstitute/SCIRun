/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


# TODO: if generating documentation every time project is built is onerous, remove the ALL keyword
SET(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
SET(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

CONFIGURE_FILE(Doxyfile.in ${DOXYFILE} @ONLY NEWLINE_STYLE UNIX)
ADD_CUSTOM_TARGET(Doxygen ALL COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE} SOURCES ${DOXYFILE})

# TODO: see if this is still useful
SET(SERVER_DIR /usr/sci/projects/sciweb/devbuilds/scirun_docs/doxygen)

IF(COPY_DOXYGEN_TO_SERVER AND IS_DIRECTORY ${SERVER_DIR})
  SET(DOXYGEN_OUTPUT_HTML_DIR ${DOXYGEN_OUTPUT_DIR}/html)
  SET(DOXYGEN_OUTPUT_LATEX_DIR ${DOXYGEN_OUTPUT_DIR}/latex)

  SET(SERVER_HTML_DIR ${SERVER_DIR}/html)
  SET(SERVER_LATEX_DIR ${SERVER_DIR}/latex)

  # cleanup
  #FILE(REMOVE_RECURSE ${SERVER_HTML_DIR} ${SERVER_LATEX_DIR})
  # try copying html and latex directories to server
  #FILE(INSTALL ${DOXYGEN_OUTPUT_HTML_DIR} ${DOXYGEN_OUTPUT_LATEX_DIR} DESTINATION ${SERVER_DIR})

  ADD_CUSTOM_COMMAND(TARGET Doxygen
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E remove_directory
    ${SERVER_HTML_DIR}
  )

  ADD_CUSTOM_COMMAND(TARGET Doxygen
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E remove_directory
    ${SERVER_LATEX_DIR}
  )

  ADD_CUSTOM_COMMAND(TARGET Doxygen
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy_directory
    ${DOXYGEN_OUTPUT_HTML_DIR} ${SERVER_HTML_DIR}
  )

  ADD_CUSTOM_COMMAND(TARGET Doxygen
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy_directory
    ${DOXYGEN_OUTPUT_LATEX_DIR} ${SERVER_LATEX_DIR}
  )
ENDIF()
