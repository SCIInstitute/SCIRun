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

# borrowed liberally from ParaView superbuild

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

SET(DEFAULT_PYTHON_VERSION "3.13.1")

set(USER_PYTHON_VERSION ${DEFAULT_PYTHON_VERSION} CACHE STRING "Branch name corresponding to Python version number")
set_property(CACHE USER_PYTHON_VERSION PROPERTY STRINGS 3.6.7 3.7.9 3.8.12 3.9.10 3.10.2)

string(REPLACE "." ";" USER_PYTHON_VERSION_LIST ${USER_PYTHON_VERSION})
list(GET USER_PYTHON_VERSION_LIST 0 USER_PYTHON_VERSION_MAJOR)
list(GET USER_PYTHON_VERSION_LIST 1 USER_PYTHON_VERSION_MINOR)
list(GET USER_PYTHON_VERSION_LIST 2 USER_PYTHON_VERSION_PATCH)

SET(PY_MAJOR ${USER_PYTHON_VERSION_MAJOR})
SET(PY_MINOR ${USER_PYTHON_VERSION_MINOR})
SET(PY_PATCH ${USER_PYTHON_VERSION_PATCH})
SET(SCI_PYTHON_VERSION "${PY_MAJOR}.${PY_MINOR}.${PY_PATCH}")
SET(SCI_PYTHON_VERSION_SHORT "${PY_MAJOR}.${PY_MINOR}")
SET(SCI_PYTHON_VERSION_SHORT_WIN32 "${PY_MAJOR}${PY_MINOR}")

# TODO: recheck when upgrading
# --with-pydebug
#SET(python_ABIFLAG_PYDEBUG "d")
SET(python_ABIFLAG_PYDEBUG)
# --with-pymalloc (default)
# if disabling pymalloc (--without-pymalloc) for valgrind or to track other memory problems,
# disable this ABI flag
SET(python_ABIFLAG_PYMALLOC "m")
SET(ABIFLAGS "${python_ABIFLAG_PYMALLOC}${python_ABIFLAG_PYDEBUG}")

SET(python_GIT_TAG "origin/${USER_PYTHON_VERSION}")
SET(python_GIT_URL "https://github.com/CIBC-Internal/python.git")

SET(python_WIN32_ARCH)
SET(python_WIN32_64BIT_DIR)
SET(python_FRAMEWORK_ARCHIVE)

IF(UNIX)
  # TODO: figure out pip package
  SET(python_CONFIGURE_FLAGS
    "--prefix=<INSTALL_DIR>"
    "--with-ensurepip=no"
  )
  IF(APPLE)
    # framework contains *.dylib
    LIST(APPEND python_CONFIGURE_FLAGS "--enable-framework=<INSTALL_DIR>")
    SET(python_FRAMEWORK_ARCHIVE "framework.tar")
  ELSE()
    LIST(APPEND python_CONFIGURE_FLAGS "--enable-shared")
  ENDIF()
ELSE()
  # TODO: 32-bit windows build?
  SET(python_WIN32_ARCH "x64")
  # 64-bit build only
  # 32-bit build outputs to PCbuild dir
  SET(python_WIN32_64BIT_DIR "/amd64")
  SET(python_ABIFLAG_PYDEBUG "_d")
ENDIF()

# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
IF(UNIX)
  ExternalProject_Add(Python_external
    GIT_REPOSITORY ${python_GIT_URL}
    GIT_TAG ${python_GIT_TAG}
    BUILD_IN_SOURCE ON
    CONFIGURE_COMMAND ./configure ${python_CONFIGURE_FLAGS}
    PATCH_COMMAND ""
  )
  IF(APPLE)
    # Preserves links, permissions
    ExternalProject_Add_Step(Python_external framework_tar_archive
      COMMAND "${CMAKE_COMMAND}" -E tar cf ${python_FRAMEWORK_ARCHIVE} Python.framework
	DEPENDEES install
	WORKING_DIRECTORY <INSTALL_DIR>
    )
  ENDIF()

