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


#ifndef INTERFACE_APPLICATION_MODULE_DIALOG_MANAGER_H
#define INTERFACE_APPLICATION_MODULE_DIALOG_MANAGER_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Interface/Modules/Base/share.h>

class QAbstractButton;

namespace SCIRun {
namespace Gui {

  class SCISHARE ModuleErrorDisplayer
  {
   public:
    virtual ~ModuleErrorDisplayer();
    virtual void displayError(const QString& msg, std::function<void()> showModule) = 0;
  };

  class ModuleDialogGeneric;
  class ModuleLogWindow;

  class SCISHARE ModuleDialogManager
  {
   public:
    explicit ModuleDialogManager(Dataflow::Networks::ModuleHandle module) : module_(module) {}
    
    ModuleLogWindow* setupLogging(ModuleErrorDisplayer* displayer, QAction* showLogAction, QWidget* parent);
    void connectDisplayLogButton(QAbstractButton* button);
    void destroyLog();

    void createOptions();
    bool hasOptions() const { return options_ != nullptr; }
    void closeOptions();
    void destroyOptions();
    ModuleDialogGeneric* options() { return options_; }

   private:
    Dataflow::Networks::ModuleHandle module_;
    ModuleDialogGeneric* options_{nullptr};
    ModuleLogWindow* logWindow_{nullptr};
  };

}
}

#endif
