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


#include <Interface/qt_include.h>
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
  setVisible(false);
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

  connect(helpPushButton_, SIGNAL(clicked()), this, SLOT(helpButtonClicked()));
  hide();
}

void TagManagerWindow::editTagColor()
{
  auto tag = sender()->property(tagIndexProperty).toInt();
  auto color = QString::fromStdString(tagColors_[tag]);
  auto newColor = QColorDialog::getColor(stringToColor(color), this, "Choose tag " + QString::number(tag) + " color");
  if (newColor.isValid())
  {
    auto colorStr = colorToString(newColor);
    qobject_cast<QPushButton*>(sender())->setStyleSheet("background-color : " + colorStr + ";");
    tagColors_[tag] = colorStr.toStdString();
  }
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
      tagColors_[i] = colorToString(defaultTagColor(i)).toStdString();
    else
      tagColors_[i] = colors[i].toStdString();
    tagButtons_[i]->setStyleSheet("background-color : " + QString::fromStdString(tagColors_[i]) + ";");
  }
}

QStringList TagManagerWindow::getTagColors() const
{
  QStringList qsl;
  std::for_each(tagColors_.cbegin(), tagColors_.cend(), [&qsl](const std::string& str) { qsl.append(QString::fromStdString(str)); });
  return qsl;
}

QColor TagManagerWindow::tagColor(int tag) const
{
  int r, g, b;
  r = g = b = 155;
  if (validTag(tag))
  {
    auto colorStr = tagColors_[tag];
    try
    {
      static boost::regex reg("rgb\\((.+), (.+), (.+)\\)");
      boost::smatch what;
      regex_match(colorStr, what, reg);
      r = boost::lexical_cast<int>(what[1]);
      g = boost::lexical_cast<int>(what[2]);
      b = boost::lexical_cast<int>(what[3]);
    }
    catch (...)
    {
      //error results in gray
    }
  }
  return QColor(r, g, b);
}

QString TagManagerWindow::tagName(int tag) const
{
  auto name = validTag(tag) ? tagNames_[tag] : "[No tag]";
  return name;
}

void TagManagerWindow::showHelp(QWidget* parent)
{
  QMessageBox::information(parent,
    "Module Tag Layer Guide",
    "This layer allows the user to group modules in a network file by tag number. Ten tags are available, labeled 0 - 9. "
    "Each tag's color can be chosen in the Tag Manager window, as well as a descriptive label. Tag colors are a global setting, while module tags are saved in the network file. \n\n"
    "To use, while in the Network Editor, hold down the Alt / Option key. Then press A to see all module tag groups(each module will be colorized "
    "according to the chosen colors). Or press 0 - 9 keys to see each tag group individually; other modules will be slightly blurred out. While in "
    "the single - tag view, you can click a module to toggle it as tagged. There is also a button in the toolbar to view all tagged modules."
    "\n\nOnce tags are being used, tag groups can be toggled using Alt-G (show) and Alt-Shift-G (hide). Boxes of the tag color, labelled with the tag's text, will be displayed overlaying the network. "
    "To display tag groups on network load, double-click on a tag group box and select the display option in the menu. "
    "\n\nTag names saved in the network file can override application-level names--double-click a displayed tag group box and select the rename option."
    );
}

void TagManagerWindow::helpButtonClicked()
{
  showHelp(this);
}
