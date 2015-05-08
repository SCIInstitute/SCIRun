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

#include <Interface/Modules/Render/ViewSceneControlsDock.h>
#include "qbrush.h"

using namespace SCIRun::Gui;
using namespace SCIRun::Render;


ViewSceneControlsDock::ViewSceneControlsDock(const QString& name, ViewSceneDialog* parent) : QDockWidget(parent)
{
  setupUi(this);

  setWindowTitle(name);
  setAllowedAreas(Qt::BottomDockWidgetArea);
  setFloating(true);
  setVisible(false);
  setEnabled(false);
  setStyleSheet(parent->styleSheet());

  parent->menuMouseControlChanged(mouseControlComboBox_->currentIndex());

  connect(orientationCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(showOrientationChecked(bool)));
  connect(mouseControlComboBox_, SIGNAL(currentIndexChanged(int)), parent, SLOT(menuMouseControlChanged(int)));
  connect(setBackgroundColorPushButton_, SIGNAL(clicked()), parent, SLOT(assignBackgroundColor()));
  connect(contSortRadioButton_, SIGNAL(clicked(bool)), parent, SLOT(setTransparencySortTypeContinuous(bool)));
  connect(updateSortRadioButton_, SIGNAL(clicked(bool)), parent, SLOT(setTransparencySortTypeUpdate(bool)));
  connect(listSortRadioButton_, SIGNAL(clicked(bool)), parent, SLOT(setTransparencySortTypeLists(bool)));

  setSampleColor(Qt::black);

  WidgetStyleMixin::tabStyle(tabWidget);

  /////Set unused widgets to be not visible
  ////Clipping tab
  //ClippingTab->setVisible(false);
  //ClippingTab->setEnabled(false);
  //groupBox_3->setVisible(false);
  //groupBox_4->setVisible(false);
  //groupBox_5->setVisible(false);

  ////View Tab
  //groupBox->setVisible(false);

  ////Render Tab
  //groupBox_6->setVisible(false);

}

void ViewSceneControlsDock::setSampleColor(const QColor& color)
{
  QString styleSheet = "QLabel{ background: rgb(" + QString::number(color.red()) + "," +
    QString::number(color.green()) + "," + QString::number(color.blue()) + "); }";

  currentBackgroundLabel_->setStyleSheet(styleSheet);
}
