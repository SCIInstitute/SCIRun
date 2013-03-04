/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <QtGui>
#include <iostream>
#include <Interface/Application/NoteEditor.h>

using namespace SCIRun::Gui;

NoteEditor::NoteEditor(const QString& moduleName, QWidget* parent) : QDialog(parent), moduleName_(moduleName)
{
  setupUi(this);
  setModal(false);
  setWindowTitle("Note for " + moduleName);
  setVisible(false);

  connect(chooseColorButton_, SIGNAL(clicked()), this, SLOT(changeTextColor()));
  connect(resetColorButton_, SIGNAL(clicked()), this, SLOT(resetTextColor()));
  connect(positionComboBox_, SIGNAL(activated(int)), this, SLOT(changeNotePosition(int)));
  connect(fontSizeComboBox_, SIGNAL(activated(const QString&)), this, SLOT(changeFontSize(const QString&)));

  connect(textEdit_, SIGNAL(textChanged()), this, SLOT(updateNote()));

  connect(buttonBox_->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(resetText()));
  connect(buttonBox_->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(ok()));
  connect(buttonBox_->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(cancel()));

  currentNote_.color_ = Qt::black;
  position_ = Default;
}

void NoteEditor::changeNotePosition(int index)
{
  position_ = (NotePosition)index; //TODO: unit test;
  updateNote();
}

//void NoteEditor::changeFont(const QFont& font)
//{
//  std::cout << "changeFont(): "  << font.toString().toStdString() << std::endl;
//  textEdit_->setFont(font);
//}

void NoteEditor::changeFontSize(const QString& text)
{
  std::cout << "changeFontSize(): "  << text.toStdString() << std::endl;
  textEdit_->setFontPointSize(text.toDouble());
  textEdit_->setPlainText(textEdit_->toPlainText());
  //updateNote();
}

void NoteEditor::changeTextColor()
{
  std::cout << "changeTextColor()" << std::endl;
  
  auto oldColor = currentNote_.color_;

  auto newColor = QColorDialog::getColor(oldColor, this, "Choose text color");
  if (newColor.isValid())
  {
    textEdit_->setTextColor(newColor);
    textEdit_->setPlainText(textEdit_->toPlainText());
    previousNote_.color_ = oldColor;
    //GuiLogger::Instance().log("Background color set to " + newColor.name());
    //updateNote();
  }
}

void NoteEditor::resetText()
{
  previousNote_.message_ = textEdit_->toPlainText();
  previousNote_.html_ = textEdit_->toHtml();

  textEdit_->clear();
  //updateNote();
}

void NoteEditor::resetTextColor()
{
  std::cout << "resetTextColor()" << std::endl;
  std::swap(previousNote_.color_, currentNote_.color_);
  textEdit_->setTextColor(currentNote_.color_);
  textEdit_->setPlainText(textEdit_->toPlainText());
  //updateNote();
}

void NoteEditor::ok()
{
  hide();
}

void NoteEditor::cancel()
{
  textEdit_->setHtml(previousNote_.html_);
  hide();
}

void NoteEditor::updateNote()
{
  currentNote_.message_ = textEdit_->toPlainText();
  currentNote_.html_ = textEdit_->toHtml();
  currentNote_.color_ = textEdit_->textColor();
  currentNote_.fontPointSize_ = textEdit_->fontPointSize();
  currentNote_.position_ = position_;
  Q_EMIT noteChanged(currentNote_);
}