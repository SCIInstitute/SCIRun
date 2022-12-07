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


#include <Interface/Application/ShortcutsInterface.h>

using namespace SCIRun;
using namespace SCIRun::Gui;

ShortcutsInterface::ShortcutsInterface(QWidget* parent) : QDialog(parent)
{
  setupUi( this );

  Qt::WindowFlags flags = windowFlags();
  Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
  flags = flags & ( ~helpFlag );
  setWindowFlags( flags );

  updateFontsAndText();
  if (minimumWidth() > 0 && minimumHeight() > 0)
  {
    setFixedSize(minimumWidth(), minimumHeight());
  }
}

void ShortcutsInterface::updateFontsAndText()
{
// ShortCut keys on Mac are different: CTRL becomes command key and we need to fix the font size
// on mac as it is too large. Default fonts on Mac and Windows are set differently.
// This function fixes the appearance

  for (auto& child : findChildren<QLabel*>())
  {
    QString text_temp = child->text();
#ifdef __APPLE__
    const QString splat = QChar(0x2318);
    text_temp.replace("CTRL", splat, Qt::CaseInsensitive );
    const QString shift = QChar(0x21E7);
    text_temp.replace("SHIFT", shift, Qt::CaseInsensitive );
    text_temp.replace("META_KEY", "Ctrl", Qt::CaseInsensitive );
    const QString option = QChar(0x2325);
    text_temp.replace("ALT", option, Qt::CaseInsensitive );
    text_temp.replace("+", "", Qt::CaseInsensitive );

    QFont font = child->font();
    font.setPointSize( 11 );
    child->setFont( font );
#else
    text_temp.replace("META_KEY", "Caps Lock", Qt::CaseInsensitive );
    text_temp.replace("CTRL", "Ctrl", Qt::CaseInsensitive);
    text_temp.replace("SHIFT", "Shift", Qt::CaseInsensitive);
    text_temp.replace("ALT", "Alt", Qt::CaseInsensitive);
#endif
    text_temp.replace("PLUS", "+", Qt::CaseInsensitive);
    child->setText( text_temp );
  }

}
