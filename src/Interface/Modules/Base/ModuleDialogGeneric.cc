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

#include <Dataflow/Network/ModuleStateInterface.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Core/Logging/Log.h>
#include <boost/foreach.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

ModuleDialogGeneric::ModuleDialogGeneric(SCIRun::Dataflow::Networks::ModuleStateHandle state, QWidget* parent) : QDialog(parent),
  state_(state),
  pulling_(false)
{
  setModal(false);

  if (state_)
  {
    //TODO: replace with pull_newVersion
    state_->connect_state_changed([this]() { pull(); });
  }
}

ModuleDialogGeneric::~ModuleDialogGeneric()
{
}

void ModuleDialogGeneric::fixSize()
{
  if (minimumWidth() > 0 && minimumHeight() > 0)
  {
    setFixedSize(minimumWidth(), minimumHeight());
  }
}

void ModuleDialogGeneric::addWidgetSlotManager(WidgetSlotManagerPtr ptr) 
{ 
  slotManagers_.push_back(ptr); 
}

void ModuleDialogGeneric::pull_newVersionToReplaceOld()
{
  Pulling p(this);
  BOOST_FOREACH(WidgetSlotManagerPtr wsm, slotManagers_)
    wsm->pull();
}

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

void ModuleDialogGeneric::addComboBoxManager(const AlgorithmParameterName& stateKey, QComboBox* comboBox)
{
  addWidgetSlotManager(boost::make_shared<ComboBoxSlotManager>(state_, *this, stateKey, comboBox));
}

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
      LOG_DEBUG("In new version of pull code for TextEdit: " << newValue.toStdString());
    }
  }
  virtual void pushImpl() override
  {
    LOG_DEBUG("In new version of push code for TextEdit: " << textEdit_->toPlainText().toStdString());
    state_->setValue(stateKey_, textEdit_->toPlainText().toStdString());
  }
private:
  AlgorithmParameterName stateKey_;
  QTextEdit* textEdit_;
};

void ModuleDialogGeneric::addTextEditManager(const AlgorithmParameterName& stateKey, QTextEdit* textEdit)
{
  addWidgetSlotManager(boost::make_shared<TextEditSlotManager>(state_, *this, stateKey, textEdit));
}

class LineEditSlotManager : public WidgetSlotManager
{
public:
  LineEditSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QLineEdit* lineEdit) : 
      WidgetSlotManager(state, dialog), stateKey_(stateKey), lineEdit_(lineEdit) 
  {
    connect(lineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto newValue = QString::fromStdString(state_->getValue(stateKey_).getString());
    if (newValue != lineEdit_->text())
    {
      lineEdit_->setText(newValue);
      LOG_DEBUG("In new version of pull code for LineEdit: " << newValue.toStdString());
    }
  }
  virtual void pushImpl() override
  {
    LOG_DEBUG("In new version of push code for LineEdit: " << lineEdit_->text().toStdString());
    state_->setValue(stateKey_, lineEdit_->text().toStdString());
  }
private:
  AlgorithmParameterName stateKey_;
  QLineEdit* lineEdit_;
};

void ModuleDialogGeneric::addLineEditManager(const AlgorithmParameterName& stateKey, QLineEdit* lineEdit)
{
  addWidgetSlotManager(boost::make_shared<LineEditSlotManager>(state_, *this, stateKey, lineEdit));
}

class SpinBoxSlotManager : public WidgetSlotManager
{
public:
  SpinBoxSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QSpinBox* spinBox) : 
    WidgetSlotManager(state, dialog), stateKey_(stateKey), spinBox_(spinBox) 
  {
    connect(spinBox_, SIGNAL(valueChanged(int)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto newValue = state_->getValue(stateKey_).getInt();
    if (newValue != spinBox_->value())
    {
      spinBox_->setValue(newValue);
      LOG_DEBUG("In new version of pull code for SpinBox: " << newValue);
    }
  }
  virtual void pushImpl() override
  {
    LOG_DEBUG("In new version of push code for SpinBox: " << spinBox_->value());
    state_->setValue(stateKey_, spinBox_->value());
  }
private:
  AlgorithmParameterName stateKey_;
  QSpinBox* spinBox_;
};

void ModuleDialogGeneric::addSpinBoxManager(const AlgorithmParameterName& stateKey, QSpinBox* spinBox)
{
  addWidgetSlotManager(boost::make_shared<SpinBoxSlotManager>(state_, *this, stateKey, spinBox));
}