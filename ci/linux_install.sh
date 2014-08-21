#!/usr/bin/env bash

sudo apt-get install -qq cmake
sudo apt-get install -qq libosmesa6-dev
sudo apt-get install -qq libqt4-dev qt4-qmake
echo "$CXX"
if [ "$CXX" = "g++" ]; then sudo apt-get install -qq g++-4.8; fi
if [ "$CXX" = "g++" ]; then export CXX="g++-4.8" CC="gcc-4.8"; fi
