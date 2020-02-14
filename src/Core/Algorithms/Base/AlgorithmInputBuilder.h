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


#ifndef ALGORITHMS_BASE_ALGORITHMINPUTBUILDER_H
#define ALGORITHMS_BASE_ALGORITHMINPUTBUILDER_H

#include <Core/Algorithms/Base/AlgorithmData.h>
#include <Core/Algorithms/Base/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {

  class SCISHARE AlgoInputBuilder
  {
  public:
    AlgoInputBuilder();
    AlgoInputBuilder& operator()(const std::string& name, Datatypes::DatatypeHandle d);
    AlgoInputBuilder& operator()(const AlgorithmParameterName& name, Datatypes::DatatypeHandle d)
    {
      return operator()(name.name(), d);
    }
    template <typename T>
    AlgoInputBuilder& operator()(const std::string& name, const std::vector<T>& vec)
    {
      //BOOST_STATIC_ASSERT(boost::is_base_of<Datatypes::Datatype,T>::value);
      map_[Name(name)] = upcast_range<Datatypes::Datatype>(vec);
      return *this;
    }
    template <typename T>
    AlgoInputBuilder& operator()(const AlgorithmParameterName& name, const std::vector<T>& vec)
    {
      return operator()(name.name(), vec);
    }
    AlgorithmInput build() const;
  private:
    AlgorithmData::Map map_;
  };

  SCISHARE AlgorithmInput makeNullInput();


}}}

#endif
