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

  setupVectorsTab();
  setupScalarsTab();
  setupTensorsTab();
  setupSecondaryTab();
  setupTertiaryTab();
  addLineEditManager(lineEdit, ShowFieldGlyphs::FieldName);
  WidgetStyleMixin::tabStyle(this->displayOptionsTabs_);


  createExecuteInteractivelyToggleAction();

  connect(defaultMeshColorButton_, SIGNAL(clicked()), this, SLOT(assignDefaultMeshColor()));
  connectButtonToExecuteSignal(defaultMeshColorButton_);
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

void ShowFieldGlyphsDialog::pullSpecial()
{
  ColorRGB color(state_->getValue(ShowFieldGlyphs::DefaultMeshColor).toString());
  //std::cout << "pull color: " << color.r() << " " << color.g() << " " << color.b() << std::endl;
  // check for old saved color format: integers 0-255.
  defaultMeshColor_ = QColor(
    static_cast<int>(color.r() > 1 ? color.r() : color.r() * 255.0),
    static_cast<int>(color.g() > 1 ? color.g() : color.g() * 255.0),
    static_cast<int>(color.b() > 1 ? color.b() : color.b() * 255.0));

  //  checkTabs();
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
  state_->setValue(ShowFieldGlyphs::DefaultMeshColor, ColorRGB(defaultMeshColor_.redF(), defaultMeshColor_.greenF(), defaultMeshColor_.blueF()).toString());
}

/**
void ShowFieldGlyphsDialog::checkTabs()
{
  // Show the vector tab
  if (state_->getValue(ShowFieldGlyphs::ShowVectorTab).toBool())
  {
    if (vectorTabIndex_ < 0)
    {
      displayOptionsTabs_->addTab(this, tr("Vectors"));
      vectorTabIndex_ = displayOptionsTabs_->count() - 3;
    }
  }
  else
  {
    if (vectorTabIndex_ > 0)
    {
      displayOptionsTabs_->removeTab(vectorTabIndex_);
      if (scalarTabIndex_ > vectorTabIndex_)
        --scalarTabIndex_;
      if (tensorTabIndex_ > vectorTabIndex_)
        --tensorTabIndex_;
      vectorTabIndex_ = -1;
    }
  }
  // Show the scalar tab
  if (state_->getValue(ShowFieldGlyphs::ShowScalarTab).toBool())
  {
    if (scalarTabIndex_ < 0)
    {
      displayOptionsTabs_->addTab(this, tr("Scalars"));
      scalarTabIndex_ = displayOptionsTabs_->count() - 3;
    }
  }
  else
  {
    if (scalarTabIndex_ > 0)
    {
      displayOptionsTabs_->removeTab(scalarTabIndex_);
      if (vectorTabIndex_ > scalarTabIndex_)
        --vectorTabIndex_;
      if (tensorTabIndex_ > scalarTabIndex_)
        --tensorTabIndex_;
      scalarTabIndex_ = -1;
    }
  }
  // Show the tensor tab
  if (state_->getValue(ShowFieldGlyphs::ShowTensorTab).toBool())
  {
    if (tensorTabIndex_ < 0)
    {
      displayOptionsTabs_->addTab(this, tr("Tensors"));
      tensorTabIndex_ = displayOptionsTabs_->count() - 3;
    }
  }
  else
  {
    if (tensorTabIndex_ > 0)
    {
      displayOptionsTabs_->removeTab(tensorTabIndex_);
      if (vectorTabIndex_ > tensorTabIndex_)
        --vectorTabIndex_;
      if (scalarTabIndex_ > tensorTabIndex_)
        --scalarTabIndex_;
      tensorTabIndex_ = -1;
    }
  }
  // Show secondary and tertiary tabs
  displayOptionsTabs_->addTab(secondaryTab_, tr("Secondary"));
  displayOptionsTabs_->addTab(tertiaryTab_, tr("Tertiary"));

  displayOptionsTabs_->setCurrentIndex(displayOptionsTabs_->count() - 3);
}
**/

