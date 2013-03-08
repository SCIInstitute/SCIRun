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

#ifndef INTERFACE_APPLICATION_DEVELOPERCONSOLE_H
#define INTERFACE_APPLICATION_DEVELOPERCONSOLE_H

#include "ui_DeveloperConsole.h"

//#include <Dataflow/Network/NetworkFwd.h>
//#include <Dataflow/Engine/Controller/ControllerInterfaces.h>
//#include <Dataflow/Serialization/Network/ModulePositionGetter.h>
//#include <Dataflow/Engine/Controller/HistoryItem.h>

namespace SCIRun {
namespace Gui {

  class NetworkEditor;

class DeveloperConsole : public QDockWidget, public Ui::DeveloperConsole
{
	Q_OBJECT
	
public:
  explicit DeveloperConsole(/*SCIRun::Dataflow::Engine::HistoryManagerHandle historyManager, */QWidget* parent = 0);
public Q_SLOTS:
  void executorButtonClicked();
  //void addHistoryItem(SCIRun::Dataflow::Engine::HistoryItemHandle item);  
Q_SIGNALS:
  void executorChosen(int type);
//  void modifyingNetwork(bool modifying);
//  void undoStateChanged(bool enabled);
//  void redoStateChanged(bool enabled);
//private:
//  SCIRun::Dataflow::Engine::HistoryManagerHandle historyManager_;
//  int lastUndoRow_;
//  
//  void setUndoEnabled(bool enable);
//  void setRedoEnabled(bool enable);*/
};

}
}

#endif
