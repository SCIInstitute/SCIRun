#!/bin/bash
#  
#  For more information, please see: http://software.sci.utah.edu
#  
#  The MIT License
#  
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
#  University of Utah.
#  
#  License for the specific language governing rights and limitations under
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

##########################################################################
# SCIRun build script
#
# This script will configure SCIRun builds using CMake
# By default, the CMake generator will be Unix Makefiles,
# in which case, the project will automatically be built using make.
#
# If an Xcode project is specified using the --xcode-build flag,
# the project will be built using the command line utility xcodebuild.
#
# Shortcuts for setting up the default OS X SDK build target and
# architecture are also available.
##########################################################################

CMAKE_MAJOR_VERSION=2
CMAKE_MINOR_VERSION=8
CMAKE_PATCH_VERSION=12
CMAKE_PATH_MINOR_VERSION=2
CMAKE_VERSION="${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}.${CMAKE_PATH_MINOR_VERSION}"
CMAKE="cmake-${CMAKE_VERSION}"

OSX_TARGET_VERSION="10.7"
OSX_TARGET_VERSION_SDK="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk"
OSX_TARGET_ARCH="x86_64"

##########################################################################
# Helper functions
##########################################################################

printhelp() {
    echo -e "build.sh: configure SCIRun with CMake and build with either GNU make or Xcode [OS X only]"
    echo -e "--debug\t\t\tBuilds SCIRun with debug symbols"
    echo -e "--release\t\tBuilds SCIRun without debug symbols [default]"
    echo -e "--verbose\t\tTurn on verbose build"
#    echo -e "--set-osx-version-min\tTarget a minimum Mac OS X version (currently ${OSX_TARGET_VERSION}, ${OSX_TARGET_ARCH}) [OS X only]"
#    echo -e "--xcode-build\t\tConfigure and build Xcode project against ALL_BUILD target [OS X only]"
    echo -e "--with-tetgen\t\tBuild SCIRun with Tetgen library"
    echo -e "--cmake=<path to cmake>\t\tUse given CMake"
    echo -e "--cmake-args=<cmake args>\t\tUse given CMake args"
    echo -e "--documentation\t\tEnable building documentation (requires LaTeX)"
    echo -e "--custom-build-dir=<dir>\t\tBuild in dir (use relative path)"
    echo -e "-j#\t\t\tRuns # parallel make processes when building [GNU make only]"
    echo -e "-D<var>:<type>=<value>\t\t\tDefine CMake variable."
    echo -e "-?\t\t\tThis help"
    exit 0
}

# will cause the script to bailout if the passed in command fails
try() {
  echo $*
  $*
  if [[ $? != "0" ]]; then
      echo -e "\n***ERROR in build script\nThe failed command was:\n$*\n"
      exit 1
  fi
}

trybuild() {
  echo $*
  $*
  if [[ $? != "0" ]]; then
      echo -e "Building SCIRun returned an error\n"
      echo -e "Either the code failed to build properly or\n"
      echo -e "the testing programs failed to complete without\n"
      echo -e "every single test program passing the test.\n"
      exit 1
  fi
}

# functionally equivalent to try,
# but it prints a different error message
ensure() {
  $* >& /dev/null
  if [[ $? != "0" ]]; then
      echo -e "\n***ERROR, $* is required but not found on this system\n"
      exit 1
  fi
}

get_cmake_version() {
    local version=`$cmakebin --version | cut -d ' ' -f 3 | sed -e "s/[^[:digit:].]//g"`
    echo "$version"
}

# Try to find a version of cmake
find_cmake() {
    if [[ ! -e $cmakebin ]]; then
        cmakebin=`which cmake`
    fi

    download=0
    #if it is not found 
    if [[ ! -e $cmakebin ]]; then
        download=1
    else
        # see if cmake is up-to-date
        version=$(get_cmake_version)
        echo "$cmakebin version $version found"
        major_version=${version:0:1}
        minor_version=${version:2:1}
        if [[ $major_version -le $CMAKE_MAJOR_VERSION && $minor_version -lt $CMAKE_MINOR_VERSION ]] ; then
            download=1
        fi 
    fi
      
    if [[ $download -eq 1 ]]; then
        # then look for our own copy made by this script previously
        cmakebin=$DIR/cmake/local/bin/cmake
        try mkdir -p $DIR/cmake/
        try cd $DIR/cmake 

        if [[ -e $cmakebin ]]; then
            # see if local cmake install is compatible
            version=$(get_cmake_version)
            echo "$cmakebin version $version found"
            major_version=${version:0:1}
            minor_version=${version:2:1}
            if [[ $major_version -ge $CMAKE_MAJOR_VERSION && $minor_version -ge $CMAKE_MINOR_VERSION ]] ; then
                download=0
            fi
        fi

        if [[ $download -eq 1 ]]; then
            # try to download and build our own copy in local
            try $getcommand http://www.cmake.org/files/v${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/${CMAKE}.tar.gz
            try tar xvzf ${CMAKE}.tar.gz
            try cd $CMAKE
            try ./bootstrap --prefix="${DIR}/cmake/local"
            try make $makeflags
            try make install
        fi
    fi

    echo "cmakebin=$cmakebin"
    ensure $cmakebin --version
}