void ShowFieldGlyphsDialog::setupScalarsTab()
{
  // Show Scalars
  addCheckableButtonManager(this->showScalarsCheckBox_, ShowFieldGlyphs::ShowScalars);
  // Display Type
  addRadioButtonGroupManager({ this->scalarsAsPointsRButton_, this->scalarsAsSpheresRButton_,
                               this->scalarsAsBoxesRButton_, this->scalarsAsAxisRButton_ }, ShowFieldGlyphs::ScalarsDisplayType);
  // Coloring
  addRadioButtonGroupManager({ this->defaultScalarsColoringRButton_, this->colormapLookupScalarsColoringRButton_,
                               this->conversionRGBScalarsColoringRButton_ }, ShowFieldGlyphs::ScalarsColoring);
  // Coloring Data Input
  addComboBoxGroupManager(this->scalarsColoringInputComboBox, ShowFieldGlyphs::ScalarsColoringDataInput);
  // Transparency
  addRadioButtonGroupManager({ this->scalarsTransparencyOffRButton_, this->scalarsTransparencyDataInputRButton_,
                               this->scalarsCustomTransparencyRButton_ }, ShowFieldGlyphs::ScalarsTransparency);
  addDoubleSpinBoxManager(this->scalarsTransparencyDoubleSpinBox_, ShowFieldGlyphs::ScalarsCustomTransparencyValue);
  // Transparency Data Input
  addComboBoxGroupManager(this->scalarsTransparencyInputComboBox, ShowFieldGlyphs::ScalarsTransparencyDataInput);
  // Scale
  addDoubleSpinBoxManager(this->scaleScalarsDoubleSpinBox_, ShowFieldGlyphs::ScalarsScale);
  // Resolution
  addSpinBoxManager(this->scalarsResolutionSpinBox_, ShowFieldGlyphs::ScalarsResolution);
  // Threshold
  addSpinBoxManager(this->scalarsThresholdDoubleSpinBox_, ShowFieldGlyphs::ScalarsThreshold);

  connectButtonToExecuteSignal(this->showScalarsCheckBox_);
  connectButtonToExecuteSignal(this->scalarsTransparencyOffRButton_);
  connectButtonToExecuteSignal(this->scalarsCustomTransparencyRButton_);
  connectButtonToExecuteSignal(this->defaultScalarsColoringRButton_);
  connectButtonToExecuteSignal(this->colormapLookupScalarsColoringRButton_);
  connectButtonToExecuteSignal(this->conversionRGBScalarsColoringRButton_);
  connectButtonToExecuteSignal(this->scalarsAsPointsRButton_);
  connectButtonToExecuteSignal(this->scalarsAsSpheresRButton_);
  //connectButtonToExecuteSignal(this->scalarsAsBoxesRButton_);
  //connectButtonToExecuteSignal(this->scalarsAsAxisRButton_);
}

void ShowFieldGlyphsDialog::setupVectorsTab()
{
  // Show Vectors
  addCheckableButtonManager(this->showVectorsCheckBox_, ShowFieldGlyphs::ShowVectors);
  // Display Type
  addRadioButtonGroupManager({ this->vectorsAsLinesRButton_, this->vectorsAsCometsRButton_,
                               this->vectorsAsArrowsRButton_, this->vectorsAsRingsRButton_,
                               this->vectorsAsNeedlesRButton_, this->vectorsAsConesRButton_,
                               this->vectorsAsDisksRButton_, this->vectorsAsSpringsRButton_ }
                              , ShowFieldGlyphs::VectorsDisplayType);
  // Coloring
  addRadioButtonGroupManager({ this->defaultVectorsColoringRButton_, this->colormapLookupVectorsColoringRButton_,
                               this->conversionRGBVectorsColoringRButton_ }, ShowFieldGlyphs::VectorsColoring);
  // Coloring Data Input
  addComboBoxGroupManager(this->vectorsColoringInputComboBox, ShowFieldGlyphs::VectorsColoringDataInput);
  // Transparency
  addRadioButtonGroupManager({ this->vectorsTransparencyOffRButton_, this->vectorsTransparencyDataInputRButton_,
                               this->vectorsCustomTransparencyRButton_ }, ShowFieldGlyphs::VectorsTransparency);
  addDoubleSpinBoxManager(this->vectorsTransparencyDoubleSpinBox_, ShowFieldGlyphs::VectorsCustomTransparencyValue);
  // Transparency Data Input
  addComboBoxGroupManager(this->vectorsTransparencyInputComboBox, ShowFieldGlyphs::VectorsTransparencyDataInput);
  // Normalize
  addCheckableButtonManager(this->normalizeVectorsCheckBox_, ShowFieldGlyphs::NormalizeGlyphs);
  // Scale
  addDoubleSpinBoxManager(this->scaleVectorsDoubleSpinBox_, ShowFieldGlyphs::VectorsScale);
  // Resolution
  addSpinBoxManager(this->vectorsResolutionSpinBox_, ShowFieldGlyphs::VectorsResolution);
  // Threshold
  addCheckableButtonManager(this->renderVectorsBelowThresholdCheckBox_, ShowFieldGlyphs::RenderVectorsBelowThreshold);
  addSpinBoxManager(this->vectorsThresholdDoubleSpinBox_, ShowFieldGlyphs::VectorsThreshold);
  // Bidirectional
  addCheckableButtonManager(this->bidirectionalVectorsCheckBox_, ShowFieldGlyphs::RenderBidirectionaly);

  connectButtonToExecuteSignal(this->showVectorsCheckBox_);
  connectButtonToExecuteSignal(this->vectorsTransparencyOffRButton_);
  connectButtonToExecuteSignal(this->vectorsCustomTransparencyRButton_);
  connectButtonToExecuteSignal(this->defaultVectorsColoringRButton_);
  connectButtonToExecuteSignal(this->colormapLookupVectorsColoringRButton_);
  connectButtonToExecuteSignal(this->conversionRGBVectorsColoringRButton_);
  connectButtonToExecuteSignal(this->vectorsAsLinesRButton_);
  connectButtonToExecuteSignal(this->vectorsAsNeedlesRButton_);
  connectButtonToExecuteSignal(this->vectorsAsCometsRButton_);
  connectButtonToExecuteSignal(this->vectorsAsConesRButton_);
  connectButtonToExecuteSignal(this->vectorsAsArrowsRButton_);
  //connectButtonToExecuteSignal(this->vectorsAsDisksRButton_);
  //connectButtonToExecuteSignal(this->vectorsAsRingsRButton_);
  //connectButtonToExecuteSignal(this->vectorsAsSpringsRButton_);
  connectButtonToExecuteSignal(this->normalizeVectorsCheckBox_);
  connectButtonToExecuteSignal(this->bidirectionalVectorsCheckBox_);
  connectButtonToExecuteSignal(this->renderVectorsBelowThresholdCheckBox_);
}

