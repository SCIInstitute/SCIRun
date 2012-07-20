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

#include <QtGui>
#include <algorithm>
#include <functional>
#include <boost/bind.hpp>
#include <Interface/Application/Logger.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Engine/Network/NetworkEditorController.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Core/Dataflow/Network/NetworkFwd.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Engine/State/SimpleMapModuleState.h>

#ifdef BUILD_VTK_SUPPORT
#include "RenderWindow.h"
#endif

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Engine;
using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Domain::State;

void visitTree(QStringList& list, QTreeWidgetItem* item){
  list << item->text(0) + "," + QString::number(item->childCount());
  if (item->childCount() != 0)
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  for (int i = 0; i < item->childCount(); ++i)
    visitTree(list, item->child(i));
}

QStringList visitTree(QTreeWidget* tree) {
  QStringList list;
  for (int i = 0; i < tree->topLevelItemCount(); ++i)
    visitTree(list, tree->topLevelItem(i));
  return list;
}

struct LogAppender : public Logger
{
  explicit LogAppender(QTextEdit* text) : text_(text) {}
  void log(const QString& message) const 
  {
    text_->append(message);
  }
  QTextEdit* text_;
};

class TreeViewModuleGetter : public CurrentModuleSelection
{
public:
  explicit TreeViewModuleGetter(QTreeWidget& tree) : tree_(tree) {}
  virtual QString text() const
  {
    return tree_.currentItem()->text(0);
  }
  virtual bool isModule() const
  {
    return tree_.currentItem()->childCount() == 0;
  }
private:
  QTreeWidget& tree_;
};

SCIRunMainWindow* SCIRunMainWindow::instance_ = 0;

SCIRunMainWindow* SCIRunMainWindow::Instance()
{
  if (!instance_)
  {
    instance_ = new SCIRunMainWindow;
  }
  return instance_;
}

SCIRunMainWindow::SCIRunMainWindow()
{
	setupUi(this);

  boost::shared_ptr<TreeViewModuleGetter> getter(new TreeViewModuleGetter(*moduleSelectorTreeWidget_));
  Logger::set_instance(new LogAppender(logTextBrowser_));
  networkEditor_ = new NetworkEditor(getter, scrollAreaWidgetContents_);
  networkEditor_->setObjectName(QString::fromUtf8("networkEditor_"));
  networkEditor_->setContextMenuPolicy(Qt::ActionsContextMenu);
  networkEditor_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  networkEditor_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  networkEditor_->setModuleDumpAction(actionDump_positions);
  networkEditor_->verticalScrollBar()->setValue(0);
  networkEditor_->horizontalScrollBar()->setValue(0);

  actionExecute_All_->setStatusTip(tr("Execute all modules"));
  connect(actionExecute_All_, SIGNAL(triggered()), networkEditor_, SLOT(executeAll()));

  ModuleFactoryHandle moduleFactory(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  boost::shared_ptr<NetworkEditorController> controller(new NetworkEditorController(moduleFactory, sf));
  boost::shared_ptr<NetworkEditorControllerGuiProxy> controllerProxy(new NetworkEditorControllerGuiProxy(controller));
  networkEditor_->setNetworkEditorController(controllerProxy);

  gridLayout_5->addWidget(networkEditor_, 0, 0, 1, 1);
	
	QWidgetAction* moduleSearchAction = new QWidgetAction(this);
	moduleSearchAction->setDefaultWidget(new QLineEdit(this));
	moduleSearchAction->setVisible(true);

	QToolBar* f = addToolBar(tr("&Search"));
	
	QWidgetAction* showModuleLabel = new QWidgetAction(this);
	showModuleLabel->setDefaultWidget(new QLabel("Module Search:", this));
	showModuleLabel->setVisible(true);
	
	f->addAction(showModuleLabel);
	f->addAction(moduleSearchAction);
	
	QToolBar* executeBar = addToolBar(tr("&Execute"));
	executeBar->addAction(actionExecute_All_);
	
	QWidgetAction* globalProgress = new QWidgetAction(this);
	globalProgress->setDefaultWidget(new QProgressBar(this));
	globalProgress->setVisible(true);
	executeBar->addAction(globalProgress);
	
	QWidgetAction* moduleCounter = new QWidgetAction(this);
	moduleCounter->setDefaultWidget(new QLabel("0/0", this));
	moduleCounter->setVisible(true);
	executeBar->addAction(moduleCounter);
	
	scrollAreaWidgetContents_->addAction(actionExecute_All_);
	scrollAreaWidgetContents_->setContextMenuPolicy(Qt::ActionsContextMenu);
	scrollArea_->viewport()->setBackgroundRole(QPalette::Dark);
	scrollArea_->viewport()->setAutoFillBackground(true);	
  networkEditor_->addActions(scrollAreaWidgetContents_);

	logTextBrowser_->setText("Hello! Welcome to the SCIRun5 Prototype.");

  QStringList result = visitTree(moduleSelectorTreeWidget_);
  std::for_each(result.begin(), result.end(), boost::bind(&Logger::log, boost::ref(*Logger::Instance()), _1));

  connect(actionSave_As_, SIGNAL(triggered()), this, SLOT(saveNetwork()));
  //connect(this, SIGNAL(closed()), this, SLOT(...));

#ifdef BUILD_VTK_SUPPORT
  // Build render window.
  renderWindow_ = new RenderWindow(this);
  renderWindow_->setEnabled(false);
  renderWindow_->setVisible(false);
  moduleFactory->setRenderer(renderWindow_);

  connect(actionRenderer, SIGNAL(triggered()), this, SLOT(ToggleRenderer()));
#endif
}

void SCIRunMainWindow::saveNetwork()
{
  QString filename = QFileDialog::getSaveFileName(this, "Save Network...", ".", "*.srn");
  networkEditor_->controller_->saveNetwork(filename.toStdString());
}

void SCIRunMainWindow::ToggleRenderer()
{
#ifdef BUILD_VTK_SUPPORT
  renderWindow_->setEnabled(true);
  renderWindow_->setVisible(true);
#endif
}
