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


#ifndef INTERFACE_APPLICATION_MODULELOGWINDOW_H
#define INTERFACE_APPLICATION_MODULELOGWINDOW_H

#include "ui_ModuleLogWindow.h"

#include <Core/Logging/LoggerInterface.h>
#include <Dataflow/Network/NetworkFwd.h>


namespace SCIRun {
namespace Gui {

class DialogErrorControl;
class ModuleErrorDisplayer;

class ModuleLogWindow : public QDialog, public Ui::ModuleLogWindow
{
	Q_OBJECT

public:
  explicit ModuleLogWindow(const QString& moduleName, ModuleErrorDisplayer* displayer, boost::shared_ptr<DialogErrorControl> dialogErrorControl, QWidget* parent = 0);
  QString name() const { return moduleName_; }
public Q_SLOTS:
  void appendMessage(const QString& message, const QColor& color = Qt::black);
  void popupMessageBox(const QString& message);

Q_SIGNALS:
  void messageReceived(const QColor& color);
  void requestModuleVisible();

private:
  QString moduleName_;
  ModuleErrorDisplayer* displayer_;
	boost::shared_ptr<DialogErrorControl> dialogErrorControl_;
};

class ModuleLogger : public QObject, public Core::Logging::LegacyLoggerInterface
{
  Q_OBJECT
public:
  explicit ModuleLogger(ModuleLogWindow* window);
  ~ModuleLogger();
  virtual void error(const std::string& msg) const override;
  virtual void warning(const std::string& msg) const override;
  virtual void remark(const std::string& msg) const override;
  virtual void status(const std::string& msg) const override;
  virtual bool errorReported() const override { return errorReported_; }
  virtual void setErrorFlag(bool flag) override { errorReported_ = flag; }

Q_SIGNALS:
  void logSignal(const QString& message, const QColor& color) const;
  void alert(const QColor& color) const;
  void popup(const QString& message) const;
private:
  std::string moduleName_;
  mutable bool errorReported_{ false };
};

}
}

#endif
