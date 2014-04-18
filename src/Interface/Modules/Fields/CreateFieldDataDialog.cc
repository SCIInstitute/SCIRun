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

#include <Interface/Modules/Fields/CreateFieldDataDialog.h>
#include <Modules/Legacy/Fields/CreateFieldData.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/Logging/Log.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
typedef SCIRun::Modules::Fields::CreateFieldData CreateFieldDataModule;

class TextEditSlotManager : public WidgetSlotManager
{
public:
  TextEditSlotManager(ModuleStateHandle state, const AlgorithmParameterName& stateKey, QTextEdit* textEdit) : 
      WidgetSlotManager(state), stateKey_(stateKey), textEdit_(textEdit) 
      {
      }
  virtual void pull() override
  {
    auto newValue = QString::fromStdString(state_->getValue(stateKey_).getString());
    LOG_DEBUG("In new version of pull code: " << newValue.toStdString());
    if (newValue != textEdit_->toPlainText())
      textEdit_->setPlainText(newValue);
  }
  virtual void push() override
  {
    //TODO
  }
private:
  AlgorithmParameterName stateKey_;
  QTextEdit* textEdit_;
};

CreateFieldDataDialog::CreateFieldDataDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  connect(functionTextEdit_, SIGNAL(textChanged()), this, SLOT(push()));
  connect(fieldOutputDataComboBox_, SIGNAL(activated(const QString&)), this, SLOT(push()));
  connect(fieldOutputBasisComboBox_, SIGNAL(activated(const QString&)), this, SLOT(push()));

  addWidgetSlotManager(boost::make_shared<TextEditSlotManager>(state_, CreateFieldDataModule::FunctionString, functionTextEdit_));
}

void CreateFieldDataDialog::push()
{
  if (!pulling_)
  {
    state_->setValue(CreateFieldDataModule::FunctionString, functionTextEdit_->toPlainText().toStdString());
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    state_->setValue(CreateFieldDataModule::FormatString,
    state_->setValue(CreateFieldDataModule::BasisString,
#endif
  }
}

void CreateFieldDataDialog::pull()
{
  pull_newVersionToReplaceOld();
  //Pulling p(this);
  //auto newValue = QString::fromStdString(state_->getValue(CreateFieldDataModule::FunctionString).getString());
  //if (newValue != functionTextEdit_->toPlainText())
  //  functionTextEdit_->setPlainText(newValue);
  //
  //hook up format, basis later.
}
