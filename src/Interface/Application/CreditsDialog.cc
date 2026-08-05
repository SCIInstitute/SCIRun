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

#include <Interface/Application/CreditsDialog.h>
#include <Core/Application/Application.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core;

namespace
{
  const int rollIntervalMs = 20;   // ~50 px/second at one pixel per tick
  const int rollStepPixels = 1;
  const QSize dialogSize(560, 460);

  QString creditsResourceText()
  {
    QFile file(":/general/Resources/contributors.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      return {};
    return QString::fromUtf8(file.readAll());
  }

  QString escaped(const QString& text)
  {
    return text.toHtmlEscaped();
  }

  /// Turns the contributors.txt line format (see that file's header) into the
  /// rich text the roll displays.
  QString creditsHtml()
  {
    QString body;
    for (const auto& raw : creditsResourceText().split('\n'))
    {
      const auto line = raw.trimmed();
      if (line.startsWith('#'))
        continue;
      if (line.isEmpty())
        body += "<div style=\"font-size:8pt;\">&nbsp;</div>";
      else if (line.startsWith("::"))
        body += "<div style=\"font-size:13pt; font-weight:bold; color:#7fb2ff; margin-top:24px; margin-bottom:10px;\">"
          + escaped(line.mid(2).trimmed()) + "</div>";
      else
        body += "<div style=\"font-size:11pt; margin:2px;\">" + escaped(line) + "</div>";
    }

    const auto version = escaped(QString::fromStdString(Application::Instance().version()));

    return "<div align=\"center\" style=\"color:#f0f0f0;\">"
      "<div style=\"font-size:26pt; font-weight:bold;\">SCIRun</div>"
      "<div style=\"font-size:10pt; color:#a0a0a0;\">Version " + version + "</div>"
      "<div style=\"font-size:10pt; color:#a0a0a0;\">Scientific Computing and Imaging Institute, University of Utah</div>"
      + body +
      "</div>";
  }
}

CreditsDialog::CreditsDialog(QWidget* parent) : QDialog(parent)
{
  setWindowTitle("SCIRun Credits");
  setFixedSize(dialogSize);

  rollLabel_ = new QLabel(creditsHtml(), this);
  rollLabel_->setTextFormat(Qt::RichText);
  rollLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
  rollLabel_->setWordWrap(true);

  // Blank leading and trailing screens, so names roll in from the bottom edge and
  // off the top one instead of appearing mid-view. These are spacers rather than
  // label margins because a stylesheet overrides a QLabel's contents margins.
  auto roll = new QWidget(this);
  auto rollLayout = new QVBoxLayout(roll);
  rollLayout->setContentsMargins(12, 0, 12, 0);
  rollLayout->addSpacing(dialogSize.height());
  rollLayout->addWidget(rollLabel_);
  rollLayout->addSpacing(dialogSize.height());

  scrollArea_ = new QScrollArea(this);
  scrollArea_->setWidget(roll);
  scrollArea_->setWidgetResizable(true);
  scrollArea_->setFrameShape(QFrame::NoFrame);
  scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea_->setStyleSheet("QScrollArea, QLabel, QWidget { background-color: black; }");
  scrollArea_->setFocusPolicy(Qt::NoFocus);  // keep key presses on the dialog
  scrollArea_->viewport()->installEventFilter(this);

  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(scrollArea_);

  rollTimer_ = new QTimer(this);
  rollTimer_->setInterval(rollIntervalMs);
  connect(rollTimer_, &QTimer::timeout, this, &CreditsDialog::advanceRoll);
}

void CreditsDialog::showEvent(QShowEvent* event)
{
  QDialog::showEvent(event);
  scrollArea_->verticalScrollBar()->setValue(0);
  setRolling(true);
}

void CreditsDialog::advanceRoll()
{
  auto bar = scrollArea_->verticalScrollBar();
  if (bar->value() >= bar->maximum())
    bar->setValue(0);  // loop, the way a DVD menu would
  else
    bar->setValue(bar->value() + rollStepPixels);
}

void CreditsDialog::setRolling(bool rolling)
{
  rolling_ = rolling;
  if (rolling)
    rollTimer_->start();
  else
    rollTimer_->stop();
}

void CreditsDialog::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Space)
  {
    setRolling(!rolling_);
    event->accept();
    return;
  }
  QDialog::keyPressEvent(event);  // Esc still closes
}

bool CreditsDialog::eventFilter(QObject* watched, QEvent* event)
{
  // The scroll area's viewport, not the dialog, receives these.
  if (watched == scrollArea_->viewport())
  {
    if (event->type() == QEvent::MouseButtonPress)
      setRolling(!rolling_);
    else if (event->type() == QEvent::Wheel)
      setRolling(false);  // hand the roll over to the reader
  }
  return QDialog::eventFilter(watched, event);  // let the scroll area still handle it
}
