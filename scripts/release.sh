#!/bin/bash

MINPARAMS=1

if [ $# -lt "$MINPARAMS" ]
then
  echo
  echo "Called with a single parameter of the form \"alpha.XX\" or \"beta.A\""
  echo
  exit 0
fi

VERSION="v5.0-$1"
echo $VERSION

cd ../src
git checkout master
rm LATEST_TAG.txt
echo $VERSION >> LATEST_TAG.txt
git add LATEST_TAG.txt
git commit -m "Tagged release $VERSION"
git push origin master
git tag -a $VERSION -m "Release $VERSION"
git push --tags
