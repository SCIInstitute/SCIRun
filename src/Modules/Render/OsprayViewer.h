/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#ifndef MODULES_RENDER_OSPRAY_VIEWER_H
#define MODULES_RENDER_OSPRAY_VIEWER_H

#include <Dataflow/Network/ModuleWithAsyncDynamicPorts.h>
#include <Core/Thread/Mutex.h>
#include <Core/Algorithms/Base/AlgorithmMacros.h>
#include <Modules/Render/share.h>

namespace SCIRun {

  namespace Core
  {
    namespace Algorithms
    {
      namespace Render
      {
        ALGORITHM_PARAMETER_DECL(ShowPlane);
        ALGORITHM_PARAMETER_DECL(AutoRotationRate);
      }
    }
  }

namespace Modules {
namespace Render {

  class SCISHARE OsprayViewer : public Dataflow::Networks::ModuleWithAsyncDynamicPorts,
    public Has1InputPort<AsyncDynamicPortTag<OsprayGeometryPortTag>>,
    public HasNoOutputPorts
  {
  public:
    OsprayViewer();
    virtual void asyncExecute(const Dataflow::Networks::PortId& pid, Core::Datatypes::DatatypeHandle data) override;
    virtual void setStateDefaults() override;

    INPUT_PORT_DYNAMIC(0, GeneralGeom, OsprayGeometryObject);
    virtual void execute() override;

    MODULE_TRAITS_AND_INFO(ModuleHasUI)

  protected:
    virtual void portRemovedSlotImpl(const Dataflow::Networks::PortId& pid) override;
  };
}}}

#endif
