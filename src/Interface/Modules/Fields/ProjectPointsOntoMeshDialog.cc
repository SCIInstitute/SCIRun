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

#include <Interface/Modules/Fields/ProjectPointsOntoMeshDialog.h>
#include <Core/Algorithms/Legacy/Fields/TransformMesh/ProjectPointsOntoMesh.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;

ProjectPointsOntoMeshDialog::ProjectPointsOntoMeshDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  connect(pointsOntoElementsRadioButton_, SIGNAL(clicked()), this, SLOT(push()));
  connect(pointsOntoNodesRadioButton_, SIGNAL(clicked()), this, SLOT(push()));
}

void ProjectPointsOntoMeshDialog::push()
{
  if (!pulling_)
  {
    using namespace Parameters;
    state_->setValue(ProjectMethod, pointsOntoElementsRadioButton_->isChecked() ? std::string("elements") : std::string("nodes"));
  }
}

void ProjectPointsOntoMeshDialog::pull()
{
  Pulling p(this);
  
  using namespace Parameters;
  auto method = state_->getValue(ProjectMethod).getString();
  pointsOntoElementsRadioButton_->setChecked("elements" == method);
  pointsOntoNodesRadioButton_->setChecked("nodes" == method);

  pull_newVersionToReplaceOld();
}
