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


#include <Interface/Modules/Visualization/ShowFieldGlyphsDialog.h>
#include <Modules/Visualization/ShowFieldGlyphs.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/Datatypes/Color.h>
#include <QColorDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;

ShowFieldGlyphsDialog::ShowFieldGlyphsDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));

  setupVectorsTab();
  setupScalarsTab();
  setupTensorsTab();

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

  // Switch tabs
  if(state_->getValue(ShowFieldGlyphs::ShowScalarTab).toBool())
    displayOptionsTabs_->setCurrentIndex(0);
  else if(state_->getValue(ShowFieldGlyphs::ShowVectorTab).toBool())
    displayOptionsTabs_->setCurrentIndex(1);
  if(state_->getValue(ShowFieldGlyphs::ShowTensorTab).toBool())
    displayOptionsTabs_->setCurrentIndex(2);
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

void ShowFieldGlyphsDialog::setupScalarsTab()
{
  // Show Scalars
  addCheckableButtonManager(this->showScalarsCheckBox_, ShowFieldGlyphs::ShowScalars);
  // Display Type
  addComboBoxManager(this->scalarsDisplayTypeComboBox_, ShowFieldGlyphs::ScalarsDisplayType);
  this->scalarsDisplayTypeComboBox_->setCurrentIndex(0);
  // Coloring
  addComboBoxManager(scalarsColorTypeComboBox_, ShowFieldGlyphs::ScalarsColoring);
  // Coloring Data Input
  addComboBoxManager(this->scalarsColoringInputComboBox_, ShowFieldGlyphs::ScalarsColoringDataInput);
  // Transparency
  addRadioButtonGroupManager({ this->scalarsTransparencyOffRButton_, this->scalarsUniformTransparencyRButton_}, ShowFieldGlyphs::ScalarsTransparency);
  addDoubleSpinBoxManager(this->scalarsTransparencyDoubleSpinBox_, ShowFieldGlyphs::ScalarsUniformTransparencyValue);
  // Transparency Data Input
  //  addComboBoxManager(this->scalarsTransparencyInputComboBox_, ShowFieldGlyphs::ScalarsTransparencyDataInput);
  // Scale
  addDoubleSpinBoxManager(this->scaleScalarsDoubleSpinBox_, ShowFieldGlyphs::ScalarsScale);
  // Resolution
  addSpinBoxManager(this->scalarsResolutionSpinBox_, ShowFieldGlyphs::ScalarsResolution);
  // Threshold
  addDoubleSpinBoxManager(this->scalarsThresholdDoubleSpinBox_, ShowFieldGlyphs::ScalarsThreshold);

  // Execute if any changed
  connectButtonToExecuteSignal(this->showScalarsCheckBox_);
  connectButtonToExecuteSignal(this->scalarsTransparencyOffRButton_);
  connectButtonToExecuteSignal(this->scalarsUniformTransparencyRButton_);

  // Text Labels
  // this->scalarColorTypeLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->scalarColorInputLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->normalizeScalarsCheckBox_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->scalarScaleLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
}

void ShowFieldGlyphsDialog::setupVectorsTab()
{
  // Show Vectors
  addCheckableButtonManager(this->showVectorsCheckBox_, ShowFieldGlyphs::ShowVectors);
  // Display Type
  addComboBoxManager(this->vectorsDisplayTypeComboBox_, ShowFieldGlyphs::VectorsDisplayType);
  // Coloring
  addComboBoxManager(this->vectorsColorTypeComboBox_, ShowFieldGlyphs::VectorsColoring);
  // Coloring Data Input
  addComboBoxManager(this->vectorsColoringInputComboBox_, ShowFieldGlyphs::VectorsColoringDataInput);
  // Transparency
  addRadioButtonGroupManager({ this->vectorsTransparencyOffRButton_, this->vectorsUniformTransparencyRButton_}, ShowFieldGlyphs::VectorsTransparency);
  addDoubleSpinBoxManager(this->vectorsTransparencyDoubleSpinBox_, ShowFieldGlyphs::VectorsUniformTransparencyValue);
  // Transparency Data Input
  //  addComboBoxManager(this->vectorsTransparencyInputComboBox_, ShowFieldGlyphs::VectorsTransparencyDataInput);
  // Normalize
  addCheckableButtonManager(this->normalizeVectorsCheckBox_, ShowFieldGlyphs::NormalizeVectors);
  // Scale
  addDoubleSpinBoxManager(this->scaleVectorsDoubleSpinBox_, ShowFieldGlyphs::VectorsScale);
  // Resolution
  addSpinBoxManager(this->vectorsResolutionSpinBox_, ShowFieldGlyphs::VectorsResolution);
  // Threshold
  addCheckableButtonManager(this->renderVectorsBelowThresholdCheckBox_, ShowFieldGlyphs::RenderVectorsBelowThreshold);
  addDoubleSpinBoxManager(this->vectorsThresholdDoubleSpinBox_, ShowFieldGlyphs::VectorsThreshold);
  // Radius/Width
  addRadioButtonGroupManager({ this->secondaryVectorParameterUniformRButton_, this->secondaryVectorParameterPortRButton_}, ShowFieldGlyphs::SecondaryVectorParameterScalingType);
  addComboBoxManager(this->secondaryVectorParameterPortComboBox_, ShowFieldGlyphs::SecondaryVectorParameterDataInput);
  addDoubleSpinBoxManager(this->secondaryVectorParameterDoubleSpinBox_, ShowFieldGlyphs::SecondaryVectorParameterScale);
  // Arrow Settings
  addDoubleSpinBoxManager(this->arrowHeadRatioDoubleSpinBox_, ShowFieldGlyphs::ArrowHeadRatio);
  addCheckableButtonManager(this->bidirectionalVectorsCheckBox_, ShowFieldGlyphs::RenderBidirectionaly);
  addCheckableButtonManager(this->renderWithBasesCheckBox_, ShowFieldGlyphs::RenderBases);

  // Execute if any changed
  connectButtonToExecuteSignal(this->showVectorsCheckBox_);
  connectButtonToExecuteSignal(this->vectorsTransparencyOffRButton_);
  connectButtonToExecuteSignal(this->vectorsUniformTransparencyRButton_);
  connectButtonToExecuteSignal(this->secondaryVectorParameterUniformRButton_);
  connectButtonToExecuteSignal(this->secondaryVectorParameterPortRButton_);
  connectButtonToExecuteSignal(this->normalizeVectorsCheckBox_);
  connectButtonToExecuteSignal(this->renderVectorsBelowThresholdCheckBox_);
  connectButtonToExecuteSignal(this->bidirectionalVectorsCheckBox_);
  connectButtonToExecuteSignal(this->renderWithBasesCheckBox_);

  // Text Labels
  // this->vectorColorTypeLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->vectorColorInputLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->normalizeVectorsCheckBox_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->vectorScaleLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->secondaryVectorParameterScaleLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->bidirectionalVectorsCheckBox_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->springsMajorRadiusInputLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->springsMajorRadiusScaleLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->springsMinorRadiusInputLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->springsMinorRadiusScaleLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->springsPitchInputLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  // this->springsPitchScaleLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
}

