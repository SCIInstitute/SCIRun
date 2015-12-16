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

#include <Dataflow/Network/ModuleStateInterface.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Core/Logging/Log.h>
#include <Core/Utils/Exception.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;

ExecutionDisablingServiceFunction ModuleDialogGeneric::disablerAdd_;
ExecutionDisablingServiceFunction ModuleDialogGeneric::disablerRemove_;

ModuleDialogGeneric::ModuleDialogGeneric(SCIRun::Dataflow::Networks::ModuleStateHandle state, QWidget* parent) : QDialog(parent),
  state_(state),
  pulling_(false),
  executeAction_(0),
  shrinkAction_(0),
  executeInteractivelyToggleAction_(0),
  collapsed_(false),
  dock_(0)
{
  setModal(false);
  setAttribute(Qt::WA_MacAlwaysShowToolWindow, true);

  if (state_)
  {
    LOG_DEBUG("ModuleDialogGeneric connecting to state" << std::endl);
    stateConnection_ = state_->connect_state_changed([this]() { pullSignal(); });
  }
  connect(this, SIGNAL(pullSignal()), this, SLOT(pull()));
  createExecuteAction();
  createShrinkAction();
  connectStateChangeToExecute(); //TODO: make this a module state variable if a module wants it saved
}

ModuleDialogGeneric::~ModuleDialogGeneric()
{
  if (disablerAdd_ && disablerRemove_)
  {
    std::for_each(needToRemoveFromDisabler_.begin(), needToRemoveFromDisabler_.end(), disablerRemove_);
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
  /*
  TODO: investigate why duplicate executes are signalled.
  connect(box, SIGNAL(currentIndexChanged(const QString&)), this, SIGNAL(executeActionTriggered()));
  if (disablerAdd_ && disablerRemove_)
  {
    disablerAdd_(box);
    needToRemoveFromDisabler_.push_back(box);
  }
  */
}

void ModuleDialogGeneric::updateWindowTitle(const QString& title)
{
  setWindowTitle(title);
  if (dock_)
    dock_->setWindowTitle(title);
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

void ModuleDialogGeneric::executeInteractivelyToggled(bool toggle)
{
  if (toggle)
    connectStateChangeToExecute();
  else
    disconnectStateChangeToExecute();
}

void ModuleDialogGeneric::connectStateChangeToExecute()
{
  connect(this, SIGNAL(executeFromStateChangeTriggered()), this, SIGNAL(executeActionTriggered()));
}

void ModuleDialogGeneric::disconnectStateChangeToExecute()
{
  disconnect(this, SIGNAL(executeFromStateChangeTriggered()), this, SIGNAL(executeActionTriggered()));
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
    oldSize_ = size();
    const int h = std::min(40, oldSize_.height());
    const int w = std::min(400, oldSize_.width());
    setFixedSize(w, h);
    dock_->setFixedSize(w, h);
  }
  else
  {
    setFixedSize(oldSize_);
    dock_->setFixedSize(oldSize_);
  }
}

void ModuleDialogGeneric::contextMenuEvent(QContextMenuEvent* e)
{
  QMenu menu(this);
  menu.addAction(executeAction_);
  if (executeInteractivelyToggleAction_)
    menu.addAction(executeInteractivelyToggleAction_);
  menu.addAction(shrinkAction_);
  menu.exec(e->globalPos());

  QDialog::contextMenuEvent(e);
}

void ModuleDialogGeneric::addWidgetSlotManager(WidgetSlotManagerPtr ptr)
{
  slotManagers_.push_back(ptr);
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
    //setWindowOpacity(0.5);
  }
  else
  {
    updateWindowTitle(windowTitle_);
    //setWindowOpacity(1);
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
      LOG_DEBUG("In new version of pull code for combobox: " << value);
      comboBox_->setCurrentIndex(comboBox_->findText(qstring));
    }
  }
  virtual void pushImpl() override
  {
    auto label = fromLabelConverter_(comboBox_->currentText());
    if (label != state_->getValue(stateKey_).toString())
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

  template <class Map>
  std::string findOrFirst(const Map& map, const std::string& key) const
  {
    auto iter = map.find(key);
    if (iter == map.end())
    {
      const std::string& first = map.begin()->second;
      Log::get() << NOTICE << "Combo box state error: key not found (" << key << "), replacing with " << first << std::endl;
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
      LOG_DEBUG("In new version of pull code for combobox, boolean mode: " << index);
      comboBox_->setCurrentIndex(index);
    }
  }
  virtual void pushImpl() override
  {
    auto index = comboBox_->currentIndex();
    if (index != (state_->getValue(stateKey_).toBool() ? 1 : 0))
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
    WidgetSlotManager(state, dialog, lineEdit, stateKey), stateKey_(stateKey), lineEdit_(lineEdit)
      {
        connect(lineEdit_, SIGNAL(textChanged(const QString&)), this, SLOT(push()));
      }
      virtual void pull() override
      {
        auto newValue = QString::number(state_->getValue(stateKey_).toDouble());
        if (newValue != lineEdit_->text())
        {
          lineEdit_->setText(newValue);
          LOG_DEBUG("In new version of pull code for DoubleLineEdit: " << newValue.toStdString());
        }
      }
      virtual void pushImpl() override
      {
        LOG_DEBUG("In new version of push code for LineEdit: " << lineEdit_->text().toStdString());
        try
        {
          auto value = boost::lexical_cast<double>(lineEdit_->text().toStdString());
          state_->setValue(stateKey_, value);
        }
        catch (boost::bad_lexical_cast&)
        {
          // ignore for now
        }
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
    WidgetSlotManager(state, dialog, checkBox, stateKey), stateKey_(stateKey), checkBox_(checkBox)
  {
    connect(checkBox_, SIGNAL(stateChanged(int)), this, SLOT(push()));
  }
  virtual void pull() override
  {
    bool newValue = state_->getValue(stateKey_).toBool();
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
    WidgetSlotManager(state, dialog, checkable, stateKey), stateKey_(stateKey), checkable_(checkable)
      {
        connect(checkable_, SIGNAL(clicked()), this, SLOT(push()));
      }
      virtual void pull() override
      {
        bool newValue = state_->getValue(stateKey_).toBool();
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
      LOG_DEBUG("In new version of pull code for checkable QAbstractButton: " << newValue);
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
      LOG_DEBUG("In new version of pull code for QSlider: " << newValue);
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
      button->setToolTip(QString::fromStdString(stateKey.name_));
    }
  }
  virtual void pull() override
  {
    auto checkedIndex = state_->getValue(stateKey_).toInt();
    if (checkedIndex >= 0 && checkedIndex < radioButtons_.size())
    {
      if (!radioButtons_[checkedIndex]->isChecked())
      {
        LOG_DEBUG("In new version of pull code for radio button group: " << checkedIndex);
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
