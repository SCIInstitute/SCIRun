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


#ifndef ALGORITHMS_BASE_ALGORITHMVARIABLE_H
#define ALGORITHMS_BASE_ALGORITHMVARIABLE_H

#include <string>
#include <vector>
#include <iosfwd>
#include <boost/variant.hpp>
#include <boost/filesystem/path.hpp>
#ifndef Q_MOC_RUN
#include <boost/iterator/counting_iterator.hpp>
#endif
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <Core/Algorithms/Base/Name.h>
#include <Core/Algorithms/Base/Option.h>
#include <Core/Algorithms/Base/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {

  class SCISHARE Variable
  {
  public:
    using List = std::vector<Variable>;
    using Value = boost::variant<
      int,
      double,
      std::string,
      bool,
      AlgoOption,
      List
    >;

    Variable() {}
    Variable(const Name& name, const Value& value);
    enum DatatypeVariableDummyEnum { DATATYPE_VARIABLE };
    Variable(const Name& name, const Datatypes::DatatypeHandle& data, DatatypeVariableDummyEnum) : name_(name), data_(data) {}

    const Name& name() const { return name_; }
    const Value& value() const { return value_; }
    void setValue(const Value& val);

    int toInt() const;
    double toDouble() const;
    std::string toString() const;
    boost::filesystem::path toFilename() const;
    bool toBool() const;
    List toVector() const;
    AlgoOption toOption() const;

    Datatypes::DatatypeHandle getDatatype() const;

    Name& nameForXml() { return name_; }
    Value& valueForXml() { return value_; }

  private:
    Name name_;
    Value value_;
    Datatypes::DatatypeHandleOption data_;
  };

  SCISHARE bool operator==(const Variable& lhs, const Variable& rhs);
  SCISHARE bool operator!=(const Variable& lhs, const Variable& rhs);
  SCISHARE std::ostream& operator<<(std::ostream& out, const Variable& var);
  SCISHARE Variable makeVariable(const std::string& name, const Variable::Value& value);

  template <typename ... Ts>
  Variable::List makeAnonymousVariableList(Ts&&... params)
  {
    std::vector<Variable::Value> values{params...};
    Variable::List vars;
    std::transform(values.begin(), values.end(), std::back_inserter(vars),
      [](const Variable::Value& val) { return makeVariable("listElement", val); });
    return vars;
  }

  template <typename ... Ts, size_t N>
  Variable::List makeNamedVariableList(const Name (&namesList)[N], Ts&&... params)
  {
    std::vector<Variable::Value> values{ params... };

    auto namesIter = &namesList[0];
    Variable::List vars;
    std::transform(values.begin(), values.end(), std::back_inserter(vars),
      [&namesIter](const Variable::Value& val) { return Variable(*namesIter++, val); });
    return vars;
  }

  template <typename Func>
  Variable::List makeHomogeneousVariableList(Func valueGenerator, size_t num)
  {
    Variable::List vars;
    std::transform(boost::counting_iterator<size_t>(0), boost::counting_iterator<size_t>(num), std::back_inserter(vars), [valueGenerator](size_t i) { return makeVariable("", valueGenerator(i)); });
    return vars;
  }

  template <typename T>
  std::vector<T> toTypedVector(const Variable::List& list, std::function<T(const Variable&)> convert)
  {
    std::vector<T> ts;
    std::transform(list.begin(), list.end(), std::back_inserter(ts), convert);
    return ts;
  }

  SCISHARE std::vector<std::string> toStringVector(const Variable::List& list);
  SCISHARE std::vector<std::string> toNameVector(const Variable::List& list);
  SCISHARE std::vector<double> toDoubleVector(const Variable::List& list);

  typedef Variable AlgorithmParameter;
  typedef Variable::List VariableList;
  typedef SharedPointer<Variable> VariableHandle;

}

//type-converting bolt-on class template
template <typename T>
class TypedVariable : public Algorithms::Variable
{
public:
  using value_type = T;
  TypedVariable(const std::string& name, const value_type& value) : Algorithms::Variable(Algorithms::Name(name), value) {}

  operator value_type() const { return {}; }
};

#define TYPED_VARIABLE_CLASS(varType, func) template <> \
class TypedVariable<varType> : public Algorithms::Variable \
{\
public:\
  using value_type = varType;\
  TypedVariable(const std::string& name, const value_type& value) : Algorithms::Variable(Algorithms::Name(name), value) {}\
  operator value_type() const { return val(); }\
  value_type val() const { return func(); }\
};

TYPED_VARIABLE_CLASS(bool, toBool)
TYPED_VARIABLE_CLASS(std::string, toString)

using BooleanVariable = TypedVariable<bool>;
using StringVariable = TypedVariable<std::string>;

}}

#endif
