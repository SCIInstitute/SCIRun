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
#include <iostream>
#include <Interface/Application/DeveloperConsole.h>

using namespace SCIRun::Gui;

DeveloperConsole::DeveloperConsole(QWidget* parent /* = 0 */) : QDockWidget(parent)
{
  setupUi(this);
  connect(serialExecutionRadioButton_, SIGNAL(clicked()), this, SLOT(executorButtonClicked()));
  connect(parallelExecutionRadioButton_, SIGNAL(clicked()), this, SLOT(executorButtonClicked()));
  connect(improvedParallelExecutionRadioButton_, SIGNAL(clicked()), this, SLOT(executorButtonClicked()));
  connect(globalPortCacheButton_, SIGNAL(stateChanged(int)), this, SLOT(globalPortCacheButtonClicked()));
}

void DeveloperConsole::updateNetworkViewLog(const QString& s)
{
  networkViewPlainTextEdit_->setPlainText(networkViewPlainTextEdit_->toPlainText() + "\n" + s);
}

void DeveloperConsole::executorButtonClicked()
{
  if (serialExecutionRadioButton_->isChecked())
    Q_EMIT executorChosen(0);
  else if (parallelExecutionRadioButton_->isChecked())
    Q_EMIT executorChosen(1);
  else if (improvedParallelExecutionRadioButton_->isChecked())
    Q_EMIT executorChosen(2);
}

void DeveloperConsole::globalPortCacheButtonClicked()
{
  Q_EMIT globalPortCachingChanged(globalPortCacheButton_->isChecked());
}
