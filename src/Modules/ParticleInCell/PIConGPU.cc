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

#include <Modules/ParticleInCell/PIConGPU.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>

#include <Core/Algorithms/ParticleInCell/PIConGPUAlgo.h>

using namespace SCIRun;
using namespace SCIRun::Modules::ParticleInCell;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::ParticleInCell;
using namespace SCIRun::Core::Datatypes;

MODULE_INFO_DEF(PIConGPU,ParticleInCell,SCIRun);

//SCIRun::Core::Algorithms::AlgorithmParameterName PIConGPU::FormatString("FormatString");
//SCIRun::Core::Algorithms::AlgorithmParameterName PIConGPU::FunctionString("FunctionString");
//SCIRun::Core::Algorithms::AlgorithmParameterName PIConGPU::CloneString("CloneString");
//SCIRun::Core::Algorithms::AlgorithmParameterName PIConGPU::OutputString("OutputString");

PIConGPU::PIConGPU() : Module(staticInfo_)
    {
    INITIALIZE_PORT(x_coordinates);
    INITIALIZE_PORT(y_coordinates);
    INITIALIZE_PORT(z_coordinates);
    }

void PIConGPU::setStateDefaults()
    {
    setStateStringFromAlgo(Parameters::SimulationFile);
    setStateStringFromAlgo(Parameters::ConfigFile);
    setStateStringFromAlgo(Parameters::CloneDir);
    setStateStringFromAlgo(Parameters::OutputDir);
    setStateIntFromAlgo(Parameters::IterationIndex);
    setStateIntFromAlgo(Parameters::MaxIndex);
    }

void PIConGPU::execute()
    {
    AlgorithmInput input;
    if(needToExecute() || running_)
        { 
        auto state = get_state();
            
        setAlgoStringFromState(Parameters::SimulationFile);
        setAlgoStringFromState(Parameters::ConfigFile);
        setAlgoStringFromState(Parameters::CloneDir);
        setAlgoStringFromState(Parameters::OutputDir);
            
        int maxIndex;

        try
        {
        auto output=algo().run(input);
        sendOutputFromAlgorithm(x_coordinates,output);
        sendOutputFromAlgorithm(y_coordinates,output);
        sendOutputFromAlgorithm(z_coordinates,output);
        maxIndex = output.additionalAlgoOutput()->toInt();
        state->setValue(Parameters::MaxIndex, maxIndex);
        }
        catch (const Core::Algorithms::AlgorithmInputException&)
        {
          running_ = false;
          throw;
        }
        
        auto nextIndex = algo().get(Parameters::IterationIndex).toInt() + 1;
        if (nextIndex >= (maxIndex + 1))
        {
            running_=false;
        }
        else
        {
            runNextIteration(nextIndex % (maxIndex + 1);
        }
        }
    }

void PIConGPU::runNextIteration(int nextIndex)
{
    auto state = get_state();
    state->setValue(Parameters::IterationIndex, nextIndex);
    running_ = true;
//    int delay = state->getValue(Parameters::PlayModeDelay).toInt();
//    //std::cout << "delaying here for " << delay << " milliseconds" << std::endl;
//    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    enqueueExecuteAgain(false);
}
