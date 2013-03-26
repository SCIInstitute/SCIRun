/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#ifndef MODULES_RENDER_VIEWSCENE_H
#define MODULES_RENDER_VIEWSCENE_H

#include <Dataflow/Network/Module.h>
#include <Modules/Render/Share.h>

#include "Spire/Interface.h"
#include "Spire/AppSpecific/SCIRun/SRInterface.h"

namespace SCIRun {
namespace Modules {
namespace Render {

  class SCISHARE ViewScene : public SCIRun::Dataflow::Networks::Module,
    public Has2InputPorts<GeometryPortTag, GeometryPortTag>
  {
  public:
    ViewScene();
    virtual void execute();

    void setRenderer(SCIRun::Dataflow::Networks::RendererInterface* r);

    /// Used to initialize spire with the context given in the Transient state.
    virtual void preExecutionInitialization();

    /// Used to join/destroy the spire thread before the rendering context
    /// is destroyed.
    virtual void preDestruction();

    INPUT_PORT(0, GeneralGeom1, GeometryObject);
    INPUT_PORT(1, GeneralGeom2, GeometryObject);

  private:
    SCIRun::Dataflow::Networks::RendererInterface* renderer_;

    std::weak_ptr<Spire::SCIRun::SRInterface>   mSpire;
  };
}}}

#endif
