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
  ComboBoxSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QComboBox* comboBox,
    const GuiStringTranslationMap& stringMap) :
  WidgetSlotManager(state, dialog), stateKey_(stateKey), comboBox_(comboBox), stringMap_(stringMap)
  {
    fromLabelConverter_ = [this](const QString& qstr) { return stringMap_.left.at(qstr.toStdString()); };
    toLabelConverter_ = [this](const std::string& str) { return QString::fromStdString(stringMap_.right.at(str)); };
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
  GuiStringTranslationMap stringMap_;
};

void ModuleDialogGeneric::addComboBoxManager(QComboBox* comboBox, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<ComboBoxSlotManager>(state_, *this, stateKey, comboBox));
}

void ModuleDialogGeneric::addComboBoxManager(QComboBox* comboBox, const AlgorithmParameterName& stateKey, const GuiStringTranslationMap& stringMap)
{
  addWidgetSlotManager(boost::make_shared<ComboBoxSlotManager>(state_, *this, stateKey, comboBox, stringMap));
}

// ASSUMEs true state = comboBox index 1, false state = comboBox index 0.
class TwoChoiceBooleanComboBoxSlotManager : public WidgetSlotManager
{
public:
  TwoChoiceBooleanComboBoxSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QComboBox* comboBox) :
    WidgetSlotManager(state, dialog), stateKey_(stateKey), comboBox_(comboBox)
  {
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto value = state_->getValue(stateKey_).getBool();
    auto index = value ? 1 : 0;
    if (index != comboBox_->currentIndex())
    {
      LOG_DEBUG("In new version of pull code for combobox, boolean mode: " << index);
      comboBox_->setCurrentIndex(index);
    }
  }
  virtual void pushImpl() override
  {
    auto index = comboBox_->currentIndex();
    if (index != (state_->getValue(stateKey_).getBool() ? 1 : 0))
    {
      LOG_DEBUG("In new version of push code for combobox, boolean mode: " << index);
      state_->setValue(stateKey_, index == 1);
    }
  }
private:
  AlgorithmParameterName stateKey_;
  QComboBox* comboBox_;
};

void ModuleDialogGeneric::addTwoChoiceBooleanComboBoxManager(QComboBox* comboBox, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<TwoChoiceBooleanComboBoxSlotManager>(state_, *this, stateKey, comboBox));
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

void ModuleDialogGeneric::addTextEditManager(QTextEdit* textEdit, const AlgorithmParameterName& stateKey)
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

void ModuleDialogGeneric::addLineEditManager(QLineEdit* lineEdit, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<LineEditSlotManager>(state_, *this, stateKey, lineEdit));
}

class DoubleLineEditSlotManager : public WidgetSlotManager
{
public:
  DoubleLineEditSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QLineEdit* lineEdit) :
      WidgetSlotManager(state, dialog), stateKey_(stateKey), lineEdit_(lineEdit)
      {
        connect(lineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(push()));
      }
      virtual void pull() override
      {
        auto newValue = QString::number(state_->getValue(stateKey_).getDouble());
        if (newValue != lineEdit_->text())
        {
          lineEdit_->setText(newValue);
          LOG_DEBUG("In new version of pull code for DoubleLineEdit: " << newValue.toStdString());
        }
      }
      virtual void pushImpl() override
      {
        LOG_DEBUG("In new version of push code for LineEdit: " << lineEdit_->text().toStdString());
        state_->setValue(stateKey_, boost::lexical_cast<double>(lineEdit_->text().toStdString()));
      }
private:
  AlgorithmParameterName stateKey_;
  QLineEdit* lineEdit_;
};

void ModuleDialogGeneric::addDoubleLineEditManager(QLineEdit* lineEdit, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<DoubleLineEditSlotManager>(state_, *this, stateKey, lineEdit));
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

void ModuleDialogGeneric::addSpinBoxManager(QSpinBox* spinBox, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<SpinBoxSlotManager>(state_, *this, stateKey, spinBox));
}

class DoubleSpinBoxSlotManager : public WidgetSlotManager
{
public:
  DoubleSpinBoxSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QDoubleSpinBox* spinBox) :
    WidgetSlotManager(state, dialog), stateKey_(stateKey), spinBox_(spinBox)
  {
    connect(spinBox_, SIGNAL(valueChanged(double)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto newValue = state_->getValue(stateKey_).getDouble();
    if (newValue != spinBox_->value())
    {
      spinBox_->setValue(newValue);
      LOG_DEBUG("In new version of pull code for DoubleSpinBox: " << newValue);
    }
  }
  virtual void pushImpl() override
  {
    LOG_DEBUG("In new version of push code for DoubleSpinBox: " << spinBox_->value());
    state_->setValue(stateKey_, spinBox_->value());
  }
private:
  AlgorithmParameterName stateKey_;
  QDoubleSpinBox* spinBox_;
};

void ModuleDialogGeneric::addDoubleSpinBoxManager(QDoubleSpinBox* spinBox, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<DoubleSpinBoxSlotManager>(state_, *this, stateKey, spinBox));
}

class CheckBoxSlotManager : public WidgetSlotManager
{
public:
  CheckBoxSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QCheckBox* checkBox) :
    WidgetSlotManager(state, dialog), stateKey_(stateKey), checkBox_(checkBox)
  {
    connect(checkBox_, SIGNAL(stateChanged(int)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    bool newValue = state_->getValue(stateKey_).getBool();
    if (newValue != checkBox_->isChecked())
    {
      LOG_DEBUG("In new version of pull code for CheckBox: " << newValue);
      checkBox_->setChecked(newValue);
    }
  }
  virtual void pushImpl() override
  {
    LOG_DEBUG("In new version of push code for CheckBox: " << checkBox_->isChecked());
    state_->setValue(stateKey_, checkBox_->isChecked());
  }
private:
  AlgorithmParameterName stateKey_;
  QCheckBox* checkBox_;
};

void ModuleDialogGeneric::addCheckBoxManager(QCheckBox* checkBox, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<CheckBoxSlotManager>(state_, *this, stateKey, checkBox));
}

class CheckableButtonSlotManager : public WidgetSlotManager
{
public:
  CheckableButtonSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QAbstractButton* checkable) :
      WidgetSlotManager(state, dialog), stateKey_(stateKey), checkable_(checkable)
      {
        connect(checkable_, SIGNAL(clicked()), this, SLOT(push()));
      }
      virtual void pull() override
      {
        bool newValue = state_->getValue(stateKey_).getBool();
        if (newValue != checkable_->isChecked())
        {
          LOG_DEBUG("In new version of pull code for checkable QAbstractButton: " << newValue);
          checkable_->setChecked(newValue);
        }
      }
      virtual void pushImpl() override
      {
        LOG_DEBUG("In new version of push code for checkable QAbstractButton: " << checkable_->isChecked());
        state_->setValue(stateKey_, checkable_->isChecked());
      }
private:
  AlgorithmParameterName stateKey_;
  QAbstractButton* checkable_;
};

void ModuleDialogGeneric::addCheckableButtonManager(QAbstractButton* checkable, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<CheckableButtonSlotManager>(state_, *this, stateKey, checkable));
}
