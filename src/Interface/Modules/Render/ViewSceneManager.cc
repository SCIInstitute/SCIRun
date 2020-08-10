/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include "Interface/Modules/Render/ViewSceneManager.h"
#include <Interface/Modules/Render/ViewScene.h>
#include <iostream>

using namespace SCIRun::Gui;

ViewSceneManager::ViewSceneManager()
{
}

ViewSceneManager::~ViewSceneManager()
{
}

void ViewSceneManager::addViewScene(ViewSceneDialog* vsd)
{
  ungroupedViewScenes.insert(vsd);
  groupsUpdated();
}

void ViewSceneManager::removeViewScene(ViewSceneDialog* vsd)
{
  if(!ungroupedViewScenes.erase(vsd))
  {
    uint32_t size = viewSceneGroups.size();
    for(uint32_t i = 0; i < size; ++i)
      if(viewSceneGroups[i].erase(vsd)) return;
  }
  groupsUpdated();
}

uint32_t ViewSceneManager::addGroup()
{
  viewSceneGroups.push_back(std::unordered_set<ViewSceneDialog*>());
  groupsUpdated();
  return static_cast<uint32_t>(viewSceneGroups.size()-1);
}

bool ViewSceneManager::removeGroup(uint32_t group)
{
  if(group < viewSceneGroups.size())
  {
    ungroupedViewScenes.insert(viewSceneGroups[group].begin(), viewSceneGroups[group].end());
    viewSceneGroups.erase(viewSceneGroups.begin() + group);
    groupsUpdated();
    return true;
  }
  return false;
}

bool ViewSceneManager::moveViewSceneToGroup(ViewSceneDialog* vsd, uint32_t group)
{
  if(group < viewSceneGroups.size() && ungroupedViewScenes.erase(vsd))
  {
    viewSceneGroups[group].insert(vsd);
    groupsUpdated();
    return true;
  }
  return false;
}

bool ViewSceneManager::removeViewSceneFromGroup(ViewSceneDialog* vsd, uint32_t group)
{
  if(group < viewSceneGroups.size() && viewSceneGroups[group].erase(vsd))
  {
    ungroupedViewScenes.insert(vsd);
    groupsUpdated();
    return true;
  }
  return false;
}

bool ViewSceneManager::getViewSceneGroupNumber(ViewSceneDialog* vsd, uint32_t& group)
{
  uint32_t size = viewSceneGroups.size();
  for(uint32_t i = 0; i < size; ++i)
  {
    if(viewSceneGroups[i].find(vsd) != viewSceneGroups[i].end())
    {
      group = i;
      return true;
    }
  }
  return false;
}

void ViewSceneManager::getViewSceneGroupAsVector(uint32_t group, std::vector<ViewSceneDialog*>& out)
{
  out.clear();
  for(auto vsd : viewSceneGroups[group]) out.push_back(vsd);
}

void ViewSceneManager::getViewSceneGroupAsVector(ViewSceneDialog* vsd, std::vector<ViewSceneDialog*>& out)
{
  uint32_t group; out.clear();
  if(getViewSceneGroupNumber(vsd, group)) getViewSceneGroupAsVector(group, out);
  else                                    out.push_back(vsd);
}

void ViewSceneManager::getUngroupedViewScenesAsVector(std::vector<ViewSceneDialog*>& viewScenes)
{
  for (auto vsd : ungroupedViewScenes)
    viewScenes.push_back(vsd);
}

void ViewSceneManager::groupsUpdated()
{
  uint32_t size = viewSceneGroups.size();
  for(uint32_t i = 0; i < size; ++i)
    for(ViewSceneDialog* vsd : viewSceneGroups[i])
      vsd->setViewScenesToUpdate(viewSceneGroups[i]);

  for(ViewSceneDialog* vsd : ungroupedViewScenes)
    vsd->setViewScenesToUpdate({vsd});

  groupsUpdatedSignal();
}
