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

#include <Interface/Modules/Visualization/ShowMeshDialog.h>
#include <Modules/Visualization/ShowMesh.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <QFileDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Visualization;

ShowMeshDialog::ShowMeshDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  connect(showEdgesCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(push()));
  connect(showFacesCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(push()));
  connect(zTestCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(push()));
}

void ShowMeshDialog::push()
{
  if (!pulling_)
  {
    state_->setValue(ShowMeshModule::ShowEdges, showEdgesCheckBox_->isChecked());
    state_->setValue(ShowMeshModule::ShowFaces, showFacesCheckBox_->isChecked());
    state_->setValue(ShowMeshModule::ZTestOn, zTestCheckBox_->isChecked());
  }
}

void ShowMeshDialog::pull()
{
  Pulling p(this);
  showEdgesCheckBox_->setChecked(state_->getValue(ShowMeshModule::ShowEdges).getBool());
  showFacesCheckBox_->setChecked(state_->getValue(ShowMeshModule::ShowFaces).getBool());
  zTestCheckBox_->setChecked(state_->getValue(ShowMeshModule::ZTestOn).getBool());
}
