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


#ifndef INTERFACE_APPLICATION_NETWORKEXECUTIONPROGRESSBAR_H
#define INTERFACE_APPLICATION_NETWORKEXECUTIONPROGRESSBAR_H

#include <QObject>
#include <QTextStream>
#include <QProgressBar>
#ifndef Q_MOC_RUN
#include <boost/timer.hpp>
#include <Core/Thread/Mutex.h>
#endif

class QAction;
class QWidgetAction;
class QLabel;

namespace SCIRun {
namespace Gui {

  //TODO: inject this for proper coloring
  class NetworkStatus
  {
  public:
    virtual ~NetworkStatus() {}
    virtual size_t total() const = 0;
    virtual size_t waiting() const = 0;
    virtual size_t errored() const = 0;
    virtual size_t executing() const = 0;
    virtual size_t nonReexecuted() const = 0;
    virtual size_t finished() const = 0;
    virtual size_t unexecuted() const = 0;
  };

  using NetworkStatusPtr = boost::shared_ptr<NetworkStatus>;

  class SCIRunProgressBar : public QProgressBar
  {
    Q_OBJECT
  public:
    explicit SCIRunProgressBar(NetworkStatusPtr status, QWidget* parent = nullptr);
  protected:
    void paintEvent(QPaintEvent*) override;
  private:
    NetworkStatusPtr status_;
  };

class NetworkExecutionProgressBar : public QObject
{
  Q_OBJECT
public:
  NetworkExecutionProgressBar(NetworkStatusPtr status, QWidget* parent);

  QList<QAction*> mainActions() const;
  QList<QAction*> advancedActions() const;

  public Q_SLOTS:
    void updateTotalModules(size_t count);
    void incrementModulesDone(double execTime, const std::string& moduleId);
    void resetModulesDone();
    void displayTimingInfo() const;

private:
  NetworkStatusPtr status_;
  QWidgetAction* barAction_;
  SCIRunProgressBar* progressBar_;
  QWidgetAction* counterAction_;
  QLabel* counterLabel_;
  QAction* timingAction_;
  size_t numModulesDone_;
  size_t totalModules_;
  double totalExecutionTime_;
  Core::Thread::Mutex mutex_;
  boost::timer executionTimer_;
  QString timingLog_;
  QTextStream timingStream_;

  QString counterLabelString() const;
};

}
}
#endif
