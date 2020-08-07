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


#ifndef INTERFACE_APPLICATION_NOTE_H
#define INTERFACE_APPLICATION_NOTE_H

#include <Interface/Application/PositionProvider.h>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <QString>
#include <QFont>
#include <QColor>
#include <QPointF>

class QAction;
class QGraphicsItem;
class QGraphicsTextItem;
class QGraphicsScene;

namespace SCIRun {
namespace Gui {

  // TODO: refactor. Combine with ModuleNoteXML, and bring together various Note-related classes to support consistent read/write.
  // IDEA: subclass QGraphicsTextItem properly and add a way to associate with a HasNotes object (either module or connection).
  // Then serialization will be uniform for all notes.

  struct Note
  {
    QString html_;
    QString plainText_;
    int fontSize_;
    NotePosition position_;
    Note() : fontSize_(-1), position_(NotePosition::Default) {}
    Note(const QString& html, const QString& plain, int font, NotePosition pos) : html_(html), plainText_(plain), fontSize_(font), position_(pos) {}
  };

  class NoteDisplayStrategy
  {
  public:
    virtual ~NoteDisplayStrategy() {}
    virtual QPointF relativeNotePosition(QGraphicsItem* item, const QGraphicsTextItem* note, NotePosition position) const = 0;
  };

  typedef boost::shared_ptr<NoteDisplayStrategy> NoteDisplayStrategyPtr;
  class ModuleWidgetNoteDisplayStrategy;
  class ConnectionNoteDisplayStrategy;

  class NoteDisplayHelper
  {
  public:
    Note currentNote() const;
  protected:
    NoteDisplayHelper(NoteDisplayStrategyPtr display, QGraphicsItem* parent);
    void updateNoteImpl(const Note& note);
    void updateNotePosition();
    void setDefaultNotePositionImpl(NotePosition position);
    void setDefaultNoteSizeImpl(int size);
    void clearNoteCursor();
    QGraphicsItem* parent_;
  private:
    QGraphicsTextItem* note_;
    NotePosition notePosition_, defaultNotePosition_;
    int defaultNoteFontSize_{ 20 };
    NoteDisplayStrategyPtr displayStrategy_;

    QPointF relativeNotePosition();
  };
}
}

#endif
