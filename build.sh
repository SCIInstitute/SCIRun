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
# Shortcuts for setting up the default OS X SDK build target and
# architecture are also available.
##########################################################################

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
    echo -e "-j#\t\t\tRuns # parallel make processes when building [GNU make only].\n\t\t\tDefaults to a count detected from cores and RAM; -j1 forces a serial build."
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

# Job count to use when the caller passed no -j.
#
# This used to default to empty, i.e. a fully serial build. CI logs showed the
# macOS jobs compiling all 1508 translation units one at a time with two of the
# runner's three cores idle for the better part of an hour (issue #2617), and
# local builds were paying the same cost.
#
# The count is capped by RAM as well as by core count: SCIRun has a number of
# heavy template translation units (Field/Mesh especially) that can each want
# well over a gigabyte, so core count alone will OOM a small machine. Budget
# ~2 GB per compiler process. Pass an explicit -j to override, including -j1 to
# get the old serial behaviour back.
detect_parallel_jobs() {
    local ncpu=0 memgb=0 jobs=0

    if [[ $osx == 1 ]]; then
        ncpu=`sysctl -n hw.ncpu 2>/dev/null`
        local membytes=`sysctl -n hw.memsize 2>/dev/null`
        [[ $membytes =~ ^[0-9]+$ ]] && memgb=$(( membytes / 1073741824 ))
    else
        ncpu=`nproc 2>/dev/null`
        local memkb=`awk '/^MemTotal:/ { print $2 }' /proc/meminfo 2>/dev/null`
        [[ $memkb =~ ^[0-9]+$ ]] && memgb=$(( memkb / 1048576 ))
    fi

    [[ $ncpu =~ ^[0-9]+$ ]] && [[ $ncpu -gt 0 ]] || ncpu=1
    # If memory could not be read, fall back to trusting the core count.
    [[ $memgb -gt 0 ]] || memgb=$(( ncpu * 2 ))

    jobs=$(( memgb / 2 ))
    [[ $jobs -le $ncpu ]] || jobs=$ncpu
    [[ $jobs -gt 0 ]] || jobs=1

    echo $jobs
}

##########################################################################
# Build configure functions
##########################################################################


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

    local COMMON_BUILD_OPTS="-DBUILD_HEADLESS:BOOL=$headless -DWITH_TETGEN:BOOL=$tetgenbuild -DBUILD_DOCUMENTATION:BOOL=$documentation"

    configure_scirun_make $COMMON_BUILD_OPTS
}

##########################################################################
# Build functions
##########################################################################

build_scirun_make() {
    echo "Building SCIRun using make..."
    trybuild make $makeflags
}

build_scirun() {
    local cwd=`pwd`
    if [[ $builddir != $cwd ]]; then
      try cd $builddir
    fi

    build_scirun_make
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
parallel_jobs=
cmakeflags=
cmakebin="cmake"
cmakeargs=
setosxmin=0
verbosebuild="OFF"
builddir="$DIR/bin"
xcodebuild=0
documentation="OFF"
# currently off by default
tetgenbuild="OFF"
headless="OFF"

echo "Parsing arguments..."
while [[ $1 != "" ]]; do
    case "$1" in
        --debug)
            buildtype="Debug";;
        --release)
            buildtype="Release";;
       --verbose)
            verbosebuild="ON";;
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
        --with-tetgen)
            tetgenbuild="ON";;
        --headless)
            headless="ON";;
        --documentation)
            documentation="ON";;
        -j*)
            makeflags="${makeflags} $1"
            parallel_jobs="${1#-j}";;
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

if [[ -z $makeflags ]]; then
    parallel_jobs=`detect_parallel_jobs`
    makeflags="-j${parallel_jobs}"
    echo "No -j given; using -j${parallel_jobs} (auto-detected from cores and RAM)."
fi

# Most external projects recurse via $(MAKE) and so inherit make's jobserver,
# which keeps the whole recursive build inside the -j budget above. Those that
# shell out to `cmake --build` instead (Qwt) do not, so hand them the same
# budget explicitly. Superbuild.cmake reads this to size Boost's b2 as well,
# since b2 is not a make at all.
if [[ $parallel_jobs =~ ^[0-9]+$ ]]; then
    export CMAKE_BUILD_PARALLEL_LEVEL=$parallel_jobs
fi

echo "CMake args: $cmakeargs"
echo "Make Flags: $makeflags"

ensure make --version

echo "Build Type: $buildtype"

configure_scirun

build_scirun
