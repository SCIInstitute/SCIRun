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


#ifndef CORE_DATAFLOW_NETWORK_MODULE_DESCRIPTION_H
#define CORE_DATAFLOW_NETWORK_MODULE_DESCRIPTION_H 

#include <Core/Dataflow/Network/NetworkFwd.h>

namespace SCIRun {
namespace Domain {
namespace Networks {

  struct InputPortDescription
  {
    std::string name;
    std::string datatype;
    std::string color;
    //iport_maker maker;
  };

  struct OutputPortDescription
  {
    std::string name;
    std::string datatype;
    std::string color;
    //oport_maker maker;
  };

  struct ModuleDescription
  {
  public:
    std::string package_name_;
    std::string category_name_;
    std::string module_name_;
    std::string module_version_;
    std::vector<InputPortDescription> input_ports_;
    std::vector<OutputPortDescription> output_ports_;
    //bool                              optional_;
    //bool                              hide_;
    //bool                              dynamic_;
    //std::vector<std::string>          authors_;
    //std::string                       summary_;
    //ModuleMaker                       maker_;
    //bool last_port_dynamic_;
    //bool                              has_gui_node_;
  };
}}}

#endif
