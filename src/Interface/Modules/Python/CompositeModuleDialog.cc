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


#include <Interface/Modules/Python/CompositeModuleDialog.h>
#include <Modules/Basic/CompositeModuleWithStaticPorts.h>
#include <Interface/Modules/Base/ModuleDialogManager.h>
#include <Interface/Modules/Base/ModuleLogWindow.h>
//#include <Interface/Modules/Base/CustomWidgets/CodeEditorWidgets.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Python;
using namespace SCIRun::Modules::Basic;

namespace SCIRun::Gui 
{
  class CompositeModuleDialogImpl
  {
  public:
    explicit CompositeModuleDialogImpl(CompositeModuleDialog* dialog) : dialog_(dialog) {}
    ~CompositeModuleDialogImpl();
    void updateModuleUIButtons();
    std::vector<SharedPointer<ModuleDialogManager>> dialogManagers_;
    SCIRun::Modules::Basic::CompositeModuleInfoMap currentModules_;
  private:
    CompositeModuleDialog* dialog_;
  };
}

CompositeModuleDialog::CompositeModuleDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent), impl_(new CompositeModuleDialogImpl(this))
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addPlainTextEditManager(networkXMLplainTextEdit_, Parameters::NetworkXml);
  addPlainTextEditManager(portReportPlainTextEdit_, Parameters::PortSettings);

  connect(clearPushButton_, &QPushButton::clicked, [this]() { networkXMLplainTextEdit_->clear(); });
  connect(pastePushButton_, &QPushButton::clicked, [this]() { networkXMLplainTextEdit_->setPlainText(QGuiApplication::clipboard()->text()); });

  state_->connectSpecificStateChanged(Parameters::ModuleIdList,
    [this]() { impl_->updateModuleUIButtons(); });
}

CompositeModuleDialog::~CompositeModuleDialog()
{

}

void CompositeModuleDialog::pullSpecial()
{
  impl_->updateModuleUIButtons();
}

CompositeModuleDialogImpl::~CompositeModuleDialogImpl()
{
  for (auto& dialog : dialogManagers_)
  {
    dialog->destroyLog();
    dialog->closeOptions();
    dialog->destroyOptions();
  }
}

void CompositeModuleDialogImpl::updateModuleUIButtons()
{
  auto moduleMap = transient_value_cast<CompositeModuleInfoMap>(dialog_->state_->getTransientValue(Parameters::ModuleIdList));

  if (moduleMap.empty())
  {
    for (int i = 0; i < dialog_->moduleUIgridLayout_->rowCount(); ++i)
    {
      for (int j = 0; j < dialog_->moduleUIgridLayout_->columnCount(); ++j)
      {
        auto grid_ij = dialog_->moduleUIgridLayout_->itemAtPosition(i, j);
        if (grid_ij)
          delete grid_ij->widget();
      }
    }
    for (auto& dialog : dialogManagers_)
    {
      dialog->destroyLog();
      dialog->closeOptions();
      dialog->destroyOptions();
    }
    dialogManagers_.clear();
    currentModules_ = {};
    return;
  }

  if (currentModules_ == moduleMap)
  {
    return;
  }

  int i = 0;
  for (const auto& p : moduleMap)
  {
    auto mod = p.second;
    auto dialogs = makeShared<ModuleDialogManager>(mod);
    if (mod->hasUI())
    {
      auto uiLabel = p.first.id_.c_str() + QString(" UI");
      auto ui = new QPushButton(uiLabel);
      dialog_->moduleUIgridLayout_->addWidget(ui, i, 0);
      dialogs->createOptions();
      auto options = dialogs->options();
      dialog_->connect(ui, &QPushButton::clicked, [options]() { options->show(); options->raise(); });
      options->pull();
    }
    {
      auto logLabel = p.first.id_.c_str() + QString(" log");
      auto log = new QPushButton(logLabel);
      dialog_->moduleUIgridLayout_->addWidget(log, i, 1);
      auto logWindow = dialogs->setupLogging(nullptr, nullptr, dialog_);
      dialog_->connect(log, &QPushButton::clicked, [logWindow]() { logWindow->show(); logWindow->raise(); });
    }
    dialogManagers_.push_back(dialogs);
    ++i;
  }
  currentModules_ = moduleMap;
}
