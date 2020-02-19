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


#include <iostream>
#include <Interface/qt_include.h>
#include <Interface/Application/NetworkExecutionProgressBar.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Thread;

NetworkExecutionProgressBar::NetworkExecutionProgressBar(NetworkStatusPtr status, QWidget* parent) : status_(status), numModulesDone_(0),
  totalModules_(0), totalExecutionTime_(0), mutex_("progress bar"), timingStream_(&timingLog_)
{
  barAction_ = new QWidgetAction(parent);
  barAction_->setDefaultWidget(progressBar_ = new SCIRunProgressBar(status, parent));
  progressBar_->setToolTip("Percentage of completed modules and total execution time");
  progressBar_->setWhatsThis("This displays the percentage of completed modules while the network is executing.");
  progressBar_->setValue(0);
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

QList<QAction*> NetworkExecutionProgressBar::mainActions() const
{
  return QList<QAction*>() << barAction_ << counterAction_;
}

QList<QAction*> NetworkExecutionProgressBar::advancedActions() const
{
  return QList<QAction*>() << timingAction_;
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
    auto wallTime = executionTimer_.elapsed();
    //Green - completed modules\n??? - Unexecuted modules\nRed - errored modules\n
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

void NetworkExecutionProgressBar::displayTimingInfo() const
{
  QApplication::clipboard()->setText(timingLog_);
}

QString NetworkExecutionProgressBar::counterLabelString() const
{
  return QString("  %1 / %2  ").arg(numModulesDone_).arg(totalModules_);
}

SCIRunProgressBar::SCIRunProgressBar(NetworkStatusPtr status, QWidget *parent) : QProgressBar(parent), status_(status)
{

}

void SCIRunProgressBar::paintEvent(QPaintEvent*)
{
  //qDebug() << "Status:" << "\n\ttotal:" << status_->total()
  //  << "\n\twaiting:" << status_->waiting()
  //  << "\n\terrored:" << status_->errored()
  //  << "\n\tnonReexecuted:" << status_->nonReexecuted()
  //  << "\n\tfinished:" << status_->finished()
  //  << "\n\tunexecuted:" << status_->unexecuted();

  int val = value();
  int pos = QStyle::sliderPositionFromValue(minimum(), maximum(), val, width());

  //qDebug() << "pos:" << pos << "value:" << val << "/" << status_->total();

  QPainter p(this);

  {
    //done modules: either green (good) or red (errored)
    //qDebug() << "done modules: green" << status_->finished() << "red" << status_->errored();
    auto finished = status_->finished();
    auto errored = status_->errored();

    auto fracFinished = (static_cast<double>(finished) / (finished + errored)) * pos;

    p.setPen(Qt::green);
    p.setBrush(QBrush(Qt::green));
    p.drawRect(0, 0, std::min((int)fracFinished, pos), height());

    if (errored > 0)
    {
      p.setPen(Qt::red);
      p.setBrush(QBrush(Qt::red));
      p.drawRect(fracFinished, 0, pos - fracFinished, height());
    }
  }

  if (val < maximum())
  {
    //not done modules: either gray (waiting), stripy green (unexecuted), dull green?? (executing)
    //qDebug() << "not done modules: gray" << status_->waiting() << "stripy green" << status_->unexecuted() << "dull green?" << status_->executing()
    //  << "rest:" << status_->finished() << status_->errored();

    auto waiting = status_->waiting();
    auto unexecuted = status_->unexecuted();
    auto executing = status_->executing();
    auto totalNotDone = waiting + unexecuted + executing;
    auto leftToFill = width() - pos;

    if (totalNotDone > 0)
    {
      auto fracExecuting = (static_cast<double>(executing) / totalNotDone) * leftToFill;
      p.setPen(Qt::blue);
      p.setBrush(QBrush(Qt::blue));
      p.drawRect(pos, 0, fracExecuting, height());

      auto fracWaiting = (static_cast<double>(waiting) / totalNotDone) * leftToFill;
      p.setPen(Qt::lightGray);
      p.setBrush(QBrush(Qt::lightGray));
      p.drawRect(pos + fracExecuting, 0, fracWaiting, height());

      auto fracUnexecuted = (static_cast<double>(unexecuted) / totalNotDone) * leftToFill;
      p.setPen(Qt::darkYellow);
      p.setBrush(QBrush(Qt::darkYellow, Qt::BDiagPattern));
      p.drawRect(pos + fracExecuting + fracWaiting, 0, fracUnexecuted, height());
    }
    else
    {
      p.setPen(Qt::lightGray);
      p.setBrush(QBrush(Qt::lightGray));
      p.drawRect(pos, 0, width(), height());
    }
  }

  if (pos != 0 && status_->finished() + status_->unexecuted() == status_->total())
  {
    p.setPen(Qt::green);
    p.setBrush(QBrush(Qt::green));
    p.drawRect(0, 0, width(), height());
    p.setPen(Qt::black);
    p.setBrush(QBrush(Qt::black));
    p.drawText(0, 0, width(), height(), Qt::AlignCenter, "100%");
  }
  else
  {
    p.setPen(Qt::black);
    p.setBrush(QBrush(Qt::black));
    p.drawText(0, 0, width(), height(), Qt::AlignCenter, text());
  }
}
