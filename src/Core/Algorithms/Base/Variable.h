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

#ifndef ALGORITHMS_BASE_ALGORITHMVARIABLE_H
#define ALGORITHMS_BASE_ALGORITHMVARIABLE_H

#include <string>
#include <vector>
#include <iosfwd>
#include <boost/variant.hpp>
#include <boost/filesystem/path.hpp>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Algorithms/Base/Name.h>
#include <Core/Algorithms/Base/Option.h>
#include <Core/Algorithms/Base/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
  
  class SCISHARE Variable
  {
  public:
    /// @todo: expand this 
    typedef boost::variant<
      int,
      double,
      std::string,
      bool,
      AlgoOption,
      std::vector<Variable>
    > Value;

    Variable() {}
    Variable(const Name& name, const Value& value) : name_(name), value_(value) {}
    Variable(const Name& name, const Datatypes::DatatypeHandle& value) : name_(name), data_(value) {}

    const Name& name() const { return name_; }
    const Value& value() const { return value_; }
    void setValue(const Value& val) { value_ = val; }

    int getInt() const;
    double getDouble() const;
    std::string getString() const;
    boost::filesystem::path getFilename() const;
    bool getBool() const;
    Datatypes::DatatypeHandle getDatatype() const;
    std::vector<Variable> getList() const;
    AlgoOption getOption() const;

    Name& nameForXml() { return name_; }
    Value& valueForXml() { return value_; }

  private:
    /*const*/ Name name_;
    Value value_;
    Datatypes::DatatypeHandleOption data_;
  };

  SCISHARE bool operator==(const Variable& lhs, const Variable& rhs);
  SCISHARE std::ostream& operator<<(std::ostream& out, const Variable& var);
  
  typedef Variable AlgorithmParameter;
  
  typedef boost::shared_ptr<Variable> VariableHandle;

}

//type-converting bolt-on class template
template <typename T>
class TypedVariable : public Algorithms::Variable
{
public:
  TypedVariable(const std::string& name, const T& value) : Algorithms::Variable(Algorithms::Name(name), value) {}
  operator T() const { throw "unspecialized type"; }
};

template <>
class TypedVariable<bool> : public Algorithms::Variable
{
public:
  TypedVariable(const std::string& name, const bool& value) : Algorithms::Variable(Algorithms::Name(name), value) {}
  operator bool() const { return getBool(); }
};

typedef TypedVariable<bool> BooleanVariable;

}}

#endif
