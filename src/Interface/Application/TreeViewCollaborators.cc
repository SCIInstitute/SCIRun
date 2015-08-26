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

#include <Interface/Application/TreeViewCollaborators.h>

using namespace SCIRun::Gui;

void GrabNameAndSetFlags::operator()(QTreeWidgetItem* item)
{
  nameList_ << item->text(0) + "," + QString::number(item->childCount());
  if (item->childCount() != 0)
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

HideItemsNotMatchingString::HideItemsNotMatchingString(bool useRegex, const QString& pattern) : match_("*" + pattern + "*", Qt::CaseInsensitive, QRegExp::Wildcard), start_(pattern), useRegex_(useRegex) {}

void HideItemsNotMatchingString::operator()(QTreeWidgetItem* item)
{
  if (item)
  {
    if (0 == item->childCount())
    {
      item->setHidden(shouldHide(item));
      if (!shouldHide(item))
      {
        if (item->parent())
        {
          item->parent()->setExpanded(true);
          if (item->parent()->parent())
          {
            item->parent()->parent()->setExpanded(true);
          }
        }
      }
    }
    else
    {
      bool shouldHideCategory = true;
      for (int i = 0; i < item->childCount(); ++i)
      {
        auto child = item->child(i);
        if (!child->isHidden())
        {
          shouldHideCategory = false;
          break;
        }
      }
      item->setHidden(shouldHideCategory);
    }
  }
}

bool HideItemsNotMatchingString::shouldHide(QTreeWidgetItem* item)
{
  auto text = item->text(0);
  if (useRegex_)
    return !match_.exactMatch(text);
  return !text.startsWith(start_, Qt::CaseInsensitive);
}

void ShowAll::operator()(QTreeWidgetItem* item)
{
  item->setHidden(false);
}
