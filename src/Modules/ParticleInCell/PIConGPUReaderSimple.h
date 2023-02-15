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

#ifndef MODULES_PARTICLEINCELL_PIConGPUReaderSimple_H
#define MODULES_PARTICLEINCELL_PIConGPUReaderSimple_H

#include <openPMD/openPMD.hpp>
#include <Modules/Fields/share.h>
#include <Dataflow/Network/Module.h>
#include <Modules/Basic/share.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

namespace SCIRun         {
namespace Modules        {
namespace ParticleInCell {

using namespace openPMD;

Series seriesSimple;
SeriesIterator itSimple, endSimple;
bool setupSimple = false;
int iteration_counterSimple = 0;
const std::string& SST_dirSimple = "/home/kj/scratch/runs/SST/simOutput/openPMD/simData.sst";

int         SampleRateSimple;
std::string ParticleTypeSimple;
std::string ScalarFieldCompSimple;
std::string VectorFieldTypeSimple;

class SCISHARE PIConGPUReaderSimple : public SCIRun::Dataflow::Networks::Module,
    public HasNoInputPorts,
    public Has3OutputPorts<FieldPortTag, FieldPortTag, FieldPortTag>
        {
        public:
            PIConGPUReaderSimple();
            void setupStream();
            virtual void execute();
            virtual void setStateDefaults();

            OUTPUT_PORT(0, Particles, Field);
            OUTPUT_PORT(1, ScalarField, Field);
            OUTPUT_PORT(2, VectorField, Field);

            MODULE_TRAITS_AND_INFO(SCIRun::Modules::ModuleFlags::ModuleHasUI);

        private:
            std::unique_ptr<class SimulationStreamingReaderBaseImpl> impl_;
        };
}}}
#endif
