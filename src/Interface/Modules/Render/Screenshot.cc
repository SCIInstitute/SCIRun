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

#include <Core/Datatypes/DenseMatrix.h>
#include <Interface/Modules/Render/Screenshot.h>
#include <QGLWidget>

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Datatypes;

const QString filePath = QDir::homePath() + QLatin1String("/scirun5screenshots");

Screenshot::Screenshot(QGLWidget *glwidget, QObject *parent)
  : QObject(parent),
  viewport_(glwidget),
  index_(0)
{
  QDir dir(filePath);
  if (!dir.exists())
  {
    dir.mkpath(filePath);
  }
}

void Screenshot::takeScreenshot()
{
  screenshot_ = viewport_->grabFrameBuffer();
}

void Screenshot::saveScreenshot()
{
  index_++;
  QString fileName = screenshotFile();
  QMessageBox::information(nullptr, "ViewScene Screenshot", "Saving ViewScene screenshot to: " + fileName);
  screenshot_.save(fileName);
}

QString Screenshot::screenshotFile() const
{
  return filePath + QString("/viewScene_%1_%2.png").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd.HHmmss.zzz")).arg(index_);
}

SCIRun::Modules::Render::RGBMatrices Screenshot::toMatrix() const
{
  DenseMatrixHandle red(new DenseMatrix(screenshot_.height(), screenshot_.width()));
  DenseMatrixHandle green(new DenseMatrix(screenshot_.height(), screenshot_.width()));
  DenseMatrixHandle blue(new DenseMatrix(screenshot_.height(), screenshot_.width()));
  for (int i = 0; i < screenshot_.height(); i++)
  {
    for (int j = 0; j < screenshot_.width(); j++)
    {
      auto rgb = screenshot_.pixel(j, i);
      (*red)(i, j) = qRed(rgb);
      (*green)(i, j) = qGreen(rgb);
      (*blue)(i, j) = qBlue(rgb);
    }
  }
  return { red, green, blue };
}