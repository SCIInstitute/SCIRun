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
//#include <Interface/Modules/Base/CustomWidgets/CodeEditorWidgets.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Python;

CompositeModuleDialog::CompositeModuleDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addPlainTextEditManager(networkXMLplainTextEdit_, Parameters::NetworkXml);
  addPlainTextEditManager(portReportPlainTextEdit_, Parameters::PortSettings);

  connect(clearPushButton_, &QPushButton::clicked, [this]() { networkXMLplainTextEdit_->clear(); });
  connect(pastePushButton_, &QPushButton::clicked, [this]() { networkXMLplainTextEdit_->setPlainText(QGuiApplication::clipboard()->text()); });

  state_->connectSpecificStateChanged(Parameters::ModuleIdList,
    [this]() { updateModuleUIButtons(); });
}

void CompositeModuleDialog::updateModuleUIButtons()
{
  auto moduleMap = transient_value_cast<SCIRun::Modules::Basic::ModuleIdMap>(state_->getTransientValue(Parameters::ModuleIdList));

  if (moduleMap.empty())
  {
    for (int i = 0; i < moduleUIgridLayout_->rowCount(); ++i)
    {
      for (int j = 0; j < moduleUIgridLayout_->columnCount(); ++j)
      {
        delete moduleUIgridLayout_->itemAtPosition(i, j)->widget();
      }
    }
    return;
  }

  int i = 0;
  for (const auto& p : moduleMap)
  {
    qDebug() << p.first.c_str() << p.second.c_str();
    auto ui = new QPushButton(p.first.c_str() + QString(" UI"));
    auto log = new QPushButton(p.first.c_str() + QString(" log"));
    moduleUIgridLayout_->addWidget(ui, i, 0);
    moduleUIgridLayout_->addWidget(log, i, 1);
    ++i;
  }
}
