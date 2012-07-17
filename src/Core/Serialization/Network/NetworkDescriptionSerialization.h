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


#ifndef CORE_SERIALIZATION_NETWORK_NETWORK_DESCRIPTION_SERIALIZATION_H
#define CORE_SERIALIZATION_NETWORK_NETWORK_DESCRIPTION_SERIALIZATION_H 

#include <Core/Serialization/Network/ModuleDescriptionSerialization.h>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <Core/Serialization/Network/Share.h>

namespace SCIRun {
namespace Domain {
namespace Networks {

  typedef std::vector<ConnectionDescriptionXML> ConnectionsXML;
  typedef std::map<std::string, ModuleLookupInfoXML> ModuleMapXML;

  class SCISHARE NetworkXML
  {
  public:
    ModuleMapXML modules;
    ConnectionsXML connections;
  private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
      ar & BOOST_SERIALIZATION_NVP(modules);
      ar & BOOST_SERIALIZATION_NVP(connections);
    }
  };

}}}

#endif
