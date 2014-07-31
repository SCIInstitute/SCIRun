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
    class ModuleDialogGeneric;

    class SCISHARE ModuleDialogFactory
    {
    public:
      explicit ModuleDialogFactory(QWidget* parentToUse);
      ModuleDialogGeneric* makeDialog(const std::string& moduleId, SCIRun::Dataflow::Networks::ModuleStateHandle state);
    private:
      QWidget* parentToUse_;
      typedef boost::function<ModuleDialogGeneric*(const std::string&, SCIRun::Dataflow::Networks::ModuleStateHandle, QWidget*)> DialogMaker;
      typedef std::map<std::string, DialogMaker> DialogMakerMap;
      DialogMakerMap dialogMakerMap_;
      void addDialogsToMakerMap1();
      void addDialogsToMakerMap2();
    };
  }
}

#define ADD_MODULE_DIALOG(module, dialog) (#module, boost::factory<dialog*>())

#endif