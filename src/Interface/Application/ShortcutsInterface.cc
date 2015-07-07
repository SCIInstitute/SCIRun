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

#include <Interface/Application/ShortcutsInterface.h>

using namespace SCIRun;
using namespace SCIRun::Gui;

ShortcutsInterface::ShortcutsInterface(QWidget* parent) : QDialog(parent)
{
  setupUi( this );
  
  /*
  std::string title = std::string( "Keyboard/Mouse Shortcuts - "  )
    + Core::Application::GetApplicationNameAndVersion();
  setWindowTitle( QString::fromStdString( title ) );
    */
  Qt::WindowFlags flags = windowFlags();
  Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
  flags = flags & ( ~helpFlag );
  setWindowFlags( flags );
  
  
  updateFontsAndText();
  
  
  // Python Shortcut visible
#ifdef BUILD_WITH_PYTHON
  this->private_->ui_.python_shortcut_label_->show();
#endif
  
}

void ShortcutsInterface::updateFontsAndText()
{

// ShortCut keys on Mac are different: CTRL becomes command key and we need to fix the font size
// on mac as it is too large. Default fonts on Mac and Windows are set differently.
// This function fixes the appearance
#ifdef __APPLE__
  QList<QLabel*> children = findChildren< QLabel* >();
  QList<QLabel*>::iterator it = children.begin();
  QList<QLabel*>::iterator it_end = children.end();
  
  while( it != it_end )
  {
    QString text_temp = ( *it )->text();
    text_temp.replace( QString::fromUtf8( "CTRL" ), QString::fromUtf8( "COMMAND" ), 
      Qt::CaseInsensitive );
    ( *it )->setText( text_temp );
    QFont font = ( *it )->font();
    font.setPointSize( 11 );
    ( *it )->setFont( font );
    ++it;
  }
#endif
}
