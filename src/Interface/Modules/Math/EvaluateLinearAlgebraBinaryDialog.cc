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

#include <Interface/Modules/Math/EvaluateLinearAlgebraBinaryDialog.h>
#include <Interface/Modules/Math/ui_EvaluateLinearAlgebraBinary.h> 
#include <Modules/Math/EvaluateLinearAlgebraBinary.h> 
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h> //TODO
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;

//typedef SCIRun::Modules::Math::EvaluateLinearAlgebraBinaryModule EvaluateLinearAlgebraBinaryModule; 

EvaluateLinearAlgebraBinaryDialog::EvaluateLinearAlgebraBinaryDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
	addLineEditManager(functionLineEdit_, Variables::FunctionString); 
  connect(addRadioButton_, SIGNAL(clicked()), this, SLOT(pushOperationToState()));
  connect(subtractRadioButton_, SIGNAL(clicked()), this, SLOT(pushOperationToState()));
  connect(multiplyRadioButton_, SIGNAL(clicked()), this, SLOT(pushOperationToState()));
	connect(functionRadioButton_,  SIGNAL(clicked()), this, SLOT(pushOperationToState()));
  //buttonBox->setVisible(false);
}

int EvaluateLinearAlgebraBinaryDialog::getSelectedOperator() const
{
  if (addRadioButton_->isChecked())
    return (int)EvaluateLinearAlgebraBinaryAlgorithm::ADD;
  if (subtractRadioButton_->isChecked())
    return (int)EvaluateLinearAlgebraBinaryAlgorithm::SUBTRACT;
  if (multiplyRadioButton_->isChecked())
    return (int)EvaluateLinearAlgebraBinaryAlgorithm::MULTIPLY;
	if (functionRadioButton_->isChecked())
		return (int)EvaluateLinearAlgebraBinaryAlgorithm::FUNCTION;
  else
    return -1;
}

void EvaluateLinearAlgebraBinaryDialog::setSelectedOperator(int op) 
{
  switch (op)
  {
  case (int)EvaluateLinearAlgebraBinaryAlgorithm::ADD:
    addRadioButton_->setChecked(true);
    break;
  case (int)EvaluateLinearAlgebraBinaryAlgorithm::SUBTRACT:
    subtractRadioButton_->setChecked(true);
    break;
  case(int)EvaluateLinearAlgebraBinaryAlgorithm::MULTIPLY:
    multiplyRadioButton_->setChecked(true);
    break;
	case(int)EvaluateLinearAlgebraBinaryAlgorithm::FUNCTION:
		functionRadioButton_->setChecked(true);
		break;
  }
}

void EvaluateLinearAlgebraBinaryDialog::pushOperationToState(const QString& str) 
{
  auto op = (EvaluateLinearAlgebraBinaryAlgorithm::Operator) getSelectedOperator();
	if(state_->getValue(Variables::Operator).toInt() != op)
			state_->setValue(Variables::Operator, op);

	std::string value = str.toStdString(); 
	if(state_->getValue(Variables::FunctionString).toString() != value)
	{
			state_->setValue(Variables::FunctionString, value); 
	}
}

void EvaluateLinearAlgebraBinaryDialog::pushOperationToState()
{
		pushOperationToState(functionLineEdit_->text()); 
}

void EvaluateLinearAlgebraBinaryDialog::pull()
{
  //TODO convert to new widget managers
  setSelectedOperator(state_->getValue(Variables::Operator).toInt());
	functionLineEdit_->setText(QString::fromStdString(state_->getValue(Variables::FunctionString).toString())); 
}