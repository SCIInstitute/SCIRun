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
#include "SCIRunMainWindow.h"
#include "NetworkEditor.h"

using namespace SCIRun;
using namespace SCIRun::Gui;

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
  void operator()(const QString& str) const
  {
    text_->append(str);
  }
  void log(const std::string& message) const 
  {
    operator()(QString(message.c_str()));
  }
  QTextEdit* text_;
};

class TreeViewModuleGetter : public CurrentModuleSelection
{
public:
  explicit TreeViewModuleGetter(QTreeWidget& tree) : tree_(tree) {}
  virtual std::string text() const
  {
    return tree_.currentItem()->text(0).toStdString();
  }
  virtual bool isModule() const
  {
    return tree_.currentItem()->childCount() == 0;
  }
private:
  QTreeWidget& tree_;
};

SCIRunMainWindow::SCIRunMainWindow()
{
	setupUi(this);

  TreeViewModuleGetter* getter = new TreeViewModuleGetter(*moduleSelectorTreeWidget_);
  LogAppender* logger = new LogAppender(logTextBrowser_);
  networkEditor_ = new NetworkEditor(getter, logger, scrollAreaWidgetContents_);
  networkEditor_->setObjectName(QString::fromUtf8("networkEditor_"));
  networkEditor_->setContextMenuPolicy(Qt::ActionsContextMenu);
  networkEditor_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  networkEditor_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  networkEditor_->setExecuteAction(actionExecute_All_);
  networkEditor_->verticalScrollBar()->setValue(0);
  networkEditor_->horizontalScrollBar()->setValue(0);

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

	logTextBrowser_->setText("Hello!");

  QStringList result = visitTree(moduleSelectorTreeWidget_);
  std::for_each(result.begin(), result.end(), *logger);

}