void ShowFieldGlyphsDialog::setupTensorsTab()
{
  // Show Tensors
  addCheckableButtonManager(this->showTensorsCheckBox_, ShowFieldGlyphs::ShowTensors);
  // Display Type
  addRadioButtonGroupManager({ this->tensorsAsEllipsoidsRButton_, this->tensorsAsBoxesRButton_,
                               this->tensorsAsColoredBoxesRButton_, this->tensorsAsSuperquadricsRButton_}
                              , ShowFieldGlyphs::TensorsDisplayType);
  // Coloring
  addRadioButtonGroupManager({ this->defaultTensorsColoringRButton_, this->colormapLookupTensorsColoringRButton_,
                               this->conversionRGBTensorsColoringRButton_ }, ShowFieldGlyphs::TensorsColoring);
  // Coloring Data Input
  addComboBoxGroupManager(this->tensorsColoringInputComboBox, ShowFieldGlyphs::TensorsColoringDataInput);
  // Transparency
  addRadioButtonGroupManager({ this->tensorsTransparencyOffRButton_, this->tensorsTransparencyDataInputRButton_,
                               this->tensorsCustomTransparencyRButton_ }, ShowFieldGlyphs::TensorsTransparency);
  addDoubleSpinBoxManager(this->tensorsTransparencyDoubleSpinBox_, ShowFieldGlyphs::TensorsCustomTransparencyValue);
  // Transparency Data Input
  addComboBoxGroupManager(this->tensorsTransparencyInputComboBox, ShowFieldGlyphs::TensorsTransparencyDataInput);
  // Normalize
  addCheckableButtonManager(this->normalizeTensorsCheckBox_, ShowFieldGlyphs::NormalizeGlyphs);
  // Scale
  addDoubleSpinBoxManager(this->scaleTensorsDoubleSpinBox_, ShowFieldGlyphs::TensorsScale);
  // Resolution
  addSpinBoxManager(this->tensorsResolutionSpinBox_, ShowFieldGlyphs::TensorsResolution);
  // Threshold
  addCheckableButtonManager(this->renderVectorsBelowThresholdCheckBox_, ShowFieldGlyphs::RenderTensorsBelowThreshold);
  addSpinBoxManager(this->tensorsThresholdDoubleSpinBox_, ShowFieldGlyphs::TensorsThreshold);

  connectButtonToExecuteSignal(this->showTensorsCheckBox_);
  connectButtonToExecuteSignal(this->tensorsTransparencyOffRButton_);
  connectButtonToExecuteSignal(this->tensorsCustomTransparencyRButton_);
  connectButtonToExecuteSignal(this->defaultTensorsColoringRButton_);
  connectButtonToExecuteSignal(this->colormapLookupTensorsColoringRButton_);
  connectButtonToExecuteSignal(this->conversionRGBTensorsColoringRButton_);
  connectButtonToExecuteSignal(this->tensorsAsEllipsoidsRButton_);
  //connectButtonToExecuteSignal(this->tensorsAsBoxesRButton_);
  //connectButtonToExecuteSignal(this->tensorsAsColoredBoxesRButton_);
  //connectButtonToExecuteSignal(this->tensorsAsSuperquadricsRButton_);
}
