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

#include <Interface/Modules/Base/WidgetSlotManagers.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core::Algorithms;

WidgetSlotManager::WidgetSlotManager(SCIRun::Dataflow::Networks::ModuleStateHandle state, ModuleDialogGeneric& dialog, QWidget* widget, const AlgorithmParameterName& name)
  : state_(state), dialog_(dialog)
{
  if (widget)
  {
    widget->setToolTip("State key: " + QString::fromStdString(name.name_));
    widget->setStyleSheet(widget->styleSheet() + " QToolTip { color: #ffffff; background - color: #2a82da; border: 1px solid white; }");
  }
}

WidgetSlotManager::~WidgetSlotManager() 
{
}

void WidgetSlotManager::push()
{
  //TODO: idea: tell state_ directly, i'm in a scoped region of pushing/editing, don't signal while i edit. signal when done editing!
  if (!dialog_.isPulling())
    pushImpl();
}