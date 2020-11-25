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


// makes sure that headers aren't loaded multiple times.
// This requires the string to be unique to this file.
// standard convention incorporates the file path and filename.
 #ifndef MODULES_FIELDS_CalculateNormals_H
 #define MODULES_FIELDS_CalculateNormals_H

 #include <Dataflow/Network/Module.h>
 #include <Modules/Legacy/Fields/share.h>
// share.h must be the last include, or it will not build on windows systems.

namespace SCIRun {
namespace Modules {
namespace Fields {
// this final namespace needs to match the .module file
// in src/Modules/Factory/Config/

  // define module ports.
  // Can have any number of ports (including none), and dynamic ports.
  class SCISHARE CalculateNormals : public SCIRun::Dataflow::Networks::Module,
    public Has1OutputPort<FieldPortTag>,
    public Has2InputPorts<FieldPortTag, FieldPortTag>
  {
  public:
    // these functions are required for all modules
    CalculateNormals();

    virtual void execute();
    virtual void setStateDefaults(){}

    //name the ports and datatype.
    INPUT_PORT(0, InputField, Field);
    INPUT_PORT(1, InputPoint, Field);
    OUTPUT_PORT(0, OutputField, Field);

    // this is needed for the module factory
    // the arguments of this function could vary as NoAlgoOrUI or ModuleHasUIAndAlgorithm
    MODULE_TRAITS_AND_INFO(NoAlgoOrUI)
  };
}}}
	#endif
