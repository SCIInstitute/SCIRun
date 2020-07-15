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


#ifndef UTILITY_H
#define UTILITY_H

#include <sstream>
#include <QAction>

namespace SCIRun {

template <class Point>
std::string to_string(const Point& p)
{
  std::ostringstream ostr;
  ostr << "QPoint(" << p.x() << "," << p.y() << ")";
  return ostr.str();
}

namespace Gui
{
  QColor to_color(const std::string& str, int alpha = 255);

  QColor defaultTagColor(int tag);

  using TagColorFunc = std::function<QColor(int)>;
  using PreexecuteFunc = std::function<void()>;
  using TagNameFunc = std::function<QString(int)>;

  QString colorToString(const QColor& color);
  QColor stringToColor(const QString& s);

  QGraphicsEffect* blurEffect(double radius = 2);

  inline QAction* separatorAction(QWidget* parent)
  {
    auto sep = new QAction(parent);
    sep->setSeparator(true);
    return sep;
  }

  inline QAction* disabled(QAction* action)
  {
    if (action)
      action->setEnabled(false);
    return action;
  }

  inline std::ostream& operator<<(std::ostream& o, const QPointF& p)
  {
    return o << "[" << p.x() << "," << p.y() << "]";
  }

  typedef boost::function<bool(const Dataflow::Networks::ModuleDescription&)> ModulePredicate;
  typedef boost::function<void(QAction*)> QActionHookup;
  QList<QAction*> fillMenuWithFilteredModuleActions(QMenu* menu, const Dataflow::Networks::ModuleDescriptionMap& moduleMap, ModulePredicate modulePred, QActionHookup hookup, QWidget* parent);
  bool portTypeMatches(const std::string& portTypeToMatch, bool isInput, const Dataflow::Networks::ModuleDescription& module);
  QPointF findCenterOfNetwork(const Dataflow::Networks::ModulePositions& positions);

  const Qt::GlobalColor CLIPBOARD_COLOR = Qt::cyan;

  // arbitrary values
  enum TagValues
  {
    MinTag = 0,
    MaxTag = 9,
    NumberOfTags = 10,
    TagDataKey = 123,
    TagLayerKey = 100,
    CurrentTagKey = 101,
    NoTag = -1,
    AllTags = -50,
    ClearTags = -77,
    ShowGroups = -100,
    HideGroups = -101
  };

  inline bool validTag(int tag) { return MinTag <= tag && tag <= MaxTag; }
}

}

#endif
