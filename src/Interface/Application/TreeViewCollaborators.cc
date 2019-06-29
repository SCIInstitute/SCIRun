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
#include <boost/algorithm/string.hpp>
#include <iostream>

using namespace SCIRun::Gui;

void GrabNameAndSetFlags::operator()(QTreeWidgetItem* item)
{
  nameList_ << item->text(0) + "," + QString::number(item->childCount());
  if (item->childCount() != 0)
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

HideItemsNotMatchingString::HideItemsNotMatchingString(SearchType searchType, const QString& pattern) : match_("*" + pattern + "*", Qt::CaseInsensitive, QRegExp::Wildcard), start_(pattern), searchType_(searchType) {}

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
  if (searchType_ == SearchType::STARTS_WITH)
    return !text.startsWith(start_, Qt::CaseInsensitive);
  else if(searchType_ == SearchType::WILDCARDS
          || boost::contains(start_, "*"))
    return !match_.exactMatch(text);
  else
    return !fuzzySearch(text, start_);
}

bool HideItemsNotMatchingString::fuzzySearch(QString text, QString pattern)
{
  std::string pattern_str = removeAllSpecialCharacters(pattern.toStdString());
  std::vector<std::string> pattern_split;
  boost::split(pattern_split,
               pattern_str,
               [](char c){return c == ' ';});

  // Remove empty vectors
  for(int i = pattern_split.size()-1; i >= 0; i--)
  {
    if(pattern_split[i].empty())
      pattern_split.erase(pattern_split.begin() + i);
  }
  // Checks permutations of the first 8 words given
  if(pattern_split.size() > 8)
    pattern_split.resize(8);

  std::vector<bool> visited(pattern_split.size(), false);

  return fuzzySearchRemainingPatterns(visited, pattern_split, text, 0);
}

std::string HideItemsNotMatchingString::removeAllSpecialCharacters(const std::string& str)
{
  std::string newStr;
  for(char qc : str)
  {
    if(qc == ' ' // Check if space
       || (qc >= 65 && qc <= 90) // Check if upper case char
       || (qc >= 97 && qc <= 122)) // Check if lower case char
    {
      newStr.push_back(qc);
    }
  }
  return newStr;
}

bool HideItemsNotMatchingString::fuzzySearchRemainingPatterns(std::vector<bool>& visited,
                                                              std::vector<std::string>& patternSplit,
                                                              QString& text,
                                                              int textIndex)
{
  // Find current pattern
  std::string currentPattern;
  for(int i = 0; i < visited.size(); i++)
  {
    if(!visited[i])
    {
      visited[i] = true;
      currentPattern = patternSplit[i];

      // Do search
      int patternIndex = 0;
      int currentTextIndex = textIndex;
      bool matched = false;

      for(int t = currentTextIndex; t < text.length(); t++)
      {
        bool isUpperChar = currentPattern[patternIndex] < 97;

        // Counts as a match if letter found
        if((isUpperChar && (text[t] == currentPattern[patternIndex]))
           || (!isUpperChar && (text[t].toLower() == std::tolower(currentPattern[patternIndex]))))
        {
          ++patternIndex;
          currentTextIndex = t;
        }

        if(patternIndex >= currentPattern.length())
        {
          matched = true;
          break;
        }
      }

      // If current matched, check the rest of remaining permutations
      bool remainingMatched = false;
      if(matched)
      {
        remainingMatched = fuzzySearchRemainingPatterns(visited, patternSplit, text, currentTextIndex);
        if(remainingMatched)
          return true;
      }
      visited[i] = false;
    }
  }

  for(int i = 0; i < visited.size(); i++)
  {
    if(!visited[i])
      return false;
  }

  return true;
}

void ShowAll::operator()(QTreeWidgetItem* item)
{
  item->setHidden(false);
}
