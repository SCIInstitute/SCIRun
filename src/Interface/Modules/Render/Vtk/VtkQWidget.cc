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

#include "VtkQWidget.h"

#include <iostream>
#include <string>

#include "VtkRenderer.h"

using namespace SCIRun::Render;

VtkQWidget::VtkQWidget(QWidget *parent, VtkRenderer* renderer) :
	QWidget(parent), renderer(renderer)
{
  renderTimer = new QTimer(this);
  connect(renderTimer, &QTimer::timeout, this, &VtkQWidget::updateRenderer);
  renderTimer->start(16);
}

VtkQWidget::~VtkQWidget()
{

}

void VtkQWidget::paintEvent(QPaintEvent* event)
{
  renderer->renderFrame();

  QPainter painter(this);
  painter.drawImage(rect(), renderer->image());
}

void VtkQWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);

  updateRenderSize();
}

void VtkQWidget::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);

  if (auto* win = window()->windowHandle())
  {
    connect(win, &QWindow::screenChanged, this, &VtkQWidget::onScreenChanged, Qt::UniqueConnection);
  }

  updateRenderSize();
}

void VtkQWidget::onScreenChanged(QScreen* screen)
{
  if (!screen) return;

  connect(screen, &QScreen::physicalDotsPerInchChanged, this, &VtkQWidget::updateRenderSize, Qt::UniqueConnection);

  connect(screen, &QScreen::logicalDotsPerInchChanged, this, &VtkQWidget::updateRenderSize, Qt::UniqueConnection);

  updateRenderSize();
}

void VtkQWidget::updateRenderer()
{
  update();
}

void VtkQWidget::updateRenderSize()
{
  if (!renderer) return;

  double dpr = window()->windowHandle() ? window()->windowHandle()->devicePixelRatio() : devicePixelRatioF();

  const int renderWidth = qRound(width() * dpr);
  const int renderHeight = qRound(height() * dpr);

  renderer->resize(renderWidth, renderHeight, dpr);

  //qDebug() << "widget =" << width() << "x" << height() << "dpr =" << dpr << "render =" << renderWidth << "x" << renderHeight;
}