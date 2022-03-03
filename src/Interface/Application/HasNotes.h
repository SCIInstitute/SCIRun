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


#ifndef INTERFACE_APPLICATION_HAS_NOTES_H
#define INTERFACE_APPLICATION_HAS_NOTES_H

#include <Interface/Application/NoteEditor.h>

class QAction;

namespace SCIRun {
namespace Gui {

  class HasNotesBase
  {
  public:
    HasNotesBase(const std::string& name, bool positionAdjustable);
    void connectNoteEditorToAction(QAction* action);
    void setCurrentNote(const Note& note, bool updateEditor);
    Note getCurrentNote() const { return currentNote_; }
    void setDefaultNoteFontSize(int size);
  protected:
    NoteEditor noteEditor_;
    Note currentNote_;
  };

  template <class ConnectedWidget>
  class HasNotes : public HasNotesBase
  {
  public:
    using HasNotesBase::HasNotesBase;
    void connectUpdateNote(ConnectedWidget* obj);
  };

  template <class ConnectedWidget>
  void HasNotes<ConnectedWidget>::connectUpdateNote(ConnectedWidget* obj)
  {
    QObject::connect(&noteEditor_, &NoteEditor::noteChanged, obj, &ConnectedWidget::updateNote);
    QObject::connect(&noteEditor_, &NoteEditor::noteChanged, obj, &ConnectedWidget::noteChanged);
  }

}
}

#endif
