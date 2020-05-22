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


#ifndef INTERFACE_MODULES_SCREENSHOT_H
#define INTERFACE_MODULES_SCREENSHOT_H

#include <Interface/qt_include.h>
#ifndef Q_MOC_RUN
#include <Modules/Render/ViewScene.h>
#endif

class QOpenGLWidget;

namespace SCIRun
{
  namespace Gui
  {
    class Screenshot : public QObject
    {
      Q_OBJECT
    public:
      explicit Screenshot(QOpenGLWidget *glwidget, QObject *parent = nullptr);
      void takeScreenshot();
      QImage getScreenshot();
      void saveScreenshot();
      void saveScreenshot(const QString& filename);
      QString screenshotFile() const;
      Modules::Render::RGBMatrices toMatrix() const;
      static QString screenshotDirectory();

    private:
      QOpenGLWidget* viewport_;
      QImage screenshot_;
      uint index_;
    };
  }
}

#endif // SPIRE_GLWIDGET_H
