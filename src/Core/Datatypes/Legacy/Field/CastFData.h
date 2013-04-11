/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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


#ifndef CORE_DATATYPES_CASTFDATA_H
#define CORE_DATATYPES_CASTFDATA_H 1

#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Tensor.h>

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
template <class T> inline T CastFData(const Vector &val);
template <class T> inline T CastFData(const Tensor &val);

template <class T> inline T CastFData(const char &val) { return (static_cast<T>(val)); }
template <> inline Vector CastFData<Vector>(const char& /*val*/) { return (Vector(0,0,0)); }
template <> inline Tensor CastFData<Tensor>(const char &val) { return (Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const unsigned char &val) { return (static_cast<T>(val)); }
template <> inline Vector CastFData<Vector>(const unsigned char& /*val*/) { return (Vector(0,0,0)); }
template <> inline Tensor CastFData<Tensor>(const unsigned char &val) { return (Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const short &val) { return (static_cast<T>(val)); }
template <> inline Vector CastFData<Vector>(const short& /*val*/) {return (Vector(0,0,0));}
template <> inline Tensor CastFData<Tensor>(const short &val) { return (Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const unsigned short &val) { return (static_cast<T>(val)); }
template <> inline Vector CastFData<Vector>(const unsigned short& /*val*/) { return (Vector(0,0,0));}
template <> inline Tensor CastFData<Tensor>(const unsigned short &val) { return (Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const int &val) { return (static_cast<T>(val)); }
template <> inline Vector CastFData<Vector>(const int& /*val*/) { return (Vector(0,0,0)); }
template <> inline Tensor CastFData<Tensor>(const int &val) { return (Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const unsigned int &val) { return (static_cast<T>(val)); }
template <> inline Vector CastFData<Vector>(const unsigned int& /*val*/) { return (Vector(0,0,0)); }
template <> inline Tensor CastFData<Tensor>(const unsigned int &val) { return (Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const long &val) { return (static_cast<T>(val)); }
template <> inline Vector CastFData<Vector>(const long& /*val*/) { return (Vector(0,0,0)); }
template <> inline Tensor CastFData<Tensor>(const long &val) { return (Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const unsigned long &val) { return (static_cast<T>(val)); }
template <> inline Vector CastFData<Vector>(const unsigned long& /*val*/) { return (Vector(0,0,0)); }
template <> inline Tensor CastFData<Tensor>(const unsigned long &val) { return (Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const long long &val) { return (static_cast<T>(val)); }
template <> inline Vector CastFData<Vector>(const long long& /*val*/) { return (Vector(0,0,0)); }
template <> inline Tensor CastFData<Tensor>(const long long &val) { return (Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const unsigned long long &val) { return (static_cast<T>(val)); }
template <> inline Vector CastFData<Vector>(const unsigned long long& /*val*/) { return (Vector(0,0,0)); }
template <> inline Tensor CastFData<Tensor>(const unsigned long long &val) { return (Tensor(static_cast<double>(val))); }

template <class T> inline T CastFData(const float &val) { return (static_cast<T>(val+0.5)); }
template <> inline Vector CastFData<Vector>(const float& /*val*/) { return (Vector(0,0,0)); }
template <> inline Tensor CastFData<Tensor>(const float &val) { return (Tensor(static_cast<double>(val))); }
template <> inline float CastFData(const float &val) { return (val); }
template <> inline double CastFData(const float &val) { return (static_cast<double>(val)); }

template <class T> inline T CastFData(const double &val) { return (static_cast<T>(val+0.5)); }
template <> inline Vector CastFData<Vector>(const double& /*val*/) { return (Vector(0,0,0)); }
template <> inline Tensor CastFData<Tensor>(const double &val) { return (Tensor(static_cast<double>(val))); }
template <> inline float CastFData(const double &val) { return (static_cast<float>(val)); }
template <> inline double CastFData(const double &val) { return (static_cast<double>(val)); }

template <class T> inline T CastFData(const Vector& /*val*/) { return (0); }
template <> inline Vector CastFData<Vector>(const Vector &val) { return (val); }
template <> inline Tensor CastFData<Tensor>(const Vector& /*val*/) { return (Tensor(0.0)); }

template <class T> inline T CastFData(const Tensor& /*val*/) { return (0); }
template <> inline Vector CastFData<Vector>(const Tensor& /*val*/) { return (Vector(0,0,0)); }
template <> inline Tensor CastFData<Tensor>(const Tensor &val) { return (val); }

} // namespace

#endif
