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

using namespace SCIRun::Modules::Render;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

ViewScene::ViewScene() : Module(ModuleLookupInfo("ViewScene", "Render", "SCIRun")),
  renderer_(0)
{}

void ViewScene::setRenderer(SCIRun::Dataflow::Networks::RendererInterface* r)
{
  renderer_ = r;
}

void ViewScene::execute()
{
  auto strInput = getRequiredInput(RenderedString);
  auto matrixInput = getRequiredInput(RenderedMatrixAsVectorField);

  if (renderer_)
  {
    renderer_->clearScene();
    status("Renderer set! Attempting to visualize some data.");

    renderer_->setText(strInput->get_underlying()->as<String>()->value().c_str());

    const DenseMatrix* dm = matrixInput->get_underlying()->as<DenseMatrix>();
    if (dm)
       renderer_->setVectorField(*dm);

    renderer_->bringToFront();
  }
  else
  {
    error("Renderer not set, nothing to do here!");
  }
}