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


#ifndef INTERFACE_MODULES_MODULEDIALOGFACTORY_H
#define INTERFACE_MODULES_MODULEDIALOGFACTORY_H

#include <QWidget>
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Interface/Modules/Factory/share.h>

namespace SCIRun
{
  namespace Gui
  {
    class SCISHARE ModuleDialogFactory
    {
    public:
      ModuleDialogFactory(QWidget* parentToUse, ExecutionDisablingServiceFunction disablerAdd,
        ExecutionDisablingServiceFunction disablerRemove);
      ModuleDialogGeneric* makeDialog(const std::string& moduleId, SCIRun::Dataflow::Networks::ModuleStateHandle state);

      typedef boost::function<ModuleDialogGeneric*(const std::string&, SCIRun::Dataflow::Networks::ModuleStateHandle, QWidget*)> DialogMaker;
      typedef std::map<std::string, DialogMaker> DialogMakerMap;

      const DialogMakerMap& getMap() const { return dialogMakerMap_; }
    private:
      QWidget* parentToUse_;
      DialogMakerMap dialogMakerMap_;
      ExecutionDisablingServiceFunction disabler_;
      void addDialogsToMakerMap1();
      void addDialogsToMakerMap2();
      void addDialogsToMakerMapGenerated();
    };
  }
}

#define MODULE_FACTORY_LAMBDA(type) [](const std::string& name,SCIRun::Dataflow::Networks::ModuleStateHandle state,QWidget* parent) { return new type(name, state, parent); }
#define ADD_MODULE_DIALOG(module, dialog) (#module, MODULE_FACTORY_LAMBDA(dialog))

#endif
