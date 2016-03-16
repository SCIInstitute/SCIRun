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

#include <Interface/Modules/Visualization/ShowColorMapDialog.h>
#include <Modules/Visualization/ShowColorMapModule.h>
#include <Dataflow/Network/ModuleStateInterface.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Visualization;

ShowColorMapDialog::ShowColorMapDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
	addRadioButtonGroupManager({ leftRadioButton_, bottomRadioButton_ }, ShowColorMapModule::DisplaySide);
	addRadioButtonGroupManager({ firstHalfRadioButton_, fullRadioButton_, secondHalfRadioButton_ }, ShowColorMapModule::DisplayLength);
	//addRadioButtonGroupManager({ TRadioButton_, SRadioButton_, MRadioButton_, LRadioButton_, XLRadioButton_ }, ShowColorMapModule::TextSize);
  addSpinBoxManager(textSizeSpinner_, ShowColorMapModule::TextSize);
	addSpinBoxManager(ticksSpinner_, ShowColorMapModule::Labels);
	addDoubleSpinBoxManager(scaleSpinner_, ShowColorMapModule::Scale);
	addLineEditManager(unitsText_, ShowColorMapModule::Units);
	addSpinBoxManager(sigDigitsSpinner_, ShowColorMapModule::SignificantDigits);

	addSpinBoxManager(xTranslationSpin_, ShowColorMapModule::XTranslation);
	addSpinBoxManager(yTranslationSpin_, ShowColorMapModule::YTranslation);

    connectButtonToExecuteSignal(leftRadioButton_);
    connectButtonToExecuteSignal(bottomRadioButton_);
    connectButtonToExecuteSignal(firstHalfRadioButton_);
    connectButtonToExecuteSignal(fullRadioButton_);
    connectButtonToExecuteSignal(secondHalfRadioButton_);
    connectButtonToExecuteSignal(textColorPushButton_);
    //connectButtonToExecuteSignal(TRadioButton_);
    //connectButtonToExecuteSignal(SRadioButton_);
    //connectButtonToExecuteSignal(MRadioButton_);
    //connectButtonToExecuteSignal(LRadioButton_);
    //connectButtonToExecuteSignal(XLRadioButton_);

    connect(textColorPushButton_,SIGNAL(clicked()),this,SLOT(getColor()));
    text_color_ = QColor(255,255,255,255);
	addDoubleSpinBoxManager(&r_, ShowColorMapModule::TextRed);
	addDoubleSpinBoxManager(&g_, ShowColorMapModule::TextGreen);
	addDoubleSpinBoxManager(&b_, ShowColorMapModule::TextBlue);
    r_.setValue(1.);
    g_.setValue(1.);
    b_.setValue(1.);
    sigDigitsSpinner_->setValue(2);
    textSizeSpinner_->setValue(8);
    ticksSpinner_->setValue(10);
    scaleSpinner_->setValue(1.);
    //TRadioButton_->setChecked(false);
    //SRadioButton_->setChecked(false);
    //MRadioButton_->setChecked(true);
    //LRadioButton_->setChecked(false);
    //XLRadioButton_->setChecked(false);
    xTranslationSpin_->setValue(0);
    yTranslationSpin_->setValue(0);

	/*********************************************************************************
	//TODO Disabling text-related widgets until font fixed for Windows
	***********************************************************************************/
/*	unitsText_->setEnabled(false);
	TRadioButton_->setEnabled(false);
	SRadioButton_->setEnabled(false);
	MRadioButton_->setEnabled(false);
	LRadioButton_->setEnabled(false);
	XLRadioButton_->setEnabled(false);
	sigDigitsSpinner_->setEnabled(false);
	ticksSpinner_->setEnabled(false);
	scaleSpinner_->setEnabled(false);
	textColorPushButton_->setEnabled(false);*/
	/*********************************************************************************
	//TODO End Disabling
	***********************************************************************************/

  createExecuteInteractivelyToggleAction();
}

void ShowColorMapDialog::pullSpecial()
{
  r_.setValue(state_->getValue(ShowColorMapModule::TextRed).toDouble());
  g_.setValue(state_->getValue(ShowColorMapModule::TextGreen).toDouble());
  b_.setValue(state_->getValue(ShowColorMapModule::TextBlue).toDouble());
}

void ShowColorMapDialog::getColor()
{
  text_color_ = QColorDialog::getColor(text_color_);
    std::stringstream ss;
    ss << "background-color: rgb(" << text_color_.red() << ", " <<
            text_color_.green() << ", " << text_color_.blue() << ");";
  textColorDisplayLabel_->setStyleSheet(QString::fromStdString(ss.str()));
  r_.setValue(text_color_.redF());
  g_.setValue(text_color_.greenF());
  b_.setValue(text_color_.blueF());
}
