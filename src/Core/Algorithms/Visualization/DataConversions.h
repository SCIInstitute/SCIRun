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


#ifndef CORE_ALGORITHMS_VISUALIZATION_DATA_CONVERSIONS_H
#define CORE_ALGORITHMS_VISUALIZATION_DATA_CONVERSIONS_H

#include <Core/Datatypes/Color.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Tensor.h>

#include <sstream>

#include <Core/Algorithms/Visualization/share.h>

namespace SCIRun {

// Conversion templates.
template <class T>
bool valToColor( const T &v, Core::Datatypes::ColorRGB &c )
{
  c = Core::Datatypes::ColorRGB(fabs(v), fabs(v), fabs(v));
  return true;
}

template <class T>
bool valToDouble( const T& data_in, double &data_out)
{
  data_out = static_cast<double>(data_in);
  return true;
}


template <class T>
bool valToVector( const T&, Core::Geometry::Vector&)
{
  return false;
}


template <class T>
bool valToTensor( const T&, Core::Geometry::Tensor&)
{
  return false;
}

template <class T>
bool valToBuffer( const T &value, std::ostringstream &buffer)
{
  buffer << value;
  return true;
}


// Conversion template specialization.
template <>
SCISHARE bool valToColor( const Core::Geometry::Vector&, Core::Datatypes::ColorRGB& );

template <>
SCISHARE bool valToColor( const Core::Geometry::Tensor&, Core::Datatypes::ColorRGB& );

template <>
SCISHARE bool valToDouble(const Core::Geometry::Vector&, double&);

template <>
SCISHARE bool valToDouble(const Core::Geometry::Tensor&, double&);

template <>
SCISHARE bool valToDouble(const std::string&, double&);

template <>
SCISHARE bool valToVector(const Core::Geometry::Vector&, Core::Geometry::Vector&);

template <>
SCISHARE bool valToVector(const Core::Geometry::Tensor&, Core::Geometry::Vector&);

template <>
SCISHARE bool valToTensor(const Core::Geometry::Tensor&, Core::Geometry::Tensor&);

template <>
SCISHARE bool valToBuffer(const unsigned char&, std::ostringstream&);

template <>
SCISHARE bool valToBuffer(const char&, std::ostringstream&);

}

#endif
