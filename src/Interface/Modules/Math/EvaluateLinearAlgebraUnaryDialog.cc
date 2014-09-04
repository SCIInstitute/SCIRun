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

#include <Interface/Modules/Math/EvaluateLinearAlgebraUnaryDialog.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraUnaryAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <QtGui>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;

EvaluateLinearAlgebraUnaryDialog::EvaluateLinearAlgebraUnaryDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  connect(transposeRadioButton_, SIGNAL(clicked()), this, SLOT(pushOperationToState()));
  connect(negateRadioButton_, SIGNAL(clicked()), this, SLOT(pushOperationToState()));
  connect(scalarMultiplyRadioButton_, SIGNAL(clicked()), this, SLOT(pushOperationToState()));
  //TODO: here is where to start on standardizing module dialog buttons.
  //TODO: need this connection 
  connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(pushOperationToState()));
  connect(scalarLineEdit_, SIGNAL(editingFinished()), this, SLOT(pushOperationToState()));
}

int EvaluateLinearAlgebraUnaryDialog::getSelectedOperator() const
{
  if (transposeRadioButton_->isChecked())
    return (int)EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE;
  if (negateRadioButton_->isChecked())
    return (int)EvaluateLinearAlgebraUnaryAlgorithm::NEGATE;
  if (scalarMultiplyRadioButton_->isChecked())
    return (int)EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY;
  else
    return -1;
}

void EvaluateLinearAlgebraUnaryDialog::setSelectedOperator(int op) 
{
  switch (op)
  {
  case (int)EvaluateLinearAlgebraUnaryAlgorithm::TRANSPOSE:
    transposeRadioButton_->setChecked(true);
    break;
  case (int)EvaluateLinearAlgebraUnaryAlgorithm::NEGATE:
    negateRadioButton_->setChecked(true); 
    break;
  case (int)EvaluateLinearAlgebraUnaryAlgorithm::SCALAR_MULTIPLY:
    scalarMultiplyRadioButton_->setChecked(true);
    break;
  }
}

void EvaluateLinearAlgebraUnaryDialog::pushOperationToState(const QString& str) 
{
  EvaluateLinearAlgebraUnaryAlgorithm::Operator op = (EvaluateLinearAlgebraUnaryAlgorithm::Operator) getSelectedOperator();

  if (state_->getValue(Variables::Operator).toInt() != op)
    state_->setValue(Variables::Operator, op);
  
  bool ok;
  double value = str.toDouble(&ok);
  if (ok && state_->getValue(Variables::ScalarValue).toDouble() != value)
  {
    state_->setValue(Variables::ScalarValue, value);
  }
}

void EvaluateLinearAlgebraUnaryDialog::pushOperationToState()
{
  pushOperationToState(scalarLineEdit_->text());
}

void EvaluateLinearAlgebraUnaryDialog::pull()
{
  //TODO convert to new widget managers
  setSelectedOperator(state_->getValue(Variables::Operator).toInt());
  scalarLineEdit_->setText(QString::number(state_->getValue(Variables::ScalarValue).toDouble()));
}