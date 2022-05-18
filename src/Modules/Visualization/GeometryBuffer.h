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


#ifndef MODULES_VISUALIZATION_GEOMETRY_BUFFER_H
#define MODULES_VISUALIZATION_GEOMETRY_BUFFER_H

#include <Dataflow/Network/ModuleWithAsyncDynamicPorts.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {

  namespace Core {
    namespace Algorithms {
      namespace Visualization {

        ALGORITHM_PARAMETER_DECL(BufferSize);
        ALGORITHM_PARAMETER_DECL(GeometryIndex);
        ALGORITHM_PARAMETER_DECL(PlayModeActive);
        ALGORITHM_PARAMETER_DECL(LoopForever);
        ALGORITHM_PARAMETER_DECL(SingleStep);
        ALGORITHM_PARAMETER_DECL(GeometryIncrement);
        ALGORITHM_PARAMETER_DECL(PlayModeDelay);
        ALGORITHM_PARAMETER_DECL(ClearFlag);
      }
    }
  }

namespace Modules {
namespace Visualization {

  class SCISHARE GeometryBuffer : public SCIRun::Dataflow::Networks::ModuleWithAsyncDynamicPorts,
    public Has1InputPort<AsyncDynamicPortTag<GeometryPortTag>>,
    public Has8OutputPorts<GeometryPortTag, GeometryPortTag, GeometryPortTag, GeometryPortTag, GeometryPortTag, GeometryPortTag, GeometryPortTag, GeometryPortTag>
  {
  public:
    GeometryBuffer();
    ~GeometryBuffer();
    void execute() override;
    void asyncExecute(const Dataflow::Networks::PortId& pid, Core::Datatypes::DatatypeHandle data) override;
    void portRemovedSlotImpl(const Dataflow::Networks::PortId& pid) override;
    void setStateDefaults() override;
    INPUT_PORT(0, GeometryInput, GeometryObject);
    OUTPUT_PORT(0, GeometryOutputSeries0, GeometryObject);
    OUTPUT_PORT(1, GeometryOutputSeries1, GeometryObject);
    OUTPUT_PORT(2, GeometryOutputSeries2, GeometryObject);
    OUTPUT_PORT(3, GeometryOutputSeries3, GeometryObject);
    OUTPUT_PORT(4, GeometryOutputSeries4, GeometryObject);
    OUTPUT_PORT(5, GeometryOutputSeries5, GeometryObject);
    OUTPUT_PORT(6, GeometryOutputSeries6, GeometryObject);
    OUTPUT_PORT(7, GeometryOutputSeries7, GeometryObject);
    MODULE_TRAITS_AND_INFO(ModuleFlags::ModuleHasUI)
  private:
    friend class GeometryBufferImpl;
    std::unique_ptr<class GeometryBufferImpl> impl_;
  };
}}}

#endif
