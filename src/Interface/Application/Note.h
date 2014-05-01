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

#ifndef INTERFACE_APPLICATION_NOTE_H
#define INTERFACE_APPLICATION_NOTE_H

#include <boost/shared_ptr.hpp>
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

  enum NotePosition
  {
    Default,
    None,
    Tooltip,
    Top,
    Left, 
    Right, 
    Bottom
  };

  struct Note
  {
    QString html_;
    NotePosition position_; 
  };

  class HasNotes 
  {
  public:
    explicit HasNotes(const std::string& name);
    virtual ~HasNotes();
    void connectNoteEditorToAction(QAction* action);
    void connectUpdateNote(QObject* obj);
    void setCurrentNote(const Note& note) { currentNote_ = note; }
  private:
    class NoteEditor* noteEditor_;
    Note currentNote_;
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
    virtual ~NoteDisplayHelper();
  protected:
    explicit NoteDisplayHelper(NoteDisplayStrategyPtr display);
    virtual void setNoteGraphicsContext() = 0;
    void updateNoteImpl(const Note& note);
    void updateNotePosition();
    void setDefaultNotePositionImpl(NotePosition position);
    QGraphicsItem* item_;
    QGraphicsScene* scene_;
  private:
    QGraphicsTextItem* note_;
    NotePosition notePosition_, defaultNotePosition_;
    NoteDisplayStrategyPtr displayStrategy_;

    QPointF relativeNotePosition();
  };
}
}

#endif
