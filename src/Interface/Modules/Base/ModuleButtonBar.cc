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


#include <QtGui>
#include <Interface/Modules/Base/ModuleButtonBar.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h>

using namespace SCIRun::Gui;

ModuleButtonBar::ModuleButtonBar(ModuleDialogGeneric* parent) : QWidget(parent)
{
  setupUi(this);

  for (auto b : std::vector<QWidget*>{ closeToolButton_, executeDownOnlyToolButton_, executeToolButton_, findToolButton_, helpToolButton_, forceAlwaysExecuteCheckBox_, executeInteractivelyCheckBox_ })
  {
    b->setStyleSheet("QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }");
  }
  closeToolButton_->setIcon(QPixmap(":/general/Resources/delete_red.png"));
  executeDownOnlyToolButton_->setIcon(QPixmap(":/general/Resources/new/modules/run_down.png"));
  executeToolButton_->setIcon(QPixmap(":/general/Resources/new/modules/run_all.png"));
  findToolButton_->setIcon(QPixmap(":/general/Resources/zoom_reset.png"));
  helpToolButton_->setIcon(QPixmap(":/general/Resources/new/modules/help.png"));
  collapseToolButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_ToolBarVerticalExtensionButton));

  connect(executeDownOnlyToolButton_, SIGNAL(clicked()), parent->getExecuteDownstreamAction(), SIGNAL(triggered()));
  connect(executeToolButton_, SIGNAL(clicked()), parent->getExecuteAction(), SIGNAL(triggered()));
  connect(closeToolButton_, SIGNAL(clicked()), parent, SIGNAL(closeButtonClicked()));
  connect(helpToolButton_, SIGNAL(clicked()), parent, SIGNAL(helpButtonClicked()));
  connect(findToolButton_, SIGNAL(clicked()), parent, SIGNAL(findButtonClicked()));
  connect(collapseToolButton_, SIGNAL(clicked()), parent, SLOT(toggleCollapse()));
  connect(collapseToolButton_, SIGNAL(clicked()), this, SLOT(switchIcons()));
}

void ModuleButtonBar::setTitle(const QString& title)
{
  titleLabel_->setText(title);
}

void ModuleButtonBar::setTitleVisible(bool visible)
{
  titleLabel_->setVisible(visible);
}

void ModuleButtonBar::switchIcons()
{
  //TODO
  //collapseToolButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarShadeButton));
  //collapseToolButton_->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarUnshadeButton));
}
