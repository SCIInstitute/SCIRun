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
#include <iostream>
#include <Interface/Application/HistoryWindow.h>

using namespace SCIRun::Gui;

HistoryWindow::HistoryWindow(QWidget* parent /* = 0 */) : QDockWidget(parent) 
{
  setupUi(this);
  networkXMLTextEdit_->setTabStopWidth(15);
}

void HistoryWindow::showFile(const QString& path)
{
  setWindowTitle("History: " + path);
  QFile xmlFile(path);
  if(!xmlFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    std::cout << "Could not open file: " << path.toStdString() << std::endl;
    return;
  }
 
  networkXMLTextEdit_->setPlainText(xmlFile.readAll());
}
