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


#ifndef ALGORITHMS_BASE_ALGORITHMDATA_H
#define ALGORITHMS_BASE_ALGORITHMDATA_H

#include <map>
#include <boost/any.hpp>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Algorithms/Base/Name.h>
#include <Core/Algorithms/Base/Variable.h>
#include <Core/Algorithms/Base/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {

  class SCISHARE AlgorithmData
  {
  public:
    using Map = std::map<Name, std::vector<Datatypes::DatatypeHandle>>;
    AlgorithmData() {}
    explicit AlgorithmData(const Map& m) : data_(m) {}

    Datatypes::DatatypeHandle& operator[](const Name& name);

    template <typename T>
    boost::shared_ptr<T> get(const Name& name) const
    {
      auto it = data_.find(name);
      /// @todo: log incorrect type if present but wrong type
      return it == data_.end() ? nullptr : boost::dynamic_pointer_cast<T>(it->second[0]);
    }

    template <typename T>
    std::vector<boost::shared_ptr<T>> getList(const Name& name) const
    {
      auto it = data_.find(name);
      /// @todo: log incorrect type if present but wrong type
      return it == data_.end() ? std::vector<boost::shared_ptr<T>>() : downcast_range<T>(it->second);
    }

    template <typename T>
    void setList(const Name& name, const std::vector<boost::shared_ptr<T>>& list)
    {
      data_[name] = upcast_range<Datatypes::Datatype>(list);
    }

    /// @todo: lame
    void setTransient(boost::any t) { transient_ = t; }
    boost::any getTransient() const { return transient_; }

  private:
    Map data_;
    boost::any transient_;
  };

  class SCISHARE AlgorithmInput : public AlgorithmData
  {
  public:
    AlgorithmInput() {}
    explicit AlgorithmInput(const Map& m) : AlgorithmData(m) {}
  };

  class SCISHARE AlgorithmOutput : public AlgorithmData
  {
  public:
    VariableHandle additionalAlgoOutput() const { return additionalAlgoOutput_; }
    void setAdditionalAlgoOutput(VariableHandle var) { additionalAlgoOutput_ = var; }
  private:
    VariableHandle additionalAlgoOutput_;
  };

  using AlgorithmInputHandle = SharedPointer<AlgorithmInput>;
  using AlgorithmOutputHandle = SharedPointer<AlgorithmOutput>;

}}}

#endif
