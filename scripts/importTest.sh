#!/bin/bash

find ../SCIRunTestData -name "*.srn" -exec ./SCIRun_test -x --import {} \;
