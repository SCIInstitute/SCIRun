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


#include <QtGui>
#include <Interface/Modules/Base/HasParserHelpDialog.h>

using namespace SCIRun::Gui;

void ModuleDialogWithParserHelp::popUpParserHelp()
{
  if (!help_)
    help_ = new ParserHelpDialog(this);

  help_->show();
  help_->activateWindow();
  help_->raise();
  help_->move(10,10);
}

void ModuleDialogWithParserHelp::connectParserHelpButton(QPushButton* button)
{
  connect(button, &QPushButton::clicked, this, &ModuleDialogWithParserHelp::popUpParserHelp);
}

ParserHelpDialog::ParserHelpDialog(QWidget* parent) : QDialog(parent)
{
  setupUi(this);
  connect(searchLineEdit_, &QLineEdit::returnPressed, this, &ParserHelpDialog::searchText);
  connect(searchButton_, &QPushButton::clicked, this, &ParserHelpDialog::searchText);
  connect(searchLineEdit_, &QLineEdit::textChanged, this, &ParserHelpDialog::resetFormatting);
}

void ParserHelpDialog::searchText()
{
  if(!textBrowser_->find(searchLineEdit_->text()))
  {
    auto cursor = textBrowser_->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    textBrowser_->setTextCursor(cursor);
    if(!textBrowser_->find(searchLineEdit_->text()))
      searchLineEdit_->setStyleSheet("QLineEdit#searchLineEdit_{color:red}");
  }
  return;
}

void ParserHelpDialog::resetFormatting(const QString& text)
{
  searchLineEdit_->setStyleSheet(styleSheet());
  if(text.size() == 0)
    textBrowser_->find("");
}
