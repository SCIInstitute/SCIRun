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


#include <iostream>
#include <stdexcept>
#include <Interface/qt_include.h>
#include <Core/Logging/Log.h>
#include <Interface/Application/Note.h>
#include <Interface/Application/HasNotes.h>
#include <Interface/Application/NoteEditor.h>
#include <Interface/Application/MainWindowCollaborators.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Logging;

HasNotes::HasNotes(const std::string& name, bool positionAdjustable) :
  noteEditor_(QString::fromStdString(name), positionAdjustable, 0)
{
  noteEditor_.setStyleSheet(scirunStylesheet());
}

void HasNotes::connectNoteEditorToAction(QAction* action)
{
  QObject::connect(action, SIGNAL(triggered()), &noteEditor_, SLOT(show()));
  QObject::connect(action, SIGNAL(triggered()), &noteEditor_, SLOT(raise()));
}

void HasNotes::connectUpdateNote(QObject* obj)
{
  QObject::connect(&noteEditor_, SIGNAL(noteChanged(const Note&)), obj, SLOT(updateNote(const Note&)));
  QObject::connect(&noteEditor_, SIGNAL(noteChanged(const Note&)), obj, SIGNAL(noteChanged()));
}

void HasNotes::setCurrentNote(const Note& note, bool updateEditor)
{
  currentNote_ = note;
  if (updateEditor)
  {
    noteEditor_.setNoteHtml(note.html_);
    noteEditor_.setNoteFontSize(note.fontSize_);
  }
}

void HasNotes::setDefaultNoteFontSize(int size)
{
  noteEditor_.setDefaultNoteFontSize(size);
}

NoteDisplayHelper::NoteDisplayHelper(NoteDisplayStrategyPtr display, QGraphicsItem* parent) :
  parent_(parent), note_(nullptr),
  notePosition_(NotePosition::Default),
  defaultNotePosition_(NotePosition::Top), //TODO
  displayStrategy_(display)
{
}

void NoteDisplayHelper::updateNoteImpl(const Note& note)
{
  if (!note_)
  {
    note_ = new QGraphicsTextItem("", parent_);
    note_->setDefaultTextColor(Qt::white);
  }

  note_->setHtml(note.html_);
  notePosition_ = note.position_;
  updateNotePosition();
  note_->setZValue(parent_->zValue() - 1);
}

void NoteDisplayHelper::clearNoteCursor()
{
  if (note_)
  {
    auto cur = note_->textCursor();
    cur.clearSelection();
    note_->setTextCursor(cur);
  }
}

QPointF NoteDisplayHelper::relativeNotePosition()
{
  if (note_ && parent_)
  {
    auto position = notePosition_ == NotePosition::Default ? defaultNotePosition_ : notePosition_;
    note_->setVisible(!(NotePosition::Tooltip == position || NotePosition::None == position));

    return displayStrategy_->relativeNotePosition(parent_, note_, position);
  }
  return QPointF();
}

void NoteDisplayHelper::setDefaultNotePositionImpl(NotePosition position)
{
  defaultNotePosition_ = position;
  updateNotePosition();
}

void NoteDisplayHelper::setDefaultNoteSizeImpl(int size)
{
  defaultNoteFontSize_ = size;
}

void NoteDisplayHelper::updateNotePosition()
{
  if (note_ && parent_)
  {
    note_->setPos(relativeNotePosition());
  }
}

Note NoteDisplayHelper::currentNote() const
{
  if (note_)
    return Note(note_->toHtml(), note_->toPlainText(), note_->font().pointSizeF(), notePosition_);
  return Note();
}
