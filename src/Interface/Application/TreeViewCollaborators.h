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


#ifndef INTERFACE_APPLICATION_TREEVIEWCOLLABORATORS_H
#define INTERFACE_APPLICATION_TREEVIEWCOLLABORATORS_H

#include <Interface/qt_include.h>

namespace SCIRun {
namespace Gui {

  struct GrabNameAndSetFlags
  {
    QStringList nameList_;
    void operator()(QTreeWidgetItem* item);
  };

  template <class Func>
  void visitItem(QTreeWidgetItem* item, Func& itemFunc)
  {
    itemFunc(item);
    for (int i = 0; i < item->childCount(); ++i)
      visitItem(item->child(i), itemFunc);
  }

  template <class Func>
  void visitTree(QTreeWidget* tree, Func& itemFunc)
  {
    for (int i = 0; i < tree->topLevelItemCount(); ++i)
      visitItem(tree->topLevelItem(i), itemFunc);
  }

  struct HideItemsNotMatchingString
  {
    enum SearchType
    {
      STARTS_WITH,
      WILDCARDS,
      FUZZY_SEARCH,
      HIDE_NON_UI
    };

    explicit HideItemsNotMatchingString(SearchType searchType, const QString& pattern);
    QRegExp match_;
    QString start_;
    SearchType searchType_;

    void operator()(QTreeWidgetItem* item);
    bool shouldHide(QTreeWidgetItem* item);
    bool fuzzySearchAllPatterns(const QString& text, const QString& pattern);
    bool fuzzySearch(const std::string& text, const std::string& pattern);
    std::string removeAllSpecialCharacters(const std::string& str);
  };

  struct ShowAll
  {
    void operator()(QTreeWidgetItem* item);
  };

}
}
#endif
