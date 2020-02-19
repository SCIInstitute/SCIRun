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


#include <Interface/Modules/Visualization/ShowColorMapDialog.h>
#include <Modules/Visualization/ShowColorMapModule.h>
#include <Dataflow/Network/ModuleStateInterface.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Visualization;

using SCM = SCIRun::Modules::Visualization::ShowColorMap;

ShowColorMapDialog::ShowColorMapDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
	addRadioButtonGroupManager({ leftRadioButton_, bottomRadioButton_ }, SCM::DisplaySide);
	addRadioButtonGroupManager({ firstHalfRadioButton_, fullRadioButton_, secondHalfRadioButton_ }, SCM::DisplayLength);
  addSpinBoxManager(textSizeSpinner_, SCM::TextSize);
	addSpinBoxManager(ticksSpinner_, SCM::Labels);
	addDoubleSpinBoxManager(scaleSpinner_, SCM::Scale);
	addLineEditManager(unitsText_, SCM::Units);
	addSpinBoxManager(sigDigitsSpinner_, SCM::SignificantDigits);

	addSpinBoxManager(xTranslationSpin_, SCM::XTranslation);
	addSpinBoxManager(yTranslationSpin_, SCM::YTranslation);
  addLineEditManager(colorMapNameLineEdit_, SCM::ColorMapName);

  connect(textColorPushButton_, SIGNAL(clicked()), this, SLOT(getColor()));
  connectButtonsToExecuteSignal({ leftRadioButton_, bottomRadioButton_, firstHalfRadioButton_, fullRadioButton_, secondHalfRadioButton_, textColorPushButton_ } );

  addDoubleSpinBoxManager(&r_, SCM::TextRed);
  addDoubleSpinBoxManager(&g_, SCM::TextGreen);
  addDoubleSpinBoxManager(&b_, SCM::TextBlue);

  if (state_->getValue(SCM::TextRed).toDouble() < 0)
  {
    text_color_ = QColor(255, 255, 255, 255);
    r_.setValue(1.);
    g_.setValue(1.);
    b_.setValue(1.);
    sigDigitsSpinner_->setValue(2);
    textSizeSpinner_->setValue(8);
    ticksSpinner_->setValue(10);
    scaleSpinner_->setValue(1.);
    xTranslationSpin_->setValue(0);
    yTranslationSpin_->setValue(0);
  }
  else
  {
    text_color_ = QColor(state_->getValue(SCM::TextRed).toDouble() * 255,
      state_->getValue(SCM::TextGreen).toDouble() * 255,
      state_->getValue(SCM::TextBlue).toDouble() * 255);
    std::stringstream ss;
    ss << "background-color: rgb(" << text_color_.red() << ", " <<
      text_color_.green() << ", " << text_color_.blue() << ");";
    textColorDisplayLabel_->setStyleSheet(QString::fromStdString(ss.str()));
  }

  createExecuteInteractivelyToggleAction();
}

void ShowColorMapDialog::pullSpecial()
{
  r_.setValue(state_->getValue(SCM::TextRed).toDouble());
  g_.setValue(state_->getValue(SCM::TextGreen).toDouble());
  b_.setValue(state_->getValue(SCM::TextBlue).toDouble());
}

void ShowColorMapDialog::getColor()
{
  text_color_ = QColorDialog::getColor(text_color_, this, "Choose text color");
  std::stringstream ss;
  ss << "background-color: rgb(" << text_color_.red() << ", " <<
    text_color_.green() << ", " << text_color_.blue() << ");";
  textColorDisplayLabel_->setStyleSheet(QString::fromStdString(ss.str()));
  r_.setValue(text_color_.redF());
  g_.setValue(text_color_.greenF());
  b_.setValue(text_color_.blueF());
}
