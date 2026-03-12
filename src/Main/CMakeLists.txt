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


SET(SCIRunMain_SOURCES
  scirunMain.cc
)

IF(BUILD_HEADLESS)
  ADD_DEFINITIONS(-DBUILD_HEADLESS)
ENDIF(BUILD_HEADLESS)

ADD_DEFINITIONS(-DAPPLICATION_NAME=${APPLICATION_NAME})

IF(BUILD_HEADLESS)
  SET(BUILD_BUNDLE FALSE)
ENDIF()

IF(APPLE)
  IF(${MACDEPLOYQT} MATCHES "NOTFOUND" OR ${SED} MATCHES "NOTFOUND" OR ${AWK} MATCHES "NOTFOUND")
    SET(BUILD_BUNDLE FALSE)
    MESSAGE(WARNING "macdeployqt, sed and awk are required to build the SCIRun app bundle. Please ensure that all are installed and in your path.")
  ENDIF()
ENDIF()

IF(APPLE)
  MESSAGE(STATUS "Build Mac bundle: ${BUILD_BUNDLE}")
  IF(BUILD_BUNDLE)
    ADD_SUBDIRECTORY(osx_bundle)
  ENDIF()
ENDIF()

IF(APPLE)
  # TODO: temporary - until testing can be configured with app bundle
  ADD_EXECUTABLE(${TEST_APPLICATION_NAME}
    ${SCIRunMain_SOURCES}
  )

  IF(BUILD_HEADLESS)
    ADD_EXECUTABLE(${APPLICATION_NAME}
      ${SCIRunMain_SOURCES}
    )
  ENDIF()

ELSEIF(WIN32)
  # TODO: windows headless mode?

  SET(SCIRunMain_SOURCES ${SCIRunMain_SOURCES} "${APPLICATION_WINDOWS_RC}")
  IF(SCIRUN_SHOW_CONSOLE)
    ADD_DEFINITIONS(-DSCIRUN_SHOW_CONSOLE=1)
  ENDIF()

  ADD_EXECUTABLE(${APPLICATION_NAME} WIN32 ${SCIRunMain_SOURCES})

  # TODO: probably need this for Xcode
  IF(WIN32 AND MSVC)
    # TODO: more build types to be supported?
    # TODO: change $<CONFIGURATION> to $<CONFIG> when upgrading to cmake 3
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND}
      ARGS -E copy_directory
      ${SCIRun_BINARY_DIR}/Shaders
      ${SCIRun_BINARY_DIR}/$<CONFIGURATION>/Shaders
    )
    ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND}
      ARGS -E copy_directory
      ${SCIRun_BINARY_DIR}/Fonts
      ${SCIRun_BINARY_DIR}/$<CONFIGURATION>/Fonts
   )
   ADD_CUSTOM_COMMAND(TARGET ${APPLICATION_NAME}
     POST_BUILD
     COMMAND ${CMAKE_COMMAND}
     ARGS -E copy_directory
     ${SCIRun_BINARY_DIR}/Assets
     ${SCIRun_BINARY_DIR}/$<CONFIGURATION>/Assets
    )
ENDIF()

ELSE()
  ADD_EXECUTABLE(${APPLICATION_NAME} ${SCIRunMain_SOURCES})
ENDIF()

IF(APPLE)
  SET(APPS ${TEST_APPLICATION_NAME})
  IF(BUILD_HEADLESS)
    LIST(APPEND APPS ${APPLICATION_NAME})
  ENDIF()
ELSE()
  SET(APPS ${APPLICATION_NAME})
ENDIF()

FOREACH(app ${APPS})
  TARGET_LINK_LIBRARIES(${app}
    PRIVATE
    Core_Util_Legacy
  )

  IF(BUILD_HEADLESS)
    TARGET_LINK_LIBRARIES(${app}
      PRIVATE
      Core_ConsoleApplication
    )
  ELSE()
    TARGET_LINK_LIBRARIES(${app}
      PRIVATE
      Interface_Application
    )
  ENDIF()

  # ---- Only add Python linkage when requested ----
  IF(BUILD_WITH_PYTHON)
    TARGET_LINK_LIBRARIES(${app}
      PRIVATE
      Core_Python
      SCIRunPython
      SCIRunPythonAPI
    )

    # ---- MSVC: Neutralize wrong defaultlibs and force correct python import lib ----
    if (MSVC)
      # Ignore the wrong defaultlib per configuration
      target_link_options(${app} PRIVATE
        $<$<CONFIG:Debug>:/NODEFAULTLIB:python313.lib>
        $<$<CONFIG:Release>:/NODEFAULTLIB:python313_d.lib>
        $<$<CONFIG:RelWithDebInfo>:/NODEFAULTLIB:python313_d.lib>
        $<$<CONFIG:MinSizeRel>:/NODEFAULTLIB:python313_d.lib>
      )

      # Prefer imported python target from superbuild/helpers
      if (TARGET SCIRunPython::Python)
        target_link_libraries(${app} PRIVATE
          $<$<CONFIG:Debug>:SCIRunPython::Python>
          $<$<NOT:$<CONFIG:Debug>>:SCIRunPython::Python>
        )
      elseif (TARGET Python::Python)
        target_link_libraries(${app} PRIVATE
          $<$<CONFIG:Debug>:Python::Python>
          $<$<NOT:$<CONFIG:Debug>>:Python::Python>
        )
      else()
        # Fallback: probe superbuild locations for python313[_d].lib
        set(_PY_DBG "")
        set(_PY_REL "")
        foreach(dir
          "${CMAKE_BINARY_DIR}/../Externals/Install/Python_external/libs"
          "${CMAKE_BINARY_DIR}/../Externals/Install/Python_external/lib"
          "${CMAKE_BINARY_DIR}/../Externals/Source/Python_external/PCbuild/amd64"
        )
          if (EXISTS "${dir}/python313_d.lib" AND NOT _PY_DBG)
            set(_PY_DBG "${dir}/python313_d.lib")
          endif()
          if (EXISTS "${dir}/python313.lib" AND NOT _PY_REL)
            set(_PY_REL "${dir}/python313.lib")
          endif()
        endforeach()

        if (_PY_DBG)
          target_link_libraries(${app} PRIVATE
            $<$<CONFIG:Debug>:${_PY_DBG}>
          )
          message(STATUS "[${app}] Using ${_PY_DBG} for Debug")
        else()
          message(WARNING "[${app}] Could not locate python313_d.lib; Debug link may still fail.")
        endif()

        if (_PY_REL)
          target_link_libraries(${app} PRIVATE
            $<$<NOT:$<CONFIG:Debug>>:${_PY_REL}>
          )
          message(STATUS "[${app}] Using ${_PY_REL} for non-Debug")
        else()
          message(WARNING "[${app}] Could not locate python313.lib; non-Debug link may fail.")
        endif()
      endif()

      # Safety net: strip any literal python313 libs that leaked in via LINK_LIBRARIES
      get_target_property(_app_link_libs ${app} LINK_LIBRARIES)
      if (_app_link_libs)
        list(FILTER _app_link_libs EXCLUDE REGEX ".*python313(_d)?\\.lib$")
        set_property(TARGET ${app} PROPERTY LINK_LIBRARIES "${_app_link_libs}")
      endif()
    endif() # MSVC
  ENDIF()   # BUILD_WITH_PYTHON
ENDFOREACH()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})