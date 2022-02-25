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


#include <Core/Datatypes/DenseMatrix.h>
#include <Interface/Modules/Render/Screenshot.h>
#include <QOpenGLWidget>
#include <Core/Application/Preferences/Preferences.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Datatypes;



Screenshot::Screenshot(QOpenGLWidget *glwidget, QObject *parent)
  : QObject(parent),
  viewport_(glwidget)
{
}

void Screenshot::takeScreenshot()
{
	screenshot_ = getScreenshot();
}

QImage Screenshot::getScreenshot()
{
  static constexpr int ALPHA_INDEX = 3;
  static constexpr uint8_t ALPHA_CHANNEL_MAX = 255;
  auto image = viewport_->grabFramebuffer();
  for (int j = 0; j < image.height(); ++j)
  {
    const auto row = reinterpret_cast<QRgb *>(image.scanLine(j));
    for (int i = 0; i < image.width(); ++i)
      reinterpret_cast<uint8_t *>(row + i)[ALPHA_INDEX] = ALPHA_CHANNEL_MAX;
  }
  return image;
}

void Screenshot::saveScreenshot(const QString& fileName)
{
  if (!fileName.isEmpty())
    screenshot_.save(fileName);
}

SCIRun::Modules::Render::RGBMatrices Screenshot::toMatrix() const
{
  const auto red = makeShared<DenseMatrix>(screenshot_.height(), screenshot_.width());
  const auto green = makeShared<DenseMatrix>(screenshot_.height(), screenshot_.width());
  const auto blue = makeShared<DenseMatrix>(screenshot_.height(), screenshot_.width());
  for (int i = 0; i < screenshot_.height(); i++)
  {
    for (int j = 0; j < screenshot_.width(); j++)
    {
      const auto rgb = screenshot_.pixel(j, i);
      (*red)(i, j) = qRed(rgb);
      (*green)(i, j) = qGreen(rgb);
      (*blue)(i, j) = qBlue(rgb);
    }
  }
  return { red, green, blue };
}
