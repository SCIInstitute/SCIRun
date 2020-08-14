/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

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
#include <Interface/Modules/Base/ModuleButtonBar.h>
#include <Core/Logging/Log.h>
#include <Core/Datatypes/Color.h>
#include <Core/Utils/Exception.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Datatypes;

ExecutionDisablingServiceFunction ModuleDialogGeneric::disablerAdd_;
ExecutionDisablingServiceFunction ModuleDialogGeneric::disablerRemove_;
std::set<ModuleDialogGeneric*> ModuleDialogGeneric::instances_;

ModuleDialogGeneric::ModuleDialogGeneric(ModuleStateHandle state, QWidget* parent) : QDialog(parent),
  state_(state),
  pulling_(false),
  collapsed_(false)
{
  setModal(false);
  setAttribute(Qt::WA_MacAlwaysShowToolWindow, true);

  if (state_)
  {
    LOG_TRACE(("ModuleDialogGeneric connecting to state"));
    stateConnection_ = state_->connectStateChanged([this]() { pullSignal(); });
  }
  connect(this, SIGNAL(pullSignal()), this, SLOT(pull()));
  createExecuteAction();
  createExecuteDownstreamAction();
  createShrinkAction();
  connectStateChangeToExecute(); //TODO: make this a module state variable if a module wants it saved
  instances_.insert(this);
}

ModuleDialogGeneric::~ModuleDialogGeneric()
{
  instances_.erase(this);
  if (disablerAdd_ && disablerRemove_)
  {
    std::for_each(needToRemoveFromDisabler_.begin(), needToRemoveFromDisabler_.end(), disablerRemove_);
  }
}

void ModuleDialogGeneric::setDockable(QDockWidget* dock)
{
  dock_ = dock;
}

void ModuleDialogGeneric::setupButtonBar()
{
  buttonBox_ = new ModuleButtonBar(this);
  dock_->setTitleBarWidget(buttonBox_);
  if (executeInteractivelyToggleAction_)
  {
    connect(buttonBox_->executeInteractivelyCheckBox_, SIGNAL(toggled(bool)), this, SLOT(executeInteractivelyToggled(bool)));
    buttonBox_->executeInteractivelyCheckBox_->setChecked(executeInteractivelyToggleAction_->isChecked());
  }
  else
  {
    buttonBox_->executeInteractivelyCheckBox_->setVisible(false);
  }

  if (forceAlwaysExecuteToggleAction_)
  {
    connect(buttonBox_->forceAlwaysExecuteCheckBox_, SIGNAL(toggled(bool)), this, SLOT(forceAlwaysExecuteToggled(bool)));
    buttonBox_->forceAlwaysExecuteCheckBox_->setChecked(forceAlwaysExecuteToggleAction_->isChecked());
  }
  else
  {
    buttonBox_->forceAlwaysExecuteCheckBox_->setVisible(false);
  }
}

void ModuleDialogGeneric::connectButtonToExecuteSignal(QAbstractButton* button)
{
  connect(button, SIGNAL(clicked()), this, SIGNAL(executeFromStateChangeTriggered()));
  if (disablerAdd_ && disablerRemove_)
  {
    disablerAdd_(button);
    needToRemoveFromDisabler_.push_back(button);
  }
}

void ModuleDialogGeneric::connectButtonsToExecuteSignal(std::initializer_list<QAbstractButton*> buttons)
{
  for (auto& button : buttons)
    connectButtonToExecuteSignal(button);
}


void ModuleDialogGeneric::connectComboToExecuteSignal(QComboBox* box)
{
  connect(box, SIGNAL(activated(const QString&)), this, SIGNAL(executeFromStateChangeTriggered()));
  if (disablerAdd_ && disablerRemove_)
  {
    disablerAdd_(box);
    needToRemoveFromDisabler_.push_back(box);
  }
}

void ModuleDialogGeneric::connectSpinBoxToExecuteSignal(QSpinBox* box)
{
  connect(box, SIGNAL(valueChanged(int)), this, SIGNAL(executeFromStateChangeTriggered()));
  if (disablerAdd_ && disablerRemove_)
  {
    disablerAdd_(box);
    needToRemoveFromDisabler_.push_back(box);
  }
}

