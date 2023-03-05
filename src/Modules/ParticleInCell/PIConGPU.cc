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
*/

#include <openPMD/openPMD.hpp>
#include <filesystem>
#include <stdlib.h>

#include <string>
#include <sstream>

#include <Modules/ParticleInCell/PIConGPU.h>
#include <Core/Algorithms/ParticleInCell/PIConGPUAlgo.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::ParticleInCell;
using namespace SCIRun::Core::Algorithms::ParticleInCell;

using std::cout;
using namespace std;
using namespace openPMD;

#define openPMDIsAvailable 1

MODULE_INFO_DEF(PIConGPU,ParticleInCell,SCIRun);

PIConGPU::PIConGPU() : Module(staticInfo_) {}

void PIConGPU::setStateDefaults()
    {
    setStateIntFromAlgo(Variables::Method);
    setStateStringFromAlgo(Parameters::CloneDir);
    setStateStringFromAlgo(Parameters::OutputDir);
    setStateStringFromAlgo(Parameters::ConfigFile);
    setStateStringFromAlgo(Parameters::SimulationFile);
    }

void PIConGPU::execute()
    {
    AlgorithmInput input;
//    if(needToExecute())
        {
        auto state = get_state();
        setAlgoIntFromState(Variables::Method);
        setAlgoStringFromState(Parameters::CloneDir);
        setAlgoStringFromState(Parameters::OutputDir);
        setAlgoStringFromState(Parameters::ConfigFile);
        setAlgoStringFromState(Parameters::SimulationFile);
#if openPMDIsAvailable
        string text_file;
        text_file = "cp -p ~/src/picongpu/etc/picongpu/bash-pc-scii/*.profile ~/";
        const char *command_prof=text_file.c_str();
        system(command_prof);
        delete[] command_prof;
/*
        text_file = "rm -f ~/scratch/runs/SST/simOutput/openPMD/simData.sst";
        const char *command_remSST=text_file.c_str();
        system(command_remSST);
        delete[] command_rmSST;
*/
#endif
        auto output=algo().run(input);
        }  //end if(needToExecute())
    }  //end PIConGPU::execute()

