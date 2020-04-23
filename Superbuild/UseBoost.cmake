
include_directories(${SCI_BOOST_INCLUDE})
link_directories(${SCI_BOOST_LIBRARY_DIR})
add_definitions(-DBOOST_ALL_NO_LIB)
if(BUILD_WITH_PYTHON)
  add_definitions(-DBOOST_PYTHON_STATIC_LIB -DBOOST_PYTHON_STATIC_MODULE)
endif()

if(UNIX)
  add_definitions(-DBOOST_NO_CXX11_ALLOCATOR)
endif()


# TODO: if static runtime link is supported, then ABI tag postfix must include s
# see:
# http://www.boost.org/doc/libs/1_58_0/more/getting_started/windows.html
# http://www.boost.org/doc/libs/1_58_0/more/getting_started/unix-variants.html
if(WIN32)
  if(BUILD_WITH_PYTHON)
    set(DEBUG_POSTFIX "-gyd")
  else()
    set(DEBUG_POSTFIX "-gd")
  endif()
  set(boost_LIB_PREFIX "lib")
else()
  set(DEBUG_POSTFIX "-yd")
  set(boost_LIB_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
endif()

foreach(lib ${SCI_BOOST_LIBRARY})
  add_library(${lib} STATIC IMPORTED GLOBAL)

  if(CMAKE_GENERATOR MATCHES "Makefiles" AND CMAKE_BUILD_TYPE MATCHES "Debug")
    set(FULL_LIB_NAME "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}${lib}${DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  elseif(CMAKE_GENERATOR MATCHES "Makefiles")
    set(FULL_LIB_NAME "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}${lib}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    if(${lib} MATCHES "boost_python-mt")
      if(WIN32)
		    set(FULL_LIB_NAME_RELEASE "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}boost_python${USER_PYTHON_VERSION_MAJOR}${USER_PYTHON_VERSION_MINOR}-mt${CMAKE_STATIC_LIBRARY_SUFFIX}")
        set(FULL_LIB_NAME_DEBUG "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}boost_python${USER_PYTHON_VERSION_MAJOR}${USER_PYTHON_VERSION_MINOR}-mt${DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
      else(WIN32)
        if (${USER_PYTHON_VERSION} VERSION_GREATER "3.7")
          set(FULL_LIB_NAME_RELEASE "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}boost_python${USER_PYTHON_VERSION_MAJOR}${USER_PYTHON_VERSION_MINOR}-mt${CMAKE_STATIC_LIBRARY_SUFFIX}")
          set(FULL_LIB_NAME_DEBUG "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}boost_python${USER_PYTHON_VERSION_MAJOR}${USER_PYTHON_VERSION_MINOR}-mt${DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
        else()
          set(FULL_LIB_NAME_RELEASE "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}boost_python3-mt${CMAKE_STATIC_LIBRARY_SUFFIX}")
          set(FULL_LIB_NAME_DEBUG "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}boost_python3-mt${DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
        endif()
      endif(WIN32)
	else() # non-python libraries
	  set(FULL_LIB_NAME_RELEASE "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}${lib}${CMAKE_STATIC_LIBRARY_SUFFIX}")
	  set(FULL_LIB_NAME_DEBUG "${SCI_BOOST_LIBRARY_DIR}/${boost_LIB_PREFIX}${lib}${DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
	endif()
  endif()

  message(STATUS "Configure Boost library ${lib}")

  if(CMAKE_GENERATOR MATCHES "Makefiles")
    set_target_properties(${lib}
      PROPERTIES
        IMPORTED_LOCATION ${FULL_LIB_NAME}
    )
  else() # IDEs: Xcode, VS, others...
    set_target_properties(${lib}
      PROPERTIES
        IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
        IMPORTED_LOCATION_RELEASE ${FULL_LIB_NAME_RELEASE}
        IMPORTED_LOCATION_DEBUG ${FULL_LIB_NAME_DEBUG}
    )
  endif()
endforeach()