void ModuleDialogGeneric::connectSpinBoxToExecuteSignal(QDoubleSpinBox* box)
{
  connect(box, SIGNAL(valueChanged(double)), this, SIGNAL(executeFromStateChangeTriggered()));
  if (disablerAdd_ && disablerRemove_)
  {
    disablerAdd_(box);
    needToRemoveFromDisabler_.push_back(box);
  }
}

void ModuleDialogGeneric::updateWindowTitle(const QString& title)
{
  setWindowTitle(title);
  if (dock_)
    dock_->setWindowTitle(title);
  if (buttonBox_)
    buttonBox_->setTitle(title);
}

void ModuleDialogGeneric::setButtonBarTitleVisible(bool visible)
{
  if (buttonBox_)
    buttonBox_->setTitleVisible(visible);
}

void ModuleDialogGeneric::fixSize()
{
  if (minimumWidth() > 0 && minimumHeight() > 0)
  {
    setFixedSize(minimumWidth(), minimumHeight());
  }
}

void ModuleDialogGeneric::createExecuteAction()
{
  executeAction_ = new QAction(this);
  executeAction_->setText("Execute");
  //TODO: doesn't work on Mac
  //executeAction_->setShortcut(QKeySequence("Ctrl+1"));
  executeAction_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
  connect(executeAction_, SIGNAL(triggered()), this, SIGNAL(executeActionTriggered()));
}

void ModuleDialogGeneric::createExecuteDownstreamAction()
{
  executeDownstreamAction_ = new QAction(this);
  executeDownstreamAction_->setText("Execute + downstream only");
  executeDownstreamAction_->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowDown));
  connect(executeDownstreamAction_, SIGNAL(triggered()), this, SIGNAL(executeActionTriggeredViaStateChange()));
}

void ModuleDialogGeneric::createShrinkAction()
{
  shrinkAction_ = new QAction(this);
  shrinkAction_->setText("Collapse");
  //TODO: redo this slot to hook up to toggled() signal
  connect(shrinkAction_, SIGNAL(triggered()), this, SLOT(toggleCollapse()));
}

void ModuleDialogGeneric::createExecuteInteractivelyToggleAction()
{
  executeInteractivelyToggleAction_ = new QAction(this);
  executeInteractivelyToggleAction_->setText("Execute Interactively");
  executeInteractivelyToggleAction_->setCheckable(true);
  executeInteractivelyToggleAction_->setChecked(true);
  connect(executeInteractivelyToggleAction_, SIGNAL(toggled(bool)), this, SLOT(executeInteractivelyToggled(bool)));
}

void ModuleDialogGeneric::createForceAlwaysExecuteToggleAction()
{
  forceAlwaysExecuteToggleAction_ = new QAction(this);
  forceAlwaysExecuteToggleAction_->setText("Execute Always");
  forceAlwaysExecuteToggleAction_->setCheckable(true);
  forceAlwaysExecuteToggleAction_->setChecked(false);
  connect(forceAlwaysExecuteToggleAction_, SIGNAL(toggled(bool)), this, SLOT(forceAlwaysExecuteToggled(bool)));
}

void ModuleDialogGeneric::executeInteractivelyToggled(bool toggle)
{
  if (qobject_cast<QCheckBox*>(sender()))
    executeInteractivelyToggleAction_->setChecked(toggle);
  else
    buttonBox_->executeInteractivelyCheckBox_->setChecked(toggle);
  if (toggle)
    connectStateChangeToExecute();
  else
    disconnectStateChangeToExecute();
}

void ModuleDialogGeneric::forceAlwaysExecuteToggled(bool toggle)
{
  logWarning("Module always execute set to {}", toggle);
  setModuleAlwaysExecute(state_, toggle);
}

void ModuleDialogGeneric::connectStateChangeToExecute()
{
  connect(this, SIGNAL(executeFromStateChangeTriggered()), this, SIGNAL(executeActionTriggeredViaStateChange()));
}

void ModuleDialogGeneric::disconnectStateChangeToExecute()
{
  disconnect(this, SIGNAL(executeFromStateChangeTriggered()), this, SIGNAL(executeActionTriggeredViaStateChange()));
}

