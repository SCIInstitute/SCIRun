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

#include <Interface/Modules/Math/ConvertMatrixTypeDialog.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Math/ConvertMatrixType.h>


using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

ConvertMatrixTypeDialog::ConvertMatrixTypeDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  connect(PassThrough_, SIGNAL(clicked()), this, SLOT(push()));
  connect(ConvertToColumnMatrix_, SIGNAL(clicked()), this, SLOT(push()));
  connect(ConvertToDenseMatrix_, SIGNAL(clicked()), this, SLOT(push()));
  connect(ConvertToSparseRowMatrix_, SIGNAL(clicked()), this, SLOT(push()));
}

void ConvertMatrixTypeDialog::push()
{
  if (!pulling_)
  {
   state_->setValue(ConvertMatrixTypeAlgorithm::PassThrough(), PassThrough_->isChecked());
   state_->setValue(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix(), ConvertToColumnMatrix_->isChecked());
   state_->setValue(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix(), ConvertToDenseMatrix_->isChecked());
   state_->setValue(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix(), ConvertToSparseRowMatrix_->isChecked());
  }
}

void ConvertMatrixTypeDialog::pull()
{
  Pulling p(this);
  
  PassThrough_->setChecked(state_->getValue(ConvertMatrixTypeAlgorithm::PassThrough()).getBool()); 
  ConvertToColumnMatrix_->setChecked(state_->getValue(ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix()).getBool());
  ConvertToDenseMatrix_->setChecked(state_->getValue(ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix()).getBool());
  ConvertToSparseRowMatrix_->setChecked(state_->getValue(ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix()).getBool());
  
}

