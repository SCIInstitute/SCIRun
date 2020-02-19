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


#include <Core/Algorithms/Visualization/DataConversions.h>
#include <Core/Algorithms/Visualization/RenderFieldState.h>

namespace SCIRun {

using namespace Core::Geometry;

template <>
bool valToColor(const Vector &v, Core::Datatypes::ColorRGB &c)
{
  c = Core::Datatypes::ColorRGB(fabs(v.x()), fabs(v.y()), fabs(v.z()));
  return true;
}

template <>
bool valToColor(const Tensor &t, Core::Datatypes::ColorRGB &c)
{
  /// get_eigenvectors was not implemented in Tensor. Can't identify colors.
  // Tensor tt = t;
  //
  // Vector e1, e2, e3;
  // tt.get_eigenvectors(e1, e2, e3);
  //
  // e1.safe_normalize();
  // double rr = fabs(e1.x());
  // double gg = fabs(e1.y());
  // double bb = fabs(e1.z());
  //
  // Core::Datatypes::ColorRGB rgb1(rr, gg, bb);
  // HSVColor hsv(rgb1);
  // hsv[1] = 0.7;
  // hsv[2] = 1.0;
  //
  // c = Core::Datatypes::ColorRGB(hsv);
  return false;
}

// Conversion template specialization.
template <>
bool valToDouble(const Vector &data_in, double &data_out)
{
  data_out = data_in.length();
  return true;
}

template <>
bool valToDouble(const Tensor &data_in, double &data_out)
{
  // Tensor t = data_in;
  // double v1, v2, v3;
  // t.get_eigenvalues(v1, v2, v3);
  // data_out = Vector(v1, v2, v3).length();
  return false;
}

template <>
bool valToDouble(const std::string&, double&)
{
  return false;
}

template <>
bool valToVector(const Vector &data_in, Vector &data_out)
{
  data_out = data_in;
  return true;
}

template <>
bool valToVector(const Tensor &data_in, Vector &data_out)
{
  // Tensor t = data_in;
  // double v1, v2, v3;
  // t.get_eigenvalues(v1, v2, v3);
  // data_out = Vector(v1, v2, v3);
  return false;
}

template <>
bool valToTensor(const Tensor &data_in, Tensor &data_out)
{
  data_out = data_in;
  return true;
}

template <>
bool valToBuffer(const unsigned char &value, std::ostringstream &buffer)
{
  buffer << static_cast<int>(value);
  return true;
}

template <>
bool valToBuffer(const char &value, std::ostringstream &buffer)
{
  buffer << static_cast<int>(value);
  return true;
}

RenderState::RenderState()
{
  for (int i = 0; i < MAX_ACTION_FLAGS; ++i)
  {
    mFlags[i] = false;
  }

  // Default settings.
  mFlags[USE_NORMALS] = true;
}

void RenderState::set(ActionFlags flag, bool truth)
{
  mFlags[flag] = truth;
}

bool RenderState::get(ActionFlags flag) const
{
  return mFlags[flag];
}

} // end namespace SCIRun