# --- Windows path (MSVC) ---
ELSE()
  # 64-bit build only (keeps your original assumptions)
  set(python_WIN32_ARCH "x64")
  set(python_WIN32_64BIT_DIR "/amd64")
  set(python_ABIFLAG_PYDEBUG "_d")

  # Helper script to resolve and copy pyconfig.h in a version-robust way
  # We embed a small -P script rather than relying on shell tricks.
  set(_copy_pyconfig_cmake "${CMAKE_CURRENT_BINARY_DIR}/CopyPyConfig.cmake")
  file(WRITE "${_copy_pyconfig_cmake}" [=[
    # CopyPyConfig.cmake
    # Inputs:
    #   _SRC  : CPython source root
    #   _DST  : Destination include directory (expects Include/)
    #   _PYEXE: Built python.exe to query sysconfig (optional if legacy fallback used)

    if(NOT DEFINED _SRC OR NOT DEFINED _DST)
      message(FATAL_ERROR "CopyPyConfig.cmake: _SRC and _DST must be defined.")
    endif()

    file(MAKE_DIRECTORY "${_DST}")

    # 1) Legacy fallback (works for older trees/tags if file exists)
    if(EXISTS "${_SRC}/PC/pyconfig.h")
      message(STATUS "[Python_external] Using legacy PC/pyconfig.h")
      file(COPY "${_SRC}/PC/pyconfig.h" DESTINATION "${_DST}")
      return()
    endif()

    # 2) Preferred: ask the built interpreter where pyconfig.h is
    if(NOT DEFINED _PYEXE OR NOT EXISTS "${_PYEXE}")
      message(FATAL_ERROR
        "CopyPyConfig.cmake: Built python.exe not found at '${_PYEXE}'.\n"
        "Cannot query sysconfig.get_config_h_filename(); please check the build output under PCbuild.")
    endif()

    execute_process(
      COMMAND "${_PYEXE}" -c "import sysconfig, sys; print(sysconfig.get_config_h_filename())"
      OUTPUT_VARIABLE _CFG
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )

    if(NOT _CFG OR NOT EXISTS "${_CFG}")
      message(FATAL_ERROR
        "CopyPyConfig.cmake: sysconfig.get_config_h_filename() returned '${_CFG}', which does not exist.")
    endif()

    message(STATUS "[Python_external] Copying pyconfig.h from: ${_CFG}")
    file(COPY "${_CFG}" DESTINATION "${_DST}")
  ]=])

  ExternalProject_Add(Python_external
    GIT_REPOSITORY ${python_GIT_URL}
    GIT_TAG        ${python_GIT_TAG}
    PATCH_COMMAND  ""
    # Pass platform to build.bat so it generates the right artifacts
    CONFIGURE_COMMAND PCbuild/build.bat -p ${python_WIN32_ARCH}
    BUILD_IN_SOURCE ON

    # Build Release first (keeps your logic)
    BUILD_COMMAND
      ${CMAKE_BUILD_TOOL} PCbuild/pcbuild.sln /nologo
        /property:Configuration=Release
        /property:Platform=${python_WIN32_ARCH}

    # INSTALL_COMMAND: robust pyconfig.h copy
    # - Prefer sysconfig.get_config_h_filename() from the built interpreter
    # - Fallback to legacy PC/pyconfig.h if present
    INSTALL_COMMAND
      "${CMAKE_COMMAND}"
        -D_SRC=<SOURCE_DIR>
        -D_DST=<SOURCE_DIR>/Include
        -D_PYEXE=<SOURCE_DIR>/PCbuild${python_WIN32_64BIT_DIR}/python.exe
        -P "${_copy_pyconfig_cmake}"
  )

  # Also build Debug (as you had), and ensure it happens before "install"
  ExternalProject_Add_Step(Python_external debug_build
    COMMAND ${CMAKE_BUILD_TOOL} PCbuild/pcbuild.sln /nologo
              /property:Configuration=Debug
              /property:Platform=${python_WIN32_ARCH}
    DEPENDEES build
    DEPENDERS install
    WORKING_DIRECTORY <SOURCE_DIR>
  )
ENDIF()

ExternalProject_Get_Property(Python_external SOURCE_DIR)
ExternalProject_Get_Property(Python_external INSTALL_DIR)

SET(SCI_PYTHON_MODULE_PARENT_PATH lib)

