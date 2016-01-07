/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifdef BUILD_WITH_PYTHON
#include <iostream>
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QPointer>
#include <QTextCursor>
#include <QTextEdit>
#include <QScrollBar>
#include <QVBoxLayout>

#include <Core/Python/PythonInterpreter.h>
#include <Interface/Application/PythonConsoleWidget.h>

using namespace SCIRun::Core;
using namespace SCIRun::Gui;

class PythonConsoleEdit;
typedef QPointer< PythonConsoleEdit > PythonConsoleEditQWeakPointer;

class PythonConsoleEdit : public QTextEdit
{
public:
  explicit PythonConsoleEdit(PythonConsoleWidget* parent);

  virtual void keyPressEvent(QKeyEvent* e);
  //virtual void focusOutEvent( QFocusEvent* e );

  const int document_end();
  QString& command_buffer();
  void update_command_buffer();
  void replace_command_buffer(const QString& text);
  void issue_command();

  void prompt(const std::string& text) { promptImpl(QString::fromStdString(text)); }
  void print_output(const std::string& text){ print_outputImpl(QString::fromStdString(text)); }
  void print_error(const std::string& text){ print_errorImpl(QString::fromStdString(text)); }
  void print_command(const std::string& text){ print_commandImpl(QString::fromStdString(text)); }

private:
  void promptImpl(const QString& text);
  void print_outputImpl(const QString& text);
  void print_errorImpl(const QString& text);
  void print_commandImpl(const QString& text);

public:
  // The beginning of the area of interactive input, outside which
  // changes can't be made to the text edit contents.
  int interactive_position_;
  // Command history plus the current command buffer
  QStringList command_history_;
  // Current position in the command history
  int command_position_;

  //public:
  //	static void Prompt( PythonConsoleEditQWeakPointer edit, const std::string& text );
  //	static void PrintOutput( PythonConsoleEditQWeakPointer edit, const std::string& text );
  //	static void PrintError( PythonConsoleEditQWeakPointer edit, const std::string& text );
  //	static void PrintCommand( PythonConsoleEditQWeakPointer edit, const std::string& text );
};

PythonConsoleEdit::PythonConsoleEdit(PythonConsoleWidget* parent) :
QTextEdit(parent)
{
  this->interactive_position_ = this->document_end();
  this->setTabChangesFocus(false);
  this->setAcceptDrops(false);
  this->setAcceptRichText(false);
  this->setUndoRedoEnabled(false);
  //this->setLineWrapMode( QTextEdit::NoWrap );
  this->document()->setMaximumBlockCount(2048);

  QFont f;
  f.setFamily("Courier");
  f.setStyleHint(QFont::TypeWriter);
  f.setFixedPitch(true);

  // Set the tab width to 4 spaces
  QFontMetrics fm(f, this);
  this->setTabStopWidth(fm.width("    "));

  QTextCharFormat format;
  format.setFont(f);
  format.setForeground(QColor(0, 0, 0));
  this->setCurrentCharFormat(format);

  this->command_history_.append("");
  this->command_position_ = 0;
  setStyleSheet("background-color: lightgray");
}

void PythonConsoleEdit::keyPressEvent(QKeyEvent* e)
{
  this->setTextColor(Qt::black);
  QTextCursor text_cursor = this->textCursor();

  // Whether there's a current selection
  const bool selection = text_cursor.anchor() != text_cursor.position();
  // Whether the cursor overlaps the history area
  const bool history_area = text_cursor.anchor() < this->interactive_position_ ||
    text_cursor.position() < this->interactive_position_;

  // Allow copying anywhere in the console ...
  if (e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier)
  {
    if (selection)
    {
      this->copy();
    }
    else
    {
      //Core::PythonInterpreter::Instance()->interrupt();
    }
    e->accept();
    return;
  }

  // Allow cut only if the selection is limited to the interactive area ...
  if (e->key() == Qt::Key_X && e->modifiers() == Qt::ControlModifier)
  {
    if (selection && !history_area)
    {
      this->cut();
    }
    e->accept();
    return;
  }

  // Allow paste only if the selection is in the interactive area ...
  if (e->key() == Qt::Key_V && e->modifiers() == Qt::ControlModifier)
  {
    if (!history_area)
    {
      const QMimeData* const clipboard = QApplication::clipboard()->mimeData();
      const QString text = clipboard->text();
      if (!text.isNull())
      {
        text_cursor.insertText(text);
        this->update_command_buffer();
      }
    }

    e->accept();
    return;
  }

  // Force the cursor back to the interactive area
  if (history_area && e->key() != Qt::Key_Control)
  {
    text_cursor.setPosition(this->document_end());
    this->setTextCursor(text_cursor);
  }

  switch (e->key())
  {
  case Qt::Key_Up:
    e->accept();
    if (this->command_position_ > 0)
    {
      this->replace_command_buffer(this->command_history_[--this->command_position_]);
    }
    break;

  case Qt::Key_Down:
    e->accept();
    if (this->command_position_ < this->command_history_.size() - 2)
    {
      this->replace_command_buffer(this->command_history_[++this->command_position_]);
    }
    else
    {
      this->command_position_ = this->command_history_.size() - 1;
      this->replace_command_buffer("");
    }
    break;

  case Qt::Key_Left:
    if (text_cursor.position() > this->interactive_position_)
    {
      QTextEdit::keyPressEvent(e);
    }
    else
    {
      e->accept();
    }
    break;

  case Qt::Key_Delete:
    e->accept();
    QTextEdit::keyPressEvent(e);
    this->update_command_buffer();
    break;

  case Qt::Key_Backspace:
    e->accept();
    if (text_cursor.position() > this->interactive_position_)
    {
      QTextEdit::keyPressEvent(e);
      this->update_command_buffer();
    }
    break;

  case Qt::Key_Tab:
    e->accept();
    QTextEdit::keyPressEvent(e);
    break;

  case Qt::Key_Home:
    e->accept();
    text_cursor.setPosition(this->interactive_position_);
    this->setTextCursor(text_cursor);
    break;

  case Qt::Key_Return:
  case Qt::Key_Enter:
    e->accept();
    text_cursor.setPosition(this->document_end());
    this->setTextCursor(text_cursor);
    this->issue_command();
    break;

  default:
    e->accept();
    QTextEdit::keyPressEvent(e);
    this->update_command_buffer();
    break;
  }
}

