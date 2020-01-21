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


/// @todo Documentation Dataflow/Serialization/Network/ModuleDescriptionSerialization.h

#ifndef CORE_SERIALIZATION_NETWORK_MODULE_DESCRIPTION_SERIALIZATION_H
#define CORE_SERIALIZATION_NETWORK_MODULE_DESCRIPTION_SERIALIZATION_H

#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/ConnectionId.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
#include <Dataflow/Serialization/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE ModuleLookupInfoXML : public ModuleLookupInfo
  {
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
      ar & BOOST_SERIALIZATION_NVP(package_name_);
      ar & BOOST_SERIALIZATION_NVP(category_name_);
      ar & BOOST_SERIALIZATION_NVP(module_name_);
    }

  public:
    ModuleLookupInfoXML();
    ModuleLookupInfoXML(const ModuleLookupInfoXML& rhs);
    ModuleLookupInfoXML(const ModuleLookupInfo& rhs);
  };

  class SCISHARE ConnectionDescriptionXML : public ConnectionDescription
  {
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
      ar & boost::serialization::make_nvp("moduleId1_", out_.moduleId_.id_);
      ar & boost::serialization::make_nvp("port1_", out_.portId_);
      ar & boost::serialization::make_nvp("moduleId2_", in_.moduleId_.id_);
      ar & boost::serialization::make_nvp("port2_", in_.portId_);
    }
  public:
    ConnectionDescriptionXML();
    ConnectionDescriptionXML(const ConnectionDescriptionXML& rhs);
    ConnectionDescriptionXML(const ConnectionDescription& rhs);
  };

  //to order connections by port index.
  SCISHARE bool operator<(const ConnectionDescriptionXML& lhs, const ConnectionDescriptionXML& rhs);

}}}

namespace boost {
  namespace serialization {

    template<class Archive>
    void serialize(Archive& ar, SCIRun::Dataflow::Networks::PortId& pid, const unsigned int version)
    {
      ar & boost::serialization::make_nvp("name", pid.name);
      ar & boost::serialization::make_nvp("id", pid.id);
    }
  }}

#endif
