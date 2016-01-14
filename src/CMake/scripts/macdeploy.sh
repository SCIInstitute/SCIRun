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
#
# macdeploy.sh: change runtime paths in app bundle libraries and executables
#!/bin/bash

print() {
  if [[ $# -lt 2 ]]; then
    echo "$0 verbose (0 off or 1 on) string"
    exit -1
  fi
  local verbose=$1
  local string=$2
  if [[ $verbose -eq 1 ]]; then
    echo "$string"
  fi 
}

if [[ $# -lt 4 ]]; then
  echo "$0 path_to_app_bundle original_lib_dir appname verbose (0 off or 1 on)"
  exit -1
fi

app_bundle=$1
original_lib_dir=$2
appname=$3
verbose=$4

if [[ $verbose -ne 0 && $verbose -ne 1 ]]; then
  echo "Arg 5 (verbose) not set to 0 or 1. Defaulting to verbose output off."
  verbose=0
fi

print $verbose "SCIRun library directory: ${original_lib_dir}"

cwd=`pwd`
cd $app_bundle/Contents/Frameworks

# TODO: when CMP0042 new is supported, switch to @rpath
scirun_libs=`ls -1 *.dylib`
for l in ${scirun_libs}; do
  if [[ $verbose -eq 1 ]]; then
    echo "processing $l"
  fi
  COMMAND=`otool -L $l | sed -n "s:"$original_lib_dir":& :p" | awk -v lib=$l '{print "install_name_tool -change "$1$2" "$2" " lib }'`
  print $verbose "${COMMAND}"
  sh -c "${COMMAND}"

  COMMAND=`otool -L $l | sed -n "s:[^/][^/]*lib[A-z0-9_.]*.dylib:&:p" | awk -v lib=$l '{print "install_name_tool -change "$1" @executable_path/../Frameworks/"$1" " lib }'`
  print $verbose "${COMMAND}"
  sh -c "${COMMAND}"

  print $verbose "install_name_tool -id @executable_path/../Frameworks/${l} ${l}"
  install_name_tool -id "@executable_path/../Frameworks/${l}" ${l}
done

cd $cwd
cd $app_bundle/Contents/MacOS

COMMAND=`otool -L $appname | sed -n "s:$original_lib_dir:& :p" | awk -v lib=$appname '{print "install_name_tool -change "$1$2" "$2" " lib }'`
print $verbose "${COMMAND}"
sh -c "${COMMAND}"

COMMAND=`otool -L $appname | sed -n "s:[^/][^/]*lib[A-z0-9_.]*.dylib:&:p" | awk -v lib=$appname '{print "install_name_tool -change "$1" @executable_path/../Frameworks/"$1" " lib }'`
print $verbose "${COMMAND}"
sh -c "${COMMAND}"

cd $cwd
