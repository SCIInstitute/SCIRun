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


#ifndef DATAFLOW_NETWORK_MODULEPORTDESCRIPTIONTAGS_H
#define DATAFLOW_NETWORK_MODULEPORTDESCRIPTIONTAGS_H

#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>
#include <vector>
#include <Core/Datatypes/DatatypeFwd.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Mesh/FieldFwd.h>
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/PortManager.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Modules
{
  struct SCISHARE MatrixPortTag {};
  struct SCISHARE ComplexMatrixPortTag {};
  struct SCISHARE ScalarPortTag {};
  struct SCISHARE StringPortTag {};
  struct SCISHARE FieldPortTag {};
  struct SCISHARE OsprayGeometryPortTag {};
  struct SCISHARE GeometryPortTag {};
  struct SCISHARE ColorMapPortTag {};
  struct SCISHARE BundlePortTag {};
  struct SCISHARE NrrdPortTag {};
  struct SCISHARE DatatypePortTag {};
  struct SCISHARE MetadataObjectPortTag {};

  template <typename Base>
  struct DynamicPortTag : Base
  {
    typedef Base type;
  };

  template <typename Base>
  struct AsyncDynamicPortTag : DynamicPortTag<Base>
  {
    typedef Base type;
  };

  template <size_t N>
  struct NumInputPorts
  {
    enum { NumIPorts = N };
  };

  template <size_t N>
  struct NumOutputPorts
  {
    enum { NumOPorts = N };
  };

  struct HasNoInputPorts : NumInputPorts<0> {};
  struct HasNoOutputPorts : NumOutputPorts<0> {};

  //MEGA TODO: these will become variadic templates in VS2013
  template <class PortTypeTag>
  class Has1InputPort : public NumInputPorts<1>
  {
  public:
    static std::vector<Dataflow::Networks::InputPortDescription> inputPortDescription(const std::string& port0Name);
  };

  template <class PortTypeTag0, class PortTypeTag1>
  class Has2InputPorts : public NumInputPorts<2>
  {
  public:
    static std::vector<Dataflow::Networks::InputPortDescription> inputPortDescription(const std::string& port0Name, const std::string& port1Name)
    {
      /// @todo: use move semantics
      auto ports = Has1InputPort<PortTypeTag0>::inputPortDescription(port0Name);
      ports.push_back(Has1InputPort<PortTypeTag1>::inputPortDescription(port1Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag0, class PortTypeTag1, class PortTypeTag2>
  class Has3InputPorts : public NumInputPorts<3>
  {
  public:
    static std::vector<Dataflow::Networks::InputPortDescription> inputPortDescription(const std::string& port0Name, const std::string& port1Name, const std::string& port2Name)
    {
      auto ports = Has2InputPorts<PortTypeTag0, PortTypeTag1>::inputPortDescription(port0Name, port1Name);
      ports.push_back(Has1InputPort<PortTypeTag2>::inputPortDescription(port2Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag0, class PortTypeTag1, class PortTypeTag2, class PortTypeTag3>
  class Has4InputPorts : public NumInputPorts<4>
  {
  public:
    static std::vector<Dataflow::Networks::InputPortDescription> inputPortDescription(const std::string& port0Name, const std::string& port1Name, const std::string& port2Name, const std::string& port3Name)
    {
      auto ports = Has3InputPorts<PortTypeTag0, PortTypeTag1, PortTypeTag2>::inputPortDescription(port0Name, port1Name, port2Name);
      ports.push_back(Has1InputPort<PortTypeTag3>::inputPortDescription(port3Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag0, class PortTypeTag1, class PortTypeTag2, class PortTypeTag3, class PortTypeTag4>
  class Has5InputPorts : public NumInputPorts<5>
  {
  public:
    static std::vector<Dataflow::Networks::InputPortDescription> inputPortDescription(const std::string& port0Name, const std::string& port1Name, const std::string& port2Name, const std::string& port3Name, const std::string& port4Name)
    {
      auto ports = Has4InputPorts<PortTypeTag0, PortTypeTag1, PortTypeTag2, PortTypeTag3>::inputPortDescription(port0Name, port1Name, port2Name, port3Name);
      ports.push_back(Has1InputPort<PortTypeTag4>::inputPortDescription(port4Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag0, class PortTypeTag1, class PortTypeTag2, class PortTypeTag3, class PortTypeTag4, class PortTypeTag5>
  class Has6InputPorts : public NumInputPorts<6>
  {
  public:
    static std::vector<Dataflow::Networks::InputPortDescription> inputPortDescription(const std::string& port0Name, const std::string& port1Name, const std::string& port2Name, const std::string& port3Name, const std::string& port4Name, const std::string& port5Name)
    {
      auto ports = Has5InputPorts<PortTypeTag0, PortTypeTag1, PortTypeTag2, PortTypeTag3, PortTypeTag4>::inputPortDescription(port0Name, port1Name, port2Name, port3Name, port4Name);
      ports.push_back(Has1InputPort<PortTypeTag5>::inputPortDescription(port5Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag0, class PortTypeTag1, class PortTypeTag2, class PortTypeTag3, class PortTypeTag4, class PortTypeTag5, class PortTypeTag6>
  class Has7InputPorts : public NumInputPorts<7>
  {
  public:
    static std::vector<Dataflow::Networks::InputPortDescription> inputPortDescription(const std::string& port0Name, const std::string& port1Name, const std::string& port2Name, const std::string& port3Name, const std::string& port4Name, const std::string& port5Name, const std::string& port6Name)
    {
      auto ports = Has6InputPorts<PortTypeTag0, PortTypeTag1, PortTypeTag2, PortTypeTag3, PortTypeTag4, PortTypeTag5>::inputPortDescription(port0Name, port1Name, port2Name, port3Name, port4Name, port5Name);
      ports.push_back(Has1InputPort<PortTypeTag6>::inputPortDescription(port6Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag>
  class Has1OutputPort : public NumOutputPorts<1>
  {
  public:
    static std::vector<Dataflow::Networks::OutputPortDescription> outputPortDescription(const std::string& port0Name);
  };

  template <class PortTypeTag0, class PortTypeTag1>
  class Has2OutputPorts : public NumOutputPorts<2>
  {
  public:
    static std::vector<Dataflow::Networks::OutputPortDescription> outputPortDescription(const std::string& port0Name, const std::string& port1Name)
    {
      /// @todo: use move semantics
      auto ports = Has1OutputPort<PortTypeTag0>::outputPortDescription(port0Name);
      ports.push_back(Has1OutputPort<PortTypeTag1>::outputPortDescription(port1Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag0, class PortTypeTag1, class PortTypeTag2>
  class Has3OutputPorts : public NumOutputPorts<3>
  {
  public:
    static std::vector<Dataflow::Networks::OutputPortDescription> outputPortDescription(const std::string& port0Name, const std::string& port1Name, const std::string& port2Name)
    {
      auto ports = Has2OutputPorts<PortTypeTag0, PortTypeTag1>::outputPortDescription(port0Name, port1Name);
      ports.push_back(Has1OutputPort<PortTypeTag2>::outputPortDescription(port2Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag0, class PortTypeTag1, class PortTypeTag2, class PortTypeTag3>
  class Has4OutputPorts : public NumOutputPorts<4>
  {
  public:
    static std::vector<Dataflow::Networks::OutputPortDescription> outputPortDescription(const std::string& port0Name, const std::string& port1Name, const std::string& port2Name, const std::string& port3Name)
    {
      auto ports = Has3OutputPorts<PortTypeTag0, PortTypeTag1, PortTypeTag2>::outputPortDescription(port0Name, port1Name, port2Name);
      ports.push_back(Has1OutputPort<PortTypeTag3>::outputPortDescription(port3Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag0, class PortTypeTag1, class PortTypeTag2, class PortTypeTag3, class PortTypeTag4>
  class Has5OutputPorts : public NumOutputPorts<5>
  {
  public:
    static std::vector<Dataflow::Networks::OutputPortDescription> outputPortDescription(const std::string& port0Name, const std::string& port1Name, const std::string& port2Name, const std::string& port3Name, const std::string& port4Name)
    {
      auto ports = Has4OutputPorts<PortTypeTag0, PortTypeTag1, PortTypeTag2, PortTypeTag3>::outputPortDescription(port0Name, port1Name, port2Name, port3Name);
      ports.push_back(Has1OutputPort<PortTypeTag4>::outputPortDescription(port4Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag0, class PortTypeTag1, class PortTypeTag2, class PortTypeTag3, class PortTypeTag4, class PortTypeTag5>
  class Has6OutputPorts : public NumOutputPorts<6>
  {
  public:
    static std::vector<Dataflow::Networks::OutputPortDescription> outputPortDescription(const std::string& port0Name, const std::string& port1Name, const std::string& port2Name, const std::string& port3Name, const std::string& port4Name, const std::string& port5Name)
    {
      auto ports = Has5OutputPorts<PortTypeTag0, PortTypeTag1, PortTypeTag2, PortTypeTag3, PortTypeTag4>::outputPortDescription(port0Name, port1Name, port2Name, port3Name, port4Name);
      ports.push_back(Has1OutputPort<PortTypeTag5>::outputPortDescription(port5Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag0, class PortTypeTag1, class PortTypeTag2, class PortTypeTag3, class PortTypeTag4, class PortTypeTag5, class PortTypeTag6>
  class Has7OutputPorts : public NumOutputPorts<7>
  {
  public:
    static std::vector<Dataflow::Networks::OutputPortDescription> outputPortDescription(const std::string& port0Name, const std::string& port1Name, const std::string& port2Name, const std::string& port3Name, const std::string& port4Name, const std::string& port5Name, const std::string& port6Name)
    {
      auto ports = Has6OutputPorts<PortTypeTag0, PortTypeTag1, PortTypeTag2, PortTypeTag3, PortTypeTag4, PortTypeTag5>::outputPortDescription(port0Name, port1Name, port2Name, port3Name, port4Name, port5Name);
      ports.push_back(Has1OutputPort<PortTypeTag6>::outputPortDescription(port6Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag0, class PortTypeTag1, class PortTypeTag2, class PortTypeTag3, class PortTypeTag4, class PortTypeTag5, class PortTypeTag6, class PortTypeTag7>
  class Has8OutputPorts : public NumOutputPorts<8>
  {
  public:
    static std::vector<Dataflow::Networks::OutputPortDescription> outputPortDescription(const std::string& port0Name, const std::string& port1Name, const std::string& port2Name, const std::string& port3Name, const std::string& port4Name, const std::string& port5Name, const std::string& port6Name, const std::string& port7Name)
    {
      auto ports = Has7OutputPorts<PortTypeTag0, PortTypeTag1, PortTypeTag2, PortTypeTag3, PortTypeTag4, PortTypeTag5, PortTypeTag6>::outputPortDescription(port0Name, port1Name, port2Name, port3Name, port4Name, port5Name, port6Name);
      ports.push_back(Has1OutputPort<PortTypeTag7>::outputPortDescription(port7Name)[0]);
      return ports;
    }
  };

  template <class PortTypeTag0, class PortTypeTag1, class PortTypeTag2, class PortTypeTag3, class PortTypeTag4, class PortTypeTag5, class PortTypeTag6, class PortTypeTag7, class PortTypeTag8>
  class Has9OutputPorts : public NumOutputPorts<9>
  {
  public:
    static std::vector<Dataflow::Networks::OutputPortDescription> outputPortDescription(const std::string& port0Name, const std::string& port1Name, const std::string& port2Name, const std::string& port3Name, const std::string& port4Name, const std::string& port5Name, const std::string& port6Name, const std::string& port7Name, const std::string& port8Name)
    {
      auto ports = Has8OutputPorts<PortTypeTag0, PortTypeTag1, PortTypeTag2, PortTypeTag3, PortTypeTag4, PortTypeTag5, PortTypeTag6, PortTypeTag7>::outputPortDescription(port0Name, port1Name, port2Name, port3Name, port4Name, port5Name, port6Name, port7Name);
      ports.push_back(Has1OutputPort<PortTypeTag8>::outputPortDescription(port8Name)[0]);
      return ports;
    }
  };

#define PORT_SPEC(type)   template <>\
  class Has1InputPort<type ##PortTag> : public NumInputPorts<1>\
  {\
  public:\
    static std::vector<SCIRun::Dataflow::Networks::InputPortDescription> inputPortDescription(const std::string& port0Name)\
    {\
      std::vector<SCIRun::Dataflow::Networks::InputPortDescription> ports;\
      ports.push_back(SCIRun::Dataflow::Networks::PortDescription(SCIRun::Dataflow::Networks::PortId(0, port0Name), #type, false)); \
      return ports;\
    }\
  };\
  template <>\
  class Has1OutputPort<type ##PortTag> : public NumOutputPorts<1>\
  {\
  public:\
    static std::vector<SCIRun::Dataflow::Networks::OutputPortDescription> outputPortDescription(const std::string& port0Name)\
    {\
      std::vector<SCIRun::Dataflow::Networks::OutputPortDescription> ports;\
      ports.push_back(SCIRun::Dataflow::Networks::PortDescription(SCIRun::Dataflow::Networks::PortId(0, port0Name), #type, false)); \
      return ports;\
    }\
  };\
  template <>\
  class Has1InputPort<DynamicPortTag<type ## PortTag>> : public NumInputPorts<1>\
  {\
  public:\
    static std::vector<SCIRun::Dataflow::Networks::InputPortDescription> inputPortDescription(const std::string& port0Name)\
    {\
      std::vector<SCIRun::Dataflow::Networks::InputPortDescription> ports;\
      ports.push_back(SCIRun::Dataflow::Networks::PortDescription(SCIRun::Dataflow::Networks::PortId(0, port0Name), #type, true)); \
      return ports;\
    }\
  };\
  template <>\
  class Has1InputPort<AsyncDynamicPortTag<type ## PortTag>> : public NumInputPorts<1>\
  {\
  public:\
  static std::vector<SCIRun::Dataflow::Networks::InputPortDescription> inputPortDescription(const std::string& port0Name)\
  {\
  std::vector<SCIRun::Dataflow::Networks::InputPortDescription> ports;\
  ports.push_back(SCIRun::Dataflow::Networks::PortDescription(SCIRun::Dataflow::Networks::PortId(0, port0Name), #type, true)); \
  return ports;\
  }\
  }\

  PORT_SPEC(Matrix);
  PORT_SPEC(Scalar);
  PORT_SPEC(String);
  PORT_SPEC(Field);
  PORT_SPEC(Geometry);
  PORT_SPEC(OsprayGeometry);
  PORT_SPEC(ColorMap);
  PORT_SPEC(Bundle);
  PORT_SPEC(Nrrd);
  PORT_SPEC(ComplexMatrix);
  PORT_SPEC(Datatype);
  PORT_SPEC(MetadataObject);

#define ATTACH_NAMESPACE(type) Core::Datatypes::type
#define ATTACH_NAMESPACE2(type) SCIRun::Core::Datatypes::type

#define INPUT_PORT(index, name, type) static std::string inputPort ## index ## Name() { return #name; } \
  Dataflow::Networks::StaticPortName< ATTACH_NAMESPACE(type), index > name;

#define INPUT_PORT_DYNAMIC(index, name, type) static std::string inputPort ## index ## Name() { return #name; } \
  Dataflow::Networks::DynamicPortName< ATTACH_NAMESPACE(type), index > name;

#define OUTPUT_PORT(index, name, type) static std::string outputPort ## index ## Name() { return #name; } \
  Dataflow::Networks::StaticPortName< ATTACH_NAMESPACE(type), index> name;

#define INITIALIZE_PORT(nameObj) do{ nameObj.id_.name = #nameObj; }while(0);

struct DummyPortName : Dataflow::Networks::StaticPortName<Core::Datatypes::Datatype, 999> {};

  // Detects whether a module uses the new-style no-arg HasInputPorts /
  // HasOutputPorts (variadic) interface vs. the old named-arg interface.
  // Used by IPortDescriber / OPortDescriber to stay compatible with both.
  template <typename T>
  concept HasVariadicInputPorts = requires { T::inputPortDescription(); };

  template <typename T>
  concept HasVariadicOutputPorts = requires { T::outputPortDescription(); };

  // Index-to-name helpers for old-style modules.
  // Each maps a compile-time index to the corresponding inputPortNName() /
  // outputPortNName() static method.  Only instantiated for old-style modules
  // (the if constexpr in IPortDescriber/OPortDescriber discards these for
  // new-style HasInputPorts/HasOutputPorts modules).
  template <size_t I, class ModuleType>
  std::string oldStyleInputPortName()
  {
    if constexpr      (I == 0) return ModuleType::inputPort0Name();
    else if constexpr (I == 1) return ModuleType::inputPort1Name();
    else if constexpr (I == 2) return ModuleType::inputPort2Name();
    else if constexpr (I == 3) return ModuleType::inputPort3Name();
    else if constexpr (I == 4) return ModuleType::inputPort4Name();
    else if constexpr (I == 5) return ModuleType::inputPort5Name();
    else if constexpr (I == 6) return ModuleType::inputPort6Name();
    else if constexpr (I == 7) return ModuleType::inputPort7Name();
    else if constexpr (I == 8) return ModuleType::inputPort8Name();
  }

  template <size_t I, class ModuleType>
  std::string oldStyleOutputPortName()
  {
    if constexpr      (I == 0) return ModuleType::outputPort0Name();
    else if constexpr (I == 1) return ModuleType::outputPort1Name();
    else if constexpr (I == 2) return ModuleType::outputPort2Name();
    else if constexpr (I == 3) return ModuleType::outputPort3Name();
    else if constexpr (I == 4) return ModuleType::outputPort4Name();
    else if constexpr (I == 5) return ModuleType::outputPort5Name();
    else if constexpr (I == 6) return ModuleType::outputPort6Name();
    else if constexpr (I == 7) return ModuleType::outputPort7Name();
    else if constexpr (I == 8) return ModuleType::outputPort8Name();
  }

  // IPortDescriber — dispatches to no-arg inputPortDescription() for new-style
  // modules (HasInputPorts<...>), or reconstructs the named-arg call for old-style.
  template <size_t numPorts, class ModuleType>
  struct IPortDescriber
  {
  private:
    template <size_t... Is>
    static std::vector<Dataflow::Networks::InputPortDescription> impl(std::index_sequence<Is...>)
    {
      if constexpr (sizeof...(Is) == 0)
        return {};
      else if constexpr (HasVariadicInputPorts<ModuleType>)
        return ModuleType::inputPortDescription();
      else
        return ModuleType::inputPortDescription(oldStyleInputPortName<Is, ModuleType>()...);
    }
  public:
    static std::vector<Dataflow::Networks::InputPortDescription> inputs()
    {
      return impl(std::make_index_sequence<numPorts>{});
    }
  };

  // OPortDescriber — same pattern for outputs.
  template <size_t numPorts, class ModuleType>
  struct OPortDescriber
  {
  private:
    template <size_t... Is>
    static std::vector<Dataflow::Networks::OutputPortDescription> impl(std::index_sequence<Is...>)
    {
      if constexpr (sizeof...(Is) == 0)
        return {};
      else if constexpr (HasVariadicOutputPorts<ModuleType>)
        return ModuleType::outputPortDescription();
      else
        return ModuleType::outputPortDescription(oldStyleOutputPortName<Is, ModuleType>()...);
    }
  public:
    static std::vector<Dataflow::Networks::OutputPortDescription> outputs()
    {
      return impl(std::make_index_sequence<numPorts>{});
    }
  };

}
}

#endif
