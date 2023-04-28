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

#ifndef MODULES_PARTICLEINCELL_PIConGPUReader_H
#define MODULES_PARTICLEINCELL_PIConGPUReader_H

#include <openPMD/openPMD.hpp>
#include <Modules/Fields/share.h>
#include <Dataflow/Network/Module.h>
#include <Modules/Basic/share.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

namespace SCIRun         {
namespace Modules        {
namespace ParticleInCell {

using namespace openPMD;
#define openPMDIsAvailable 1

#if openPMDIsAvailable
Series series;
SeriesIterator it, end;
#endif

int iteration_filter_i     = 1;
int iteration_filter_j     = 1;
int iteration_filter_k     = 1;
bool setup_                = false;
bool particlesPresent      = false;
bool vectorFieldPresent    = false;
bool scalarFieldPresent    = false;
//const std::string& home_   = std::getenv("HOME");
//const std::string& SST_dir = home_+"/scratch/runs/SST/simOutput/openPMD/simData.sst";
const std::string& SST_dir = "/Project/scratch/runs/SST/simOutput/openPMD/simData.sst";

int         DataSet;
int         Dim_i_max;
int         Dim_j_max;
int         Dim_k_max;
int         SampleRate;
std::string ParticleType;
std::string ScalarFieldComp;
std::string VectorFieldType;

class SCISHARE PIConGPUReader : public SCIRun::Dataflow::Networks::Module,
    public HasNoInputPorts,
    public Has3OutputPorts<FieldPortTag, FieldPortTag, FieldPortTag>
        {
        public:
            PIConGPUReader();
            void setupStream();
            void showDataSet();
            void shutdownStream();
            virtual void execute();
            virtual void setStateDefaults();

            OUTPUT_PORT(0, Particles,   Field);
            OUTPUT_PORT(1, ScalarField, Field);
            OUTPUT_PORT(2, VectorField, Field);

            MODULE_TRAITS_AND_INFO(SCIRun::Modules::ModuleFlags::ModuleHasUI);

        private:
            std::unique_ptr<class SimulationStreamingReaderBaseImpl> impl_;
        };
}}}
#endif
