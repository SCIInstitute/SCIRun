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
    Figure out why including PSypy in the output (TBG Plugins option in the .cfg file) causes an execution crash
    Create a post sim visualization module that does not do real time visualization
    Review the set of modules in the PIConGPU category and decide what should be kept 
*/

#include <Core/Algorithms/ParticleInCell/PIConGPUAlgo.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::ParticleInCell;

ALGORITHM_PARAMETER_DEF(ParticleInCell, BackEnd);
ALGORITHM_PARAMETER_DEF(ParticleInCell, CloneDir);
ALGORITHM_PARAMETER_DEF(ParticleInCell, OutputDir);
ALGORITHM_PARAMETER_DEF(ParticleInCell, ConfigFile);
ALGORITHM_PARAMETER_DEF(ParticleInCell, SimulationFile);

PIConGPUAlgo::PIConGPUAlgo()
    {
    addParameter(Parameters::SimulationFile, std::string("$PIC_EXAMPLES/LaserWakefield"));
    addParameter(Parameters::ConfigFile, std::string("$PIC_CFG/sst.cfg"));
    addParameter(Parameters::CloneDir, std::string("$PIC_CLONE/myLWFA"));
    addParameter(Parameters::OutputDir, std::string("$PIC_OUTPUT/SST"));

    addParameter(Parameters::BackEnd, std::string("CPUonly"));

    addParameter(Variables::Method,1);
    //addParameter(Variables::CPUMethod,1);
    }

bool PIConGPUAlgo::StartPIConGPU(const std::string sim_input, const std::string cfg_input, const std::string sim_clone, const std::string sim_output, const int reRun) const
    {
    #include <stdlib.h>
    using namespace std;
    string text_file;

                                  //Set up the program that runs the Sim_run script
    text_file = "printf 'import os\nimport sys\nos.system(\"bash Sim_run\")' > ~/Sim.py";
    const char *command_Sim=text_file.c_str();
    system(command_Sim);

                                  //Assemble the script that creates the Sim_run script
    text_file = "printf '#!/usr/bin bash\n\nsource $HOME/picongpu.profile && pic-create "
                        +sim_input+" "+sim_clone+"\ncd "+sim_clone+" && pic-build && tbg -s bash -c "
                        +cfg_input+" -t etc/picongpu/bash/mpiexec.tpl "+sim_output+" &' > $HOME/Sim_run";

    if(reRun==0)
        {
        text_file = "printf '#!/usr/bin bash\n\nsource $HOME/picongpu_reRun.profile && rm -rf "+sim_output+" && cd "+sim_clone+" && tbg -s bash -c "
                  +cfg_input+" -t etc/picongpu/bash/mpiexec.tpl "+sim_output+" &' > $HOME/Sim_run";
        }

                                  //Run the script that creates the Sim_run script
    const char *command=text_file.c_str();
    system(command);

                                  //Run the Sim.py program that runs the Sim_run script
    string str_py="cd $HOME && python3 Sim.py";
    const char *command_py=str_py.c_str();
    system(command_py);

    return true;
    }

AlgorithmOutput PIConGPUAlgo::run(const AlgorithmInput&) const
    {
    AlgorithmOutput output;
    auto reRun       = get(Variables::Method).toInt();
    //auto CPU_Method  = get(Variables::CPUMethod).toInt();
    auto sim_clone   = get(Parameters::CloneDir).toString();
    auto sim_output  = get(Parameters::OutputDir).toString();
    auto cfg_input   = get(Parameters::ConfigFile).toString();
    auto sim_input   = get(Parameters::SimulationFile).toString();

    StartPIConGPU(sim_input, cfg_input, sim_clone, sim_output, reRun);
    return output;
    }