IF(UNIX)
  SET(SCI_PYTHON_NAME python${SCI_PYTHON_VERSION_SHORT})
  IF(APPLE)
    # TODO: check Xcode IDE builds...

    SET(SCI_PYTHON_FRAMEWORK ${INSTALL_DIR}/Python.framework)
    SET(SCI_PYTHON_ROOT_DIR ${SCI_PYTHON_FRAMEWORK}/Versions/${SCI_PYTHON_VERSION_SHORT})
    SET(SCI_PYTHON_INCLUDE ${SCI_PYTHON_ROOT_DIR}/Headers)
    SET(SCI_PYTHON_LIBRARY_DIR ${SCI_PYTHON_ROOT_DIR}/lib)
    SET(SCI_PYTHON_LINK_LIBRARY_DIRS ${SCI_PYTHON_LIBRARY_DIR})
    SET(SCI_PYTHON_EXE ${SCI_PYTHON_ROOT_DIR}/bin/${SCI_PYTHON_NAME})
    SET(SCI_PYTHON_LIBRARY ${SCI_PYTHON_NAME})

    # required by interpreter interface
    IF(BUILD_HEADLESS)
      SET(PYTHON_MODULE_SEARCH_PATH Python.framework/Versions/${SCI_PYTHON_VERSION_SHORT}/${SCI_PYTHON_MODULE_PARENT_PATH}/${SCI_PYTHON_NAME} CACHE INTERNAL "Python modules." FORCE)
    ELSE()
      SET(PYTHON_MODULE_SEARCH_PATH Frameworks/Python.framework/Versions/${SCI_PYTHON_VERSION_SHORT}/${SCI_PYTHON_MODULE_PARENT_PATH}/${SCI_PYTHON_NAME} CACHE INTERNAL "Python modules." FORCE)
    ENDIF()
    SET(SCI_PYTHON_FRAMEWORK_ARCHIVE ${INSTALL_DIR}/${python_FRAMEWORK_ARCHIVE})
  ELSE()
    SET(SCI_PYTHON_ROOT_DIR ${INSTALL_DIR})
    SET(SCI_PYTHON_INCLUDE ${INSTALL_DIR}/include/${SCI_PYTHON_NAME})
    SET(SCI_PYTHON_LIBRARY_DIR ${SCI_PYTHON_ROOT_DIR}/lib)
    SET(SCI_PYTHON_LINK_LIBRARY_DIRS ${SCI_PYTHON_LIBRARY_DIR})
    IF(SCIRUN_BITS MATCHES 64)
      LIST(APPEND SCI_PYTHON_LINK_LIBRARY_DIRS ${INSTALL_DIR}/lib64)
      SET(SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH ${INSTALL_DIR}/lib64/${SCI_PYTHON_NAME} CACHE INTERNAL "Python modules." FORCE)
    ENDIF()
    SET(SCI_PYTHON_EXE ${INSTALL_DIR}/bin/${SCI_PYTHON_NAME})
    SET(SCI_PYTHON_LIBRARY ${SCI_PYTHON_NAME})

    # required by interpreter interface
    SET(PYTHON_MODULE_SEARCH_PATH ${SCI_PYTHON_MODULE_PARENT_PATH}/${SCI_PYTHON_NAME} CACHE INTERNAL "Python modules." FORCE)
    SET(SCI_PYTHON_MODULE_LIBRARY_PATH ${INSTALL_DIR}/${SCI_PYTHON_MODULE_PARENT_PATH}/${SCI_PYTHON_NAME})
  ENDIF()
ELSE()
  # Windows does not do install step
  SET(SCI_PYTHON_ROOT_DIR ${SOURCE_DIR}/PCbuild)
  SET(SCI_PYTHON_INCLUDE ${SOURCE_DIR}/Include)
  SET(SCI_PYTHON_LIBRARY_DIR ${SCI_PYTHON_ROOT_DIR}${python_WIN32_64BIT_DIR})
  SET(SCI_PYTHON_LINK_LIBRARY_DIRS ${SCI_PYTHON_LIBRARY_DIR})
  SET(SCI_PYTHON_NAME python${SCI_PYTHON_VERSION_SHORT_WIN32})

  SET(SCI_PYTHON_EXE ${SCI_PYTHON_ROOT_DIR}${python_WIN32_64BIT_DIR}/python.exe)
  SET(SCI_PYTHON_DEBUG_EXE ${SCI_PYTHON_ROOT_DIR}${python_WIN32_64BIT_DIR}/python${python_ABIFLAG_PYDEBUG}.exe)
  SET(SCI_PYTHON_LIBRARY ${SCI_PYTHON_NAME})
  SET(SCI_PYTHON_LIBRARY_RELEASE ${SCI_PYTHON_ROOT_DIR}${python_WIN32_64BIT_DIR}/${SCI_PYTHON_NAME}.lib)
  SET(SCI_PYTHON_LIBRARY_DEBUG ${SCI_PYTHON_ROOT_DIR}${python_WIN32_64BIT_DIR}/${SCI_PYTHON_NAME}${python_ABIFLAG_PYDEBUG}.lib)
  SET(SCI_PYTHON_DLL_PATH ${SCI_PYTHON_ROOT_DIR}${python_WIN32_64BIT_DIR}/${SCI_PYTHON_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
  SET(SCI_PYTHON_DLL_DEBUG_PATH ${SCI_PYTHON_ROOT_DIR}${python_WIN32_64BIT_DIR}/${SCI_PYTHON_NAME}${python_ABIFLAG_PYDEBUG}${CMAKE_SHARED_LIBRARY_SUFFIX})

  # required by interpreter interface
  SET(PYTHON_MODULE_SEARCH_PATH ${SCI_PYTHON_MODULE_PARENT_PATH}/${SCI_PYTHON_NAME} CACHE INTERNAL "Python modules." FORCE)
  SET(SCI_PYTHON_MODULE_LIBRARY_PATH ${SOURCE_DIR}/Lib)
ENDIF()

SET(SCI_PYTHON_USE_FILE ${INSTALL_DIR}/UsePython.cmake)

# Python is special case - normally this should be handled in external library repo
CONFIGURE_FILE(${SUPERBUILD_DIR}/PythonConfig.cmake.in ${INSTALL_DIR}/PythonConfig.cmake @ONLY)
CONFIGURE_FILE(${SUPERBUILD_DIR}/UsePython.cmake ${SCI_PYTHON_USE_FILE} COPYONLY)

SET(Python_DIR ${INSTALL_DIR} CACHE PATH "")

MESSAGE(STATUS "Python_DIR: ${Python_DIR}")
