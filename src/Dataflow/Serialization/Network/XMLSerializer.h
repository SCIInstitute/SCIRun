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


/// @todo Documentation Dataflow/Serialization/Network/XMLSerializer.h

#ifndef CORE_SERIALIZATION_NETWORK_XML_SERIALIZER_H
#define CORE_SERIALIZATION_NETWORK_XML_SERIALIZER_H

#include <boost/noncopyable.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <fstream>

#include <Dataflow/Serialization/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  namespace XMLSerializer
  {
    template <class Serializable>
    bool save_xml(const Serializable& data, std::ostream& ostr, const std::string& rootName)
    {
      if (!ostr.good())
        return false;
      boost::archive::xml_oarchive oa(ostr);
      oa << boost::serialization::make_nvp(rootName.c_str(), data);
      return true;
    }

    template <class Serializable>
    bool save_xml(const Serializable& data, const std::string& filename, const std::string& rootName)
    {
      std::ofstream ofs(filename.c_str());
      if (!ofs)
        return false;
      return save_xml(data, ofs, rootName);
    }

    template <class Serializable>
    boost::shared_ptr<Serializable> load_xml(std::istream& istr)
    {
      if (!istr.good())
        return nullptr;
      boost::archive::xml_iarchive ia(istr);
      boost::shared_ptr<Serializable> nh(new Serializable);
      ia >> BOOST_SERIALIZATION_NVP(*nh);
      return nh;
    }

    template <class Serializable>
    boost::shared_ptr<Serializable> load_xml(const std::string& filename)
    {
      std::ifstream ifs(filename.c_str());
      return load_xml<Serializable>(ifs);
    }
  }
}}}

#endif
