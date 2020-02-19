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


/// @todo Documentation Core/Datatypes/Scalar.h

#ifndef CORE_DATATYPES_SCALAR_H
#define CORE_DATATYPES_SCALAR_H

#include <Core/Datatypes/Datatype.h>
#include <boost/variant.hpp>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  using ScalarValue = boost::variant<int, double>;

  class SCISHARE Scalar : public Datatype
  {
  public:
    explicit Scalar(const ScalarValue& val) : value_(val) {}
    ScalarValue value() const { return value_; }
    virtual double doubleValue() const = 0;
    virtual std::string dynamic_type_name() const override { return "Scalar"; }
  protected:
    ScalarValue value_;
  };

  class SCISHARE Double : public Scalar
  {
  public:
    explicit Double(double value) : Scalar(value) {}
    virtual Scalar* clone() const override { return new Double(toDouble()); }
    double toDouble() const;
    double doubleValue() const override { return toDouble(); }
  };

  class SCISHARE Int32 : public Scalar
  {
  public:
    explicit Int32(int value) : Scalar(value) {}
    virtual Scalar* clone() const override { return new Int32(toInt()); }
    int toInt() const;
    double doubleValue() const override { return static_cast<double>(toInt()); }
  };

}}}


#endif
