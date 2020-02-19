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


/// @todo Documentation Dataflow/Serialization/Network/StateSerialization.h

#ifndef CORE_SERIALIZATION_NETWORK_STATE_SERIALIZATION_H
#define CORE_SERIALIZATION_NETWORK_STATE_SERIALIZATION_H

#include <Dataflow/State/SimpleMapModuleState.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <Dataflow/Serialization/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace State {

  class SCISHARE SimpleMapModuleStateXML : public SimpleMapModuleState
  {
  public:
    SimpleMapModuleStateXML();
    explicit SimpleMapModuleStateXML(const SimpleMapModuleState& state);
  private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
      ar & boost::serialization::make_nvp("stateMap", stateMap_);
    }
  };

  SCISHARE boost::shared_ptr<SimpleMapModuleStateXML> make_state_xml(SCIRun::Dataflow::Networks::ModuleStateHandle state);

}}}

namespace boost {
  namespace serialization {

    template<class Archive>
    void serialize(Archive& ar, SCIRun::Core::Algorithms::AlgorithmParameterName& apn, const unsigned int version)
    {
      ar & boost::serialization::make_nvp("name", apn.name_);
    }

    template<class Archive>
    void serialize(Archive& ar, SCIRun::Core::Algorithms::AlgorithmParameter& ap, const unsigned int version)
    {
      ar & boost::serialization::make_nvp("name", ap.nameForXml().name_);
      ar & boost::serialization::make_nvp("value", ap.valueForXml());
    }

    template<class Archive>
    void serialize(Archive& ar, SCIRun::Core::Algorithms::AlgoOption& opt, const unsigned int version)
    {
      ar & boost::serialization::make_nvp("option", opt.option_);
      ar & boost::serialization::make_nvp("options", opt.options_);
    }
  }
}

#endif
