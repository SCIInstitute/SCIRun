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

#ifndef DATAFLOW_NETWORK_MODULEBUILDER_H
#define DATAFLOW_NETWORK_MODULEBUILDER_H

#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/atomic.hpp>
#include <atomic>
#include <vector>
#include <Core/Logging/LoggerInterface.h>
#include <Core/Datatypes/DatatypeFwd.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Mesh/FieldFwd.h>
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/PortManager.h>
#include <Dataflow/Network/DefaultModuleFactories.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE ModuleBuilder : boost::noncopyable
  {
  public:
    ModuleBuilder();
    ModuleBuilder& with_name(const std::string& name);
    ModuleBuilder& using_func(ModuleMaker create);
    ModuleBuilder& add_input_port(const Port::ConstructionParams& params);
    ModuleBuilder& add_output_port(const Port::ConstructionParams& params);
    ModuleBuilder& setStateDefaults();
    ModuleHandle build();

    /// @todo: these don't quite belong here, think about extracting
    PortId cloneInputPort(ModuleHandle module, const PortId& id);
    void removeInputPort(ModuleHandle module, const PortId& id);

    typedef boost::function<DatatypeSinkInterface*()> SinkMaker;
    typedef boost::function<DatatypeSourceInterface*()> SourceMaker;
    static void use_sink_type(SinkMaker func);
    static void use_source_type(SourceMaker func);
  private:
    void addInputPortImpl(Module& module, const Port::ConstructionParams& params);
    boost::shared_ptr<Module> module_;
    static SinkMaker sink_maker_;
    static SourceMaker source_maker_;
  };

}}}

#endif
