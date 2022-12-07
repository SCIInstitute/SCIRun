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


#ifndef MODULES_FIELDS_InterfaceWithCleaver_H
#define MODULES_FIELDS_InterfaceWithCleaver_H
#include <Dataflow/Network/Module.h>
#include <Modules/Fields/share.h>

namespace SCIRun {
namespace Modules {
namespace Fields {

  class SCISHARE InterfaceWithCleaver : public SCIRun::Dataflow::Networks::Module,
    public Has1InputPort<DynamicPortTag<FieldPortTag>>,
    public Has1OutputPort<FieldPortTag>
  {
  public:
    InterfaceWithCleaver();
    void execute() override;
    void setStateDefaults() override;

    DEPRECATED_MODULE_REPLACE_WITH("InterfaceWithCleaver2")

    HAS_DYNAMIC_PORTS
    INPUT_PORT_DYNAMIC(0, InputFields, Field);
    OUTPUT_PORT(0, OutputField, Field);

    MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm)
  };
}}}

#endif
