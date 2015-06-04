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

#include <QtGui>
#include <Interface/Application/TagManagerWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/Utility.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

using namespace SCIRun::Gui;

namespace 
{
  const char* tagIndexProperty = "tagIndex";
}

TagManagerWindow::TagManagerWindow(QWidget* parent /* = 0 */) : QDockWidget(parent)
{
  setupUi(this);

  tagButtons_ = { tagPushButton0_, tagPushButton1_, tagPushButton2_,
    tagPushButton3_, tagPushButton4_, tagPushButton5_,
    tagPushButton6_, tagPushButton7_, tagPushButton8_, tagPushButton9_ };
  tagLineEdits_ = { taglineEdit_0, taglineEdit_1, taglineEdit_2,
    taglineEdit_3, taglineEdit_4, taglineEdit_5,
    taglineEdit_6, taglineEdit_7, taglineEdit_8, taglineEdit_9 };

  for (int i = 0; i < NumberOfTags; ++i)
  {
    tagButtons_[i]->setProperty(tagIndexProperty, i);
    connect(tagButtons_[i], SIGNAL(clicked()), this, SLOT(editTagColor()));
    tagLineEdits_[i]->setProperty(tagIndexProperty, i);
    connect(tagLineEdits_[i], SIGNAL(textChanged(const QString&)), this, SLOT(updateTagName(const QString&)));
  }

  tagNames_.resize(NumberOfTags);
  tagColors_.resize(NumberOfTags);
}

void TagManagerWindow::editTagColor()
{
  auto tag = sender()->property(tagIndexProperty).toInt();
  auto newColor = QColorDialog::getColor(tagColors_[tag], this, "Choose tag " + QString::number(tag) + " color");
  auto colorStr = colorToString(newColor);
  qobject_cast<QPushButton*>(sender())->setStyleSheet("background-color : " + colorStr + ";");
  tagColors_[tag] = colorStr;
  //TODO next: propagate to tag color manager class. 
}

void TagManagerWindow::setTagNames(const QVector<QString>& names)
{
  tagNames_ = names;
  for (int i = 0; i < NumberOfTags; ++i)
  {
    tagLineEdits_[i]->setText(names[i]);
  }
}

void TagManagerWindow::updateTagName(const QString& name)
{
  tagNames_[sender()->property(tagIndexProperty).toInt()] = name;
}

void TagManagerWindow::setTagColors(const QVector<QString>& colors)
{ 
  for (int i = 0; i < NumberOfTags; ++i)
  {
    if (i >= colors.size() || colors[i].isEmpty())
      tagColors_[i] = colorToString(defaultTagColor(i));
    else
      tagColors_[i] = colors[i];
    tagButtons_[i]->setStyleSheet("background-color : " + tagColors_[i] + ";");
  }
}

QColor TagManagerWindow::tagColor(int tag) const
{
  //rgb(128, 128, 0)
  auto colorStr = tagColors_[tag];
  int r, g, b;
  try
  {
    static boost::regex reg("rgb\\((.+), (.+), (.+)\\)");
    boost::smatch what;
    regex_match(colorStr.toStdString(), what, reg);
    r = boost::lexical_cast<int>(what[1]);
    g = boost::lexical_cast<int>(what[2]);
    b = boost::lexical_cast<int>(what[3]);
  }
  catch (...)
  {
    //error results in gray 
    r = g = b = 155;
  }
  return QColor(r, g, b);
}