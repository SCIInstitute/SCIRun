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

#include "RenderWindow.h"
#include "ui_RenderWindow.h"

#include <QVTKWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

RenderWindow::RenderWindow(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::RenderWindow),
  mVtkWidget(new QVTKWidget(this, QFlag(0)))
{
  ui->setupUi(this);

  // Add VTK widget to vertical layout.
  ui->verticalLayout->addWidget(mVtkWidget);
  ui->verticalLayout->update();

  // Create renderer
  mRen = vtkSmartPointer<vtkRenderer>::New();
  mVtkWidget->GetRenderWindow()->AddRenderer(mRen);
  mRen->SetBackground(1.0,0.0,0.0);
}


RenderWindow::~RenderWindow()
{
  mRen->Delete();
  delete mVtkWidget;
  delete ui;
}
