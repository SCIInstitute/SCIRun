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


#include <Interface/qt_include.h>
#include <iostream>
#include <Interface/Application/NoteEditor.h>

using namespace SCIRun::Gui;

// Applies a character format to all text in the document without
// destroying existing formatting (unlike the setPlainText(toPlainText()) hack).
static void applyFormatToAll(QTextEdit* edit, const QTextCharFormat& format)
{
  QTextCursor cursor = edit->textCursor();
  cursor.select(QTextCursor::Document);
  cursor.mergeCharFormat(format);
  edit->setTextCursor(cursor);
}

NoteEditor::NoteEditor(const QString& moduleName, bool positionAdjustable, QWidget* parent) : QDialog(parent), moduleName_(moduleName)
{
  setupUi(this);
  setModal(false);
  setWindowTitle("Note for " + moduleName);
  setVisible(false);

  connect(chooseColorButton_, &QPushButton::clicked, this, &NoteEditor::changeTextColor);
  connect(resetColorButton_, &QPushButton::clicked, this, &NoteEditor::resetTextColor);
  if (positionAdjustable)
    connect(positionComboBox_, qOverload<int>(&QComboBox::activated), this, &NoteEditor::changeNotePosition);
  else
  {
    positionComboBox_->setVisible(false);
    positionLabel_->setVisible(false);
  }
  connect(fontSizeComboBox_, COMBO_BOX_ACTIVATED_STRING, this, &NoteEditor::changeFontSize);

  connect(textEdit_, &QTextEdit::textChanged, this, &NoteEditor::updateNote);

  connect(buttonBox_->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &NoteEditor::resetText);
  connect(buttonBox_->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &NoteEditor::ok);
  connect(buttonBox_->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &NoteEditor::cancel);

  //TODO: settable notes
  previousColor_ = Qt::white;
  position_ = NotePosition::Default;
}

void NoteEditor::changeNotePosition(int index)
{
  position_ = static_cast<NotePosition>(index); //TODO: unit test;
  updateNote();
}

void NoteEditor::changeFontSize(const QString& text)
{
  double size;
  if (text == "Default")
    size = defaultNoteFontSize_;
  else
    size = text.toDouble();
  QTextCharFormat fmt;
  fmt.setFontPointSize(size);
  applyFormatToAll(textEdit_, fmt);
}

void NoteEditor::changeTextAlignment(const QString& text)
{
  Qt::Alignment alignment;
  if (text == "Left")
    alignment = Qt::AlignLeft;
  else if (text == "Center")
    alignment = Qt::AlignHCenter;
  else if (text == "Right")
    alignment = Qt::AlignRight;
  else // text == "Justify")
    alignment = Qt::AlignJustify;
  QTextCursor cursor = textEdit_->textCursor();
  cursor.select(QTextCursor::Document);
  QTextBlockFormat blockFmt;
  blockFmt.setAlignment(alignment);
  cursor.mergeBlockFormat(blockFmt);
  textEdit_->setTextCursor(cursor);
}

void NoteEditor::changeTextColor()
{
  previousColor_ = currentColor_;
  currentColor_ = QColorDialog::getColor(currentColor_, this, "Choose text color");
  setNoteColor(currentColor_);
}

void NoteEditor::setNoteHtml(const QString& text)
{
  textEdit_->blockSignals(true);
  textEdit_->setHtml(text);
  // Sync color tracking state from the loaded HTML so that Reset Color and
  // Cancel work correctly against the actual loaded color, not Qt::white.
  QTextCursor cursor = textEdit_->textCursor();
  cursor.movePosition(QTextCursor::Start);
  const auto loadedColor = cursor.charFormat().foreground().color();
  if (loadedColor.isValid())
    currentColor_ = previousColor_ = loadedColor;
  textEdit_->blockSignals(false);
}

void NoteEditor::setNoteFontSize(int size)
{
  textEdit_->blockSignals(true);
  fontSizeComboBox_->blockSignals(true);
  QTextCharFormat fmt;
  fmt.setFontPointSize(size);
  applyFormatToAll(textEdit_, fmt);
  int index = fontSizeComboBox_->findText(QString::number(size));
  if (index != -1)
    fontSizeComboBox_->setCurrentIndex(index);
  textEdit_->blockSignals(false);
  fontSizeComboBox_->blockSignals(false);
}

void NoteEditor::setDefaultNoteFontSize(int size)
{
  callCount_++;
  defaultNoteFontSize_ = size;

  if (fontSizeComboBox_->currentText() == "Default")
  {
    textEdit_->blockSignals(true);
    QTextCharFormat fmt;
    fmt.setFontPointSize(size);
    applyFormatToAll(textEdit_, fmt);
    currentNote_.html_ = textEdit_->toHtml();
    if (callCount_ > 1)
      updateNote();

    textEdit_->blockSignals(false);
  }
}

void NoteEditor::setNoteColor(const QColor& color)
{
  if (color.isValid())
  {
    QTextCharFormat fmt;
    fmt.setForeground(QBrush(color));
    applyFormatToAll(textEdit_, fmt);
    updateNote();
  }
  else
  {
    currentColor_ = previousColor_;
  }
}

void NoteEditor::resetText()
{
  textEdit_->clear();
}

void NoteEditor::resetTextColor()
{
  const auto oldColor = currentColor_;
  QTextCharFormat fmt;
  fmt.setForeground(QBrush(previousColor_));
  applyFormatToAll(textEdit_, fmt);
  previousColor_ = currentColor_ = oldColor;
  updateNote();
}

void NoteEditor::ok()
{
  hide();
}

void NoteEditor::cancel()
{
  textEdit_->setHtml(noteHtmlBackup_);
  fontSizeComboBox_->setCurrentIndex(fontSizeBackup_);
  currentColor_ = previousColor_ = colorBackup_;
  hide();
}

void NoteEditor::updateNote()
{
  currentNote_.html_ = textEdit_->toHtml();
  currentNote_.position_ = position_;
  Q_EMIT noteChanged(currentNote_);
}

void NoteEditor::showEvent(QShowEvent* event)
{
  noteHtmlBackup_ = textEdit_->toHtml();
  fontSizeBackup_ = fontSizeComboBox_->currentIndex();
  colorBackup_ = currentColor_;
  QDialog::showEvent(event);
}
