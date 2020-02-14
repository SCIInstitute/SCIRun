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


/// @todo Documentation Modules/Fields/ReportFieldInfo.h

#ifndef MODULES_FIELDS_REPORTFIELDINFO_H
#define MODULES_FIELDS_REPORTFIELDINFO_H

#include <Dataflow/Network/Module.h>
#include <Modules/Fields/share.h>

namespace SCIRun {
namespace Modules {
namespace Fields {

  class SCISHARE ReportFieldInfo : public Dataflow::Networks::Module,
    public Has1InputPort<FieldPortTag>,
    public Has9OutputPorts<ScalarPortTag, ScalarPortTag, ScalarPortTag, ScalarPortTag, ScalarPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, ScalarPortTag>
  {
  public:
    ReportFieldInfo();
    virtual void execute() override;
    virtual void setStateDefaults() override {}
    INPUT_PORT(0, InputField, Field);
    OUTPUT_PORT(0, NumNodes, Int32);
    OUTPUT_PORT(1, NumElements, Int32);
    OUTPUT_PORT(2, NumData, Int32);
    OUTPUT_PORT(3, DataMin, Double);
    OUTPUT_PORT(4, DataMax, Double);
    OUTPUT_PORT(5, FieldSize, DenseMatrix);
    OUTPUT_PORT(6, FieldCenter, DenseMatrix);
    OUTPUT_PORT(7, Dimensions, DenseMatrix);
    OUTPUT_PORT(8, GeomSize, Double);

    MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm)
  };
}}}

#endif
