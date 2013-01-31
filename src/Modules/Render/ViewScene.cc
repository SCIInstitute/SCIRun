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

#include <Modules/Render/ViewScene.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Dataflow/Network/RendererInterface.h>

#include "Spire/Interface.h"

using namespace SCIRun::Modules::Render;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

ViewScene::ViewScene() : Module(ModuleLookupInfo("ViewScene", "Render", "SCIRun")),
  renderer_(0)
{
}

void ViewScene::setRenderer(SCIRun::Dataflow::Networks::RendererInterface* r)
{
  renderer_ = r;
}

void ViewScene::preExecutionInitialization()
{
  // Lookup state information in order to create Spire
  boost::any context = get_state()->getTransientValue("glContext");
  std::vector<std::string> shaderDirs;

  if (!context.empty())
  {
    try
    {
      std::shared_ptr<Spire::Context> glContext = 
          boost::any_cast<std::weak_ptr<Spire::Context>>(context).lock();

      // Note: On windows, we *need* to create a non-threaded renderer.
      mSpire = std::shared_ptr<Spire::Interface>(
          new Spire::Interface(glContext, shaderDirs, true));
    }
    catch (const boost::bad_any_cast& e)
    {
      error("Unable to cast glContext transient value to boost::any.");
    }
  }
  else
  {
    error("Unable to find transient context value.");
  }

  /// \todo Add signal that terminates spire (resets the shared pointer) when
  ///       the context is lost.
}

void ViewScene::preDestruction()
{
  // Destroy spire.
  mSpire.reset();
}

void ViewScene::execute()
{
  // render updated 

  error("Renderer not set, nothing to do here!");
}
