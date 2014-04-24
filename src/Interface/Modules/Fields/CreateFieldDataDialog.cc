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
  TextEditSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QTextEdit* textEdit) : 
    WidgetSlotManager(state, dialog), stateKey_(stateKey), textEdit_(textEdit) 
  {
    connect(textEdit, SIGNAL(textChanged()), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto newValue = QString::fromStdString(state_->getValue(stateKey_).getString());
    if (newValue != textEdit_->toPlainText())
    {
      textEdit_->setPlainText(newValue);
      LOG_DEBUG("In new version of pull code: " << newValue.toStdString());
    }
  }
  virtual void pushImpl() override
  {
    LOG_DEBUG("In new version of push code: " << textEdit_->toPlainText().toStdString());
    state_->setValue(stateKey_, textEdit_->toPlainText().toStdString());
  }
private:
  AlgorithmParameterName stateKey_;
  QTextEdit* textEdit_;
};

class ComboBoxSlotManager : public WidgetSlotManager
{
public:
  typedef boost::function<std::string(const QString&)> FromQStringConverter;
  typedef boost::function<QString(const std::string&)> ToQStringConverter;
  ComboBoxSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QComboBox* comboBox, 
    FromQStringConverter fromLabelConverter = boost::bind(&QString::toStdString, _1),
    ToQStringConverter toLabelConverter = &QString::fromStdString) : 
      WidgetSlotManager(state, dialog), stateKey_(stateKey), comboBox_(comboBox), fromLabelConverter_(fromLabelConverter), toLabelConverter_(toLabelConverter)
      {
        connect(comboBox, SIGNAL(activated(const QString&)), this, SLOT(push()));
      }
      virtual void pull() override
      {
        auto value = state_->getValue(stateKey_).getString();
        auto qstring = toLabelConverter_(value);
        if (qstring != comboBox_->currentText())
        {
          LOG_DEBUG("In new version of pull code for combobox: " << value);
          comboBox_->setCurrentIndex(comboBox_->findText(qstring));
        }
      }
      virtual void pushImpl() override
      {
        auto label = fromLabelConverter_(comboBox_->currentText());
        if (label != state_->getValue(stateKey_).getString())
        {
          LOG_DEBUG("In new version of push code for combobox: " << label);
          state_->setValue(stateKey_, label);
        }
      }
private:
  AlgorithmParameterName stateKey_;
  QComboBox* comboBox_;
  FromQStringConverter fromLabelConverter_;
  ToQStringConverter toLabelConverter_;
};

CreateFieldDataDialog::CreateFieldDataDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  addWidgetSlotManager(boost::make_shared<TextEditSlotManager>(state_, *this, CreateFieldDataModule::FunctionString, functionTextEdit_));
  addWidgetSlotManager(boost::make_shared<ComboBoxSlotManager>(state_, *this, CreateFieldDataModule::FormatString, fieldOutputDataComboBox_));
  addWidgetSlotManager(boost::make_shared<ComboBoxSlotManager>(state_, *this, CreateFieldDataModule::BasisString, fieldOutputBasisComboBox_));
}

void CreateFieldDataDialog::pull()
{
  pull_newVersionToReplaceOld();
}
