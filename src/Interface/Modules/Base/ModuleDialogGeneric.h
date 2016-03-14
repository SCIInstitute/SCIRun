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

#ifndef INTERFACE_APPLICATION_MODULE_DIALOG_GENERIC_H
#define INTERFACE_APPLICATION_MODULE_DIALOG_GENERIC_H

#include <QtGui>
#ifndef Q_MOC_RUN
#include <Interface/Modules/Base/WidgetSlotManagers.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Core/Algorithms/Base/Name.h>
#include <boost/atomic.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bimap.hpp>
#include <boost/signals2/connection.hpp>
#endif
#include <Interface/Modules/Base/share.h>

namespace SCIRun {
namespace Gui {

  typedef std::function<void(QWidget*)> ExecutionDisablingServiceFunction;
  typedef boost::bimap<std::string,std::string> GuiStringTranslationMap;
  typedef GuiStringTranslationMap::value_type StringPair;

  enum DynamicPortChange
  {
    INITIAL_PORT_CONSTRUCTION,
    USER_ADDED_PORT_DURING_FILE_LOAD,
    USER_ADDED_PORT,
    USER_REMOVED_PORT
  };

  //TODO: pull into separate header; figure out how to automatically style child widgets of these types
  class SCISHARE WidgetStyleMixin
  {
  public:
    static void tabStyle(QTabWidget* tabs);
    static void tableHeaderStyle(QTableWidget* tableHeader);
    static void toolbarStyle(QToolBar* toolbar);
    static void setStateVarTooltipWithStyle(QWidget* widget, const std::string& stateVarName);
  };

  class SCISHARE ModuleDialogGeneric : public QDialog, boost::noncopyable
  {
    Q_OBJECT
  public:
    virtual ~ModuleDialogGeneric();
    bool isPulling() const { return pulling_; } //yuck
    QAction* getExecuteAction() { return executeAction_; }
    void setDockable(QDockWidget* dock) { dock_ = dock; } // to enable title changes
    void updateWindowTitle(const QString& title);
    virtual void createStartupNote() {}
    static void setExecutionDisablingServiceFunctionAdd(ExecutionDisablingServiceFunction add) { disablerAdd_ = add; }
    static void setExecutionDisablingServiceFunctionRemove(ExecutionDisablingServiceFunction remove) { disablerRemove_ = remove; }

    //TODO: input state hookup?
    //yeah: eventually replace int with generic dialog state object, but needs to be two-way (set/get)
    //virtual int moduleExecutionTime() = 0;
    //TODO: how to genericize this?  do we need to?
  public Q_SLOTS:
    virtual void moduleExecuted() {}
    //need a better name: read/updateUI
    virtual void pull() final;
    void moduleSelected(bool selected);
    void toggleCollapse();
    virtual void updateFromPortChange(int numPorts, const std::string& portName, DynamicPortChange type) {}
  Q_SIGNALS:
    void pullSignal();
    void executionTimeChanged(int time);
    void executeActionTriggered();
    void executeFromStateChangeTriggered();
    void setStartupNote(const QString& text);
    void fatalError(const QString& message);
    void executionLoopStarted();
    void executionLoopHalted();
  protected:
    explicit ModuleDialogGeneric(SCIRun::Dataflow::Networks::ModuleStateHandle state, QWidget* parent = 0);
    virtual void contextMenuEvent(QContextMenuEvent* e) override;
    void fixSize();
    void connectButtonToExecuteSignal(QAbstractButton* button);
    void connectButtonsToExecuteSignal(std::initializer_list<QAbstractButton*> buttons);
    void connectComboToExecuteSignal(QComboBox* box);

    void pullManagedWidgets();
    // Dialog classes should override this method to provide pull behavior not available from the widget managers.
    virtual void pullSpecial() {}

    SCIRun::Dataflow::Networks::ModuleStateHandle state_;

    //TODO: need a better push/pull model
    boost::atomic<bool> pulling_;
    struct Pulling
    {
      explicit Pulling(ModuleDialogGeneric* m) : m_(m) { m->pulling_ = true; }
      ~Pulling() { m_->pulling_ = false; }
      ModuleDialogGeneric* m_;
    };
    //TODO: highlight this section in code and documentation
		void addComboBoxManager(QComboBox* comboBox, const Core::Algorithms::AlgorithmParameterName& stateKey);
    void addComboBoxManager(QComboBox* comboBox, const Core::Algorithms::AlgorithmParameterName& stateKey, const GuiStringTranslationMap& stringMap);
    void addTextEditManager(QTextEdit* textEdit, const Core::Algorithms::AlgorithmParameterName& stateKey);
    void addLineEditManager(QLineEdit* lineEdit, const Core::Algorithms::AlgorithmParameterName& stateKey);
    void addDoubleLineEditManager(QLineEdit* lineEdit, const Core::Algorithms::AlgorithmParameterName& stateKey);
    void addSpinBoxManager(QSpinBox* spinBox, const Core::Algorithms::AlgorithmParameterName& stateKey);
    void addDoubleSpinBoxManager(QDoubleSpinBox* spinBox, const Core::Algorithms::AlgorithmParameterName& stateKey);
    void addCheckBoxManager(QCheckBox* checkBox, const Core::Algorithms::AlgorithmParameterName& stateKey);
    void addCheckableButtonManager(QAbstractButton* checkable, const Core::Algorithms::AlgorithmParameterName& stateKey);
    void addTwoChoiceBooleanComboBoxManager(QComboBox* comboBox, const Core::Algorithms::AlgorithmParameterName& stateKey);
    void addDynamicLabelManager(QLabel* label, const Core::Algorithms::AlgorithmParameterName& stateKey);
    void addRadioButtonGroupManager(std::initializer_list<QRadioButton*> radioButtons, const Core::Algorithms::AlgorithmParameterName& stateKey);
    void addSliderManager(QSlider* slider, const Core::Algorithms::AlgorithmParameterName& stateKey);
    void removeManager(const Core::Algorithms::AlgorithmParameterName& stateKey);

    typedef std::vector<std::function<QTableWidgetItem*()>> TableItemMakerList;
    void syncTableRowsWithDynamicPort(const std::string& portId, const std::string& type,
      QTableWidget* table, int lineEditIndex, DynamicPortChange portChangeType, const TableItemMakerList& tableItemMakers);
    static std::tuple<std::string, int> getConnectedDynamicPortId(const std::string& portId, const std::string& type, bool isLoadingFile);

    void createExecuteInteractivelyToggleAction();
  private Q_SLOTS:
    void executeInteractivelyToggled(bool toggle);
  private:
    void addWidgetSlotManager(WidgetSlotManagerPtr ptr);
    void createExecuteAction();
    void createShrinkAction();
    void doCollapse();
    void connectStateChangeToExecute();
    void disconnectStateChangeToExecute();
    std::vector<WidgetSlotManagerPtr> slotManagers_;
    boost::signals2::connection stateConnection_;
    QAction* executeAction_;
    QAction* shrinkAction_;
    QAction* executeInteractivelyToggleAction_;
    bool collapsed_;
    QString windowTitle_;
    QDockWidget* dock_;
    QSize oldSize_;
    std::vector<QWidget*> needToRemoveFromDisabler_;
    static ExecutionDisablingServiceFunction disablerAdd_;
    static ExecutionDisablingServiceFunction disablerRemove_;
  };

  class SCISHARE ScopedWidgetSignalBlocker
  {
  public:
    explicit ScopedWidgetSignalBlocker(QWidget* widget);
    ~ScopedWidgetSignalBlocker();
  private:
    QWidget* widget_;
  };

}
}

#endif
