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

#ifndef INTERFACE_APPLICATION_NETWORKEXECUTIONPROGRESSBAR_H
#define INTERFACE_APPLICATION_NETWORKEXECUTIONPROGRESSBAR_H

#include <QObject>
#include <QTextStream>
#ifndef Q_MOC_RUN
#include <boost/timer.hpp>
#include <Core/Thread/Mutex.h>
#endif

namespace SCIRun {
namespace Gui {

class NetworkExecutionProgressBar : public QObject
{
  Q_OBJECT
public:
  explicit NetworkExecutionProgressBar(QWidget* parent);

  QList<class QAction*> actions() const;

  public Q_SLOTS:
    void updateTotalModules(size_t count);
    void incrementModulesDone(double execTime, const std::string& moduleId);
    void resetModulesDone();
    void displayTimingInfo();

private:
  class QWidgetAction* barAction_;
  class QProgressBar* progressBar_;
  class QWidgetAction* counterAction_;
  class QLabel* counterLabel_;
  class QAction* timingAction_;
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
