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

#include <Interface/Modules/Math/CreateMatrixDialog.h>
#include <Core/Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <QFileDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Domain::Networks;

CreateMatrixDialog::CreateMatrixDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(to_QString(name));
  executionTimeHorizontalSlider_->setValue(moduleExecutionTime());
  
  connect(executeButton_, SIGNAL(clicked()), this, SIGNAL(executeButtonPressed()));
  //connect(saveFileButton_, SIGNAL(clicked()), this, SLOT(saveFile()));
  //connect(fileNameLineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(pushFileNameToState(const QString&)));
}

int CreateMatrixDialog::moduleExecutionTime()
{
  return 2000;
}

void CreateMatrixDialog::pull()
{
  std::cout << "TODO" << std::endl;
  //fileNameLineEdit_->setText(to_QString(state_->getValue(ReadMatrixAlgorithm::Filename).getString()));
}

//void CreateMatrixDialog::pushFileNameToState(const QString& str) 
//{
//  std::cout << "filename set on state object" << std::endl;
//  state_->setValue("FileName", str.toStdString());
//}
//
//void CreateMatrixDialog::saveFile()
//{
//  fileNameLineEdit_->setText(QFileDialog::getSaveFileName(this, "Save Matrix Text File", ".", "*.txt"));
//}