void ShowFieldGlyphsDialog::emphasisSliderChanged(int val)
{
  superquadricEmphasisDoubleSpinBox_->setValue(val * 0.1);
}

void ShowFieldGlyphsDialog::emphasisSpinBoxChanged(double val)
{
  superquadricEmphasisSlider_->setValue(int(val * 10));
}

void ShowFieldGlyphsDialog::setupTensorsTab()
{
  // Show Tensors
  addCheckableButtonManager(this->showTensorsCheckBox_, ShowFieldGlyphs::ShowTensors);
  // Display Type
  addComboBoxManager(this->tensorsDisplayTypeComboBox_, ShowFieldGlyphs::TensorsDisplayType);
  // Coloring
  addComboBoxManager(this->tensorsColorTypeComboBox_, ShowFieldGlyphs::TensorsColoring);
  // Coloring Data Input
  addComboBoxManager(this->tensorsColoringInputComboBox_, ShowFieldGlyphs::TensorsColoringDataInput);
  // Transparency
  addRadioButtonGroupManager({ this->tensorsTransparencyOffRButton_, this->tensorsUniformTransparencyRButton_}, ShowFieldGlyphs::TensorsTransparency);
  addDoubleSpinBoxManager(this->tensorsTransparencyDoubleSpinBox_, ShowFieldGlyphs::TensorsUniformTransparencyValue);
  // Transparency Data Input
  //  addComboBoxManager(this->tensorsTransparencyInputComboBox_, ShowFieldGlyphs::TensorsTransparencyDataInput);
  // Normalize
  addCheckableButtonManager(this->normalizeTensorsCheckBox_, ShowFieldGlyphs::NormalizeTensors);
  // Scale
  addDoubleSpinBoxManager(this->scaleTensorsDoubleSpinBox_, ShowFieldGlyphs::TensorsScale);
  // Resolution
  addSpinBoxManager(this->tensorsResolutionSpinBox_, ShowFieldGlyphs::TensorsResolution);
  // Threshold
  addCheckableButtonManager(this->renderVectorsBelowThresholdCheckBox_, ShowFieldGlyphs::RenderTensorsBelowThreshold);
  addDoubleSpinBoxManager(this->tensorsThresholdDoubleSpinBox_, ShowFieldGlyphs::TensorsThreshold);
  addDoubleSpinBoxManager(this->superquadricEmphasisDoubleSpinBox_, ShowFieldGlyphs::SuperquadricEmphasis);
  connect(this->superquadricEmphasisSlider_, SIGNAL(valueChanged(int)), this, SLOT(emphasisSliderChanged(int)));
  connect(this->superquadricEmphasisDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(emphasisSpinBoxChanged(double)));

  connectButtonToExecuteSignal(this->showTensorsCheckBox_);
  connectButtonToExecuteSignal(this->tensorsTransparencyOffRButton_);
  connectButtonToExecuteSignal(this->tensorsUniformTransparencyRButton_);
  connectButtonToExecuteSignal(this->normalizeTensorsCheckBox_);
  connectButtonToExecuteSignal(this->renderTensorsBelowThresholdCheckBox_);

  // Text Labels
  this->superquadricEmphasisSlider_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  this->tensorColorTypeLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  this->tensorColorInputLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  this->normalizeTensorsCheckBox_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  this->tensorScaleLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
}
