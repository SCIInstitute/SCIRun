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

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Visualization;

ShowUncertaintyGlyphsDialog::ShowUncertaintyGlyphsDialog(const std::string& name,
                                                         ModuleStateHandle state,
                                                         QWidget* parent)
: ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  setupTensorsTab();
  addLineEditManager(lineEdit, ShowUncertaintyGlyphs::FieldName);
}

void ShowUncertaintyGlyphsDialog::setupTensorsTab()
{
  addCheckableButtonManager(this->showTensorsCheckBox_, ShowUncertaintyGlyphs::ShowTensors);
  addDoubleSpinBoxManager(this->tensorsTransparencyDoubleSpinBox_, ShowUncertaintyGlyphs::TensorsUniformTransparencyValue);
  addDoubleSpinBoxManager(this->scaleTensorsDoubleSpinBox_, ShowUncertaintyGlyphs::TensorsScale);
  addSpinBoxManager(this->tensorsResolutionSpinBox_, ShowUncertaintyGlyphs::TensorsResolution);
  addDoubleSpinBoxManager(this->superquadricEmphasisDoubleSpinBox_, ShowUncertaintyGlyphs::SuperquadricEmphasis);
  connect(this->superquadricEmphasisSlider_, SIGNAL(valueChanged(int)), this, SLOT(emphasisSliderChanged(int)));
  connect(this->superquadricEmphasisDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(emphasisSpinBoxChanged(double)));

  connectButtonToExecuteSignal(this->showTensorsCheckBox_);
  connectButtonToExecuteSignal(this->tensorsTransparencyOffRButton_);
  connectButtonToExecuteSignal(this->tensorsUniformTransparencyRButton_);

  // Text Labels
  this->superquadricEmphasisSlider_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
  this->tensorScaleLabel_->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
}

void ShowUncertaintyGlyphsDialog::emphasisSliderChanged(int val)
{
  superquadricEmphasisDoubleSpinBox_->setValue(val * 0.1);
}

void ShowUncertaintyGlyphsDialog::emphasisSpinBoxChanged(double val)
{
  superquadricEmphasisSlider_->setValue(int(val * 10));
}
