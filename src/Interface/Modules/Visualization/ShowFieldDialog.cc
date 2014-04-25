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

#include <Interface/Modules/Visualization/ShowFieldDialog.h>
#include <Modules/Visualization/ShowField.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/Datatypes/Color.h>
#include <QColorDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;

ShowFieldDialog::ShowFieldDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent),
  defaultMeshColor_(Qt::gray)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addCheckBoxManager(showNodesCheckBox_, ShowFieldModule::ShowNodes);
  addCheckBoxManager(showEdgesCheckBox_, ShowFieldModule::ShowEdges);
  addCheckBoxManager(showFacesCheckBox_, ShowFieldModule::ShowFaces);
  addCheckBoxManager(enableTransparencyNodesCheckBox_, ShowFieldModule::NodeTransparency);
  addCheckBoxManager(enableTransparencyEdgesCheckBox_, ShowFieldModule::EdgeTransparency);
  addCheckBoxManager(enableTransparencyFacesCheckBox_, ShowFieldModule::FaceTransparency);
  addCheckBoxManager(invertNormalsCheckBox, ShowFieldModule::FaceInvertNormals);
  buttonBox->setVisible(false);
  connect(defaultMeshColorButton_, SIGNAL(clicked()), this, SLOT(assignDefaultMeshColor()));
}

void ShowFieldDialog::push()
{
  if (!pulling_)
  {
    pushColor();
  }
}

void ShowFieldDialog::pull()
{
  pull_newVersionToReplaceOld();
  Pulling p(this);
  ColorRGB color(state_->getValue(ShowFieldModule::DefaultMeshColor).getString());
  defaultMeshColor_ = QColor(color.r(), color.g(), color.b());
}

void ShowFieldDialog::assignDefaultMeshColor()
{
  auto newColor = QColorDialog::getColor(defaultMeshColor_, this, "Choose default mesh color");
  if (newColor.isValid())
  {
    defaultMeshColor_ = newColor;
    //TODO: set color of button to this color
    //defaultMeshColorButton_->set
    pushColor();
  }
}

void ShowFieldDialog::pushColor()
{
  state_->setValue(ShowFieldModule::DefaultMeshColor, ColorRGB(defaultMeshColor_.red(), defaultMeshColor_.green(), defaultMeshColor_.blue()).toString());
}
