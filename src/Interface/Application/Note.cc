/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <iostream>
#include <stdexcept>
#include <QtGui>
#include <Core/Logging/Log.h>
#include <Interface/Application/Note.h>
#include <Interface/Application/HasNotes.h>
#include <Interface/Application/NoteEditor.h>
#include <Interface/Application/SCIRunMainWindow.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Logging;

HasNotes::HasNotes(const std::string& name, bool positionAdjustable) : 
  noteEditor_(QString::fromStdString(name), positionAdjustable, 0),
  destroyed_(false)
{
  noteEditor_.setStyleSheet(SCIRunMainWindow::Instance()->styleSheet());
}

HasNotes::~HasNotes()
{
  destroy();
}

void HasNotes::destroy()
{
  if (!destroyed_)
  {
    destroyed_ = true;
  }
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

NoteDisplayHelper::NoteDisplayHelper(NoteDisplayStrategyPtr display) : 
  item_(0), scene_(0), note_(0), 
  notePosition_(Default),
  defaultNotePosition_(Top), //TODO
  displayStrategy_(display),
  destroyed_(false)
{
}

NoteDisplayHelper::~NoteDisplayHelper()
{
  destroy();
}

void NoteDisplayHelper::destroy()
{
  if (!destroyed_)
  {
    if (note_ && scene_)
    {
      scene_->removeItem(note_);
    }
    delete note_;
    destroyed_ = true;
  }
}

void NoteDisplayHelper::updateNoteImpl(const Note& note)
{
  if (!note_)
  {
    setNoteGraphicsContext();
    if (!scene_)
      Log::get() << WARN << "Scene not set, network notes will not be displayed!" << std::endl;
    note_ = new QGraphicsTextItem("", 0, scene_);
  }

  note_->setHtml(note.html_);
  notePosition_ = note.position_;
  updateNotePosition();
  note_->setZValue(item_->zValue() - 1);
}

QPointF NoteDisplayHelper::relativeNotePosition()
{
  if (note_ && item_)
  {
    auto position = notePosition_ == Default ? defaultNotePosition_ : notePosition_;
    note_->setVisible(!(Tooltip == position || None == position));
    item_->setToolTip("");

    return displayStrategy_->relativeNotePosition(item_, note_, position);
  }
  return QPointF();
}

void NoteDisplayHelper::setDefaultNotePositionImpl(NotePosition position)
{
  defaultNotePosition_ = position;
  updateNotePosition();
}

void NoteDisplayHelper::updateNotePosition()
{
  if (note_ && item_)
  {
    auto position = positioner_->currentPosition() + relativeNotePosition();
    note_->setPos(position);
  }
}

Note NoteDisplayHelper::currentNote() const
{
  if (note_)
    return Note(note_->toHtml(), note_->toPlainText(), note_->font().pointSizeF(), notePosition_);
  return Note();
}