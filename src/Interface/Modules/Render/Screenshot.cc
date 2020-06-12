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

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Datatypes;



Screenshot::Screenshot(QOpenGLWidget *glwidget, QObject *parent)
  : QObject(parent),
  viewport_(glwidget),
  index_(0)
{
}

QString Screenshot::screenshotDirectory()
{
  static const QString filePath = QDir::homePath() + QLatin1String("/scirun5screenshots");
  QDir dir(filePath);
  if (!dir.exists())
  {
    dir.mkpath(filePath);
  }
  return filePath;
}

void Screenshot::takeScreenshot()
{
	screenshot_ = getScreenshot();
}

QImage Screenshot::getScreenshot()
{
  static constexpr int ALPHA_INDEX = 3;
  static constexpr uint8_t ALPHA_CHANNEL_MAX = 255;
	QImage image = viewport_->grabFramebuffer();
	for (int j = 0; j < image.height(); ++j)
	{
    auto row = reinterpret_cast<QRgb*>(image.scanLine(j));
		for (int i = 0; i < image.width(); ++i)
			reinterpret_cast<uint8_t*>(row + i)[ALPHA_INDEX] = ALPHA_CHANNEL_MAX;
	}
	return image;
}

void Screenshot::saveScreenshot()
{
  index_++;
  auto fileName = screenshotFile();
  if (!fileName.isEmpty())
  {
    QMessageBox::information(nullptr, "ViewScene Screenshot", "Saving ViewScene screenshot to: " + fileName);
    screenshot_.save(fileName);
  }
}

void Screenshot::saveScreenshot(const QString& filename)
{
  screenshot_.save(filename);
}

QString Screenshot::screenshotFile() const
{
  return QFileDialog::getSaveFileName(viewport_, "Save screenshot...", screenshotDirectory(), "*.png");
  //TODO: might want to have an option to save to an auto-generated file for speed
  //return filePath + QString("/viewScene_%1_%2.png").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd.HHmmss.zzz")).arg(index_);
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
