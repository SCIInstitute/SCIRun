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

QList<QAction*> NetworkExecutionProgressBar::actions() const
{
  return QList<QAction*>() << barAction_ << counterAction_ << timingAction_;
}

void NetworkExecutionProgressBar::updateTotalModules(size_t count)
{
  Guard g(mutex_.get());
  //qDebug() << "updateTotalModules" << numModulesDone_ << totalModules_;
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
  //qDebug() << "~updateTotalModules" << numModulesDone_ << totalModules_;
}
void NetworkExecutionProgressBar::incrementModulesDone(double execTime, const std::string& moduleId)
{
  Guard g(mutex_.get());
  //qDebug() << "incrementModulesDone" << numModulesDone_ << totalModules_;
  if (numModulesDone_ < totalModules_)
  {
    numModulesDone_++;
    counterLabel_->setText(counterLabelString());
    progressBar_->setValue(numModulesDone_);
    totalExecutionTime_ += execTime;
    auto wallTime = executionTimer_.elapsed();
    progressBar_->setToolTip(QString("Green - completed modules\n??? - Unexecuted modules\nRed - errored modules\nTotal execution time: %1\nTotal wall time: %2")
      .arg(totalExecutionTime_).arg(wallTime));
    timingStream_ << '\t' << moduleId.c_str() << "," << execTime << ',' << totalExecutionTime_
      << ','  << wallTime << '\n';

    if (numModulesDone_ == totalModules_)
      timingStream_ << "TIMING LOG: " << "execution ended at " << QTime::currentTime().toString("hh:mm:ss.zzz") << '\n';
  }
  //qDebug() << "~incrementModulesDone" << numModulesDone_ << totalModules_;
}

void NetworkExecutionProgressBar::resetModulesDone()
{
  Guard g(mutex_.get());
  numModulesDone_ = 0;
  //qDebug() << "resetModulesDone" << numModulesDone_ << totalModules_;
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

  qDebug() << "pos:" << pos << "value:" << val << "/" << status_->total();

  QPainter p(this);

  {
    //done modules: either green (good) or red (errored)

    qDebug() << "done modules: green" << status_->finished() << "red" << status_->errored();
    auto finished = status_->finished();
    auto errored = status_->errored();

    auto fracFinished = (static_cast<double>(finished) / (finished + errored)) * pos;

    p.setPen(Qt::green);
    p.setBrush(QBrush(Qt::green));
    p.drawRect(0, 0, std::min((int)fracFinished, pos), height());

    //if (fracFinished < pos)
    //{
      p.setPen(Qt::red);
      p.setBrush(QBrush(Qt::red));
      p.drawRect(fracFinished, 0, pos, height());
    //}
  }

/*
  if (val >= 0 && val <= 3)
  {
    p.drawRect(0, 0, pos, height());
  }
  else if (val > 3 && val <= 6)
  {
    p.drawRect(0, 0, pos60, height());
    p.setPen(QColor(255, 127, 0));
    p.setBrush(QBrush(QColor(255, 127, 0)));
    p.drawRect(pos60, 0, pos - pos60, height());
  }
  else
  {
    p.drawRect(0, 0, pos60, height());
    p.setPen(QColor(255, 127, 0));
    p.setBrush(QBrush(QColor(255, 127, 0)));
    p.drawRect(pos60, 0, pos80 - pos60, height());
    p.setPen(Qt::red);
    p.setBrush(QBrush(Qt::red));
    p.drawRect(pos80, 0, pos - pos80, height());
  }
*/
  {
    //not done modules: either gray (waiting), stripy green (unexecuted), dull green?? (executing)
    qDebug() << "not done modules: gray" << status_->waiting() << "stripy green" << status_->unexecuted() << "dull green?" << status_->executing();

    auto waiting = status_->waiting();
    auto unexecuted = status_->unexecuted();
    auto executing = status_->executing();
    auto totalNotDone = waiting + unexecuted + executing;
    auto leftToFill = width() - pos;

    auto fracExecuting = (static_cast<double>(executing) / totalNotDone) * leftToFill;
    p.setPen(Qt::darkGreen);
    p.setBrush(QBrush(Qt::darkGreen));
    p.drawRect(pos, 0, pos + fracExecuting, height());
    auto fracWaiting = (static_cast<double>(waiting) / totalNotDone) * leftToFill;
    p.setPen(Qt::lightGray);
    p.setBrush(QBrush(Qt::lightGray));
    p.drawRect(pos + fracExecuting, 0, pos + fracExecuting + fracWaiting, height());
    auto fracUnexecuted = (static_cast<double>(unexecuted) / totalNotDone) * leftToFill;
    p.setPen(Qt::darkGray);
    p.setBrush(QBrush(Qt::darkGray));
    p.drawRect(pos + fracExecuting + fracWaiting, 0, width(), height());
    

    
  }

  p.setPen(Qt::black);
  p.setBrush(QBrush(Qt::black));
  p.drawText(0, 0, width(), height(), Qt::AlignCenter, text());
}