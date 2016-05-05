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

#include <iostream>
#include <QtGui>
#include <Interface/Application/NetworkExecutionProgressBar.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Thread;

NetworkExecutionProgressBar::NetworkExecutionProgressBar(QWidget* parent) : numModulesDone_(0),
  totalModules_(0), totalExecutionTime_(0), mutex_("progress bar"), timingStream_(&timingLog_)
{
  barAction_ = new QWidgetAction(parent);
  barAction_->setDefaultWidget(progressBar_ = new QProgressBar(parent));
  progressBar_->setToolTip("Percentage of completed modules and total execution time");
  progressBar_->setWhatsThis("This displays the percentage of completed modules while the network is executing.");
  barAction_->setVisible(true);

  counterAction_ = new QWidgetAction(parent);
  counterAction_->setDefaultWidget(counterLabel_ = new QLabel(counterLabelString(), parent));
  counterLabel_->setToolTip("modules done executing / total modules");
  counterLabel_->setWhatsThis("This shows the fraction of completed modules while the network is executing.");
  counterAction_->setVisible(true);

  timingAction_ = new QAction(parent);
  timingAction_->setToolTip("Click to copy execution times to clipboard");
  timingAction_->setVisible(true);
  timingAction_->setIcon(QPixmap(":/general/Resources/timepiece-512.png"));
  connect(timingAction_, SIGNAL(triggered()), this, SLOT(displayTimingInfo()));
  timingStream_.setRealNumberPrecision(4);

  progressBar_->setStyleSheet(parent->styleSheet());
}

QList<QAction*> NetworkExecutionProgressBar::actions() const
{
  return QList<QAction*>() << barAction_ << counterAction_ << timingAction_;
}

void NetworkExecutionProgressBar::updateTotalModules(size_t count)
{
  Guard g(mutex_.get());
  if (count != totalModules_)
  {
    totalModules_ = count;
    numModulesDone_ = 0;
    totalExecutionTime_ = 0;
    counterLabel_->setText(counterLabelString());
    if (0 != count)
      progressBar_->setMaximum(count);
    progressBar_->setValue(0);
  }
}
void NetworkExecutionProgressBar::incrementModulesDone(double execTime, const std::string& moduleId)
{
  Guard g(mutex_.get());
  if (numModulesDone_ < totalModules_)
  {
    numModulesDone_++;
    counterLabel_->setText(counterLabelString());
    progressBar_->setValue(numModulesDone_);
    totalExecutionTime_ += execTime;
    double wallTime = executionTimer_.elapsed();
    progressBar_->setToolTip(QString("Total execution time: %1\nTotal wall time: %2")
      .arg(totalExecutionTime_).arg(wallTime));
    timingStream_ << '\t' << moduleId.c_str() << "," << execTime << ',' << totalExecutionTime_
      << ','  << wallTime << '\n';

    if (numModulesDone_ == totalModules_)
      timingStream_ << "TIMING LOG: " << "execution ended at " << QTime::currentTime().toString("hh:mm:ss.zzz") << '\n';
  }
}

void NetworkExecutionProgressBar::resetModulesDone()
{
  Guard g(mutex_.get());
  numModulesDone_ = 0;
  totalExecutionTime_ = 0;
  executionTimer_.restart();
  counterLabel_->setText(counterLabelString());
  progressBar_->setValue(numModulesDone_);
  progressBar_->setToolTip("");
  timingLog_.clear();
  timingStream_ << "TIMING LOG: " << "execution began at " << QTime::currentTime().toString("hh:mm:ss.zzz")
    << "\n\tModule ID,module time,total module time,total wall time\n";
}

void NetworkExecutionProgressBar::displayTimingInfo()
{
  //qDebug() << timingLog_;
  QApplication::clipboard()->setText(timingLog_);
  //QMessageBox::information(nullptr, "Execution timing", timingLog_);
}

QString NetworkExecutionProgressBar::counterLabelString() const
{
  return QString("  %1 / %2  ").arg(numModulesDone_).arg(totalModules_);
}
