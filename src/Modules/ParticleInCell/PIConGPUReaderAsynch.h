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

#ifndef MODULES_PARTICLEINCELL_PIConGPUReaderAsynch_H
#define MODULES_PARTICLEINCELL_PIConGPUReaderAsynch_H

#include <openPMD/openPMD.hpp>
#include <Modules/Fields/share.h>
#include <Dataflow/Network/Module.h>
#include <Modules/Basic/share.h>

namespace SCIRun         {
namespace Modules        {
namespace ParticleInCell {

using namespace openPMD;


    //  These variables need to be initially created somewhere that is not here and made accessible from here.  Thinking of expanding getSeries to do that
    Series series;
    SeriesIterator it, end;
    bool setup_{ false };

    int iteration_counter=0;

SCISHARE Core::Datatypes::BundleHandle bundleOutputs(std::initializer_list<std::string> names, std::initializer_list<Core::Datatypes::DatatypeHandle> dataList);

class SCISHARE PIConGPUReaderAsynch : public SCIRun::Dataflow::Networks::Module,
    public HasNoInputPorts,
    public Has4OutputPorts<FieldPortTag, FieldPortTag, FieldPortTag, BundlePortTag>
        {
        public:
            PIConGPUReaderAsynch();
            virtual void execute();
            virtual void setStateDefaults();

            // override these methods in subclass
            //virtual void setupStream();
            //virtual bool hasData() const;
            //virtual Core::Datatypes::BundleHandle nextData() const;
            //virtual void shutdownStream();

            OUTPUT_PORT(0, Particles, Field);
            OUTPUT_PORT(1, ScalarField, Field);
            OUTPUT_PORT(2, VectorField, Field);
            OUTPUT_PORT(3, OutputData, Bundle);

            MODULE_TRAITS_AND_INFO(SCIRun::Modules::ModuleFlags::ModuleHasUIAndAlgorithm);

        private:
            //std::unique_ptr<class StreamAppenderImpl> streamer_;
            std::unique_ptr<class SimulationStreamingReaderBaseImpl> impl_;
        };
}}}
#endif
