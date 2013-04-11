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
#include <Interface/Application/MainWindowCollaborators.h>

using namespace SCIRun::Gui;

void TextEditAppender::log(const QString& message) const 
{
  text_->append(message);
}

void TextEditAppender::error(const std::string& msg) const
{
  log("Error: " + QString::fromStdString(msg));
}

void TextEditAppender::warning(const std::string& msg) const
{
  log("Warning: " + QString::fromStdString(msg));
}

void TextEditAppender::remark(const std::string& msg) const
{
  log("Remark: " + QString::fromStdString(msg));
}

void TextEditAppender::status(const std::string& msg) const
{
  log(QString::fromStdString(msg));
}

QString TreeViewModuleGetter::text() const
{
  return tree_.currentItem()->text(0);
}
bool TreeViewModuleGetter::isModule() const
{
  return tree_.currentItem()->childCount() == 0;
}

NotePosition ComboBoxDefaultNotePositionGetter::position() const
{
  return NotePosition(combo_.currentIndex() + 1);
}
