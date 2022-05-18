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

#ifndef INTERFACE_MODULES_VIEW_SCENE_MANAGER_H
#define INTERFACE_MODULES_VIEW_SCENE_MANAGER_H

#include <unordered_set>
#include <vector>
#include <cstdint>
#include <QApplication>

namespace SCIRun
{
  namespace Gui
  {
    class ViewSceneDialog;
    class ViewSceneManager : public QObject
    {
    Q_OBJECT;

    public:
      ViewSceneManager();
      ~ViewSceneManager();

      void addViewScene(ViewSceneDialog* vsd);
      void removeViewScene(ViewSceneDialog* vsd);

      uint32_t addGroup();
      bool removeGroup(uint32_t group);

      bool moveViewSceneToGroup(ViewSceneDialog* vsd, uint32_t group);
      bool removeViewSceneFromGroup(ViewSceneDialog* vsd, uint32_t group);

      bool getViewSceneGroupNumber(ViewSceneDialog* vsd, uint32_t& group);
      void getViewSceneGroupAsVector(uint32_t group, std::vector<ViewSceneDialog*>& out);
      void getViewSceneGroupAsVector(ViewSceneDialog* vsd, std::vector<ViewSceneDialog*>& out);
      uint32_t getGroupCount() const { return static_cast<uint32_t>(viewSceneGroups.size()); }

      void getUngroupedViewScenesAsVector(std::vector<ViewSceneDialog*>& viewScenes);
      void groupsUpdated();

    Q_SIGNALS:
        void groupsUpdatedSignal();

    private:
      std::vector<std::unordered_set<ViewSceneDialog*>> viewSceneGroups     {};
      std::unordered_set<ViewSceneDialog*>              ungroupedViewScenes {};
    };
  }
}

#endif