void ModuleDialogGeneric::toggleCollapse()
{
  if (collapsed_)
  {
    shrinkAction_->setText("Collapse");
  }
  else
  {
    shrinkAction_->setText("Expand");
  }
  collapsed_ = !collapsed_;
  doCollapse();
}

void ModuleDialogGeneric::doCollapse()
{
  if (collapsed_)
  {
    oldSize_ = dock_->size();
    const int h = std::min(40, oldSize_.height());
    const int w = std::min(400, oldSize_.width());
    dock_->setFixedSize(w, h);
  }
  else
  {
    dock_->setFixedSize(oldSize_);
  }
}

void ModuleDialogGeneric::contextMenuEvent(QContextMenuEvent* e)
{
  QMenu menu(this);
  menu.addAction(executeAction_);
  if (executeInteractivelyToggleAction_)
    menu.addAction(executeInteractivelyToggleAction_);
  menu.addAction(executeDownstreamAction_);
  menu.addAction(shrinkAction_);
  menu.exec(e->globalPos());

  QDialog::contextMenuEvent(e);
}

void ModuleDialogGeneric::addWidgetSlotManager(WidgetSlotManagerPtr ptr)
{
  slotManagers_.push_back(ptr);
}

void ModuleDialogGeneric::removeManager(const AlgorithmParameterName& stateKey)
{
  slotManagers_.erase(std::remove_if(slotManagers_.begin(), slotManagers_.end(), [&](WidgetSlotManagerPtr wsm) { return wsm->name() == stateKey; } ));
}

void ModuleDialogGeneric::pullManagedWidgets()
{
  Pulling p(this);
  for (auto& wsm : slotManagers_)
    wsm->pull();
}

void ModuleDialogGeneric::pull()
{
  pullManagedWidgets();
  Pulling p(this);
  pullSpecial();
}

void ModuleDialogGeneric::moduleSelected(bool selected)
{
  if (selected)
  {
    windowTitle_ = windowTitle();
    updateWindowTitle(">>> " + windowTitle_ + " <<<");
  }
  else
  {
    updateWindowTitle(windowTitle_);
  }
}

class ComboBoxSlotManager : public WidgetSlotManager
{
public:
  typedef boost::function<std::string(const QString&)> FromQStringConverter;
  typedef boost::function<QString(const std::string&)> ToQStringConverter;
  ComboBoxSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QComboBox* comboBox,
    FromQStringConverter fromLabelConverter = boost::bind(&QString::toStdString, _1),
    ToQStringConverter toLabelConverter = &QString::fromStdString) :
  WidgetSlotManager(state, dialog, comboBox, stateKey), stateKey_(stateKey), comboBox_(comboBox), fromLabelConverter_(fromLabelConverter), toLabelConverter_(toLabelConverter)
  {
    connect(comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(push()));
  }
  ComboBoxSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QComboBox* comboBox,
    const GuiStringTranslationMap& stringMap) :
    WidgetSlotManager(state, dialog, comboBox, stateKey), stateKey_(stateKey), comboBox_(comboBox), stringMap_(stringMap)
  {
    if (stringMap_.empty())
    {
      THROW_INVALID_ARGUMENT("empty combo box string mapping");
    }
    if (0 == comboBox->count())
    {
      for (const auto& choices : stringMap_.left)
      {
        comboBox->addItem(QString::fromStdString(choices.first));
      }
    }
    fromLabelConverter_ = [this](const QString& qstr) { return findOrFirst(stringMap_.left, qstr.toStdString()); };
    toLabelConverter_ = [this](const std::string& str) { return QString::fromStdString(findOrFirst(stringMap_.right, str)); };
    connect(comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto value = state_->getValue(stateKey_).toString();
    auto qstring = toLabelConverter_(value);
    if (qstring != comboBox_->currentText())
    {
      LOG_TRACE("In new version of pull code for combobox: {}", value);
      comboBox_->setCurrentIndex(comboBox_->findText(qstring));
    }
  }
  virtual void pushImpl() override
  {
    auto label = fromLabelConverter_(comboBox_->currentText());
    if (label != state_->getValue(stateKey_).toString())
    {
      LOG_TRACE("In new version of push code for combobox: {}", label);
      state_->setValue(stateKey_, label);
    }
  }
private:
  AlgorithmParameterName stateKey_;
  QComboBox* comboBox_;
  FromQStringConverter fromLabelConverter_;
  ToQStringConverter toLabelConverter_;
  GuiStringTranslationMap stringMap_;

  template <class Map>
  std::string findOrFirst(const Map& map, const std::string& key) const
  {
    auto iter = map.find(key);
    if (iter == map.end())
    {
      const std::string& first = map.begin()->second;
      logWarning("Combo box state error: key not found ({}), replacing with {}", key, first);
      return first;
    }
    return iter->second;
  }
};

