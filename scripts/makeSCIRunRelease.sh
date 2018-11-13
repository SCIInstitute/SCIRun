#!/bin/bash

MINPARAMS=1

if [ $# -lt "$MINPARAMS" ]
then
  echo
  echo "Specify branch/tag name."
  echo
  exit 0
fi

VERSION="$1"
echo $VERSION

cd ../..
mkdir installer_$VERSION
cd installer_$VERSION
git clone https://github.com/SCIInstitute/SCIRun.git
cd SCIRun
git checkout $VERSION
cd bin
cmake -DBUILD_TESTING:BOOL=OFF -DWITH_TETGEN:BOOL=OFF -DWITH_OSPRAY:BOOL=OFF ../Superbuild
make -j6
cd SCIRun
cmake -DBUILD_BUNDLE:BOOL=ON ../../src
make -j6
make package
chmod +x repair_package.sh
./repair_package.sh
open .
