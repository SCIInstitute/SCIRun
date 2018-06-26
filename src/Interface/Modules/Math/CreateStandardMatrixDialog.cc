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

#include <Interface/Modules/Math/CreateStandardMatrixDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Math/CreateStandardMatrixAlgo.h>
#include <boost/shared_ptr.hpp>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <QtGui>

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Math;


namespace SCIRun {
    namespace Gui{
        class CreateStandardMatrixDialogImpl
        {
        public:
            CreateStandardMatrixDialogImpl()
            {
                value_.insert(StringPair("Matrix with Zeros","Zero"));
                value_.insert(StringPair("Matrix with Ones","One"));
                value_.insert(StringPair("Matrix with NaNs","NaN"));
                value_.insert(StringPair("Identity Matrix","Identity"));
                value_.insert(StringPair("Matrix with Series","Series"));
    
            }
            
            
            GuiStringTranslationMap value_;
        };
    }
}

CreateStandardMatrixDialog::CreateStandardMatrixDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent),
   impl_(new CreateStandardMatrixDialogImpl)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

    addComboBoxManager(matrixType_, Parameters::MatrixType,impl_->value_);
    addSpinBoxManager(noOfRows_, Parameters::Rows);
    addSpinBoxManager(noOfColumns_, Parameters::Columns);
    addSpinBoxManager(sizeOfStep_,Parameters::Size);
    addSpinBoxManager(startPointer_,Parameters::StartPointer);

    
    connect(matrixType_, SIGNAL(activated(const QString&)), this, SLOT(enableWidgets(const QString&)));
  }

void CreateStandardMatrixDialog::enableWidgets(const QString& mode)
{
    sizeOfStep_->setReadOnly(mode!="Matrix with Series" && mode!="Series");
    startPointer_->setReadOnly(mode!="Matrix with Series" && mode!="Series");
    }

void CreateStandardMatrixDialog::pullSpecial()
{
    enableWidgets(QString::fromStdString(state_->getValue(Parameters::MatrixType).toString()));
}
