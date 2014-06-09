/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#ifndef CORE_ALGORITHMS_VISUALIZATION_DATA_CONVERSIONS_H
#define CORE_ALGORITHMS_VISUALIZATION_DATA_CONVERSIONS_H

#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/share.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Tensor.h>

#include <sstream>
#include <iomanip>
#include <iostream>

namespace SCIRun {

// Conversion templates.
template <class T>
SCISHARE bool valToColor( const T &v, Color &c )
{
  c = Color(fabs(v), fabs(v), fabs(v));
  return true;
}

template <class T>
SCISHARE bool valToDouble( const T& data_in, double &data_out)
{
  data_out = static_cast<double>(data_in);
  return true;
}


template <class T>
SCISHARE bool valToVector( const T&, Vector&)
{
  return false;
}


template <class T>
SCISHARE bool valToTensor( const T&, Tensor&)
{
  return false;
}

template <class T>
SCISHARE bool valToBuffer( const T &value, std::ostringstream &buffer)
{
  buffer << value;
  return true;
}


// Conversion template specialization.
template <>
SCISHARE bool valToColor( const Vector&, Color& );

template <>
SCISHARE bool valToColor( const Tensor&, Color& );

template <>
SCISHARE bool valToDouble(const Vector&, double&);

template <>
SCISHARE bool valToDouble(const Tensor&, double&);

template <>
SCISHARE bool valToDouble(const std::string&, double&);

template <>
SCISHARE bool valToVector(const Vector&, Vector&);

template <>
SCISHARE bool valToVector(const Tensor&, Vector&);

template <>
SCISHARE bool valToTensor(const Tensor&, Tensor&);

template <>
SCISHARE bool valToBuffer(const unsigned char&, std::ostringstream&);

template <>
SCISHARE bool valToBuffer(const char&, std::ostringstream&);

}

#endif