#if 0
//Interesting idea but hard to manage lifetime of Widget pointers, if they live in a dynamic table. This will need to be melded into the TableWidget subclass.
template <class Manager, class Widget>
class CompositeSlotManager : public WidgetSlotManager
{
public:
  CompositeSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, const std::vector<Widget*>& widgets)
    : WidgetSlotManager(state, dialog)
  {
    std::transform(widgets.begin(), widgets.end(), std::back_inserter(managers_), [&](Widget* w) { return boost::make_shared<Manager>(state, dialog, stateKey, w); });
  }
private:
  std::vector<boost::shared_ptr<Manager>> managers_;
};
#endif

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
    WidgetSlotManager(state, dialog, comboBox, stateKey), stateKey_(stateKey), comboBox_(comboBox)
  {
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto value = state_->getValue(stateKey_).toBool();
    auto index = value ? 1 : 0;
    if (index != comboBox_->currentIndex())
    {
      LOG_TRACE("In new version of pull code for combobox, boolean mode: {}", index);
      comboBox_->setCurrentIndex(index);
    }
  }
  virtual void pushImpl() override
  {
    auto index = comboBox_->currentIndex();
    if (index != (state_->getValue(stateKey_).toBool() ? 1 : 0))
    {
      LOG_TRACE("In new version of push code for combobox, boolean mode: {}", index);
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
    WidgetSlotManager(state, dialog, textEdit, stateKey), stateKey_(stateKey), textEdit_(textEdit)
  {
    connect(textEdit, SIGNAL(textChanged()), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto newValue = QString::fromStdString(state_->getValue(stateKey_).toString());
    if (newValue != textEdit_->toPlainText())
    {
      textEdit_->setPlainText(newValue);
      LOG_TRACE("In new version of pull code for TextEdit: {}", newValue.toStdString());
    }
  }
  virtual void pushImpl() override
  {
    LOG_TRACE("In new version of push code for TextEdit: {}", textEdit_->toPlainText().toStdString());
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

class PlainTextEditSlotManager : public WidgetSlotManager
{
public:
  PlainTextEditSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QPlainTextEdit* textEdit) :
    WidgetSlotManager(state, dialog, textEdit, stateKey), stateKey_(stateKey), textEdit_(textEdit)
  {
    connect(textEdit, SIGNAL(textChanged()), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto newValue = QString::fromStdString(state_->getValue(stateKey_).toString());
    if (newValue != textEdit_->toPlainText())
    {
      textEdit_->setPlainText(newValue);
      LOG_TRACE("In new version of pull code for PlainTextEdit: {}", newValue.toStdString());
    }
  }
  virtual void pushImpl() override
  {
    LOG_TRACE("In new version of push code for PlainTextEdit: {}", textEdit_->toPlainText().toStdString());
    state_->setValue(stateKey_, textEdit_->toPlainText().toStdString());
  }
private:
  AlgorithmParameterName stateKey_;
  QPlainTextEdit* textEdit_;
};

void ModuleDialogGeneric::addPlainTextEditManager(QPlainTextEdit* plainTextEdit, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<PlainTextEditSlotManager>(state_, *this, stateKey, plainTextEdit));
}

class LineEditSlotManager : public WidgetSlotManager
{
public:
  LineEditSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QLineEdit* lineEdit) :
    WidgetSlotManager(state, dialog, lineEdit, stateKey), stateKey_(stateKey), lineEdit_(lineEdit)
  {
    connect(lineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto newValue = QString::fromStdString(state_->getValue(stateKey_).toString());
    if (newValue != lineEdit_->text())
    {
      lineEdit_->setText(newValue);
      LOG_TRACE("In new version of pull code for LineEdit: {}", newValue.toStdString());
    }
  }
  virtual void pushImpl() override
  {
    LOG_TRACE("In new version of push code for LineEdit: {}", lineEdit_->text().toStdString());
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

class TabSlotManager : public WidgetSlotManager
{
public:
  TabSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QTabWidget* tabWidget) :
    WidgetSlotManager(state, dialog, tabWidget, stateKey), stateKey_(stateKey), tabWidget_(tabWidget)
  {
    connect(tabWidget_, SIGNAL(currentChanged(int)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto newValue = QString::fromStdString(state_->getValue(stateKey_).toString());
    if (newValue != tabWidget_->tabText(tabWidget_->currentIndex()))
    {
      for (int i = 0; i < tabWidget_->count(); ++i)
      {
        if (tabWidget_->tabText(i) == newValue)
        {
          tabWidget_->setCurrentIndex(i);
          LOG_TRACE("In new version of pull code for LineEdit: {}", newValue.toStdString());
          return;
        }
      }
    }
  }
  virtual void pushImpl() override
  {
    LOG_TRACE("In new version of push code for QTabWidget: {}", tabWidget_->tabText(tabWidget_->currentIndex()).toStdString());
    state_->setValue(stateKey_, tabWidget_->tabText(tabWidget_->currentIndex()).toStdString());
  }
private:
  AlgorithmParameterName stateKey_;
  QTabWidget* tabWidget_;
};

void ModuleDialogGeneric::addTabManager(QTabWidget* tab, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<TabSlotManager>(state_, *this, stateKey, tab));
}

class DoubleLineEditSlotManager : public WidgetSlotManager
{
public:
  DoubleLineEditSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QLineEdit* lineEdit) :
    WidgetSlotManager(state, dialog, lineEdit, stateKey), stateKey_(stateKey), lineEdit_(lineEdit)
      {
        connect(lineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(push()));
        lineEdit_->setValidator(new QDoubleValidator(lineEdit_));
      }
      virtual void pull() override
      {
        auto newValue = QString::number(state_->getValue(stateKey_).toDouble());
        if (newValue != lineEdit_->text())
        {
          lineEdit_->setText(newValue);
          LOG_TRACE("In new version of pull code for DoubleLineEdit: {}", newValue.toStdString());
        }
      }
      virtual void pushImpl() override
      {
        LOG_TRACE("In new version of push code for LineEdit: {}", lineEdit_->text().toStdString());
        bool ok;
        auto value = lineEdit_->text().toDouble(&ok);
        if (ok)
          state_->setValue(stateKey_, value);
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
    WidgetSlotManager(state, dialog, spinBox, stateKey), stateKey_(stateKey), spinBox_(spinBox)
  {
    connect(spinBox_, SIGNAL(valueChanged(int)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto newValue = state_->getValue(stateKey_).toInt();
    if (newValue != spinBox_->value())
    {
      spinBox_->setValue(newValue);
      LOG_TRACE("In new version of pull code for SpinBox: {}", newValue);
    }
  }
  virtual void pushImpl() override
  {
    LOG_TRACE("In new version of push code for SpinBox: {}", spinBox_->value());
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
    WidgetSlotManager(state, dialog, spinBox, stateKey), stateKey_(stateKey), spinBox_(spinBox)
  {
    connect(spinBox_, SIGNAL(valueChanged(double)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    auto newValue = state_->getValue(stateKey_).toDouble();
    if (newValue != spinBox_->value())
    {
      spinBox_->setValue(newValue);
      LOG_TRACE("In new version of pull code for DoubleSpinBox: {}", newValue);
    }
  }
  virtual void pushImpl() override
  {
    LOG_TRACE("In new version of push code for DoubleSpinBox: {}", spinBox_->value());
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
    WidgetSlotManager(state, dialog, checkBox, stateKey), stateKey_(stateKey), checkBox_(checkBox)
  {
    connect(checkBox_, SIGNAL(stateChanged(int)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    bool newValue = state_->getValue(stateKey_).toBool();
    if (newValue != checkBox_->isChecked())
    {
      LOG_TRACE("In new version of pull code for CheckBox: {}", newValue);
      checkBox_->setChecked(newValue);
    }
  }
  virtual void pushImpl() override
  {
    LOG_TRACE("In new version of push code for CheckBox: {}", checkBox_->isChecked());
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
    WidgetSlotManager(state, dialog, checkable, stateKey), stateKey_(stateKey), checkable_(checkable)
      {
        connect(checkable_, SIGNAL(clicked()), this, SLOT(push()));
      }
      virtual void pull() override
      {
        bool newValue = state_->getValue(stateKey_).toBool();
        if (newValue != checkable_->isChecked())
        {
          LOG_TRACE("In new version of pull code for checkable QAbstractButton: {}", newValue);
          checkable_->setChecked(newValue);
        }
      }
      virtual void pushImpl() override
      {
        LOG_TRACE("In new version of push code for checkable QAbstractButton: {}", checkable_->isChecked());
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

class DynamicLabelSlotManager : public WidgetSlotManager
{
public:
  DynamicLabelSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QLabel* label) :
    WidgetSlotManager(state, dialog, label, stateKey), stateKey_(stateKey), label_(label)
  {
  }
  virtual void pull() override
  {
    auto newValue = state_->getValue(stateKey_).toString();
    if (newValue != label_->text().toStdString())
    {
      LOG_TRACE("In new version of pull code for dynamic label: {}", newValue);
      label_->setText(QString::fromStdString(newValue));
    }
  }
  virtual void pushImpl() override
  {
  }
private:
  AlgorithmParameterName stateKey_;
  QLabel* label_;
};

void ModuleDialogGeneric::addDynamicLabelManager(QLabel* label, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<DynamicLabelSlotManager>(state_, *this, stateKey, label));
}

class SliderSlotManager : public WidgetSlotManager
{
public:
  SliderSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, QSlider* slider) :
    WidgetSlotManager(state, dialog, slider, stateKey), stateKey_(stateKey), slider_(slider)
  {
  }
  virtual void pull() override
  {
    auto newValue = state_->getValue(stateKey_).toInt();
    if (newValue != slider_->value())
    {
      LOG_TRACE("In new version of pull code for QSlider: {}", newValue);
      slider_->setValue(newValue);
    }
  }
  virtual void pushImpl() override
  {
  }
private:
  AlgorithmParameterName stateKey_;
  QSlider* slider_;
};

void ModuleDialogGeneric::addSliderManager(QSlider* slider, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<SliderSlotManager>(state_, *this, stateKey, slider));
}

class RadioButtonGroupSlotManager : public WidgetSlotManager
{
public:
  RadioButtonGroupSlotManager(ModuleStateHandle state, ModuleDialogGeneric& dialog, const AlgorithmParameterName& stateKey, std::initializer_list<QRadioButton*> radioButtons) :
    WidgetSlotManager(state, dialog, nullptr, stateKey), stateKey_(stateKey), radioButtons_(radioButtons) //TODO: need to pass all of them...
  {
    for (auto button : radioButtons_)
    {
      connect(button, SIGNAL(clicked()), this, SLOT(push()));
      WidgetStyleMixin::setStateVarTooltipWithStyle(button, stateKey.name_);
    }
  }
  virtual void pull() override
  {
    auto checkedIndex = state_->getValue(stateKey_).toInt();
    if (checkedIndex >= 0 && checkedIndex < radioButtons_.size())
    {
      if (!radioButtons_[checkedIndex]->isChecked())
      {
        LOG_TRACE("In new version of pull code for radio button group: {}", checkedIndex);
        radioButtons_[checkedIndex]->setChecked(true);
      }
    }
  }
  virtual void pushImpl() override
  {
    auto firstChecked = std::find_if(radioButtons_.begin(), radioButtons_.end(), [](QRadioButton* button) { return button->isChecked(); });
    int indexOfChecked = firstChecked - radioButtons_.begin();
    state_->setValue(stateKey_, indexOfChecked);
  }
private:
  AlgorithmParameterName stateKey_;
  std::vector<QRadioButton*> radioButtons_;
};

void ModuleDialogGeneric::addRadioButtonGroupManager(std::initializer_list<QRadioButton*> radioButtons, const AlgorithmParameterName& stateKey)
{
  addWidgetSlotManager(boost::make_shared<RadioButtonGroupSlotManager>(state_, *this, stateKey, radioButtons));
}

void WidgetStyleMixin::tabStyle(QTabWidget* tabs)
{
	tabs->setStyleSheet(
		"QTabBar::tab::selected, QTabBar::tab::hover         {color:black; background-color: #F0F0F0; border: 1px solid rgb(66,66,69); min-width:2ex; padding: 5px 10px;} "
		"QTabBar::tab:!selected {color: white; background-color: rgb(66,66,69); border: 1px solid #FFFFFF; min-width:2ex; padding: 5px 10px; }"
		"QTabBar::tab:selected  {color:black; background-color: #F0F0F0; border: 1px solid rgb(66,66,69); min-width:2ex; padding: 5px 10px;}"
		);
}

void WidgetStyleMixin::tableHeaderStyle(QTableWidget* tableHeader)
{
	tableHeader->setStyleSheet(
		"QHeaderView::section {background: rgb(66,66,69);}"
		);
}

void WidgetStyleMixin::toolbarStyle(QToolBar* toolbar)
{
  toolbar->setStyleSheet("QToolBar { background-color: rgb(66,66,69); border: 1px solid black; color: black }"
    "QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }");
}

void WidgetStyleMixin::setStateVarTooltipWithStyle(QWidget* widget, const std::string& stateVarName)
{
  widget->setToolTip("State key: " + QString::fromStdString(stateVarName));
  widget->setStyleSheet(widget->styleSheet() + " QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }");
}

std::tuple<std::string, int> ModuleDialogGeneric::getConnectedDynamicPortId(const std::string& portId, const std::string& type, bool isLoadingFile)
{
  //note: the incoming portId is the port that was just added, not connected to. we assume the connected port
  // is one index less.
  // UNLESS we are loading a file, in which case this function is called when the connected port is the same as portId.
  //std::cout << "REGEX: " << "Input" + type + "\\:(.+)" << std::endl;
  boost::regex portIdRegex("Input" + type + "\\:(.+)");
  boost::smatch what;
  //std::cout << "MATCHING WITH: " << portId << std::endl;
  regex_match(portId, what, portIdRegex);
  const int connectedPortNumber = boost::lexical_cast<int>(what[1]) - (isLoadingFile ? 0 : 1);
  return std::make_tuple("Input" + type + ":" + boost::lexical_cast<std::string>(connectedPortNumber), connectedPortNumber);
}

void ModuleDialogGeneric::syncTableRowsWithDynamicPort(const std::string& portId, const std::string& type,
  QTableWidget* table, int lineEditIndex, DynamicPortChange portChangeType, const TableItemMakerMap& tableItems, const WidgetItemMakerMap& widgetItems)
{
  ScopedWidgetSignalBlocker swsb(table);
  if (portId.find(type) != std::string::npos)
  {
    if (portChangeType == DynamicPortChange::USER_ADDED_PORT || portChangeType == DynamicPortChange::USER_ADDED_PORT_DURING_FILE_LOAD)
    {
      int connectedPortNumber;
      std::string connectedPortId;
      std::tie(connectedPortId, connectedPortNumber) = getConnectedDynamicPortId(portId, type, portChangeType == DynamicPortChange::USER_ADDED_PORT_DURING_FILE_LOAD);

      Name name(connectedPortId);
      QString lineEditText;

      if (state_->containsKey(name))
        lineEditText = QString::fromStdString(state_->getValue(name).toString());
      else
      {
        lineEditText = QString::fromStdString(type).toLower() + "Input" + QString::number(connectedPortNumber + 1);
      }

      {
        table->insertRow(table->rowCount());
        auto newRowIndex = table->rowCount() - 1;

        table->setItem(newRowIndex, 0, new QTableWidgetItem(QString::fromStdString(connectedPortId)));

        auto lineEdit = new QLineEdit;

        lineEdit->setText(lineEditText);
        if (!state_->containsKey(name))
        {
          state_->setValue(name, lineEditText.toStdString());
        }

        addLineEditManager(lineEdit, name);
        table->setCellWidget(newRowIndex, lineEditIndex, lineEdit);

        for (int i = 1; i < table->columnCount(); ++i)
        {
          if (i != lineEditIndex)
          {
            auto itemMakerIter = tableItems.find(i);
            if (itemMakerIter != tableItems.end())
            {
              table->setItem(newRowIndex, i, itemMakerIter->second());
            }
            else
            {
              auto widgetItemMaker = widgetItems.find(i);
              if (widgetItemMaker != widgetItems.end())
              {
                table->setCellWidget(newRowIndex, i, widgetItemMaker->second());
              }
            }
          }
        }
      }
    }
    else
    {
      auto items = table->findItems(QString::fromStdString(portId), Qt::MatchFixedString);
      if (!items.empty())
      {
        auto item = items[0];
        int row = table->row(item);
        table->removeRow(row);
        removeManager(Name(portId));
      }
      else
        qDebug() << "Inconsistent rows versus dynamic ports!";
    }
    table->resizeColumnsToContents();
  }
}

ScopedWidgetSignalBlocker::ScopedWidgetSignalBlocker(QWidget* widget) : widget_(widget)
{
  if (widget_)
    widget_->blockSignals(true);
}

ScopedWidgetSignalBlocker::~ScopedWidgetSignalBlocker()
{
  if (widget_)
    widget_->blockSignals(false);
}

void SCIRun::Gui::openUrl(const QString& url, const std::string& name)
{
  if (!QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode)))
    logError("Failed to open {}", name);
}

void SCIRun::Gui::openPythonAPIDoc()
{
  openUrl("http://sciinstitute.github.io/SCIRun/python.html", "SCIRun Python API page");
}

namespace detail
{
  QColor toColor(const std::string& str)
  {
    ColorRGB textColor(str);
    return QColor(
      static_cast<int>(textColor.r() > 1 ? textColor.r() : textColor.r() * 255.0),
      static_cast<int>(textColor.g() > 1 ? textColor.g() : textColor.g() * 255.0),
      static_cast<int>(textColor.b() > 1 ? textColor.b() : textColor.b() * 255.0));
  }

  std::string fromColor(const QColor& color)
  {
    return ColorRGB(color.redF(), color.greenF(), color.blueF()).toString();
  }
}

QColor ModuleDialogGeneric::colorFromState(const AlgorithmParameterName& stateKey) const
{
  return detail::toColor(state_->getValue(stateKey).toString());
}

void ModuleDialogGeneric::colorToState(const AlgorithmParameterName& stateKey, const QColor& color)
{
  state_->setValue(stateKey, detail::fromColor(color));
}

std::vector<QColor> ModuleDialogGeneric::colorsFromState(const AlgorithmParameterName& stateKey) const
{
  auto conv = [](const Variable& var) -> QColor { return detail::toColor(var.toString()); };
  std::vector<QColor> colors;
  auto vars = state_->getValue(stateKey).toVector();
  std::transform(vars.begin(), vars.end(), std::back_inserter(colors), conv);
  return colors;
}

void ModuleDialogGeneric::colorsToState(const AlgorithmParameterName& stateKey, const std::vector<QColor>& colors)
{
  VariableList vars;
  std::transform(colors.begin(), colors.end(), std::back_inserter(vars),
    [](const QColor& color) { return makeVariable("color", detail::fromColor(color)); });
  state_->setValue(stateKey, vars);
}

std::vector<QString> SCIRun::Gui::toQStringVector(const std::vector<std::string>& strVec)
{
  std::vector<QString> qv;
  std::transform(strVec.begin(), strVec.end(), back_inserter(qv), QString::fromStdString);
  return qv;
}

void ModuleDialogGeneric::adjustToolbarForHighResolution(QToolBar* toolbar)
{
  for (auto& child : toolbar->children())
  {
    auto button = qobject_cast<QPushButton*>(child);
    if (button)
    {
      button->setFixedSize(button->size() * 2);
      button->setIconSize(button->iconSize() * 2);
    }
  }
}

void ModuleDialogGeneric::keyPressEvent(QKeyEvent* e)
{
  if (e->key() != Qt::Key_Escape)
    QDialog::keyPressEvent(e);
  else //Esc = close dialog
  {
    Q_EMIT closeButtonClicked();
  }
}
