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

NoteEditor::NoteEditor(const QString& moduleName, bool positionAdjustable, QWidget* parent) : QDialog(parent), moduleName_(moduleName)
{
  setupUi(this);
  setModal(false);
  setWindowTitle("Note for " + moduleName);
  setVisible(false);

  connect(chooseColorButton_, SIGNAL(clicked()), this, SLOT(changeTextColor()));
  connect(resetColorButton_, SIGNAL(clicked()), this, SLOT(resetTextColor()));
  if (positionAdjustable)
    connect(positionComboBox_, SIGNAL(activated(int)), this, SLOT(changeNotePosition(int)));
  else
  {
    positionComboBox_->setVisible(false);
    positionLabel_->setVisible(false);
  }
  connect(fontSizeComboBox_, SIGNAL(activated(const QString&)), this, SLOT(changeFontSize(const QString&)));

  connect(textEdit_, SIGNAL(textChanged()), this, SLOT(updateNote()));

  connect(buttonBox_->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(resetText()));
  connect(buttonBox_->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(ok()));
  connect(buttonBox_->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(cancel()));

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
  textEdit_->setFontPointSize(size);
  textEdit_->setPlainText(textEdit_->toPlainText());
}

void NoteEditor::changeTextAlignment(const QString& text)
{
  //TODO: only changes one line at a time...may just chuck this option
  Qt::Alignment alignment;
  if (text == "Left")
    alignment = Qt::AlignLeft;
  else if (text == "Center")
    alignment = Qt::AlignHCenter;
  else if (text == "Right")
    alignment = Qt::AlignRight;
  else // text == "Justify")
    alignment = Qt::AlignJustify;
  textEdit_->setAlignment(alignment);
  textEdit_->setPlainText(textEdit_->toPlainText());
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
  textEdit_->blockSignals(false);
}

void NoteEditor::setNoteFontSize(int size)
{
  textEdit_->blockSignals(true);
  fontSizeComboBox_->blockSignals(true);
  textEdit_->setFontPointSize(size);
  textEdit_->setPlainText(textEdit_->toPlainText());
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

    textEdit_->setFontPointSize(size);
    textEdit_->setPlainText(textEdit_->toPlainText());
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
    previousColor_ = textEdit_->textColor();
    textEdit_->setTextColor(color);
    textEdit_->setPlainText(textEdit_->toPlainText());
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
  auto oldColor = textEdit_->textColor();
  textEdit_->setTextColor(previousColor_);
  textEdit_->setPlainText(textEdit_->toPlainText());
  previousColor_ = currentColor_ = oldColor;
}

void NoteEditor::ok()
{
  hide();
}

void NoteEditor::cancel()
{
  textEdit_->setHtml(noteHtmlBackup_);
  fontSizeComboBox_->setCurrentIndex(fontSizeBackup_);
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
  QDialog::showEvent(event);
}
