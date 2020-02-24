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


/// @todo Documentation Core/Datatypes/Legacy/Field/CastFData.h

#ifndef CORE_DATATYPES_CASTFDATA_H
#define CORE_DATATYPES_CASTFDATA_H 1

#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Tensor.h>

namespace SCIRun {

template <class T> inline T CastFData(const char &val);
template <class T> inline T CastFData(const unsigned char &val);
template <class T> inline T CastFData(const short &val);
template <class T> inline T CastFData(const unsigned short &val);
template <class T> inline T CastFData(const int &val);
template <class T> inline T CastFData(const unsigned int &val);
template <class T> inline T CastFData(const long &val);
template <class T> inline T CastFData(const unsigned long &val);
template <class T> inline T CastFData(const long long &val);
template <class T> inline T CastFData(const unsigned long long &val);
template <class T> inline T CastFData(const float &val);
template <class T> inline T CastFData(const double &val);
template <class T> inline T CastFData(const std::complex<double> &val);
template <class T> inline T CastFData(const Core::Geometry::Vector &val);
template <class T> inline T CastFData(const Core::Geometry::Tensor &val);

template <class T> inline T CastFData(const char &val) { return (static_cast<T>(val)); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const char& /*val*/) { return (Core::Geometry::Vector(0,0,0)); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const char &val) { return (Core::Geometry::Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const unsigned char &val) { return (static_cast<T>(val)); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const unsigned char& /*val*/) { return (Core::Geometry::Vector(0,0,0)); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const unsigned char &val) { return (Core::Geometry::Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const short &val) { return (static_cast<T>(val)); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const short& /*val*/) {return (Core::Geometry::Vector(0,0,0));}
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const short &val) { return (Core::Geometry::Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const unsigned short &val) { return (static_cast<T>(val)); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const unsigned short& /*val*/) { return (Core::Geometry::Vector(0,0,0));}
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const unsigned short &val) { return (Core::Geometry::Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const int &val) { return (static_cast<T>(val)); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const int& /*val*/) { return (Core::Geometry::Vector(0,0,0)); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const int &val) { return (Core::Geometry::Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const unsigned int &val) { return (static_cast<T>(val)); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const unsigned int& /*val*/) { return (Core::Geometry::Vector(0,0,0)); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const unsigned int &val) { return (Core::Geometry::Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const long &val) { return (static_cast<T>(val)); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const long& /*val*/) { return (Core::Geometry::Vector(0,0,0)); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const long &val) { return (Core::Geometry::Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const unsigned long &val) { return (static_cast<T>(val)); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const unsigned long& /*val*/) { return (Core::Geometry::Vector(0,0,0)); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const unsigned long &val) { return (Core::Geometry::Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const long long &val) { return (static_cast<T>(val)); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const long long& /*val*/) { return (Core::Geometry::Vector(0,0,0)); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const long long &val) { return (Core::Geometry::Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const unsigned long long &val) { return (static_cast<T>(val)); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const unsigned long long& /*val*/) { return (Core::Geometry::Vector(0,0,0)); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const unsigned long long &val) { return (Core::Geometry::Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const float &val) { return (static_cast<T>(val+0.5)); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const float& /*val*/) { return (Core::Geometry::Vector(0,0,0)); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const float &val) { return (Core::Geometry::Tensor(static_cast<double>(val))); }
template <> inline float CastFData(const float &val) { return (val); }
template <> inline double CastFData(const float &val) { return (static_cast<double>(val)); }

template <class T> inline T CastFData(const double &val) { return (static_cast<T>(val+0.5)); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const double& /*val*/) { return (Core::Geometry::Vector(0,0,0)); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const double &val) { return (Core::Geometry::Tensor(static_cast<double>(val))); }
template <> inline float CastFData(const double &val) { return (static_cast<float>(val)); }
template <> inline double CastFData(const double &val) { return val; }
template <> inline std::complex<double> CastFData<std::complex<double>>(const double &val) { return { val, 0 }; }

template <class T> inline T CastFData(const std::complex<double>& /*val*/) { return (0); }
template <> inline double CastFData<double>(const std::complex<double> &val) { return std::abs(val); }
template <> inline std::complex<double> CastFData<std::complex<double>>(const std::complex<double> &val) { return (val); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const std::complex<double>& val) { return Core::Geometry::Vector(val.real(), val.imag(), 0); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const std::complex<double> &val) { return Core::Geometry::Tensor(static_cast<double>(std::norm(val))); }

template <class T> inline T CastFData(const Core::Geometry::Vector& /*val*/) { return (0); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const Core::Geometry::Vector &val) { return (val); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const Core::Geometry::Vector& /*val*/) { return (Core::Geometry::Tensor(0.0)); }

template <class T> inline T CastFData(const Core::Geometry::Tensor& /*val*/) { return (0); }
template <> inline Core::Geometry::Vector CastFData<Core::Geometry::Vector>(const Core::Geometry::Tensor& /*val*/) { return (Core::Geometry::Vector(0,0,0)); }
template <> inline Core::Geometry::Tensor CastFData<Core::Geometry::Tensor>(const Core::Geometry::Tensor &val) { return (val); }

} // namespace

#endif
