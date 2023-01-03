/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.


Coding Sources:
   https://openpmd-api.readthedocs.io/en/latest/usage/streaming.html#c
   https://openpmd-api.readthedocs.io/en/latest/usage/serial.html#c
   email from Franz Poschel, dated 17 May 2022
   email from Franz Poschel, dated 16 June 2022: PIConGPU Issue 4162; https://github.com/ComputationalRadiationPhysics/picongpu/issues/4162


To Do:
    Implement a process for particle sample rate, mesh (node) sample rate, and mesh slice through a specified x, y, or z axis point
    Implement all of the above as config settings
    Figure out why including PSypy in the output (TBG Plugins option in the .cfg file) causes an execution crash

The current known to be functional implementation is the LaserWakefield simulation using any of the following config files 1.cfg, 2.cfg and sst.cfg
I have successfully tested the following PIConGPU module UI entries for Simulation and Config file:
    $PIC_EXAMPLES/LaserWakefield
    $PIC_CFG/sst.cfg
    $PIC_CFG/1.cfg
*/

//#include <openPMD/openPMD.hpp>
//#include <filesystem>

#include<Core/Algorithms/ParticleInCell/ScalarMeshReaderAlgo.h>
#include<Core/Datatypes/MatrixTypeConversions.h>
#include <chrono>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::ParticleInCell;
//using namespace openPMD;

AlgorithmOutput ScalarMeshReaderAlgo::run(const AlgorithmInput&) const
    {
    AlgorithmOutput output;
    return output;
    } //end of the ScalarMeshReaderAlgo algorithm run
