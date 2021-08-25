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


#include <Interface/Modules/Visualization/ShowUncertaintyGlyphsDialog.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Modules/Visualization/ShowUncertaintyGlyphs.h>
// #include <Core/Algorithms/Visualization/ShowUncertaintyGlyphsAlgo.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Visualization;

ShowUncertaintyGlyphsDialog::ShowUncertaintyGlyphsDialog(const std::string& name,
                                                         ModuleStateHandle state,
                                                         QWidget* parent)
: ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  // setWindowTitle(QString::fromStdString(name));
  // fixSize();
}

// void ShowUncertaintyGlyphsDialog::pull()
// {
  // pull the code from the module and set in the dialog.
  // make changes necessary.
  // pull_newVersionToReplaceOld();
// }

void ShowUncertaintyGlyphsDialog::setupTensorsTab()
{
  // Show Tensors
  addCheckableButtonManager(this->showTensorsCheckBox_, ShowUncertaintyGlyphs::ShowTensors);
  // Display Type
  // addComboBoxManager(this->tensorsDisplayTypeComboBox_, ShowUncertaintyGlyphs::TensorsDisplayType);
  // Coloring
  addComboBoxManager(this->tensorsColorTypeComboBox_, ShowUncertaintyGlyphs::TensorsColoring);
  // Coloring Data Input
  addComboBoxManager(this->tensorsColoringInputComboBox_, ShowUncertaintyGlyphs::TensorsColoringDataInput);
  // Transparency
  addRadioButtonGroupManager({ this->tensorsTransparencyOffRButton_, this->tensorsUniformTransparencyRButton_}, ShowUncertaintyGlyphs::TensorsTransparency);
  addDoubleSpinBoxManager(this->tensorsTransparencyDoubleSpinBox_, ShowUncertaintyGlyphs::TensorsUniformTransparencyValue);
  // Transparency Data Input
  //  addComboBoxManager(this->tensorsTransparencyInputComboBox_, ShowFieldGlyphs::TensorsTransparencyDataInput);
  // Normalize
  addCheckableButtonManager(this->normalizeTensorsCheckBox_, ShowUncertaintyGlyphs::NormalizeTensors);
  // Scale
  addDoubleSpinBoxManager(this->scaleTensorsDoubleSpinBox_, ShowUncertaintyGlyphs::TensorsScale);
  // Resolution
  addSpinBoxManager(this->tensorsResolutionSpinBox_, ShowUncertaintyGlyphs::TensorsResolution);
  // Threshold
  addCheckableButtonManager(this->renderVectorsBelowThresholdCheckBox_, ShowUncertaintyGlyphs::RenderTensorsBelowThreshold);
  addDoubleSpinBoxManager(this->tensorsThresholdDoubleSpinBox_, ShowUncertaintyGlyphs::TensorsThreshold);
  addDoubleSpinBoxManager(this->superquadricEmphasisDoubleSpinBox_, ShowUncertaintyGlyphs::SuperquadricEmphasis);
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