const int PythonConsoleEdit::document_end()
{
  QTextCursor c(this->document());
  c.movePosition(QTextCursor::End);
  return c.position();
}

QString& PythonConsoleEdit::command_buffer()
{
  return this->command_history_.back();
}

void PythonConsoleEdit::update_command_buffer()
{
  this->command_buffer() = this->toPlainText().mid(this->interactive_position_);
}

void PythonConsoleEdit::replace_command_buffer(const QString& text)
{
  this->command_buffer() = text;

  QTextCursor c(this->document());
  c.setPosition(this->interactive_position_);
  c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
  c.removeSelectedText();

  QTextCharFormat char_format = this->currentCharFormat();
  char_format.setForeground(Qt::black);
  c.setCharFormat(char_format);
  c.insertText(text);
}

void PythonConsoleEdit::issue_command()
{
  QString command = this->command_buffer();
  // Update the command history.
  if (!command.isEmpty())
  {
    this->command_history_.push_back("");
    this->command_position_ = this->command_history_.size() - 1;
  }

  QTextCursor c(this->document());
  c.movePosition(QTextCursor::End);
  c.insertText("\n");

  this->interactive_position_ = this->document_end();
  PythonInterpreter::Instance().run_string(command.toStdString());
}

void PythonConsoleEdit::promptImpl(const QString& text)
{
  QTextCursor text_cursor(this->document());
  // Move the cursor to the end of the document
  text_cursor.setPosition(this->document_end());

  // if the cursor is currently on a clean line, do nothing, otherwise we move
  // the cursor to a new line before showing the prompt.
  text_cursor.movePosition(QTextCursor::StartOfLine);
  int startpos = text_cursor.position();
  text_cursor.movePosition(QTextCursor::EndOfLine);
  int endpos = text_cursor.position();

  // Make sure the new text will be in the right color
  QTextCharFormat char_format = this->currentCharFormat();
  char_format.setForeground(Qt::green);
  text_cursor.setCharFormat(char_format);

  if (endpos != startpos)
  {
    text_cursor.insertText("\n");
  }

  text_cursor.insertText(text);
  this->setTextCursor(text_cursor);
  this->interactive_position_ = this->document_end();
  this->ensureCursorVisible();
}

void PythonConsoleEdit::print_outputImpl(const QString& text)
{
  QTextCursor text_cursor(this->document());
  // Move the cursor to the end of the document
  text_cursor.setPosition(this->document_end());

  // Set the proper text color
  QTextCharFormat char_format = this->currentCharFormat();
  char_format.setForeground(Qt::black);
  text_cursor.setCharFormat(char_format);

  text_cursor.insertText(text);
  this->setTextCursor(text_cursor);
  this->interactive_position_ = this->document_end();
  this->ensureCursorVisible();
}

void PythonConsoleEdit::print_errorImpl(const QString& text)
{
  QTextCursor text_cursor(this->document());
  // Move the cursor to the end of the document
  text_cursor.setPosition(this->document_end());

  // Set the proper text color
  QTextCharFormat char_format = this->currentCharFormat();
  char_format.setForeground(Qt::red);
  text_cursor.setCharFormat(char_format);

  text_cursor.insertText(text);
  this->setTextCursor(text_cursor);
  this->interactive_position_ = this->document_end();
  this->ensureCursorVisible();
}

void PythonConsoleEdit::print_commandImpl(const QString& text)
{
  QTextCursor text_cursor = this->textCursor();
  text_cursor.setPosition(this->document_end());
  this->setTextCursor(text_cursor);
  text_cursor.insertText(text);
  this->update_command_buffer();
  this->ensureCursorVisible();
}

class PythonConsoleWidgetPrivate
{
public:
  PythonConsoleEdit* console_edit_;
};

PythonConsoleWidget::PythonConsoleWidget(QWidget* parent) :
QDockWidget(parent),
private_(new PythonConsoleWidgetPrivate)
{
  this->private_->console_edit_ = new PythonConsoleEdit(this);
  setWidget(this->private_->console_edit_);

  this->setMinimumSize(500, 500);

  setWindowTitle("Python console");

  PythonInterpreter::Instance().prompt_signal_.connect(boost::bind(&PythonConsoleEdit::prompt, private_->console_edit_, _1));
  PythonInterpreter::Instance().output_signal_.connect(boost::bind(&PythonConsoleEdit::print_output, private_->console_edit_, _1));
  PythonInterpreter::Instance().error_signal_.connect(boost::bind(&PythonConsoleEdit::print_error, private_->console_edit_, _1));

  showBanner();
  PythonInterpreter::Instance().run_string("import SCIRunPythonAPI; from SCIRunPythonAPI import *");
}

PythonConsoleWidget::~PythonConsoleWidget()
{
  //this->disconnect_all();
}

void PythonConsoleWidget::showBanner()
{
  PythonInterpreter::Instance().print_banner();
}

#endif