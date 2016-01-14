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

# TODO: update when upgrading
SET(PY_MAJOR 3)
SET(PY_MINOR 4)
SET(PY_PATCH 3)
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

SET(python_GIT_TAG "origin/master")
SET(python_GIT_URL "https://github.com/CIBC-Internal/python.git")

SET(python_WIN32_ARCH)
SET(python_WIN32_64BIT_DIR)
SET(python_FRAMEWORK_ARCHIVE)

IF(UNIX)
  # TODO: figure out pip package
  SET(python_CONFIGURE_FLAGS
    "--prefix=<INSTALL_DIR>"
    "--with-threads"
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
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${python_CONFIGURE_FLAGS}
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
ELSE()
  ExternalProject_Add(Python_external
    GIT_REPOSITORY ${python_GIT_URL}
    GIT_TAG ${python_GIT_TAG}
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE ON
    BUILD_COMMAND ${CMAKE_BUILD_TOOL} PCbuild/pcbuild.sln /nologo /property:Configuration=Release /property:Platform=${python_WIN32_ARCH}
    INSTALL_COMMAND "${CMAKE_COMMAND}" -E copy_if_different
      <SOURCE_DIR>/PC/pyconfig.h
      <SOURCE_DIR>/Include/pyconfig.h
  )
  # build both Release and Debug versions
  ExternalProject_Add_Step(Python_external debug_build
    COMMAND ${CMAKE_BUILD_TOOL} PCbuild/pcbuild.sln /nologo /property:Configuration=Debug /property:Platform=${python_WIN32_ARCH}
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
    SET(SCI_PYTHON_INCLUDE ${INSTALL_DIR}/include/${SCI_PYTHON_NAME}${ABIFLAGS})
    SET(SCI_PYTHON_LIBRARY_DIR ${SCI_PYTHON_ROOT_DIR}/lib)
    SET(SCI_PYTHON_LINK_LIBRARY_DIRS ${SCI_PYTHON_LIBRARY_DIR})
    IF(SCIRUN_BITS MATCHES 64)
      LIST(APPEND SCI_PYTHON_LINK_LIBRARY_DIRS ${INSTALL_DIR}/lib64)
      SET(SCI_PYTHON_64BIT_MODULE_LIBRARY_PATH ${INSTALL_DIR}/lib64/${SCI_PYTHON_NAME} CACHE INTERNAL "Python modules." FORCE)
    ENDIF()
    SET(SCI_PYTHON_EXE ${INSTALL_DIR}/bin/${SCI_PYTHON_NAME})
    SET(SCI_PYTHON_LIBRARY ${SCI_PYTHON_NAME}${ABIFLAGS})

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
