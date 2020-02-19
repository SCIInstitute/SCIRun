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


#ifndef INTERFACE_APPLICATION_TAGMANAGERWINDOW_H
#define INTERFACE_APPLICATION_TAGMANAGERWINDOW_H

#include "ui_TagManager.h"

namespace SCIRun {
namespace Gui {

class TagManagerWindow : public QDockWidget, public Ui::TagManager
{
	Q_OBJECT

public:
  explicit TagManagerWindow(QWidget* parent = nullptr);
  void setTagNames(const QVector<QString>& names);
  void setTagColors(const QVector<QString>& colors);
  QStringList getTagNames() const { return tagNames_.toList(); }
  QStringList getTagColors() const;
  QColor tagColor(int tag) const;
  QString tagName(int tag) const;
	static void showHelp(QWidget* parent);
public Q_SLOTS:
	void editTagColor();
  void updateTagName(const QString& name);
private Q_SLOTS:
  void helpButtonClicked();
private:
  std::vector<QLineEdit*> tagLineEdits_;
  std::vector<QPushButton*> tagButtons_;
  QVector<QString> tagNames_;
  std::vector<std::string> tagColors_;
};

}
}

#endif
