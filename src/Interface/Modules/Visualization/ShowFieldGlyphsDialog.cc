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

#include <Interface/Modules/Visualization/ShowFieldGlyphsDialog.h>
#include <Modules/Visualization/ShowFieldGlyphs.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/Datatypes/Color.h>
#include <QColorDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
//using namespace SCIRun::Core::Algorithms::Visualization;

ShowFieldGlyphsDialog::ShowFieldGlyphsDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();  
  
  setupVectorsTab();

  WidgetStyleMixin::tabStyle(this->displayOptionsTabs_); 


  createExecuteInteractivelyToggleAction();
  
  connect(defaultMeshColorButton_, SIGNAL(clicked()), this, SLOT(assignDefaultMeshColor()));
}

void ShowFieldGlyphsDialog::push()
{
  if (!pulling_)
  {
    pushColor();
  }
}

void ShowFieldGlyphsDialog::createStartupNote()
{
  auto showFieldGlyphId = windowTitle().split(':')[1];
  setStartupNote("ID: " + showFieldGlyphId);
}

void ShowFieldGlyphsDialog::pull()
{
  pull_newVersionToReplaceOld();
  Pulling p(this);
  ColorRGB color(state_->getValue(ShowFieldGlyphs::DefaultMeshColor).toString());
  //std::cout << "pull color: " << color.r() << " " << color.g() << " " << color.b() << std::endl;
  // check for old saved color format: integers 0-255.
  defaultMeshColor_ = QColor(
    static_cast<int>(color.r() > 1 ? color.r() : color.r() * 255.0),
    static_cast<int>(color.g() > 1 ? color.g() : color.g() * 255.0),
    static_cast<int>(color.b() > 1 ? color.b() : color.b() * 255.0));
}


void ShowFieldGlyphsDialog::assignDefaultMeshColor()
{
  auto newColor = QColorDialog::getColor(defaultMeshColor_, this, "Choose default mesh color");
  if (newColor.isValid())
  {
    defaultMeshColor_ = newColor;
    //TODO: set color of button to this color
    pushColor();
  }
}

void ShowFieldGlyphsDialog::pushColor()
{
  //std::cout << "push color: " << defaultMeshColor_.redF() << " " << defaultMeshColor_.greenF() << " " << defaultMeshColor_.blueF() << std::endl;
  state_->setValue(ShowFieldGlyphs::DefaultMeshColor, ColorRGB(defaultMeshColor_.redF(), defaultMeshColor_.greenF(), defaultMeshColor_.blueF()).toString());
  Q_EMIT executeActionTriggered();
}

void ShowFieldGlyphsDialog::setupVectorsTab()
{
  vectorTab_ = new ShowFieldGlyphsVectorTabDialog(this);
  displayOptionsTabs_->addTab(vectorTab_, tr("Vectors"));
  //displayOptionsTabs_->removeTab(1);
  addCheckableButtonManager(vectorTab_->showVectorsCheckBox_, ShowFieldGlyphs::ShowVectors);
  addCheckableButtonManager(vectorTab_->enableTransparencyVectorsCheckBox_, ShowFieldGlyphs::VectorsTransparency);
  addDoubleSpinBoxManager(vectorTab_->vectorsTransparencyDoubleSpinBox_, ShowFieldGlyphs::VectorsTransparencyValue);
  addDoubleSpinBoxManager(vectorTab_->scaleVectorsDoubleSpinBox_, ShowFieldGlyphs::VectorsScale);
  addSpinBoxManager(vectorTab_->vectorsResolutionSpinBox_, ShowFieldGlyphs::VectorsResolution);
  addRadioButtonGroupManager({ vectorTab_->defaultVectorsColoringRButton_, vectorTab_->colormapLookupVectorsColoringRButton_,
    vectorTab_->conversionRGBVectorsColoringRButton_ }, ShowFieldGlyphs::VectorsColoring);
  addRadioButtonGroupManager({ vectorTab_->vectorsAsLinesRButton_, vectorTab_->vectorsAsNeedlesRButton_,
    vectorTab_->vectorsAsCometsRButton_, vectorTab_->vectorsAsConesRButton_, vectorTab_->vectorsAsArrowsRButton_,
    vectorTab_->vectorsAsDisksRButton_, vectorTab_->vectorsAsRingsRButton_, vectorTab_->vectorsAsSpringsRButton_ },
    ShowFieldGlyphs::VectorsDisplayType);
}