##########################################################################
# Build configure functions
##########################################################################

# handle both types of OS X generators
configure_scirun_osx() {
    local build_opts=$@

    if [[ $setosxmin -eq 1 ]]; then
        build_opts="${build_opts} -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${OSX_TARGET_VERSION} -DCMAKE_OSX_SYSROOT:PATH=${OSX_TARGET_VERSION_SDK} -DCMAKE_OSX_ARCHITECTURES:STRING=${OSX_TARGET_ARCH}"
    fi

    if [[ $xcodebuild -eq 1 ]]; then
        try $cmakebin $DIR/Superbuild -G Xcode $build_opts $cmakeargs
    else
        configure_scirun_make $build_opts
    fi
}

# generic Unix makefile build
configure_scirun_make() {
    local build_opts=$@

    build_opts="${build_opts} -DCMAKE_BUILD_TYPE:STRING=${buildtype} -DCMAKE_VERBOSE_MAKEFILE:BOOL=${verbosebuild}"

    try $cmakebin $DIR/Superbuild $build_opts $cmakeargs
}

configure_scirun() {
    if [[ ! -d $builddir ]]; then
        echo "Creating build directory $builddir"
        try mkdir -p $builddir
    fi
    try cd $builddir

    local COMMON_BUILD_OPTS="-DWITH_TETGEN:BOOL=$tetgenbuild -DBUILD_DOCUMENTATION:BOOL=$documentation"

    if [[ $osx -eq 1 ]]; then
        configure_scirun_osx $COMMON_BUILD_OPTS
    else
        configure_scirun_make $COMMON_BUILD_OPTS
    fi
}

##########################################################################
# Build functions
##########################################################################

build_scirun_make() {
    echo "Building SCIRun using make..."
    trybuild make $makeflags
}

build_scirun_xcode() {
    echo "Building SCIRun using Xcode..."
    trybuild xcodebuild -project SCIRun.xcodeproj -target ALL_BUILD -configuration $buildtype
}

build_scirun() {
    local cwd=`pwd`
    if [[ $builddir != $cwd ]]; then
      try cd $builddir
    fi

    if [[ $xcodebuild -eq 1 ]]; then
        build_scirun_xcode
    else
        build_scirun_make
    fi
}

##########################################################################
# build.sh script execution starts here
##########################################################################

export DIR=`pwd`

linux=0
osx=0
platform=`uname`

if [[ $platform =~ [dD]arwin ]]; then
    getcommand="curl -OL"
    osx=1
    echo "Build platform is OS X."
elif [[ $platform =~ [lL]inux ]]; then
    # TODO: could also try lsb_release (if available) to attempt to identify distro
    getcommand="wget"
    linux=1
    echo "Build platform is Linux."
else
    echo "Unsupported system.  Please run on OS X or Linux"
    exit 1
fi

buildtype="Release"
makeflags=
cmakeflags=
cmakebin=
cmakeargs=
setosxmin=0
verbosebuild="OFF"
builddir="$DIR/bin"
xcodebuild=0
documentation="OFF"
# currently off by default
tetgenbuild="OFF"

echo "Parsing arguments..."
while [[ $1 != "" ]]; do
    case "$1" in
        --debug)
            buildtype="Debug";;
        --release)
            buildtype="Release";;
       --set-osx-version-min)
            if [[ $osx -eq 1 ]]; then
              setosxmin=1
            else
              echo "WARNING: Only OS X supports the --set-osx-version-min flag."
            fi;;
       --verbose)
            verbosebuild="ON";;
       --cmake=*)
            cmakebin=`echo $1 | cut -c 9-`;;
       --cmake-args=*)
            cmakeargs=`echo $1 | cut -c 14-`;;
       --custom-build-dir=*)
            dirarg=`echo $1 | cut -c 20-`
            first_char=${dirarg:0:1}
            last_char=${dirarg: -1:1}

            if [[ $first_char == '/' ]]; then
                builddir=$dirarg
            else
                if [[ $last_char == '/' ]]; then
                    builddir="${DIR}${dirarg}"
                else
                    builddir="${DIR}/${dirarg}"
                fi
            fi;;
        --xcode-build)
            if [[ $osx -eq 1 ]]; then
              xcodebuild=1
            else
              echo "WARNING: Only OS X supports the --xcode-build flag."
            fi;;
        --with-tetgen)
            tetgenbuild="ON";;
        --documentation)
            documentation="ON";;
        -j*)
            makeflags="${makeflags} $1";;
        -D*)
            cmakeflags="${cmakeflags} $1";;
        -?|--?|-help|--help)
            printhelp;;
        *)
            echo "WARNING: \'$1\' parameter ignored";;
    esac
    shift 1
done

cmakeargs="${cmakeargs} ${cmakeflags}"

if [[ ! -z $cmakebin ]]; then
  echo "Using CMake: $cmakebin"
fi
echo "CMake args: $cmakeargs"
echo "Make Flags: $makeflags"
if [[ $xcodebuild -eq 0 ]]; then
    # ensure make is on the system
    ensure make --version

    echo "Build Type: $buildtype"
else
    ensure xcodebuild -version

    echo "Generating Xcode project"
fi

find_cmake

configure_scirun

build_scirun
